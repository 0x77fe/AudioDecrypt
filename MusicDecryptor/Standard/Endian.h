
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(_MSC_VER)
#define bswap_u16 _byteswap_ushort
#define bswap_u32 _byteswap_ulong
#define bswap_u64 _byteswap_uint64
#else
#define bswap_u16 __builtin_bswap16
#define bswap_u32 __builtin_bswap32
#define bswap_u64 __builtin_bswap64
#endif

class Endian 
{
    public:
    template <typename T>
    static T be_read(const uint8_t* p)
        requires(std::is_integral_v<T>)
    {
        // ReSharper disable once CppDFAUnreachableCode
        if constexpr (std::endian::native == std::endian::big) {
            return *reinterpret_cast<const T*>(p);
        }
        else if constexpr (sizeof(T) == 2) {
            return bswap_u16(*reinterpret_cast<const T*>(p));
        }
        else if constexpr (sizeof(T) == 4) {
            return bswap_u32(*reinterpret_cast<const T*>(p));
        }
        else if constexpr (sizeof(T) == 8) {
            return bswap_u64(*reinterpret_cast<const T*>(p));
        }
        else {
            T result{};
            for (size_t i = 0; i < sizeof(T); i++) {
                reinterpret_cast<uint8_t*>(&result)[i] = p[sizeof(T) - i - 1];
            }
            return result;
        }
    }

    template <typename T>
    static void be_write(uint8_t* p, const T value)
        requires(std::is_integral_v<T>)
    {
        // ReSharper disable once CppDFAUnreachableCode
        if constexpr (std::endian::native == std::endian::big) {
            *reinterpret_cast<T*>(p) = value;
        }
        else if constexpr (sizeof(T) == 2) {
            *reinterpret_cast<T*>(p) = bswap_u16(value);
        }
        else if constexpr (sizeof(T) == 4) {
            *reinterpret_cast<T*>(p) = bswap_u32(value);
        }
        else if constexpr (sizeof(T) == 8) {
            *reinterpret_cast<T*>(p) = bswap_u64(value);
        }
        else {
            for (size_t i = 0; i < sizeof(T); i++) {
                p[sizeof(T) - i - 1] = reinterpret_cast<const uint8_t*>(&value)[i];
            }
        }
    }

    template <typename T>
    static T le_read(const uint8_t* p)
        requires(std::is_integral_v<T>)
    {
        // ReSharper disable once CppDFAUnreachableCode
        if constexpr (std::endian::native == std::endian::little) {
            return *reinterpret_cast<const T*>(p);
        }
        else if constexpr (sizeof(T) == 2) {
            return bswap_u16(*reinterpret_cast<const T*>(p));
        }
        else if constexpr (sizeof(T) == 4) {
            return bswap_u32(*reinterpret_cast<const T*>(p));
        }
        else if constexpr (sizeof(T) == 8) {
            return bswap_u64(*reinterpret_cast<const T*>(p));
        }
        else {
            T result{};
            for (size_t i = 0; i < sizeof(T); i++) {
                reinterpret_cast<uint8_t*>(&result)[i] = p[i];
            }
            return result;
        }
    }

    template <typename T>
    static void le_write(uint8_t* p, const T value)
        requires(std::is_integral_v<T>)
    {
        // ReSharper disable once CppDFAUnreachableCode
        if constexpr (std::endian::native == std::endian::little) {
            *reinterpret_cast<T*>(p) = value;
        }
        else if constexpr (sizeof(T) == 2) {
            *reinterpret_cast<T*>(p) = bswap_u16(value);
        }
        else if constexpr (sizeof(T) == 4) {
            *reinterpret_cast<T*>(p) = bswap_u32(value);
        }
        else if constexpr (sizeof(T) == 8) {
            *reinterpret_cast<T*>(p) = bswap_u64(value);
        }
        else {
            for (size_t i = 0; i < sizeof(T); i++) {
                p[i] = reinterpret_cast<const uint8_t*>(&value)[i];
            }
        }
    }
};