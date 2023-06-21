#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

string aes_ecb_decrypt(const string& cipherText, const string& key)
{
	string plaintext;
	int len = cipherText.size();

	//初始化AES密钥和解密器
	AES_KEY aesKey;
	AES_set_decrypt_key((const unsigned char*)key.c_str(), 128, &aesKey);

	//分块解密
	for (int i = 0; i < len; i += AES_BLOCK_SIZE)
	{
		unsigned char out[AES_BLOCK_SIZE];
		AES_decrypt((const unsigned char*)&cipherText[i], out, &aesKey);
		plaintext.append((const char*)out, AES_BLOCK_SIZE);
	}

	//去除填充
	int padding = plaintext[plaintext.size() - 1];
	if (padding < 1 || padding > 128) { throw new exception("[AES]填充错误"); }

	bool validPadding = true;
	for (int i = 1; i <= padding; i++)
	{
		if (plaintext[plaintext.size() - i] != padding)
		{
			validPadding = false;
			break;
		}
	}

	if (validPadding)
	{
		plaintext.erase(plaintext.size() - padding);
	}
	else
	{
		throw new exception("[AES]填充错误");
	}
	return plaintext;
}

string base64_decode(const string& base64_str)
{
	BIO* bio, * b64;

	//计算解码后的数据长度
	int decode_len = base64_str.size() * 3 / 4;
	string buf(decode_len, '\0');

	//配置
	bio = BIO_new_mem_buf(base64_str.data(), -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	//解码
	decode_len = BIO_read(bio, buf.data(), base64_str.size());
	buf.resize(decode_len);

	BIO_free_all(bio);
	return buf;
}