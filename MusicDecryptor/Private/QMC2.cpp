#include "QMC2.h"

/*
* class Ekey
*/
std::vector<uint8_t> Ekey::decrypt_ekey_v1(std::string_view ekey)
{
    std::vector<uint8_t> result = Base64::decode(ekey);

    uint32_t tea_key[4] =
    {
        0x69005600 | static_cast<uint32_t>(result[0] << 16) | (result[1]),
        0x46003800 | static_cast<uint32_t>(result[2] << 16) | (result[3]),
        0x2b002000 | static_cast<uint32_t>(result[4] << 16) | (result[5]),
        0x15000b00 | static_cast<uint32_t>(result[6] << 16) | (result[7]),
    };
    auto decrypted = TEA::tc_tea_cbc_decrypt(std::span(result).subspan(8), tea_key);
    if (decrypted.empty())
    {
        return {};
    }
    result.resize(8);
    result.insert(result.end(), decrypted.begin(), decrypted.end());
    return result;
}

std::vector<uint8_t> Ekey::decrypt_ekey_v2(std::string_view ekey) 
{
    std::vector<uint8_t> result;
    result = TEA::tc_tea_cbc_decrypt(ss2span<uint8_t>(ekey), (uint32_t*)kEKeyV2Key1.data());
    result = TEA::tc_tea_cbc_decrypt(std::span(result), (uint32_t*)kEKeyV2Key2.data());
    return decrypt_ekey_v1(span2ss(std::span(result)));
}

std::vector<uint8_t> Ekey::decrypt_ekey(std::string_view ekey) 
{
    if (ekey.starts_with(kEKeyV2Prefix)) {
        ekey.remove_prefix(kEKeyV2Prefix.size());
        return decrypt_ekey_v2(ekey);
    }

    return decrypt_ekey_v1(ekey);
}

/*
* class QMC2
*/
template <typename T>
inline static std::span<T> Ekey::ss2span(std::string_view sv)
{
    auto* data = reinterpret_cast<const T*>(sv.data());
    return std::span<T>(const_cast<T*>(data), sv.size());
}

template<typename T>
std::string_view Ekey::span2ss(std::span<T> span)
{
    return std::string_view(reinterpret_cast<char*>(span.data()), span.size());
}

inline size_t QMC2_RC4::get_segment_key(double key_hash, size_t segment_id, uint8_t seed)
{
    if (seed == 0) 
    {
        return 0;
    }
    const double result = key_hash / static_cast<double>(seed * (segment_id + 1)) * 100.0;
    return static_cast<size_t>(result);
}

inline double QMC2_RC4::hash(const uint8_t* key, size_t len)
{
    uint32_t hash = { 1 };
    const uint8_t* end = key + len;
    for (; key < end; ++key) 
    {
        if (*key == 0) {
            continue;
        }

        // Overflow check.
        uint32_t next_hash = hash * static_cast<uint32_t>(*key);
        if (next_hash <= hash) 
        {
            break;
        }
        hash = next_hash;
    }
    return (double)hash;
}

QMC2_RC4::QMC2_RC4(std::span<uint8_t> key) {
    hash_ = hash(key.data(), key.size());
    key_ = std::vector(key.begin(), key.end());

    RC4 rc4(key.data(), key.size());
    rc4.Derive(std::span<uint8_t>(key_stream_));
}

void QMC2_RC4::Decrypt(std::span<uint8_t> data, size_t offset) const {
    if (offset < kFirstSegmentSize) 
    {
        const auto n = DecryptFirstSegment(data, offset);
        offset += n;
        data = data.subspan(n);
    }

    while (!data.empty()) 
    {
        const auto n = DecryptOtherSegment(data, offset);
        offset += n;
        data = data.subspan(n);
    }
}

size_t QMC2_RC4::DecryptFirstSegment(std::span<uint8_t> data, size_t offset) const {
    const uint8_t* key = key_.data();
    const size_t n = this->key_.size();

    size_t process_len = std::min(data.size(), kFirstSegmentSize - offset);
    for (auto& it : data.subspan(0, process_len)) 
    {
        const auto idx = get_segment_key(hash_, offset, key[offset % n]) % n;
        it ^= key[idx];
        offset++;
    }
    return process_len;
}

size_t QMC2_RC4::DecryptOtherSegment(std::span<uint8_t> data, size_t offset) const {
    const size_t n = this->key_.size();

    size_t segment_idx = offset / kOtherSegmentSize;
    size_t segment_offset = offset % kOtherSegmentSize;

    size_t skip_len = get_segment_key(hash_, segment_idx, key_[segment_idx % n]) & 0x1FF;
    size_t process_len = std::min(data.size(), kOtherSegmentSize - segment_offset);
    const uint8_t* rc4_stream = &key_stream_[skip_len + segment_offset];
    for (auto& it : data.subspan(0, process_len)) 
    {
        it ^= *rc4_stream++;
    }
    return process_len;
}

QMC2_MAP::QMC2_MAP(std::span<uint8_t> key) 
{
    auto n = key.size();
    for (size_t i = 0; i < kMapKeySize; i++) 
    {
        size_t j = (i * i + kMapIndexOffset) % n;
        const auto shift = (j + 4) % 8;
        key_[i] = (key[j] << shift) | (key[j] >> shift);
    }
}

void QMC2_MAP::Decrypt(std::span<uint8_t> data, size_t offset) const 
{
    for (auto& it : data) 
    {
        size_t idx = (offset <= kMapOffsetBoundary) ? offset : (offset % kMapOffsetBoundary);
        it ^= key_[idx % key_.size()];
        offset++;
    }
}

std::unique_ptr<QMC2_Base> QMC2::Create(std::string_view ekey)
{
    auto key = Ekey::decrypt_ekey(ekey);
    auto key_len = key.size();
    if (key_len == 0)
    {
        return nullptr;
    }

    if (key_len < 300)
    {
        return std::make_unique<QMC2_MAP>(std::span(key));
    }
    else
    {
        return std::make_unique<QMC2_RC4>(std::span(key));
    }
}