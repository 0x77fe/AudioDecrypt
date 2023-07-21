#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>

using namespace std;
namespace kgma {

	enum MType { VPR, OTHER };

	MType H;

	constexpr static const unsigned char VprHeader[] = { 0x05, 0x28, 0xBC, 0x96, 0xE9, 0xE4, 0x5A, 0x43,0x91, 0xAA, 0xBD, 0xD0, 0x7A, 0xF5, 0x36, 0x31 };
	constexpr static const unsigned char KgmHeader[] = { 0x7C, 0xD5, 0x32, 0xEB, 0x86, 0x02, 0x7F, 0x4B,0xA8, 0xAF, 0xA6, 0x8E, 0x0F, 0xFF, 0x99, 0x14 };
	constexpr static const unsigned char VprMaskDiff[] = { 0x25, 0xDF, 0xE8, 0xA6, 0x75, 0x1E, 0x75, 0x0E,0x2F, 0x80, 0xF3, 0x2D, 0xB8, 0xB6, 0xE3, 0x11, 0x00 };

	constexpr static const unsigned char table1[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x21, 0x01, 0x61, 0x01, 0x21, 0x01, 0xe1, 0x01, 0x21, 0x01, 0x61, 0x01, 0x21, 0x01,
		0xd2, 0x23, 0x02, 0x02, 0x42, 0x42, 0x02, 0x02, 0xc2, 0xc2, 0x02, 0x02, 0x42, 0x42, 0x02, 0x02,
		0xd3, 0xd3, 0x02, 0x03, 0x63, 0x43, 0x63, 0x03, 0xe3, 0xc3, 0xe3, 0x03, 0x63, 0x43, 0x63, 0x03,
		0x94, 0xb4, 0x94, 0x65, 0x04, 0x04, 0x04, 0x04, 0x84, 0x84, 0x84, 0x84, 0x04, 0x04, 0x04, 0x04,
		0x95, 0x95, 0x95, 0x95, 0x04, 0x05, 0x25, 0x05, 0xe5, 0x85, 0xa5, 0x85, 0xe5, 0x05, 0x25, 0x05,
		0xd6, 0xb6, 0x96, 0xb6, 0xd6, 0x27, 0x06, 0x06, 0xc6, 0xc6, 0x86, 0x86, 0xc6, 0xc6, 0x06, 0x06,
		0xd7, 0xd7, 0x97, 0x97, 0xd7, 0xd7, 0x06, 0x07, 0xe7, 0xc7, 0xe7, 0x87, 0xe7, 0xc7, 0xe7, 0x07,
		0x18, 0x38, 0x18, 0x78, 0x18, 0x38, 0x18, 0xe9, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x08, 0x09, 0x29, 0x09, 0x69, 0x09, 0x29, 0x09,
		0xda, 0x3a, 0x1a, 0x3a, 0x5a, 0x3a, 0x1a, 0x3a, 0xda, 0x2b, 0x0a, 0x0a, 0x4a, 0x4a, 0x0a, 0x0a,
		0xdb, 0xdb, 0x1b, 0x1b, 0x5b, 0x5b, 0x1b, 0x1b, 0xdb, 0xdb, 0x0a, 0x0b, 0x6b, 0x4b, 0x6b, 0x0b,
		0x9c, 0xbc, 0x9c, 0x7c, 0x1c, 0x3c, 0x1c, 0x7c, 0x9c, 0xbc, 0x9c, 0x6d, 0x0c, 0x0c, 0x0c, 0x0c,
		0x9d, 0x9d, 0x9d, 0x9d, 0x1d, 0x1d, 0x1d, 0x1d, 0x9d, 0x9d, 0x9d, 0x9d, 0x0c, 0x0d, 0x2d, 0x0d,
		0xde, 0xbe, 0x9e, 0xbe, 0xde, 0x3e, 0x1e, 0x3e, 0xde, 0xbe, 0x9e, 0xbe, 0xde, 0x2f, 0x0e, 0x0e,
		0xdf, 0xdf, 0x9f, 0x9f, 0xdf, 0xdf, 0x1f, 0x1f, 0xdf, 0xdf, 0x9f, 0x9f, 0xdf, 0xdf, 0x0e, 0x0f,
		0x00, 0x20, 0x00, 0x60, 0x00, 0x20, 0x00, 0xe0, 0x00, 0x20, 0x00, 0x60, 0x00, 0x20, 0x00, 0xf1
	};

	constexpr static const unsigned char table2[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x23, 0x01, 0x67, 0x01, 0x23, 0x01, 0xef, 0x01, 0x23, 0x01, 0x67, 0x01, 0x23, 0x01,
		0xdf, 0x21, 0x02, 0x02, 0x46, 0x46, 0x02, 0x02, 0xce, 0xce, 0x02, 0x02, 0x46, 0x46, 0x02, 0x02,
		0xde, 0xde, 0x02, 0x03, 0x65, 0x47, 0x65, 0x03, 0xed, 0xcf, 0xed, 0x03, 0x65, 0x47, 0x65, 0x03,
		0x9d, 0xbf, 0x9d, 0x63, 0x04, 0x04, 0x04, 0x04, 0x8c, 0x8c, 0x8c, 0x8c, 0x04, 0x04, 0x04, 0x04,
		0x9c, 0x9c, 0x9c, 0x9c, 0x04, 0x05, 0x27, 0x05, 0xeb, 0x8d, 0xaf, 0x8d, 0xeb, 0x05, 0x27, 0x05,
		0xdb, 0xbd, 0x9f, 0xbd, 0xdb, 0x25, 0x06, 0x06, 0xca, 0xca, 0x8e, 0x8e, 0xca, 0xca, 0x06, 0x06,
		0xda, 0xda, 0x9e, 0x9e, 0xda, 0xda, 0x06, 0x07, 0xe9, 0xcb, 0xe9, 0x8f, 0xe9, 0xcb, 0xe9, 0x07,
		0x19, 0x3b, 0x19, 0x7f, 0x19, 0x3b, 0x19, 0xe7, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x08, 0x09, 0x2b, 0x09, 0x6f, 0x09, 0x2b, 0x09,
		0xd7, 0x39, 0x1b, 0x39, 0x5f, 0x39, 0x1b, 0x39, 0xd7, 0x29, 0x0a, 0x0a, 0x4e, 0x4e, 0x0a, 0x0a,
		0xd6, 0xd6, 0x1a, 0x1a, 0x5e, 0x5e, 0x1a, 0x1a, 0xd6, 0xd6, 0x0a, 0x0b, 0x6d, 0x4f, 0x6d, 0x0b,
		0x95, 0xb7, 0x95, 0x7b, 0x1d, 0x3f, 0x1d, 0x7b, 0x95, 0xb7, 0x95, 0x6b, 0x0c, 0x0c, 0x0c, 0x0c,
		0x94, 0x94, 0x94, 0x94, 0x1c, 0x1c, 0x1c, 0x1c, 0x94, 0x94, 0x94, 0x94, 0x0c, 0x0d, 0x2f, 0x0d,
		0xd3, 0xb5, 0x97, 0xb5, 0xd3, 0x3d, 0x1f, 0x3d, 0xd3, 0xb5, 0x97, 0xb5, 0xd3, 0x2d, 0x0e, 0x0e,
		0xd2, 0xd2, 0x96, 0x96, 0xd2, 0xd2, 0x1e, 0x1e, 0xd2, 0xd2, 0x96, 0x96, 0xd2, 0xd2, 0x0e, 0x0f,
		0x00, 0x22, 0x00, 0x66, 0x00, 0x22, 0x00, 0xee, 0x00, 0x22, 0x00, 0x66, 0x00, 0x22, 0x00, 0xfe
	};

	constexpr static const unsigned char MaskV2PreDef[] = {
		0xB8, 0xD5, 0x3D, 0xB2, 0xE9, 0xAF, 0x78, 0x8C, 0x83, 0x33, 0x71, 0x51, 0x76, 0xA0, 0xCD, 0x37,
		0x2F, 0x3E, 0x35, 0x8D, 0xA9, 0xBE, 0x98, 0xB7, 0xE7, 0x8C, 0x22, 0xCE, 0x5A, 0x61, 0xDF, 0x68,
		0x69, 0x89, 0xFE, 0xA5, 0xB6, 0xDE, 0xA9, 0x77, 0xFC, 0xC8, 0xBD, 0xBD, 0xE5, 0x6D, 0x3E, 0x5A,
		0x36, 0xEF, 0x69, 0x4E, 0xBE, 0xE1, 0xE9, 0x66, 0x1C, 0xF3, 0xD9, 0x02, 0xB6, 0xF2, 0x12, 0x9B,
		0x44, 0xD0, 0x6F, 0xB9, 0x35, 0x89, 0xB6, 0x46, 0x6D, 0x73, 0x82, 0x06, 0x69, 0xC1, 0xED, 0xD7,
		0x85, 0xC2, 0x30, 0xDF, 0xA2, 0x62, 0xBE, 0x79, 0x2D, 0x62, 0x62, 0x3D, 0x0D, 0x7E, 0xBE, 0x48,
		0x89, 0x23, 0x02, 0xA0, 0xE4, 0xD5, 0x75, 0x51, 0x32, 0x02, 0x53, 0xFD, 0x16, 0x3A, 0x21, 0x3B,
		0x16, 0x0F, 0xC3, 0xB2, 0xBB, 0xB3, 0xE2, 0xBA, 0x3A, 0x3D, 0x13, 0xEC, 0xF6, 0x01, 0x45, 0x84,
		0xA5, 0x70, 0x0F, 0x93, 0x49, 0x0C, 0x64, 0xCD, 0x31, 0xD5, 0xCC, 0x4C, 0x07, 0x01, 0x9E, 0x00,
		0x1A, 0x23, 0x90, 0xBF, 0x88, 0x1E, 0x3B, 0xAB, 0xA6, 0x3E, 0xC4, 0x73, 0x47, 0x10, 0x7E, 0x3B,
		0x5E, 0xBC, 0xE3, 0x00, 0x84, 0xFF, 0x09, 0xD4, 0xE0, 0x89, 0x0F, 0x5B, 0x58, 0x70, 0x4F, 0xFB,
		0x65, 0xD8, 0x5C, 0x53, 0x1B, 0xD3, 0xC8, 0xC6, 0xBF, 0xEF, 0x98, 0xB0, 0x50, 0x4F, 0x0F, 0xEA,
		0xE5, 0x83, 0x58, 0x8C, 0x28, 0x2C, 0x84, 0x67, 0xCD, 0xD0, 0x9E, 0x47, 0xDB, 0x27, 0x50, 0xCA,
		0xF4, 0x63, 0x63, 0xE8, 0x97, 0x7F, 0x1B, 0x4B, 0x0C, 0xC2, 0xC1, 0x21, 0x4C, 0xCC, 0x58, 0xF5,
		0x94, 0x52, 0xA3, 0xF3, 0xD3, 0xE0, 0x68, 0xF4, 0x00, 0x23, 0xF3, 0x5E, 0x0A, 0x7B, 0x93, 0xDD,
		0xAB, 0x12, 0xB2, 0x13, 0xE8, 0x84, 0xD7, 0xA7, 0x9F, 0x0F, 0x32, 0x4C, 0x55, 0x1D, 0x04, 0x36,
		0x52, 0xDC, 0x03, 0xF3, 0xF9, 0x4E, 0x42, 0xE9, 0x3D, 0x61, 0xEF, 0x7C, 0xB6, 0xB3, 0x93, 0x50,
	};

	bool CheakHeader(stringstream& ms)
	{
		char magic_hander[16];
		ms.read(magic_hander, 16);
		if (strncmp(magic_hander, (char*)VprHeader, 16) == 0) { H = VPR; return true; };
		if (strncmp(magic_hander, (char*)KgmHeader, 16) == 0) { H = OTHER; return true; };
		//throw new runtime_error("[MAIN]文件已损坏或不是一个支持的文件");
		return false;
	}

	int HeaderLength(stringstream& ms)
	{
		int length = 0;
		ms.read((char*)&length, 4);
		return length;
	}

	string* GetKey(stringstream& ms)
	{
		auto key = new string(17, 0);
		ms.seekg(28, ios_base::beg);
		ms.read(key->data(), 16);
		return key;
	}

	void DecodeAudio(stringstream& ms, ofstream& f, const string* key)
	{
		size_t pos = 0, offset = 0;
		char med8, msk8;
		char buffer[4096];
		while (!ms.eof())
		{
			ms.read(buffer, 4096);

			for (int i = 0; i < 4096; i++)
			{
			med8 = (*key)[(pos) % 17] ^ buffer[i];
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
			if (H == VPR) { buffer[i] ^= VprMaskDiff[pos % 17]; };
			pos++;
			}
			f.write(buffer, ms.gcount());
		}
		f.flush();
		f.close();
	}

	musicInfo* GetMusicInfo(filesystem::path filepath)
	{
		using namespace TagLib;
		auto info = new musicInfo;
		ifstream file(filepath);
		char magic_hander[3];
		file.read(magic_hander, 3);
		if (strncmp(magic_hander, (char*)FLAC_HEADER, 3) == 0)
		{
			file.close();
			info->format = "flac";
		}
		else
		{
			file.close();
			info->format = "mp3";
		}
		FileRef f(filepath.c_str());
		auto tag = f.tag();
		info->artist.push_back(tag->artist().to8Bit(true));
		info->musicName = tag->title().to8Bit(true);
		return info;
	}

	void Save(filesystem::path filepath, stringstream& ms, filesystem::path outputfile_path, ofstream& fs)
	{
		auto info = GetMusicInfo(filepath);
		string name = w32(info->musicName + " - " + join(info->artist, (string)",") + "." + info->format);

		//将斜杆替换为全角字符,防止出错
		name = replace_(name, "/", { char(-93),char(-81) });

		//复制文件
		ms.seekg(0, ios_base::beg);
		fs << ms.rdbuf();
		fs.flush();
		fs.close();

		filesystem::path save_path = outputfile_path.parent_path().append(name);
		rename(outputfile_path, save_path);

		delete info;
	}

	void Decrypt(const filesystem::path& filename, filesystem::path& outputpath = *new filesystem::path(), bool skip = false)
	{
		ifstream f(filename, ios::binary);
		if (!f) { throw new runtime_error("打开文件失败"); return; };

		//读入文件
		stringstream ms;
		ms << f.rdbuf();
		f.close();

		//临时文件
		filesystem::path outputfile_path;
		if (outputpath.empty())
		{
			outputfile_path = CreateTempFile(filename.parent_path());
		}
		else
		{
			outputfile_path = CreateTempFile(outputpath);
		}
		ofstream fs(outputfile_path, ios::out | ios_base::binary);

		//检查文件头
		if (!CheakHeader(ms)) { f.close(); Save(filename,ms, outputfile_path, fs); return; };
		//头部数据长度
		int length = HeaderLength(ms);
		//key
		auto key = GetKey(ms);

		//正式解密
		ms.seekg(length, ios_base::beg);
		DecodeAudio(ms, fs, key);

		auto info = GetMusicInfo(outputfile_path);
		string name = w32(info->musicName + " - " + join(info->artist, (string)",") + "." + info->format);

		//将斜杆替换为全角字符,防止出错
		name = replace_(name, "/", { char(-93),char(-81) });

		filesystem::path save_path = outputfile_path.parent_path().append(name);

		rename(outputfile_path, save_path);

		delete key, info;
	}
};

