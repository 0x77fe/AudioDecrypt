#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <qstring>

using namespace std;

//判断是否存在某元素
template <typename T>
bool has(const vector<T>& list, const T& obj)
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
	std::string musicName;
	std::string format;
	std::vector<string> artist;
	std::string cover;
	std::string ncmkey;
	std::string album;
};

//flac文件头
constexpr char FLAC_HEADER[4] = { "fLa" };
constexpr char MP3_HEADER[4] = { "ID3" };

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
	regex reg("\\" + oldstr);
	string replaced_str = regex_replace(tarstr, reg, newstr);
	return replaced_str;
}

//拼接数组
template<typename T>
string join(T list, string split)
{
	string out;
	for (int i = 0; i < list.size() - 1; i++)
	{
		out += list[i] + split;
	}
	out += list[list.size() - 1];
	return out;
}

template<typename T>
QString Qjoin(T list, string split)
{
	QString out;
	for (int i = 0; i < list.size() - 1; i++)
	{
		out += list[i] + QString::fromUtf8(split);
	}
	out += list[list.size() - 1];
	return out;
}

//创建临时文件
filesystem::path CreateTempFile(filesystem::path indir)
{
	int pos = 100;
	string pat = "";
	string org = indir.string();
	if (('\\' == org[org.size() - 1]) or ('/' == org[org.size() - 1]))
	{
		pat = indir.string() + "temp_";
	}
	else
	{
		pat = indir.string() + "\\" + "temp_";
	}
	ifstream file(pat + to_string(pos));
	while (file)
	{
		pos += 1;
		file = ifstream(pat + to_string(pos));
	}
	return (pat + to_string(pos));
}