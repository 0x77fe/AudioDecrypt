#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
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

#include <filesystem>
#include <fstream>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include "Cde.h"

#include "Common.h"

using namespace std;

namespace ncm {
	constexpr static const char NCM_hander[] = { 0x43, 0x54, 0x45, 0x4e, 0x46, 0x44, 0x41, 0x4d, 0x01, 0x70 };//文件头
	constexpr static const char _info_key[] = { 0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28 };//"#14ljk_!\]&0U<'(";//163key
	constexpr static const char _core_key[] = { 0x68,0x7A,0x48,0x52,0x41,0x6D,0x73,0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57 };//"hzHRAmso5kInbaxW";//rc4corekey

	void CheakHeader(stringstream& ms)
	{
		char magic_hander[10];
		ms.read(magic_hander, 10);
		if (strncmp(NCM_hander, magic_hander, 10) != 0) { throw new exception("[HANDER]ncm已损坏或不是一个ncm文件"); }
	}

	string* GetRC4Key(stringstream& ms)
	{
		int length = 0;//加密RC4密钥的密钥长度
		ms.read((char*)&length, 4);

		string data(length, '\0');//加密后的RC4密钥
		ms.read(data.data(), length);
		for (int i = 0; i <= (length - 1); i++)
		{
			data[i] ^= 0x64;//对每个字节与0x64进行异或
		}
		auto RC4_key = new string(aes_ecb_decrypt(data, _core_key).substr(17));//AES后 去除前17位
		return RC4_key;
	}

	musicInfo* GetMusicInfo(stringstream& ms)
	{
		int length = 0;//音乐信息长度
		ms.read((char*)&length, 4);

		string data(length, '\0');//信息数据
		ms.read(data.data(), length);
		for (int i = 0; i < length; i++)
		{ //对每个字节与0x63进行异或
			data[i] ^= 0x63;
		}
		data = data.substr(22);//去除前22位
		data = base64_decode(data);//base64
		string info = aes_ecb_decrypt(data, _info_key).substr(6);//aes后去除前6位

		rapidjson::Document doc;
		doc.Parse(info.c_str());

		auto inf = new musicInfo;

		//音乐名
		if (doc.HasMember("musicName"))
		{
			rapidjson::Value& a = doc["musicName"];
			inf->musicName = a.GetString();
		}

		//后缀名
		if (doc.HasMember("format"))
		{
			rapidjson::Value& a = doc["format"];
			inf->format = a.GetString();
		}

		//歌手
		if (doc.HasMember("artist"))
		{
			auto artists = doc["artist"].GetArray();
			for (int i = 0; i < artists.Size(); i++)
			{
				inf->artist.push_back(artists[i][0].GetString());
			}
		}

		return inf;
	}

	int GetCRCCode(stringstream& ms)
	{
		int CRC = 0;
		ms.read((char*)&CRC, 4);
		return CRC;
	}

	string GetImage(stringstream& ms)
	{
		int length = 0;
		ms.read((char*)&length, 4);

		string data(length, '\0');
		ms.read(data.data(), length);
		return data;
	}

	void DecodeAudio(stringstream& ms, ofstream& f, const string* RC4_key)
	{
		//初始化
		char S[256] = {};
		char K[256] = {};
		for (int i = 0; i < 256; i++) { S[i] = i; }

		//初始替换
		int j = 0;
		for (int i = 0, temp; i < 256; i++)
		{
			j = (j + S[i] + (*RC4_key)[i % RC4_key->size()]) & 255;
			temp = S[i];
			S[i] = S[j];
			S[j] = temp;
		}

		//密钥流
		for (int i = 0, a, b; i < 256; i++)
		{
			a = (i + 1) & 255;
			b = S[(a + S[a]) & 255];
			K[i] = (S[(S[a] + b) & 255] & 255);
		}

		//解密流
		char buffer[256];
		stringstream output;
		while (!ms.eof())
		{
			ms.read(buffer, 256);
			for (int i = 0; i < 256; i++) { buffer[i] ^= K[i]; };
			f.write(buffer, ms.gcount());
		};
		f.flush();
		f.close();
	}

	void SetMusicInfo(filesystem::path& filepath, musicInfo* info)
	{
		using namespace TagLib;
		if (info->format == "flac")
		{
			FLAC::File file(filepath.c_str());

			auto pic = new FLAC::Picture();
			pic->setData(ByteVector(info->cover.data(), info->cover.length()));
			pic->setMimeType("image/jpeg");
			pic->setType(TagLib::FLAC::Picture::FrontCover);

			file.addPicture(pic);
			file.save();
		}
		else
		{
			MPEG::File file(filepath.c_str());
			auto tag = file.ID3v2Tag(true);

			auto frame = new ID3v2::AttachedPictureFrame();
			frame->setMimeType("image/jpeg");
			frame->setType(ID3v2::AttachedPictureFrame::FrontCover);
			frame->setPicture(ByteVector(info->cover.data(), info->cover.length()));

			tag->removeFrames("APIC");
			tag->addFrame(frame);

			file.save();
		}

	}


	void Decrypt(const filesystem::path& filepath, filesystem::path& outputpath = *new filesystem::path(), bool skip = false)
	{
		ifstream f(filepath, ios::binary);
		if (!f) { throw new exception("[MAIN]打开文件失败"); return; };

		stringstream ms;
		ms << f.rdbuf();
		f.close();


		CheakHeader(ms);
		auto RC4_key = GetRC4Key(ms);
		auto info = GetMusicInfo(ms);
		int CRC = GetCRCCode(ms);
		ms.seekg(5, ios::cur);
		//获取封面
		info->cover = GetImage(ms);


		//拼接文件名
		string name = w32(info->musicName + " - " + join(info->artist, (string)",") + "." + info->format);

		//将斜杆替换为全角字符,防止出错
		name = replace_(name, "/", { char(-93),char(-81) });

		filesystem::path outputfile_path;
		if (outputpath.empty())
		{
			outputfile_path = filepath.parent_path().append(name);
		}
		else
		{
			outputfile_path = outputpath.string().append("\\").append(name);
		}
		ifstream infile(outputfile_path);
		if ((!infile.good()) && skip) { return; }
		infile.close();

		//正式解密文件
		ofstream file(outputfile_path, ios::out|ios::binary);
		if (!file.good()) { throw new exception("[MAIN]写文件错误"); }
		DecodeAudio(ms, file, RC4_key);

		SetMusicInfo(outputfile_path, info);
		delete RC4_key, info;
	}
};