#include "NCM.h"

std::vector<uint8_t> NCM::readFile()
{
	int len = 0;
	this->_file.read((char*)&len, 4);
	auto buffer = std::vector<uint8_t>(len);
	this->_file.read((char*)buffer.data(), len);
	return buffer;
}

void NCM::jumpHeader()
{
	this->_file.seekg(10, std::ios::beg);
}

void NCM::decryptRC4key()
{
	auto key_data = this->readFile();
	for (int i = 0; i < key_data.size(); i++)
	{
		key_data[i] ^= 0x64;
	}
	auto RC4_key = _aes.ecb_decrypt(key_data, (const unsigned char*)_core_key);
	RC4_key.erase(RC4_key.begin(), RC4_key.begin() + 17); // AES解密后去除前17位
	this->_rc4key = std::vector<uint8_t>(RC4_key.begin(), RC4_key.end());
}

void NCM::decryptmMetaInfo()
{
	auto meta_data = this->readFile();
	for (int i = 0; i < meta_data.size(); i++)
	{
		meta_data[i] ^= 0x63;
	}

	this->_cloudkey = std::u8string(meta_data.begin(), meta_data.end());
	meta_data.erase(meta_data.begin(), meta_data.begin() + 22);// 去除前22位
	auto meta_info_d = _base64.decode(meta_data);
	auto meta_info = _aes.ecb_decrypt(meta_info_d, (const unsigned char*)_meta_key);
	meta_info.erase(meta_info.begin(), meta_info.begin() + 6); // 去除前6位

	rapidjson::Document doc;
	doc.Parse(std::string(meta_info.begin(), meta_info.end()).c_str());

#if _DEBUG
	qDebug() << "meta_info:" << (char*)meta_info.data();
	if (doc.HasParseError())
	{
		rapidjson::ParseResult result = doc.Parse((char*)meta_info.data());
		if (!result) {
			qDebug() << "错误: " << rapidjson::GetParseError_En(result.Code())
				<< "，位置: " << result.Offset();
		}
	}
#else
	if (doc.HasParseError())
	{
		throw std::runtime_error("Filed to parse ncm json text");
	}
#endif

	// 音乐名
	if (doc.HasMember("musicName"))
	{
		rapidjson::Value& a = doc["musicName"];
		this->_title = (char8_t*)a.GetString();
	}

	// 后缀名
	if (doc.HasMember("format"))
	{
		rapidjson::Value& a = doc["format"];
		this->_file_format = (char8_t*)a.GetString();
	}

	// 歌手
	if (doc.HasMember("artist"))
	{
		auto artists = doc["artist"].GetArray();
		for (rapidjson::SizeType i = 0; i < artists.Size(); i++)
		{
			this->_artists.push_back((char8_t*)artists[i][0].GetString());
		}
	}

	// 专辑
	if (doc.HasMember("album"))
	{
		rapidjson::Value& a = doc["album"];
		this->_album = (char8_t*)a.GetString();
	}
#if _DEBUG
	qDebug() << "title:" << this->_title.c_str();
	qDebug() << "file_format:" << this->_file_format.c_str();
	qDebug() << "album:" << this->_album.c_str() << "\n";
#endif
}

void NCM::jumpCRCcode()
{
	_file.seekg(4, std::ios::cur); // 此段CRC32不知用途
}

void NCM::getCover()
{
	this->_cover = readFile();
}

fs::path NCM::genOutputfilepath(fs::path& output_dir)
{
	fs::path output;
	if (output_dir.empty())
	{
		output = fs::current_path();
	}
	if (!fs::is_directory(output_dir))
	{
		output = output_dir.parent_path();
	}
	else
	{
		output = output_dir;
	}
	std::u8string artist_str = u8"";
	size_t count = 0;
	if (this->_artists.size() == 1)
	{
		artist_str = this->_artists[0];
	}
	else if (this->_artists.size() > 1)
	{
		for (auto& artist : this->_artists)
		{
			artist_str += artist;
			count++;
			if (count >= 3)
			{
				break;
			}
			artist_str += u8",";
		}
	}
	std::u8string filename = this->_title + u8" - " + artist_str + u8"." + this->_file_format;
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
	output /= new_filename;
	return output;
}

void NCM::writeMetaInfo()
{
	using namespace TagLib;
	std::u8string artist_str = u8"";
	if (this->_artists.size() == 1)
	{
		artist_str = this->_artists[0];
	}
	else if (this->_artists.size() > 1)
	{
		for (auto& artist : this->_artists)
		{
			artist_str += artist;
			artist_str += u8";";
		}
	}
	FileName fileName(this->_output_filepath.c_str());
	if (this->_file_format == u8"flac")
	{
		FLAC::File file(fileName);
		if (!file.isOpen())
		{
			throw DMusicRuntimeError(u8"Failed to write meta info to flac file: " +
				this->_filepath.u8string());
		}
		auto img = new FLAC::Picture();
		img->setMimeType("image/jpeg");
		img->setType(FLAC::Picture::FrontCover);
		img->setData(ByteVector((char*)this->_cover.data(), (unsigned int)this->_cover.size()));
		file.addPicture(img);
		auto xiph = file.xiphComment(true);
		if (this->_enableCloudkey)xiph->addField("DESCRIPTION", String((char*)this->_cloudkey.data(), String::UTF8));
		xiph->addField("ALBUM", String((char*)this->_album.data(), String::UTF8));
		xiph->addField("ARTIST", String((char*)artist_str.data(), String::UTF8));
		file.save();
	}
	else
	{
		MPEG::File file(fileName);
		if (!file.isOpen())
		{
			throw DMusicRuntimeError(u8"Failed to write meta info to mp3 file: " +
				this->_filepath.u8string());
		}
		auto tag = file.ID3v2Tag(true);
		auto img = new ID3v2::AttachedPictureFrame();
		img->setMimeType("image/jpeg");
		img->setType(ID3v2::AttachedPictureFrame::FrontCover);
		img->setPicture(ByteVector((char*)this->_cover.data(), (unsigned int)this->_cover.size()));
		tag->removeFrames("APIC");
		tag->addFrame(img);
		if (this->_enableCloudkey)tag->setComment(String((char*)this->_cloudkey.data(), String::UTF8));
		tag->setArtist(String((char*)artist_str.data(), String::UTF8));
		tag->setAlbum(String((char*)this->_album.data(), String::UTF8));
		file.save();
	}
}

void NCM::decrypt(DMusicIOConfig& config)
{
	this->_enableCloudkey = config.enWriteCloudkey;
	this->_filepath = config.filepath;
	this->_file = std::ifstream(this->_filepath, std::ios::binary);
	if (!this->_file.is_open())
	{
		throw DMusicRuntimeError(
			u8"Decrypt Err - Failed to open file: " +
			this->_filepath.u8string());
	}
	this->jumpHeader();
	this->decryptRC4key();
	this->decryptmMetaInfo();
	this->jumpCRCcode();
	this->_file.seekg(5, std::ios::cur);// 跳过5位未知数据
	this->getCover();
	this->_output_filepath = this->genOutputfilepath(config.outputDir);
	/*
	* 正式解密, 算法为RC4变种
	*/
	// SBOX初始化
	char S[256] = {};
	char K[256] = {};
	for (int i = 0; i < 256; i++) { S[i] = i; }

	// 初始替换
	int j = 0;
	for (int i = 0, temp; i < 256; i++)
	{
		j = (j + S[i] + (this->_rc4key)[i % _rc4key.size()]) & 255;
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}

	// 密钥流
	for (int i = 0, a, b; i < 256; i++)
	{
		a = (i + 1) & 255;
		b = S[(a + S[a]) & 255];
		K[i] = (S[(S[a] + b) & 255] & 255);
	}

	// 解密流
	char buffer[256];
	std::ofstream of(this->_output_filepath,std::ios::binary);
	if (!of.is_open())
	{
		throw std::runtime_error("Failed to create output file");
	}
	while (!this->_file.eof())
	{
		this->_file.read(buffer, 256);
		for (int i = 0; i < 256; i++) { buffer[i] ^= K[i]; };
		of.write(buffer, _file.gcount());
	};
	of.flush();
	of.close();
	writeMetaInfo();
}