#pragma once
#if _DEBUG
#include <qdebug.h>
#endif
#include <vector>
#include <filesystem>
#include <fstream>
#include <map>
//taglib
#include <taglib/fileref.h>
#include <taglib/tag.h>
//
#include <sqlite3.h>
//
#include "DMusic.h"
#include "QMC2.h"
#include ".\Standard\AES.h"
#include ".\Standard\MD5.h"


class KGDatabase
{
private:
	constexpr static char _kg_sqlite_database_header[] = { "SQLite format 3" };
	constexpr static unsigned char _kg_default_master_key[] = { 0x1d, 0x61, 0x31, 0x45, 0xb2, 0x47, 0xbf, 0x7f, 0x3d, 0x18, 0x96, 0x72, 0x14, 0x4f, 0xe4, 0xbf };

	static const size_t _kg_page_size = 1024;
	bool isValidPage1Header(const uint8_t* page1);
	void derivePageKey(unsigned char* aes_key, unsigned char* aes_iv, const uint8_t* p_master_key, uint32_t page_no);
	std::vector<uint8_t> _db;
	KeyMap _key_map;
public:
	void loadDecryptedDatabase(const fs::path& db_path);
	static KeyMap getKeyMap(const fs::path& keyfile_path); // 直接从文件中读取
	KeyMap getKeyMap(); // 解密数据库后读取
};

namespace fs = std::filesystem;

class KGG : public DMusic
{
private:
	fs::path _filepath;
	std::ifstream _file;
	std::ofstream _output_file;
	fs::path _output_filepath;
	int32_t _header_len = 0;
	std::string_view _key;
	KeyMap _key_map;
	std::unique_ptr<QMC2_Base> _qmc2_decryptor;
	enum{FLAC,MP3,OGG} _musictype = MP3;
	//
	void getHeaderLen();
	void checkSupported();
	void findKey();
	void createDecryptor();
	void judgeMusicType();
	void createTempOutputFile(fs::path& output_dir);
	void renameFile();
public:
	void decrypt(DMusicIOConfig& config) override;
};