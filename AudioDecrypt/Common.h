﻿#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <locale>
#include <codecvt>

using namespace std;
//

template <typename T>
bool has(const vector<T>& list,const T& obj)
{
	for (auto& item : list)
	{
		if (item == obj) { return true; }
	}
	return false;
}

//音乐信息
struct musicInfo
{
	string musicName;
	string format;
	vector<string> artist;
	string cover;
};

//flac文件头
const char FLAC_HEADER[4] = "fLa";

//搜索文件
 vector<filesystem::path> SerchFiles(const filesystem::path& Dir, const vector<string>& Suffixs)
{
	using namespace filesystem;
	auto files = vector<path>();
	for (const auto& entry : recursive_directory_iterator(Dir))
	{
		if (is_regular_file(entry.path()) and has(Suffixs, entry.path().extension().string()))
		{
			files.push_back(entry.path());
		}
	}
	return files;
}

//替换文本
string replace_(string tarstr, const string& oldstr, const string& newstr)
{
	regex reg(oldstr);
	string replaced_str = regex_replace(tarstr, reg, newstr);
	return replaced_str;
}

//拼接数组
template<typename T, typename E>
E join(T list, E split)
{
	E out;
	for (int i = 0; i < list.size() - 1; i++)
	{
		out += list[i] + split;
	}
	out += list[list.size() - 1];
	return out;
}

//win下的字符转换
#ifdef WIN32
#include <Windows.h>
string Utf8ToGbk(const string& utf8_str)
{
	// 将UTF-8编码的字符串转换为UTF-16编码的字符串
	int utf16_length = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
	wchar_t* utf16_buffer = new wchar_t[utf16_length];
	MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, utf16_buffer, utf16_length);

	// 将UTF-16编码的字符串转换为GBK编码的字符串
	int gbk_length = WideCharToMultiByte(CP_ACP, 0, utf16_buffer, -1, nullptr, 0, nullptr, nullptr);
	char* gbk_buffer = new char[gbk_length];
	WideCharToMultiByte(CP_ACP, 0, utf16_buffer, -1, gbk_buffer, gbk_length, nullptr, nullptr);

	string result(gbk_length, '\0');
	memcpy(result.data(), gbk_buffer, gbk_length);
	delete[] gbk_buffer, utf16_buffer;
	return result;
}
string GbkToUtf8(const std::string& gbkStr)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, wstr, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* utf8Str = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8Str, len, NULL, NULL);

	std::string result(utf8Str);
	delete[] wstr;
	delete[] utf8Str;
	return result;
}
#endif // WIN32
string w32(const string& utf8_str)
{
#ifdef WIN32
	return Utf8ToGbk(utf8_str);
#else
	return utf8_str;
#endif
}

string w32b(const string& gbk_str)
{
#ifdef WIN32
	return GbkToUtf8(gbk_str);
#else
	return gbk_str;
#endif
}

//创建临时文件
filesystem::path CreateTempFile(filesystem::path indir)
{
	int pos = 0;
	ifstream file(indir.string() + "\\" + to_string(pos));
	while (file)
	{
		pos += 1;
		file = ifstream(indir.string() + "\\" + to_string(pos));
	}
	return (indir.string().append("\\").append(to_string(pos)));
}