#include "StringBuilder.h"

#include <algorithm>


using namespace std;


size_t StringBuilderUtil::appendString(char * dest, size_t destSize,
        size_t leftPad, char const * str, size_t len) {
    size_t used = 0;

    if((used < destSize) && (len < leftPad)) {
        size_t padAmount = min(leftPad - len, destSize);
        memset(dest, ' ', padAmount);
        used += padAmount;
    }

    if(used < destSize) {
        size_t copyAmount = min(len, destSize - used);
        memcpy(dest + used, str, copyAmount);
        used += copyAmount;
    }

    return used;
}


size_t StringBuilderUtil::appendDataHex(char * dest, size_t destSize,
        size_t leftPad, void const * data, size_t len) {
    size_t used = 0;
    size_t formattedAmount = 0;

    if(len > 0) {
        formattedAmount = len * 3 - 1;
    }

    if((used < destSize) && (formattedAmount < leftPad)) {
        size_t padAmount = min(leftPad - formattedAmount, destSize);
        memset(dest, ' ', padAmount);
        used += padAmount;
    }

    if((used < destSize) && (len > 0)) {
        used += appendUnsignedHex(dest + used, destSize - used, 0,
            ((uint8_t const *)data)[0]);
        for(size_t i = 1; (used < destSize) && (i < len); ++i) {
            dest[used++] = ' ';
            used += appendUnsignedHex(dest + used, destSize - used, 0,
                ((uint8_t const *)data)[i]);
        }
    }

    return used;
}


size_t StringBuilderUtil::appendSignedDecimal(char * dest, size_t destSize,
        size_t leftPad, bool alwaysSign, int32_t val) {
    uint32_t unsignedVal;
    char signChar = 0;
    char buf[MAX_DECIMAL_CHARS_32];

    if(val < 0) {
        signChar = '-';
        // note that -(-0x80000000) -> -0x80000000, but this is okay exactly
        // because of this identity -- -0x80000000 == 0x80000000
        unsignedVal = (uint32_t)-val;
    } else {
        if(alwaysSign) {
            signChar = ((val == 0) ? ' ' : '+');
        }
        unsignedVal = val;
    }

    size_t bufUsed = getBackwardsDecimalChars(buf, unsignedVal);
    return appendBackwardsChars(dest, destSize, leftPad, signChar, buf, bufUsed);
}


size_t StringBuilderUtil::appendSignedHex(char * dest, size_t destSize,
        size_t leftPad, bool alwaysSign, int32_t val) {
    uint32_t unsignedVal;
    char signChar = 0;
    char buf[MAX_HEX_CHARS_32];

    if(val < 0) {
        signChar = '-';
        // note that -(-0x80000000) -> -0x80000000, but this is okay exactly
        // because of this identity -- -0x80000000 == 0x80000000
        unsignedVal = (uint32_t)-val;
    } else {
        if(alwaysSign) {
            signChar = ((val == 0) ? ' ' : '+');
        }
        unsignedVal = val;
    }

    size_t bufUsed = getBackwardsHexChars(buf, unsignedVal);
    return appendBackwardsChars(dest, destSize, leftPad, signChar, buf, bufUsed);
}


size_t StringBuilderUtil::appendUnsignedDecimal(char * dest, size_t destSize,
        size_t leftPad, uint32_t val) {
    char buf[MAX_DECIMAL_CHARS_32];

    size_t bufUsed = getBackwardsDecimalChars(buf, val);
    return appendBackwardsChars(dest, destSize, leftPad, 0, buf, bufUsed);
}


size_t StringBuilderUtil::appendUnsignedHex(char * dest, size_t destSize,
        size_t leftPad, uint32_t val) {
    char buf[MAX_HEX_CHARS_32];

    size_t bufUsed = getBackwardsHexChars(buf, val);
    return appendBackwardsChars(dest, destSize, leftPad, 0, buf, bufUsed);
}


size_t StringBuilderUtil::getBackwardsDecimalChars(char * dest,
        uint32_t val) {
    size_t used = 0;

    do {
        dest[used++] = '0' + (val % 10);
        val /= 10;
    } while(used < MAX_DECIMAL_CHARS_32 && val > 0);

    return used;
}


size_t StringBuilderUtil::getBackwardsHexChars(char * dest, uint32_t val) {
    static char const digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    size_t used = 0;

    do {
        dest[used++] = digits[val % 16];
        val /= 16;
    } while(used < MAX_HEX_CHARS_32 && val > 0);

    return used;
}


size_t StringBuilderUtil::appendBackwardsChars(char * dest, size_t destSize,
        size_t leftPad, char signChar, char const * buf, size_t bufAmount) {
    size_t used = 0;
    size_t formattedAmount = bufAmount;

    if(signChar != 0) {
        ++formattedAmount;
    }

    if((used < destSize) && (formattedAmount < leftPad)) {
        size_t padAmount = min(leftPad - formattedAmount, destSize);
        memset(dest, ' ', padAmount);
        used += padAmount;
    }

    if((used < destSize) && (signChar != 0)) {
        dest[used++] = signChar;
    }

    if((used < destSize) && (bufAmount > 0)) {
        for(size_t i = bufAmount; i > 0; --i) {
            dest[used++] = buf[i - 1];
        }
    }

    return used;
}
