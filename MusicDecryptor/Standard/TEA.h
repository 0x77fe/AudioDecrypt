#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <span>

#include "Endian.h"

class TEA
{
public:
    static std::vector<uint8_t> tc_tea_cbc_decrypt(std::span<uint8_t> cipher, const uint32_t* key) {
        // It needs to have at least 2 blocks long, due to the nature of the padding
        // scheme used.
        if (cipher.size() % kTeaBlockSize != 0 || cipher.size() < kTeaBlockSize * 2) {
            return {};
        }

        uint64_t iv1 = 0;
        uint64_t iv2 = 0;
        uint8_t header[kTeaBlockSize * 2];
        const uint8_t* in_cipher = cipher.data();
        decrypt_round(header, in_cipher, &iv1, &iv2, key);
        in_cipher += kTeaBlockSize;
        decrypt_round(header + kTeaBlockSize, in_cipher, &iv1, &iv2, key);
        in_cipher += kTeaBlockSize;

        size_t hdr_skip_len = 1 + (header[0] & 7) + kFixedSaltLen;
        size_t real_plain_len = cipher.size() - hdr_skip_len - kZeroPadLen;
        std::vector<uint8_t> result(real_plain_len);

        auto p_output = result.data();

        // copy first block of plain text
        size_t copy_len = std::min(sizeof(header) - hdr_skip_len, real_plain_len);
        std::copy_n(header + hdr_skip_len, real_plain_len, p_output);
        p_output += copy_len;

        if (real_plain_len != copy_len) {
            // Decrypt the rest of the blocks
            for (size_t i = cipher.size() - kTeaBlockSize * 3; i != 0; i -= kTeaBlockSize) {
                decrypt_round(p_output, in_cipher, &iv1, &iv2, key);
                in_cipher += kTeaBlockSize;
                p_output += kTeaBlockSize;
            }

            decrypt_round(header + kTeaBlockSize, in_cipher, &iv1, &iv2, key);
            p_output[0] = header[kTeaBlockSize];
        }
        // Validate zero padding
        //auto verify = Endian::be_read<uint64_t>(header + kTeaBlockSize) << 8;
        //if (verify != 0) {
        //    result.resize(0);
        //}
        return result;
    }


    static inline uint64_t tc_tea_ecb_decrypt(uint64_t value, const uint32_t* key) {
        uint32_t y = (uint32_t)(value >> 32);
        uint32_t z = (uint32_t)(value);
        uint32_t sum = { TEA_EXPECTED_SUM };

        for (size_t i = 0; i < TEA_ROUNDS; i++) {
            z -= tc_tea_single_round(y, sum, key[2], key[3]);
            y -= tc_tea_single_round(z, sum, key[0], key[1]);
            sum -= TEA_ROUND_DELTA;
        }

        return ((uint64_t)(y) << 32) | (uint64_t)(z);
    }

    static inline uint64_t tc_tea_ecb_encrypt(uint64_t value, const uint32_t* key) {
        uint32_t y = (uint32_t)(value >> 32);
        uint32_t z = (uint32_t)(value);
        uint32_t sum = { 0 };

        for (size_t i = 0; i < TEA_ROUNDS; i++) {
            sum += TEA_ROUND_DELTA;
            y += tc_tea_single_round(z, sum, key[0], key[1]);
            z += tc_tea_single_round(y, sum, key[2], key[3]);
        }

        return ((uint64_t)(y) << 32) | (uint64_t)(z);
    }

private:
    constexpr static size_t TEA_ROUNDS = (16);
    constexpr static uint32_t TEA_ROUND_DELTA = (0x9e3779b9);
    constexpr static uint32_t TEA_EXPECTED_SUM = (static_cast<uint32_t>(TEA_ROUNDS * TEA_ROUND_DELTA));
    constexpr static size_t kTeaBlockSize = 8;
    constexpr static size_t kFixedSaltLen = 2;
    constexpr static size_t kZeroPadLen = 7;

    inline static void decrypt_round(uint8_t* p_plain,
        const uint8_t* p_cipher,
        uint64_t* iv1,
        uint64_t* iv2,
        const uint32_t* key) {
        uint64_t iv1_next = Endian::be_read<uint64_t>(p_cipher);
        uint64_t iv2_next = tc_tea_ecb_decrypt(iv1_next ^ *iv2, key);
        uint64_t plain = iv2_next ^ *iv1;
        *iv1 = iv1_next;
        *iv2 = iv2_next;
        Endian::be_write(p_plain, plain);
    }

    inline static uint32_t tc_tea_single_round(uint32_t value, uint32_t sum, uint32_t key1, uint32_t key2) {
        return ((value << 4) + key1) ^ (value + sum) ^ ((value >> 5) + key2);
    }
};