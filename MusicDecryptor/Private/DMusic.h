#pragma once
//std
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <map>

namespace fs = std::filesystem;

class KeyMap
{
private:
	std::map<std::u8string, std::vector<uint8_t>> _keymap;

public:
	KeyMap();
	KeyMap(std::u8string keystr);
	KeyMap(std::map<std::u8string, std::u8string> stdmap);
	~KeyMap() {};
	//
	std::u8string getStr();
	std::vector<uint8_t>& operator[](std::u8string& id);
};

struct DMusicIOConfig
{
	fs::path filepath;
	fs::path outputDir;
	KeyMap kggkeymap;
	bool enWriteCloudkey = true;
};

class DMusic
{
public:	
	virtual void decrypt(DMusicIOConfig& config) = 0;
	virtual ~DMusic() = default;
};

class DMusicRuntimeError : public std::exception
{	
public:
	DMusicRuntimeError(const std::u8string msg) : _msg(msg) {}
	const char* what() const noexcept override { return reinterpret_cast<const char*>(_msg.c_str()); }
private:
	std::u8string _msg;
};