#pragma once
#include <openssl/evp.h>
#include <vector>
#include <array>

class MD5
{
public:
    static std::vector<uint8_t> getHash(const std::vector<uint8_t>& data)
	{
        std::vector<uint8_t> hash(16,0);
        EVP_MD_CTX* md5_ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(md5_ctx, EVP_md5(), nullptr);
        EVP_DigestUpdate(md5_ctx, data.data(), data.size());
        EVP_DigestFinal_ex(md5_ctx, hash.data(), nullptr);
        EVP_MD_CTX_free(md5_ctx);
        return hash;
	}
    static void getHash(const std::vector<uint8_t>& data, unsigned char* hash)
    {
        EVP_MD_CTX* md5_ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(md5_ctx, EVP_md5(), nullptr);
        EVP_DigestUpdate(md5_ctx, data.data(), data.size());
        EVP_DigestFinal_ex(md5_ctx, hash, nullptr);
        EVP_MD_CTX_free(md5_ctx);
    }
};