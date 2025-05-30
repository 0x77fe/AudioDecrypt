#pragma once
// std
#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>
#include <string>

#include ".\Standard\RC4.h"
#include ".\Standard\Base64.h"
#include ".\Standard\TEA.h"

class Ekey
{
private:
    constexpr static std::string_view kEKeyV2Prefix = "UVFNdXNpYyBFbmNWMixLZXk6";
    constexpr static std::array<uint8_t, 16> kEKeyV2Key1{ 0x33, 0x38, 0x36, 0x5A, 0x4A, 0x59, 0x21, 0x40, 0x23, 0x2A, 0x24, 0x25, 0x5E, 0x26, 0x29, 0x28, };
    constexpr static std::array<uint8_t, 16> kEKeyV2Key2{ 0x2A, 0x2A, 0x23, 0x21, 0x28, 0x23, 0x24, 0x25, 0x26, 0x5E, 0x61, 0x31, 0x63, 0x5A, 0x2C, 0x54, };
    //
    template <typename T>
    inline static std::span<T> ss2span(std::string_view sv);
    template <typename T>
    inline static std::string_view span2ss(std::span<T> span);
    static std::vector<uint8_t> decrypt_ekey_v1(std::string_view ekey);
    static std::vector<uint8_t> decrypt_ekey_v2(std::string_view ekey);

public:
    static std::vector<uint8_t> decrypt_ekey(std::string_view ekey);
};

class QMC2_Base
{
public:
    QMC2_Base() = default;
    virtual ~QMC2_Base() = default;
    virtual void Decrypt(std::span<uint8_t> data, size_t offset) const = 0;
};

class QMC2_MAP : public QMC2_Base 
{
public:
    explicit QMC2_MAP(std::span<uint8_t> key);

    void Decrypt(std::span<uint8_t> data, size_t offset) const override;

private:
    static constexpr size_t kMapOffsetBoundary = 0x7FFF;
    static constexpr size_t kMapIndexOffset = 71214;
    static constexpr size_t kMapKeySize = 128;
    std::array<uint8_t, kMapKeySize> key_{};
};

class QMC2_RC4 : public QMC2_Base 
{
public:
    explicit QMC2_RC4(std::span<uint8_t> key);

    void Decrypt(std::span<uint8_t> data, size_t offset) const override;

private:
    inline static size_t get_segment_key(double key_hash, size_t segment_id, uint8_t seed);
    inline static double hash(const uint8_t* key, size_t len);
    static constexpr size_t kFirstSegmentSize = 0x0080;
    static constexpr size_t kOtherSegmentSize = 0x1400;
    static constexpr size_t kRC4StreamSize = kOtherSegmentSize + 512;
    std::vector<uint8_t> key_{};
    double hash_{ 0 };
    std::array<uint8_t, kRC4StreamSize> key_stream_{};

    [[nodiscard]] size_t DecryptFirstSegment(std::span<uint8_t> data, size_t offset) const;
    [[nodiscard]] size_t DecryptOtherSegment(std::span<uint8_t> data, size_t offset) const;
};

class QMC2
{
public:
	static std::unique_ptr<QMC2_Base> Create(std::string_view ekey);
};