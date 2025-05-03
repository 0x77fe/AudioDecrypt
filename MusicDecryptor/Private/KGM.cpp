#include "KGM.h"

void KGMBase::jumpHeader()
{
	this->_file.seekg(16, std::ios::beg);
}

void KGMBase::ceeateTempOutputFile(fs::path& output_dir)
{
	fs::path temppath = output_dir;
	temppath /= this->_filepath.filename();
	temppath.replace_extension(L".DmusicTemp");
	this->_output_file = std::ofstream(temppath, std::ios::binary);
	this->_output_filepath = temppath;
}

void KGMBase::getHeaderLen()
{
	this->_file.read((char*)&this->_header_len, 4);
}

void KGMBase::getKey()
{
	this->_key.resize(17,0);//最后一个字节为0
	this->_file.seekg(28, std::ios_base::beg);
	this->_file.read((char*)this->_key.data(), 16);
}

void KGMBase::judgeMusicType()
{
	constexpr char FLAC_HEADER[4] = { "fLa" };
	constexpr char MP3_HEADER[4] = { "ID3" };
	uint8_t header[4] = { 0 };
	auto music_file = std::ifstream(this->_output_filepath, std::ios::binary);
	if (!music_file.is_open())
	{
		throw DMusicRuntimeError(
			u8"Rename Err - Failed to open file: " +
			this->_output_filepath.u8string());
	}
	// 判断
	music_file.seekg(0, std::ios_base::beg);
	music_file.read((char*)header, 3);
	if (std::memcmp(header, FLAC_HEADER, 3) == 0)
	{
		this->_musictype = FLAC;
	}
	else if (std::memcmp(header, MP3_HEADER, 3) == 0)
	{
		this->_musictype = MP3;
	}
	else
	{
		music_file.close();
		fs::remove(this->_output_filepath);
		throw DMusicRuntimeError(
			u8"Rename Err - Unsupport file type: " +
			this->_output_filepath.u8string());
	}
	music_file.close();
}

void KGMBase::renameFile()
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
	newpath /= new_filename;
	fs::rename(this->_output_filepath, newpath);
	
#if _DEBUG
	qDebug() << L"Title: " << title << L"\n";
	qDebug() << L"Artist: " << artist << L"\n";
#endif
}

void KGM_kgm::decrypt(DMusicIOConfig& config)
{
	this->_filepath = config.filepath;
	this->_file.open(this->_filepath, std::ios::binary);
	if (!this->_file.is_open())
	{
		throw DMusicRuntimeError(
			u8"Decrypt Err - Failed to open file: " +
			this->_filepath.u8string());
	}
	this->jumpHeader();
	this->getHeaderLen();
	this->getKey();
	this->ceeateTempOutputFile(config.outputDir);
	// 正式解密
	this->_file.seekg(this->_header_len, std::ios_base::beg);
	size_t pos = 0, offset = 0;
	uint8_t med8, msk8;
	uint8_t buffer[4096] = { 0 };
	while (!this->_file.eof())
	{
		this->_file.read((char*)buffer, sizeof(buffer));
		for (int i = 0; i < sizeof(buffer); i++)
		{
			med8 = (this->_key)[(pos) % 17] ^ buffer[i];
			med8 ^= (med8 & 15) << 4;
			msk8 = 0;
			offset = pos >> 4;
			while (offset >= 0x11)
			{
				msk8 ^= table1[offset % 272];
				offset >>= 4;
				msk8 ^= table2[offset % 272];
				offset >>= 4;
			}
			msk8 = MaskV2PreDef[pos % 272] ^ msk8;
			msk8 ^= (msk8 & 15) << 4;

			buffer[i] = med8 ^ msk8;
			pos++;
		}
		this->_output_file.write((char*)buffer, this->_file.gcount());
	}
	this->_output_file.flush();
	this->_output_file.close();
	this->_file.close();
	//end
	this->judgeMusicType();
	this->renameFile();
}

void KGM_vpr::decrypt(DMusicIOConfig& config)
{
	this->_filepath = config.filepath;
	this->_file.open(this->_filepath, std::ios::binary);
	if (!this->_file.is_open())
	{
		throw DMusicRuntimeError(
			u8"Decrypt Err - Failed to open file: " +
			this->_filepath.u8string());
	}
	this->jumpHeader();
	this->getHeaderLen();
	this->getKey();
	this->ceeateTempOutputFile(config.outputDir);
	// 正式解密
	this->_file.seekg(this->_header_len, std::ios_base::beg);
	size_t pos = 0, offset = 0;
	uint8_t med8, msk8;
	uint8_t buffer[4096] = { 0 };
	while (!this->_file.eof())
	{
		this->_file.read((char*)buffer, sizeof(buffer));
		for (int i = 0; i < sizeof(buffer); i++)
		{
			med8 = (this->_key)[(pos) % 17] ^ buffer[i];
			med8 ^= (med8 & 15) << 4;
			msk8 = 0;
			offset = pos >> 4;
			while (offset >= 0x11)
			{
				msk8 ^= table1[offset % 272];
				offset >>= 4;
				msk8 ^= table2[offset % 272];
				offset >>= 4;
			}
			msk8 = MaskV2PreDef[pos % 272] ^ msk8;
			msk8 ^= (msk8 & 15) << 4;
			buffer[i] = med8 ^ msk8;
			buffer[i] ^= VprMaskDiff[pos % 17];
			pos++;
		}
		this->_output_file.write((char*)buffer, this->_file.gcount());
	}
	this->_output_file.flush();
	this->_output_file.close();
	this->_file.close();
	//end
	this->judgeMusicType();
	this->renameFile();
}
