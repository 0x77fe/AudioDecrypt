#pragma once
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <vector>

class Base64
{
public:
	std::vector<uint8_t> decode(const std::vector<uint8_t>& base64_str)
	{
        std::vector<uint8_t> decoded(0);
        BIO* bio = nullptr;
        BIO* b64 = nullptr;
        b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_new_mem_buf(base64_str.data(), static_cast<int>(base64_str.size()));
        if (!bio) {
            BIO_free_all(b64);
            return decoded;
        }
        bio = BIO_push(b64, bio);

        // 读取
        uint8_t buffer[512];
        int bytes_read = 0;
        while ((bytes_read = BIO_read(bio, buffer, sizeof(buffer))))
        {
            if (bytes_read < 0) {
                decoded.clear();
                break;
            }
            decoded.insert(decoded.end(), buffer, buffer + bytes_read);
        }
        BIO_free_all(bio);

        return decoded;
	}
};


