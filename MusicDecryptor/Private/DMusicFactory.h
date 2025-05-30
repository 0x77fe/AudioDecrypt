#pragma once
#if _DEBUG
#include <qdebug.h>
#endif
//user
#include "NCM.h"
#include "KGM.h"
#include "KGG.h"

class DMusicFactory
{
public:
	static std::unique_ptr<DMusic> create(const DMusicIOConfig& config)
	{
		char magic_header[16];
		size_t i = sizeof(magic_header);
		std::ifstream file(config.filepath, std::ios::binary);
		if (!file.is_open())
		{
			throw DMusicRuntimeError(
				u8"Create DMusic Failed - Failed to open file: " +
				config.filepath.u8string());
		}
		file.read(magic_header, sizeof(magic_header));
		file.close();
		if (is_ncm(magic_header))
		{
			return std::make_unique<NCM>();
		}
		else if (config.filepath.extension() == ".kgg")
		{ 
			 return std::make_unique<KGG>();
		}
		else if (is_kgm_kgm(magic_header))
		{
			 return std::make_unique<KGM_kgm>();
		}
		else if (is_kgm_vpr(magic_header))
		{
			return std::make_unique<KGM_vpr>();
		}
		{
			throw DMusicRuntimeError(
				u8"Create DMusic Failed - Unsupported file: " +
				config.filepath.u8string() +
				u8"\nmagic header: " +
				std::u8string((char8_t*)magic_header,sizeof(magic_header)));
		}
	}
private:
	//magic header
	static constexpr unsigned char _ncm_header[] = { 0x43, 0x54, 0x45, 0x4e, 0x46, 0x44, 0x41, 0x4d, 0x01 };
	static constexpr unsigned char _kgm_vpr_header[] = { 0x05, 0x28, 0xBC, 0x96, 0xE9, 0xE4, 0x5A, 0x43,0x91, 0xAA, 0xBD, 0xD0, 0x7A, 0xF5, 0x36, 0x31 };
	static constexpr unsigned char _kgm_kgm_header[] = { 0x7C, 0xD5, 0x32, 0xEB, 0x86, 0x02, 0x7F, 0x4B,0xA8, 0xAF, 0xA6, 0x8E, 0x0F, 0xFF, 0x99, 0x14 };
	//functions
	static bool is_ncm(char* _magic_header)
	{
		if (memcmp(_ncm_header, _magic_header, sizeof(_ncm_header)) == 0)
		{
			return true;
		}
		return false;
	}
	static bool is_kgm_kgm(char* _magic_header)
	{
		if (memcmp(_kgm_kgm_header, _magic_header, sizeof(_kgm_kgm_header)) == 0)
		{
			return true;
		}
		return false;
	}
	static bool is_kgm_vpr(char* _magic_header)
	{
		if (memcmp(_kgm_vpr_header, _magic_header, sizeof(_kgm_vpr_header)) == 0)
		{
			return true;
		}
		return false;
	}
};