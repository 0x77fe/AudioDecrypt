#include "KGG.h"

bool KGDatabase::isValidPage1Header(const uint8_t* page1)
{
    const uint32_t o10 = page1[16] | (page1[17] << 8) | (page1[18] << 16) | (page1[19] << 24);
    const uint32_t o14 = page1[20] | (page1[21] << 8) | (page1[22] << 16) | (page1[23] << 24);
    const uint32_t v6 = ((o10 & 0xFF) << 8) | ((o10 & 0xFF00) << 16);
    return (o14 == 0x20204000) &&
        (v6 - 0x200 <= 0xFE00) &&
        ((v6 & (v6 - 1)) == 0);
}

void KGDatabase::derivePageKey(unsigned char* aes_key, unsigned char* aes_iv, const uint8_t* p_master_key, uint32_t page_no)
{
    std::vector<uint8_t> buffer(0x18);
    memcpy(buffer.data(), p_master_key, 0x10);

    // 将page_no按小端序写入0x10~0x13
    buffer[16] = (uint8_t)(page_no);
    buffer[17] = (uint8_t)(page_no >> 8);
    buffer[18] = (uint8_t)(page_no >> 16);
    buffer[19] = (uint8_t)(page_no >> 24);

    // 将固定值0x546C4173按小端序写入0x14~0x17
    const uint32_t magic = 0x546C4173;
    buffer[20] = (uint8_t)(magic);
    buffer[21] = (uint8_t)(magic >> 8);
    buffer[22] = (uint8_t)(magic >> 16);
    buffer[23] = (uint8_t)(magic >> 24);
    MD5::getHash(buffer, aes_key);

    // 计算IV部分
    uint32_t ebx = page_no + 1;
    for (size_t i = 0; i < 16; i += 4) 
    {
        // 计算eax和ecx
        const uint32_t divisor = 0xCE26;
        uint32_t quotient = ebx / divisor;
        uint32_t eax = 0x7FFFFF07 * quotient;
        uint32_t ecx = 0x9EF4 * ebx - eax;

        // 处理符号位
        if (ecx & 0x80000000) 
        {
            ecx += 0x7FFFFF07;
        }

        // 更新ebx并写入buffer
        ebx = ecx;
        buffer[i] = (uint8_t)(ebx);
        buffer[i + 1] = (uint8_t)(ebx >> 8);
        buffer[i + 2] = (uint8_t)(ebx >> 16);
        buffer[i + 3] = (uint8_t)(ebx >> 24);
    }
    buffer.resize(16); // 截断到16字节
    MD5::getHash(buffer, aes_iv);
}

void KGDatabase::loadDecryptedDatabase(const fs::path& db_path)
{
    std::ifstream ifs_db(db_path, std::ios::binary);
    if (!ifs_db.is_open()) 
    {
        throw DMusicRuntimeError(u8"无法打开数据库文件: \n" + db_path.u8string());
    }

    ifs_db.seekg(0, std::ios::end);
    const size_t db_size = ifs_db.tellg();
    const size_t last_page = db_size / this->_kg_page_size;
    if (db_size % this->_kg_page_size != 0) 
    {
        throw DMusicRuntimeError(u8"不受支持的文件: \n" + db_path.u8string());
    }
    ifs_db.seekg(0, std::ios::beg);

    this->_db.resize(db_size);
    
    std::vector<uint8_t> buffer(this->_kg_page_size);
    uint8_t aes_key[16] = { 0 };
    uint8_t aes_iv[16] = { 0 };
    size_t out_size = 0;
    for (uint32_t page_no = 1; page_no <= last_page; page_no++, out_size+= this->_kg_page_size)
    {
        ifs_db.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        this->derivePageKey(aes_key, aes_iv,this->_kg_default_master_key, page_no);

        if (page_no == 1)
        {
            if (memcmp(buffer.data(), this->_kg_sqlite_database_header, sizeof(this->_kg_sqlite_database_header)) == 0) 
            {
                //数据库未加密，直接读取
                this->_db.resize(db_size);
                memcpy_s(this->_db.data(), db_size, buffer.data(), buffer.size());
                ifs_db.read(reinterpret_cast<char*>(this->_db.data()) + this->_kg_page_size,db_size - this->_kg_page_size);
                return;
            }

            if (!this->isValidPage1Header(buffer.data()))
            {
                this->_db.clear();
                throw DMusicRuntimeError(u8"不受支持的文件: \n" + db_path.u8string());
            }

            auto backup = std::make_unique<uint8_t[]>(8);
            memcpy_s(backup.get(), 8, buffer.data() + 16, 8);
            memcpy_s(buffer.data() + 16, buffer.size() - 16, buffer.data() + 8, 8);

            // 解密首个切片需要跳过前16个字节
            std::vector<uint8_t> cipher_data(buffer.begin() + 0x10, buffer.end());
            std::vector plain_data = AES::cbc_decrypt(cipher_data, aes_key, aes_iv);
            memcpy_s(this->_db.data(), this->_db.size(), this->_kg_sqlite_database_header, sizeof(this->_kg_sqlite_database_header));
            memcpy_s(this->_db.data() + sizeof(this->_kg_sqlite_database_header), this->_db.size() - sizeof(this->_kg_sqlite_database_header), plain_data.data(), plain_data.size());
            // 测试是否解密成功
            if (memcmp(backup.get(), plain_data.data(), 8) != 0)
            {
                throw DMusicRuntimeError(u8"数据库解密失败: \n" + db_path.u8string());
            }
        }
        else
        {
            // 解密其他切片
            std::vector plain_data = AES::cbc_decrypt(buffer, aes_key, aes_iv);
            memcpy_s(this->_db.data() + out_size, this->_kg_page_size, plain_data.data(), plain_data.size());
        }
    }
    return;
}

KeyMap KGDatabase::getKeyMap(const fs::path& keyfile_path)
{
    return KeyMap();
}

KeyMap KGDatabase::getKeyMap()
{
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) 
    {
        throw DMusicRuntimeError(u8"无法打开数据库.");
    }
    int flags = SQLITE_DESERIALIZE_READONLY;
    if (sqlite3_deserialize(
        db,                         // 数据库连接
        "main",                     // 附加到主数据库
        static_cast<unsigned char*>(this->_db.data()), // 数据指针
        static_cast<sqlite3_int64>(this->_db.size()),  // 数据大小
        static_cast<sqlite3_int64>(this->_db.size()),  // 缓冲区总大小
        flags
    ) != SQLITE_OK) 
    {
        sqlite3_close(db);
        throw DMusicRuntimeError(u8"数据库反序列化失败.");
    }

    // 准备查询语句（排除NULL和空字符串）
    const char* querySQL =
        "SELECT EncryptionKeyId, EncryptionKey FROM ShareFileItems "
        "WHERE EncryptionKeyId IS NOT NULL AND EncryptionKeyId != '' "
        "AND EncryptionKey IS NOT NULL AND EncryptionKey != '';";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr) != SQLITE_OK) 
    {
        sqlite3_close(db);
        throw DMusicRuntimeError(u8"无法准备查询语句.");
    }

    // 遍历结果并输出
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        auto keyId = std::u8string(reinterpret_cast<const char8_t*>(sqlite3_column_text(stmt, 0)));
        auto key = reinterpret_cast<const uint8_t*>(sqlite3_column_text(stmt, 1));
        auto key_size = sqlite3_column_bytes(stmt, 1);
        auto key_vec = std::vector<uint8_t>(key, key + key_size);
        this->_key_map[keyId] = key_vec;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return this->_key_map;
}

void KGG::getHeaderLen()
{
    // 文件头长度
    this->_file.seekg(16, std::ios::beg);
    this->_file.read(reinterpret_cast<char*>(&this->_header_len), 4);
}

void KGG::checkSupported()
{
    // 校验kgg版本
    uint32_t mode = 0;
    this->_file.read(reinterpret_cast<char*>(&mode), 4);
    if (mode != 5)
    {
        throw DMusicRuntimeError(u8"不受支持的KGG文件: \n" + this->_filepath.u8string());
    }
}

void KGG::findKey()
{
    // 读取音频文件哈希, 以查找key
    uint32_t audio_hash_len = 0;
    this->_file.seekg(68, std::ios::beg);
    this->_file.read(reinterpret_cast<char*>(&audio_hash_len), 4);

    auto audio_hash = std::u8string(audio_hash_len, 0);
    this->_file.read(reinterpret_cast<char*>(audio_hash.data()), audio_hash_len);

    this->_key = std::string_view(reinterpret_cast<const char*>(this->_key_map[audio_hash].data()), this->_key_map[audio_hash].size());
    if (this->_key.empty())
    {
        throw DMusicRuntimeError(u8"找不到对应的key. 可能是kgg.key被删除或已经过期或音频并非在此设备上进行下载.");
    }
}

void KGG::createDecryptor()
{
    this->_qmc2_decryptor = QMC2::Create(this->_key);
    if (!this->_qmc2_decryptor)
    {
        throw DMusicRuntimeError(u8"无法创建QMC2解密器.");
    }
}

void KGG::judgeMusicType()
{
    // 音频文件头
    std::array<uint8_t, 4> magic{};
    this->_file.seekg(this->_header_len, std::ios::beg);
    this->_file.read(reinterpret_cast<char*>(magic.data()), 4);
    this->_qmc2_decryptor->Decrypt(magic, 0);

    constexpr char FLAC_HEADER[4] = { "fLa" };
    constexpr char MP3_HEADER[4] = { "ID3" };   
    // 判断
    if (std::memcmp(magic.data(), FLAC_HEADER, 3) == 0)
    {
        this->_musictype = FLAC;
    }
    else if (std::memcmp(magic.data(), MP3_HEADER, 3) == 0)
    {
        this->_musictype = MP3;
    }
    else
    {
        this->_musictype = OGG;
    }
}

void KGG::createTempOutputFile(fs::path& output_dir)
{
    fs::path temppath = output_dir;
    temppath /= this->_filepath.filename();
    temppath.replace_extension(L".DmusicTemp");
    this->_output_file = std::ofstream(temppath, std::ios::binary);
    this->_output_filepath = temppath;
    if (!this->_output_file.is_open()) 
    {
        throw DMusicRuntimeError(u8"无法写入临时文件: \n" + temppath.u8string());
    }
}

void KGG::renameFile()
{
    // 读取
    TagLib::FileName fileName(this->_output_filepath.c_str());
    std::u8string title;
    std::u8string artist;
    TagLib::FileRef file(fileName);
    if (file.isNull())
    {
        throw DMusicRuntimeError(
            u8"Rename Err - Failed to open file: " +
            this->_output_filepath.u8string());
    }
    title = (char8_t*)file.tag()->title().data(TagLib::String::UTF8).data();
    artist = (char8_t*)file.tag()->artist().data(TagLib::String::UTF8).data();
    file = TagLib::FileRef();// 释放文件
    // 无法确定完整标题则使用源文件名
    if (title.empty() or artist.empty())
    {
        fs::path newpath = this->_output_filepath;
        if (this->_musictype == FLAC)
        {
            newpath.append(u8".flac");
        }
        else if (this->_musictype == MP3)
        {
            newpath.append(u8".mp3");
        }
        fs::rename(this->_output_filepath, newpath);
        return;
    }
    // 重命名
    fs::path newpath = this->_output_filepath.parent_path();
    std::u8string filename = title + u8" - ";
    for (size_t i = 0, j = 0; i < artist.length(); i++)
    {
        if (artist[i] == u8';' and (j >= 2 or i == artist.length() - 1))
        {
            break;
        }
        else if (artist[i] == u8',')
        {
            j++;
            filename += u8",";
            continue;
        }
        filename += artist[i];
    }
    std::u8string new_filename = u8"";
    for (int i = 0; i < filename.length(); i++)
    {
        switch (filename[i])
        {
        case('?'):  new_filename += u8"？"; break;
        case(':'):  new_filename += u8"："; break;
        case('*'):  new_filename += u8"＊"; break;
        case('\"'): new_filename += u8"＂"; break;
        case('<'):  new_filename += u8"＜"; break;
        case('>'):  new_filename += u8"＞"; break;
        case('|'):  new_filename += u8"｜"; break;
        case('/'):  new_filename += u8"／"; break;
        case('\\'): new_filename += u8"＼"; break;
        default: new_filename += filename[i];
        }
    }
    if (this->_musictype == FLAC)
    {
        new_filename.append(u8".flac");
    }
    else if (this->_musictype == MP3)
    {
        new_filename.append(u8".mp3");
    }
    else if (this->_musictype == OGG)
    {
        new_filename.append(u8".ogg");
    }
    newpath /= new_filename;
    fs::rename(this->_output_filepath, newpath);

#if _DEBUG
    qDebug() << L"Title: " << title << L"\n";
    qDebug() << L"Artist: " << artist << L"\n";
#endif
}

void KGG::decrypt(DMusicIOConfig& config)
{
    this->_filepath = config.filepath;
    this->_key_map = config.kggkeymap;
    this->_file.open(config.filepath, std::ios::binary);
    if (!this->_file.is_open()) 
    {
        throw DMusicRuntimeError(u8"无法打开文件: \n" + config.filepath.u8string());
    }

    this->getHeaderLen();
    this->checkSupported();
    this->findKey();
    this->createDecryptor();
    this->judgeMusicType();
    this->createTempOutputFile(config.outputDir);

    // 正式解密
    this->_file.seekg(this->_header_len, std::ios::beg);
    size_t offset{ 0 };
    thread_local std::vector<uint8_t> temp_buffer(1024 * 1024, 0);
    auto read_page_len = static_cast<std::streamsize>(temp_buffer.size());

    while (!this->_file.eof()) 
    {
        this->_file.read(reinterpret_cast<char*>(temp_buffer.data()), read_page_len);
        const auto n = this->_file.gcount();
        this->_qmc2_decryptor->Decrypt(std::span(temp_buffer.begin(), temp_buffer.begin() + n), offset);
        this->_output_file.write(reinterpret_cast<char*>(temp_buffer.data()), n);
        offset += n;
    }
    this->_file.close();
    this->_output_file.close();
    this->renameFile();
}
