#pragma once
#if _DEBUG
#include <qdebug.h>
#endif
//
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
//taglib
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
//openssl
#include <openssl/bio.h>
#include <openssl/evp.h>
//rapidjson
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>
//user
#include "./Standard/AES.h"
#include "./Standard/Base64.h"
#include "DMusic.h"

namespace fs = std::filesystem;

class NCM : public DMusic
{
public:
    void decrypt(DMusicIOConfig& config) override;
private:
    //file info
    fs::path _filepath;
    std::ifstream _file;
    fs::path _output_filepath;
    bool _enableCloudkey = true;
    //metainfo
    std::u8string _title;
    std::vector<std::u8string> _artists;
    std::u8string _file_format;
    std::u8string _album;
    std::u8string _cloudkey;
    std::vector<uint8_t> _cover;
    //keys
    static inline constexpr const char _meta_key[] = { 0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28 };//"#14ljk_!\]&0U<'("
    static inline constexpr const char _core_key[] = { 0x68,0x7A,0x48,0x52,0x41,0x6D,0x73,0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57 };//"hzHRAmso5kInbaxW"
    std::vector<uint8_t> _rc4key;
    //functions
    std::vector<uint8_t> readFile();
    void jumpHeader();
    void decryptRC4key();
    void decryptmMetaInfo();
    void jumpCRCcode();
    void getCover();
    fs::path genOutputfilepath(fs::path& output_dir);
    void writeMetaInfo();
};