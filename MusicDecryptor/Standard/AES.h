#pragma once
#include <openssl/evp.h>
#include <vector>

class AES
{
public:
	static std::vector<uint8_t> ecb_decrypt(const std::vector<uint8_t>& cipherText, const unsigned char* key)
	{
		std::vector<uint8_t> plaintext(cipherText.size() + EVP_MAX_BLOCK_LENGTH);
		int cipherText_len = (int)cipherText.size();

		// 初始化解密器
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);

		// 开始解密
		int plaintext_len1 = 0, plaintext_len2 = 0;
		EVP_DecryptUpdate(ctx, plaintext.data(), &plaintext_len1, cipherText.data(), cipherText_len);
		EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len1, &plaintext_len2);

		// 释放解密器
		EVP_CIPHER_CTX_free(ctx);

		// 去除填充
		plaintext.resize((size_t)plaintext_len1 + (size_t)plaintext_len2);

		// 去除填充
		//int padding = plaintext[plaintext.size() - 1];
		//if (padding < 1 || padding > 128) { throw runtime_error("AES paddings are invalid"); }

		//bool validPadding = true;
		//for (int i = 1; i <= padding; i++)
		//{
		//	if (plaintext[plaintext.size() - i] != padding)
		//	{
		//		validPadding = false;
		//		break;
		//	}
		//}

		//if (validPadding)
		//{
		//	plaintext.erase(plaintext.end() - padding, plaintext.end());
		//}
		//else
		//{
		//	throw runtime_error("AES paddings are invalid");
		//}
		return plaintext;
	}
	static std::vector<uint8_t> cbc_decrypt(const std::vector<uint8_t>& cipherText, const uint8_t* key, const uint8_t* iv)
	{
		std::vector<uint8_t> plaintext(cipherText.size() + EVP_MAX_BLOCK_LENGTH);

		// 初始化解密器
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv);
		EVP_CIPHER_CTX_set_padding(ctx, 0);

		// 开始解密
		int out_len1, out_len2;
		EVP_DecryptUpdate(ctx, plaintext.data(), &out_len1, cipherText.data(), (int)cipherText.size());
		EVP_DecryptFinal_ex(ctx, plaintext.data() + out_len1, &out_len2);

		// 释放解密器
		EVP_CIPHER_CTX_free(ctx);

		// 去除填充
		plaintext.resize((size_t)out_len1 + (size_t)out_len2);
		return plaintext;
	}
};