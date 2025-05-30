#pragma once

#include <vector>
#include <cstdint>
#include <span>
class RC4
{
public:
    inline RC4(const uint8_t* key, size_t key_len) {
        state_.resize(key_len);
        for (size_t i = 0; i < key_len; i++) {
            state_[i] = static_cast<uint8_t>(i);
        }

        for (size_t i = 0, j = 0; i < key_len; i++) {
            j = (j + state_[i] + key[i]) % key_len;
            std::swap(state_[i], state_[j]);
        }

        state_len_ = key_len;
    }

    inline void Derive(std::span<uint8_t> buffer) {
        size_t i = i_;
        size_t j = j_;
        uint8_t* s = state_.data();
        const size_t n = state_len_;

        for (auto& it : buffer) {
            i = (i + 1) % n;
            j = (j + s[i]) % n;
            std::swap(s[i], s[j]);

            const size_t final_idx = (s[i] + s[j]) % n;
            it ^= s[final_idx];
        }

        i_ = i;
        j_ = j;
    }

private:
    std::vector<uint8_t> state_{};
    size_t state_len_{ 0 };
    size_t i_{ 0 };
    size_t j_{ 0 };
};

