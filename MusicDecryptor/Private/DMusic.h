#pragma once
//std
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

struct DMusicIOConfig
{
	fs::path filepath;
	fs::path outputDir;
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
	const char* what() const noexcept override { return (char*)_msg.c_str(); }
private:
	std::u8string _msg;
};