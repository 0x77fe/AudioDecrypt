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
#include <taglib/xiphcomment.h>
#include <taglib/id3v2framefactory.h>
#include <taglib/textidentificationframe.h>

#include <filesystem>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "Cde.h"


using namespace std;
class NCM
{
	constexpr static const char NCM_hander[] = { 0x43, 0x54, 0x45, 0x4e, 0x46, 0x44, 0x41, 0x4d, 0x01 };//文件头
	constexpr static const char _info_key[] = { 0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28 };//"#14ljk_!\]&0U<'(";//163key
	constexpr static const char _core_key[] = { 0x68,0x7A,0x48,0x52,0x41,0x6D,0x73,0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57 };//"hzHRAmso5kInbaxW";//rc4corekey

	void CheakHeader(stringstream& ms)
	{
		char magic_hander[10];
		ms.read(magic_hander, 10);
		if (strncmp(NCM_hander, magic_hander, 9) != 0) { throw runtime_error("ncm已损坏或不是一个ncm文件"); }
	}

	string GetRC4Key(stringstream& ms)
	{
		int length = 0;//加密RC4密钥的密钥长度
		ms.read((char*)&length, 4);

		string data(length, '\0');//加密后的RC4密钥
		ms.read(data.data(), length);
		for (int i = 0; i <= (length - 1); i++)
		{
			data[i] ^= 0x64;//对每个字节与0x64进行异或
		}
		auto RC4_key = aes_ecb_decrypt(data, _core_key).substr(17);//AES后 去除前17位
		return RC4_key;
	}

	musicInfo GetMusicInfo(stringstream& ms)
	{
		musicInfo inf;
		int length = 0;//音乐信息长度
		ms.read((char*)&length, 4);

		string data(length, '\0');//信息数据
		ms.read(data.data(), length);
		for (int i = 0; i < length; i++)
		{ //对每个字节与0x63进行异或
			data[i] ^= 0x63;
		}
		inf.ncmkey = data;
		data = data.substr(22);//去除前22位
		data = base64_decode(data);//base64
		string info = aes_ecb_decrypt(data, _info_key).substr(6);//aes后去除前6位

		rapidjson::Document doc;
		doc.Parse(info.c_str());

		//音乐名
		if (doc.HasMember("musicName"))
		{
			rapidjson::Value& a = doc["musicName"];
			inf.musicName = a.GetString();
		}

		//后缀名
		if (doc.HasMember("format"))
		{
			rapidjson::Value& a = doc["format"];
			inf.format = a.GetString();
		}

		//歌手
		if (doc.HasMember("artist"))
		{
			auto artists = doc["artist"].GetArray();
			for (int i = 0; i < artists.Size(); i++)
			{
				inf.artist.push_back(artists[i][0].GetString());
			}
		}

		//专辑
		if (doc.HasMember("album"))
		{
			rapidjson::Value& a = doc["album"];
			inf.album = a.GetString();
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

	void DecodeAudio(stringstream& ms, ofstream& f, const string& RC4_key)
	{
		//初始化
		char S[256] = {};
		char K[256] = {};
		for (int i = 0; i < 256; i++) { S[i] = i; }

		//初始替换
		int j = 0;
		for (int i = 0, temp; i < 256; i++)
		{
			j = (j + S[i] + (RC4_key)[i % RC4_key.size()]) & 255;
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

	void SetMusicInfo(filesystem::path& originalFilePath, musicInfo& info, bool write163Key)
	{
		using namespace TagLib;
		if (info.format == "flac")
		{
			FLAC::File file(originalFilePath.c_str());
			auto img = new FLAC::Picture();
			img->setData(ByteVector(info.cover.data(), info.cover.length()));
			img->setMimeType("image/jpeg");
			img->setType(FLAC::Picture::FrontCover);
			file.addPicture(img);
			auto xiph = file.xiphComment(true);
			if (write163Key)xiph->addField("DESCRIPTION", String(info.ncmkey, String::UTF8));
			xiph->addField("ALBUM", String(info.album, String::UTF8));
			xiph->addField("ARTIST", String(join(info.artist, ";"), String::UTF8));

			file.save();
		}
		else
		{
			MPEG::File file(originalFilePath.c_str());
			auto tag = file.ID3v2Tag(true);
			auto img = new ID3v2::AttachedPictureFrame();
			img->setMimeType("image/jpeg");
			img->setType(ID3v2::AttachedPictureFrame::FrontCover);
			img->setPicture(ByteVector(info.cover.data(), info.cover.length()));
			tag->removeFrames("APIC");
			tag->addFrame(img);
			if (write163Key)tag->setComment(String(info.ncmkey, String::UTF8));
			tag->setArtist(String(join(info.artist, ";"), String::UTF8));
			tag->setAlbum(String(info.album, String::UTF8));

			file.save();
		}
	}

public:
	void NCMDecrypt(const filesystem::path& originalFilePath, bool write163Key, filesystem::path _outputPath)
	{
		ifstream f(originalFilePath, ios::binary);
		if (!f) { throw runtime_error("打开文件失败"); return; };

		stringstream ms;
		ms << f.rdbuf();
		f.close();


		CheakHeader(ms);
		auto RC4_key = GetRC4Key(ms);
		auto info = GetMusicInfo(ms);
		int CRC = GetCRCCode(ms);
		ms.seekg(5, ios::cur);
		//获取封面
		info.cover = GetImage(ms);

		//拼接文件名
		if (info.artist.size() > 3) { info.artist = { info.artist[0],info.artist[1],info.artist[2],"..." }; };
		string name = (info.musicName + " - " + join(info.artist, (string)",") + "." + info.format);

		//替换为全角字符,防止出错
		name = replace_(name, "?", { "？" });
		name = replace_(name, "*", { "＊" });
		name = replace_(name, ":", { "：" });
		name = replace_(name, "<", { "＜" });
		name = replace_(name, ">", { "＞" });
		name = replace_(name, "/", { "／" });
		name = replace_(name, "\\", { "＼" });
		name = replace_(name, "|", { "｜" });
		name = replace_(name, "\"", "＂");

		//utf-8文件名
		wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		wstring wideFilename = converter.from_bytes(name);

		filesystem::path outputPath = _outputPath;

		filesystem::path outoriginalFilePath;
		if (outputPath.empty())
		{
			outoriginalFilePath = originalFilePath.parent_path().append(wideFilename);
		}
		else
		{
			outoriginalFilePath = outputPath.wstring() + (L"\\" + wideFilename);
		}

		//正式解密文件
		ofstream file(outoriginalFilePath, ios::out | ios::binary);
		if (!file.good()) { throw runtime_error("写文件错误"); }
		DecodeAudio(ms, file, RC4_key);

		SetMusicInfo(outoriginalFilePath, info, write163Key);
	}
};