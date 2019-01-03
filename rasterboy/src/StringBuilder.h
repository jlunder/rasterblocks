#ifndef STRINGBUILDER_INCLUDED
#define STRINGBUILDER_INCLUDED

#include <cstdint>
#include <cstring>


class StringBuilderUtil
{
public:
    static std::size_t appendString(char * dest, std::size_t destSize,
        std::size_t leftPad, char const * str, std::size_t len);
    //static std::size_t appendDataAscii(char * dest, std::size_t destSize,
    //    std::size_t leftPad, char const * str, std::size_t len);
    static std::size_t appendDataHex(char * dest, std::size_t destSize,
        std::size_t leftPad, void const * data, std::size_t len);
    static std::size_t appendSignedDecimal(char * dest, size_t destSize,
        std::size_t leftPad, bool alwaysSign, std::int32_t val);
    static std::size_t appendSignedHex(char * dest, size_t destSize,
        std::size_t leftPad, bool alwaysSign, std::int32_t val);
    //static std::size_t appendSignedDecimal(char * dest,
    //    std::size_t destSize, std::size_t leftPad, std::int64_t val);
    //static std::size_t appendSignedHex(char * dest, std::size_t destSize,
    //    std::size_t leftPad, std::int64_t val);
    static std::size_t appendUnsignedDecimal(char * dest,
        std::size_t destSize, std::size_t leftPad, std::uint32_t val);
    static std::size_t appendUnsignedHex(char * dest, std::size_t destSize,
        std::size_t leftPad, std::uint32_t val);
    //static std::size_t appendUnsignedDecimal(char * dest,
    //    std::size_t destSize, std::size_t leftPad, std::uint64_t val);
    //static std::size_t appendUnsignedHex(char * dest, std::size_t destSize,
    //    std::size_t leftPad, std::uint64_t val);

private:
    static std::size_t const MAX_DECIMAL_CHARS_32 = 10;
    static std::size_t const MAX_HEX_CHARS_32 = 8;

    static std::size_t getBackwardsDecimalChars(char * dest,
        std::uint32_t val);
    static std::size_t getBackwardsHexChars(char * dest, std::uint32_t val);
    static std::size_t appendBackwardsChars(char * dest, std::size_t destSize,
        std::size_t leftPad, char signChar, char const * buf,
        std::size_t bufAmount);
};

template<std::size_t Length>
class StringBuilder
{
public:
    StringBuilder() {
        reset();
    }

    void reset() {
        _used = 0;
    }

    void append(char const * str) {
        _used += StringBuilderUtil::appendString(_buf + _used,
            Length - _used, 0, str, std::strlen(str));
    }

    void appendS(std::int32_t val) {
        _used += StringBuilderUtil::appendSignedDecimal(_buf + _used,
            Length - _used, 0, val);
    }

    void appendU(std::uint32_t val) {
        _used += StringBuilderUtil::appendUnsignedDecimal(_buf + _used,
            Length - _used, 0, val);
    }

    void appendSHex(std::int32_t val) {
        _used += StringBuilderUtil::appendSignedHex(_buf + _used,
            Length - _used, 0, val);
    }

    void appendUHex(std::uint32_t val) {
        _used += StringBuilderUtil::appendUnsignedHex(_buf + _used,
            Length - _used, 0, val);
    }

    void appendDataHex(void const * data, std::size_t size) {
        _used += StringBuilderUtil::appendDataHex(_buf + _used,
            Length - _used, 0, data, size);
    }

    char const * str() { _buf[_used] = 0; return _buf; }

private:
    std::size_t _used;
    char _buf[Length + 1];
};


#endif
