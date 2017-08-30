#include "graphics_util.h"

#include "font_debug_4x6.h"


#define RB_MAX_REASONABLE_SIZE 10000


uint8_t const g_rbCieTable[256] = {
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 
    3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 
    5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 
    7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 
    10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 
    13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 
    17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 
    22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 
    28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 
    34, 34, 35, 36, 37, 37, 38, 39, 39, 40, 
    41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 
    49, 50, 51, 52, 53, 54, 54, 55, 56, 57, 
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 
    68, 70, 71, 72, 73, 74, 75, 76, 77, 79, 
    80, 81, 82, 83, 85, 86, 87, 88, 90, 91, 
    92, 94, 95, 96, 98, 99, 100, 102, 103, 105, 
    106, 108, 109, 110, 112, 113, 115, 116, 118, 120, 
    121, 123, 124, 126, 128, 129, 131, 132, 134, 136, 
    138, 139, 141, 143, 145, 146, 148, 150, 152, 154, 
    155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 
    175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 
    196, 198, 200, 202, 204, 207, 209, 211, 214, 216, 
    218, 220, 223, 225, 228, 230, 232, 235, 237, 240, 
    242, 245, 247, 250, 252, 255, 
};


#define RB_ASSERT_TEXTURE1_VALID(pTex) \
    do { \
        rbAssert(pTex->width > 0); \
        rbAssert(pTex->size >= pTex->width); \
    } while(false);


#define RB_ASSERT_TEXTURE2_VALID(pTex) \
    do { \
        rbAssert(pTex->width > 0); \
        rbAssert(pTex->height > 0); \
        rbAssert(pTex->stride >= pTex->width); \
        rbAssert(pTex->size >= pTex->stride * pTex->height); \
    } while(false);


static void rbClipRect(RBTexture2 * pDestTex, RBRect2I * pRect,
    RBCoord2I * pOffset);


static void rbClipRect(RBTexture2 * pDestTex, RBRect2I * pRect,
    RBCoord2I * pOffset)
{
    int32_t const tw = t2getw(pDestTex);
    int32_t const th = t2geth(pDestTex);
    int32_t ox = 0;
    int32_t oy = 0;
    
    // The double negation works for 0x80000000, naive thing doesn't
    rbAssert(-abs(pRect->x) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(pRect->y) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(pRect->w) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(pRect->h) > -RB_MAX_REASONABLE_SIZE);
    
    if(pRect->x < 0) {
        ox = -pRect->x;
        pRect->x = 0;
        pRect->w -= ox;
    }
    if(pRect->y < 0) {
        oy = -pRect->y;
        pRect->y = 0;
        pRect->h -= oy;
    }
    
    if(pRect->x + pRect->w > tw) {
        pRect->w = tw - pRect->x;
    }
    if(pRect->y + pRect->h > th) {
        pRect->h = th - pRect->y;
    }
    
    if(pRect->w < 0) {
        pRect->w = 0;
    }
    if(pRect->h < 0) {
        pRect->h = 0;
    }
    
    if(pOffset != NULL) {
        pOffset->x = ox;
        pOffset->y = oy;
    }
}


RBVector2 rbVector2Normalize(RBVector2 a)
{
    float l = rbVector2Length(a);
    if(l != 0.0f) {
        RBVector2 res = {a.x / l, a.y / l};
        return res;
    }
    else {
        // Putting this in the "else" case should catch NaN
        RBVector2 res = {1.0f, 0.0f};
        return res;
    }
}


RBVector4 rbVector4Normalize(RBVector4 a)
{
    float l = rbVector4Length(a);
    if(l != 0.0f) {
        RBVector4 res = {a.x / l, a.y / l, a.z / l, a.w / l};
        return res;
    }
    else {
        // Putting this in the "else" case should catch NaN
        RBVector4 res = {1.0f, 0.0f, 0.0f, 0.0f};
        return res;
    }
}


RBMatrix2 rbMatrix2Identity(void)
{
    RBMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    return res;
}


RBMatrix2 rbMatrix2Scale(RBVector2 s)
{
    RBMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(s);
    return res;
}


RBMatrix2 rbMatrix2Rotate(float r)
{
    RBMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(r);
    return res;
}


RBMatrix2 rbMatrix2RotateScale(RBVector2 rs)
{
    RBMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(rs);
    return res;
}


RBMatrix2 rbMatrix2Multiply(RBMatrix2 a, RBMatrix2 b)
{
    RBMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(a);
    UNUSED(b);
    return res;
}


RBVector2 rbMatrix2Transform(RBMatrix2 m, RBVector2 v)
{
    UNUSED(m);
    return v;
}


RBColor rbColorMakeF(float r, float g, float b, float a)
{
    RBColor res = {
        (uint8_t)rbClampF(r * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(g * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(b * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(a * 256.0f, 0.0f, 255.0f),
    };
    
    return res;
}


RBColor rbColorMakeCT(RBColorTemp ct)
{
    RBColor res = {
        (uint8_t)rbClampF(ct.x * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(ct.y * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(ct.z * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(ct.w * 256.0f, 0.0f, 255.0f),
    };
    
    return res;
}


RBColor rbColorScaleI(RBColor a, uint8_t alpha)
{
    uint32_t rr = (uint32_t)a.r * (uint32_t)alpha + 127;
    uint32_t rg = (uint32_t)a.g * (uint32_t)alpha + 127;
    uint32_t rb = (uint32_t)a.b * (uint32_t)alpha + 127;
    uint32_t ra = (uint32_t)a.a * (uint32_t)alpha + 127;
    
    return rbColorMakeI(
        (uint8_t)(rr >= (255 * 255) ? 255 : rr / 255),
        (uint8_t)(rg >= (255 * 255) ? 255 : rg / 255),
        (uint8_t)(rb >= (255 * 255) ? 255 : rb / 255),
        (uint8_t)(ra >= (255 * 255) ? 255 : ra / 255));
}


RBColor rbColorScaleF(RBColor a, float alpha)
{
    return rbColorMakeI(
        (uint8_t)rbClampF(roundf(a.r * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.g * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.b * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.a * alpha), 0.0f, 255.0f));
}


RBColor rbColorAdd(RBColor a, RBColor b)
{
    uint32_t rr = (uint32_t)a.r + (uint32_t)b.r;
    uint32_t rg = (uint32_t)a.g + (uint32_t)b.g;
    uint32_t rb = (uint32_t)a.b + (uint32_t)b.b;
    uint32_t ra = (uint32_t)a.a + (uint32_t)b.a;
    
    return rbColorMakeI(
        (uint8_t)(rr >= 255 ? 255 : rr),
        (uint8_t)(rg >= 255 ? 255 : rg),
        (uint8_t)(rb >= 255 ? 255 : rb),
        (uint8_t)(ra >= 255 ? 255 : ra));
}


RBColor rbColorMul(RBColor a, RBColor b)
{
    uint32_t rr = ((uint32_t)a.r * (uint32_t)b.r + 127) / 255;
    uint32_t rg = ((uint32_t)a.g * (uint32_t)b.g + 127) / 255;
    uint32_t rb = ((uint32_t)a.b * (uint32_t)b.b + 127) / 255;
    uint32_t ra = ((uint32_t)a.a * (uint32_t)b.a + 127) / 255;
    
    return rbColorMakeI(
        (uint8_t)(rr >= 255 ? 255 : rr),
        (uint8_t)(rg >= 255 ? 255 : rg),
        (uint8_t)(rb >= 255 ? 255 : rb),
        (uint8_t)(ra >= 255 ? 255 : ra));
}


RBColor rbColorMixI(RBColor a, uint8_t aAlpha, RBColor b, uint8_t bAlpha)
{
    uint32_t rr = (uint32_t)a.r * aAlpha +  (uint32_t)b.r * bAlpha + 127;
    uint32_t rg = (uint32_t)a.g * aAlpha +  (uint32_t)b.g * bAlpha + 127;
    uint32_t rb = (uint32_t)a.b * aAlpha +  (uint32_t)b.b * bAlpha + 127;
    uint32_t ra = (uint32_t)a.a * aAlpha +  (uint32_t)b.a * bAlpha + 127;
    
    return rbColorMakeI(
        (uint8_t)(rr >= (255 * 255) ? 255 : rr / 255),
        (uint8_t)(rg >= (255 * 255) ? 255 : rg / 255),
        (uint8_t)(rb >= (255 * 255) ? 255 : rb / 255),
        (uint8_t)(ra >= (255 * 255) ? 255 : ra / 255));
}


RBColor rbColorMixF(RBColor a, float aAlpha, RBColor b, float bAlpha)
{
    return rbColorMakeI(
        (uint8_t)rbClampF(roundf(a.r * aAlpha + b.r * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.g * aAlpha + b.g * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.b * aAlpha + b.b * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.a * aAlpha + b.a * bAlpha), 0.0f, 255.0f));
}


RBTexture1 * rbTexture1Alloc(size_t width)
{
    size_t size = rbTexture1ComputeSize(width);
    RBTexture1 * pTex = malloc(size);
    
    rbTexture1Construct(pTex, width);
    
    return pTex;
}


void rbTexture1Construct(RBTexture1 * pTex, size_t width)
{
    rbAssert(width <= RB_MAX_REASONABLE_SIZE);
    rbAssert(width > 0);
    
    pTex->width = width;
    pTex->size = width;
    rbZero(pTex->data, pTex->size * sizeof (RBColor));
}


void rbTexture1Free(RBTexture1 * pTex)
{
    RB_ASSERT_TEXTURE1_VALID(pTex);
    
    pTex->width = 0;
    pTex->size = 0;
    free(pTex);
}


RBColorTemp rbTexture1SampleNearestRepeat(RBTexture1 const * pTex, float tc)
{
    size_t sampleIndex = (size_t)(pTex->width * (tc - floorf(tc)));
    
    RB_ASSERT_TEXTURE1_VALID(pTex);
    rbAssert(!isnan(tc));
    rbAssert(sampleIndex < pTex->width);
    
    return rbColorTempMakeC(pTex->data[sampleIndex]);
}


RBColorTemp rbTexture1SampleNearestClamp(RBTexture1 const * pTex, float tc)
{
    size_t sampleIndex;
    
    RB_ASSERT_TEXTURE1_VALID(pTex);
    rbAssert(!isnan(tc));
    
    if(tc >= 0.0f) {
        if(tc < 1.0f) {
            sampleIndex = (size_t)(pTex->width * tc);
            
            rbAssert(sampleIndex < pTex->width);
        }
        else {
            sampleIndex = pTex->width - 1;
        }
    }
    else {
        sampleIndex = 0;
    }
    
    return rbColorTempMakeC(pTex->data[sampleIndex]);
}


RBColorTemp rbTexture1SampleLinearRepeat(RBTexture1 const * pTex, float tc)
{
    float sampleIndexF = pTex->width * (tc - floorf(tc));
    size_t sampleIndex0 = (size_t)sampleIndexF;
    size_t sampleIndex1 = sampleIndex0 + 1;
    float alpha = sampleIndexF - floorf(sampleIndexF);
    
    RB_ASSERT_TEXTURE1_VALID(pTex);
    rbAssert(!isnan(tc));
    rbAssert(sampleIndex0 < pTex->width);
    
    if(sampleIndex1 >= pTex->width) {
        sampleIndex1 -= pTex->width;
    }
    
    
    return rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndex0]),
        1.0f - alpha, rbColorTempMakeC(pTex->data[sampleIndex1]), alpha);
}


RBColorTemp rbTexture1SampleLinearClamp(RBTexture1 const * pTex, float tc)
{
    RB_ASSERT_TEXTURE1_VALID(pTex);
    rbAssert(!isnan(tc));
    
    if(tc > 0) {
        if(tc < 1.0f) {
            float sampleIndexF = (pTex->width - 1) * (tc - floorf(tc));
            size_t sampleIndex0 = (size_t)sampleIndexF;
            size_t sampleIndex1 = sampleIndex0 + 1;
            float alpha = sampleIndexF - floorf(sampleIndexF);
            
            rbAssert(sampleIndex0 < pTex->width);
            
            if(sampleIndex1 >= pTex->width) {
                sampleIndex1 = pTex->width - 1;
            }
            
            return rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndex0]),
                1.0f - alpha, rbColorTempMakeC(pTex->data[sampleIndex1]),
                alpha);
        }
        else {
            return rbColorTempMakeC(pTex->data[pTex->width - 1]);
        }
    }
    else {
        return rbColorTempMakeC(pTex->data[0]);
    }
}


void rbTexture1Clear(RBTexture1 * pDestTex, RBColor clearColor)
{
    size_t const size = pDestTex->size;
    
    RB_ASSERT_TEXTURE1_VALID(pDestTex);
    
    for(size_t i = 0; i < size; ++i) {
        pDestTex->data[i] = clearColor;
    }
}


void rbTexture1FillFromPiecewiseLinear(RBTexture1 * pTex,
    RBPiecewiseLinearColorSegment * pSegments, size_t count, bool repeat)
{
    size_t width = pTex->width;
    size_t divWidth = repeat ? width : width - 1;
    size_t totalLength = 0;
    size_t seg;
    size_t rem;
    RBColorTemp ca;
    RBColorTemp cb;
    
    RB_ASSERT_TEXTURE1_VALID(pTex);
    rbAssert(count > 0);
    
    for(size_t i = 0; i < count; ++i) {
        totalLength += pSegments[i].length;
        rbAssert(totalLength <= RB_MAX_REASONABLE_SIZE);
    }
    
    seg = pSegments[0].length * divWidth;
    rem = 0;
    ca = rbColorTempMakeC(pSegments[0].color);
    cb = rbColorTempMakeC(pSegments[1].color);
    
    for(size_t i = 0, j = 0; i < width; ++i) {
        float alpha;
        
        while(rem > seg) {
            rem -= seg;
            ++j;
            rbAssert(j < count);
            ca = cb;
            cb = rbColorTempMakeC(pSegments[j + 1].color);
            seg = pSegments[j].length * divWidth;
        }
        
        alpha = (float)rem / (float)seg;
        pTex->data[i] =
            rbColorMakeCT(rbColorTempMix(ca, 1.0f - alpha, cb, alpha));
        rem += totalLength;
    }
}


RBTexture2 * rbTexture2Alloc(size_t width, size_t height)
{
    size_t size = rbTexture2ComputeSize(width, height);
    RBTexture2 * pTex = malloc(size);
    
    rbTexture2Construct(pTex, width, height);
    
    return pTex;
}


void rbTexture2Construct(RBTexture2 * pTex, size_t width, size_t height)
{
    size_t stride = rbTexture2ComputeStride(width, height);
    
    rbAssert(width <= RB_MAX_REASONABLE_SIZE);
    rbAssert(width > 0);
    rbAssert(height <= RB_MAX_REASONABLE_SIZE);
    rbAssert(height > 0);
    
    pTex->width = width;
    pTex->height = height;
    pTex->stride = stride;
    pTex->size = stride * height;
    rbZero(pTex->data, pTex->size * sizeof (RBColor));
}


void rbTexture2Free(RBTexture2 * pTex)
{
    RB_ASSERT_TEXTURE2_VALID(pTex);
    
    pTex->width = 0;
    pTex->size = 0;
    free(pTex);
}


RBColorTemp rbTexture2SampleNearestRepeat(RBTexture2 const * pTex,
    RBVector2 tc)
{
    size_t sampleIndexU = (size_t)(pTex->width * (tc.x - floorf(tc.x)));
    size_t sampleIndexV = (size_t)(pTex->height * (tc.y - floorf(tc.y)));
    
    RB_ASSERT_TEXTURE2_VALID(pTex);
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    rbAssert(sampleIndexU < pTex->width);
    rbAssert(sampleIndexV < pTex->height);
    
    return rbColorTempMakeC(pTex->data[sampleIndexU +
        sampleIndexV * pTex->stride]);
}


RBColorTemp rbTexture2SampleNearestClamp(RBTexture2 const * pTex, RBVector2 tc)
{
    size_t sampleIndexU;
    size_t sampleIndexV;
    
    RB_ASSERT_TEXTURE2_VALID(pTex);
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    
    if(tc.x > 0) {
        if(tc.x < 1.0f) {
            sampleIndexU = (size_t)(pTex->width * tc.x);
        }
        else {
            sampleIndexU = pTex->width - 1;
        }
    }
    else {
        sampleIndexU = 0;
    }
    
    if(tc.y > 0) {
        if(tc.y < 1.0f) {
            sampleIndexV = (size_t)(pTex->height * tc.y);
        }
        else {
            sampleIndexV = pTex->height - 1;
        }
    }
    else {
        sampleIndexV = 0;
    }
    
    rbAssert(sampleIndexU < pTex->width);
    rbAssert(sampleIndexV < pTex->height);
    
    return rbColorTempMakeC(pTex->data[sampleIndexU +
        sampleIndexV * pTex->stride]);
}


RBColorTemp rbTexture2SampleLinearRepeat(RBTexture2 const * pTex, RBVector2 tc)
{
    size_t const width = pTex->width;
    size_t const stride = pTex->stride;
    size_t const height = pTex->height;
    float sampleIndexUF = width * (tc.x - floorf(tc.x));
    float sampleIndexVF = height * (tc.y - floorf(tc.y));
    size_t sampleIndexU0 = (size_t)sampleIndexUF;
    size_t sampleIndexU1 = (size_t)sampleIndexUF + 1;
    size_t sampleIndexV0 = (size_t)sampleIndexVF;
    size_t sampleIndexV1 = (size_t)sampleIndexVF + 1;
    float alphaU = sampleIndexUF - floorf(sampleIndexUF);
    float alphaV = sampleIndexVF - floorf(sampleIndexVF);
    
    RB_ASSERT_TEXTURE2_VALID(pTex);
    
    if(sampleIndexU1 >= width) {
        sampleIndexU1 = 0;
    }
    if(sampleIndexV1 >= height) {
        sampleIndexV1 = 0;
    }
    
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    rbAssert(sampleIndexU0 < width);
    rbAssert(sampleIndexV0 < height);
    
    return rbColorTempMix(
        rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndexU0 +
                stride * sampleIndexV0]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[sampleIndexU1 +
                stride * sampleIndexV0]), alphaU),
        1.0f - alphaV,
        rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndexU0 +
                stride * sampleIndexV1]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[sampleIndexU1 +
                stride * sampleIndexV1]), alphaU),
        alphaV);
}


RBColorTemp rbTexture2SampleLinearClamp(RBTexture2 const * pTex, RBVector2 tc)
{
    size_t const width = pTex->width;
    size_t const stride = pTex->stride;
    size_t const height = pTex->height;
    float sampleIndexUF;
    float sampleIndexVF;
    size_t sampleIndexU0;
    size_t sampleIndexU1;
    size_t sampleIndexV0;
    size_t sampleIndexV1;
    float alphaU;
    float alphaV;
    
    RB_ASSERT_TEXTURE2_VALID(pTex);
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    
    if(tc.x > 0) {
        if(tc.x < 1.0f) {
            sampleIndexUF = (width - 1) * tc.x;
        }
        else {
            sampleIndexUF = width - 1;
        }
    }
    else {
        sampleIndexUF = 0;
    }
    
    if(tc.y > 0) {
        if(tc.y < 1.0f) {
            sampleIndexVF = (height - 1) * tc.y;
        }
        else {
            sampleIndexVF = height - 1;
        }
    }
    else {
        sampleIndexVF = 0;
    }
    
    sampleIndexU0 = (size_t)sampleIndexUF;
    sampleIndexU1 = sampleIndexU0 + 1;
    alphaU = sampleIndexUF - floorf(sampleIndexUF);
    if(sampleIndexU1 >= width) {
        sampleIndexU1 = width - 1;
    }
    
    sampleIndexV0 = (size_t)sampleIndexVF;
    sampleIndexV1 = sampleIndexV0 + 1;
    alphaV = sampleIndexVF - floorf(sampleIndexVF);
    if(sampleIndexV1 >= height) {
        sampleIndexV1 = height - 1;
    }
    
    rbAssert(sampleIndexU0 < width);
    rbAssert(sampleIndexV0 < height);
    
    return rbColorTempMix(
        rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndexU0 +
                stride * sampleIndexV0]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[sampleIndexU1 +
                stride * sampleIndexV0]), alphaU),
        1.0f - alphaV,
        rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndexU0 +
                stride * sampleIndexV1]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[sampleIndexU1 +
                stride * sampleIndexV1]), alphaU),
        alphaV);
}


void rbTexture2Clear(RBTexture2 * pDestTex, RBColor clearColor)
{
    size_t const size = pDestTex->size;
    
    RB_ASSERT_TEXTURE2_VALID(pDestTex);
    
    for(size_t i = 0; i < size; ++i) {
        pDestTex->data[i] = clearColor;
    }
}

void rbTexture2Blt(RBTexture2 * pDestTex, int32_t du, int32_t dv, int32_t dw,
    int32_t dh, RBTexture2 const * pSrcTex, int32_t su, int32_t sv)
{
    size_t const dStride = pDestTex->stride;
    size_t const sStride = pSrcTex->stride;
    int32_t const sWidth = pSrcTex->width;
    int32_t const sHeight = pSrcTex->height;
    
    RB_ASSERT_TEXTURE2_VALID(pDestTex);
    RB_ASSERT_TEXTURE2_VALID(pSrcTex);
    
    // The double negation works for 0x80000000, naive thing doesn't
    rbAssert(-abs(du) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(dv) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(dw) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(dh) > -RB_MAX_REASONABLE_SIZE);
    
    if(du < 0) {
        dw += du;
        su -= du;
        du = 0;
    }
    if(dv < 0) {
        dh += dv;
        sv -= dv;
        dv = 0;
    }
    
    if(du + dw > (int32_t)pDestTex->width) {
        dw = pDestTex->width - du;
    }
    if(dv + dh > (int32_t)pDestTex->height) {
        dh = pDestTex->height - dv;
    }
    
    if(dw <= 0 || dh <= 0) {
        return;
    }
    
    su %= pSrcTex->width;
    sv %= pSrcTex->height;
    
    if(su < 0) {
        su += pSrcTex->width;
    }
    if(sv < 0) {
        sv += pSrcTex->height;
    }
    
    for(size_t i = 0; i < (size_t)dh; ++i) {
        size_t tsu = su;
        size_t chunkSize = 0;
        for(size_t j = 0; j < (size_t)dw; j += chunkSize) {
            if(tsu + (size_t)dw - j < (size_t)sWidth) {
                chunkSize = dw - j;
            }
            else {
                chunkSize = sWidth - tsu;
            }
            rbAssert((j + du) + (i + dv) * dStride + chunkSize <=
                pDestTex->size);
            rbAssert(tsu + sv * sStride + chunkSize <= pSrcTex->size);
            memcpy(pDestTex->data + (j + du) + (i + dv) * dStride,
                pSrcTex->data + tsu + sv * sStride,
                chunkSize * sizeof (RBColor));
            tsu = 0;
        }
        ++sv;
        if(sv >= sHeight) {
            sv -= sHeight;
        }
    }
}


void rbTexture2BltSrcAlpha(RBTexture2 * pDestTex, int32_t du, int32_t dv,
    int32_t dw, int32_t dh, RBTexture2 const * pSrcTex, int32_t su, int32_t sv)
{
    size_t const dStride = pDestTex->stride;
    size_t const sStride = pSrcTex->stride;
    int32_t const sWidth = pSrcTex->width;
    int32_t const sHeight = pSrcTex->height;
    
    RB_ASSERT_TEXTURE2_VALID(pDestTex);
    RB_ASSERT_TEXTURE2_VALID(pSrcTex);
    
    // The double negation works for 0x80000000, naive thing doesn't
    rbAssert(-abs(du) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(dv) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(dw) > -RB_MAX_REASONABLE_SIZE);
    rbAssert(-abs(dh) > -RB_MAX_REASONABLE_SIZE);
    
    if(du < 0) {
        dw += du;
        su -= du;
        du = 0;
    }
    if(dv < 0) {
        dh += dv;
        sv -= dv;
        dv = 0;
    }
    
    if(du + dw > (int32_t)pDestTex->width) {
        dw = pDestTex->width - du;
    }
    if(dv + dh > (int32_t)pDestTex->height) {
        dh = pDestTex->height - dv;
    }
    
    if(dw <= 0 || dh <= 0) {
        return;
    }
    
    su %= pSrcTex->width;
    sv %= pSrcTex->height;
    
    if(su < 0) {
        su += pSrcTex->width;
    }
    if(sv < 0) {
        sv += pSrcTex->height;
    }
    
    for(size_t i = 0; i < (size_t)dh; ++i) {
        size_t tsu = su;
        for(size_t j = 0; j < (size_t)dw; ++j) {
            size_t dIndex = (j + du) + (i + dv) * dStride;
            RBColor c = pSrcTex->data[tsu + sv * sStride];
            pDestTex->data[dIndex] = rbColorAdd(
                rbColorScaleI(pDestTex->data[dIndex], 255 - c.a), c);
            ++tsu;
            if(tsu >= (size_t)sWidth) {
                tsu -= sWidth;
            }
        }
        ++sv;
        if(sv >= sHeight) {
            sv -= sHeight;
        }
    }
}


void rbTexture2Mix(RBTexture2 * pDestTex, RBTexture2 const * pSrcTexA,
    uint8_t alphaA, RBTexture2 const * pSrcTexB, uint8_t alphaB)
{
    size_t const width = pDestTex->width;
    size_t const height = pDestTex->height;
    size_t const stride = pDestTex->stride;
    
    RB_ASSERT_TEXTURE2_VALID(pDestTex);
    RB_ASSERT_TEXTURE2_VALID(pSrcTexA);
    RB_ASSERT_TEXTURE2_VALID(pSrcTexB);
    
    rbAssert(pSrcTexA->width == width);
    rbAssert(pSrcTexB->width == width);
    rbAssert(pSrcTexA->height == height);
    rbAssert(pSrcTexB->height == height);
    rbAssert(pSrcTexA->stride == stride);
    rbAssert(pSrcTexB->stride == stride);
    
    for(size_t j = 0; j < height; ++j) {
        for(size_t i = 0; i < width; ++i) {
            size_t o = j * stride + i;
            pDestTex->data[o] = rbColorMixI(pSrcTexA->data[o], alphaA,
                pSrcTexB->data[o], alphaB);
        }
    }
}


void rbTexture2Rescale(RBTexture2 * pDestTex, RBTexture2 const * pSrcTex)
{
    // This algorithm is very inefficient
    
    size_t const dWidth = pDestTex->width;
    size_t const dHeight = pDestTex->height;
    size_t const dStride = pDestTex->stride;
    size_t const oversample = 8;
    float const swmul = 1.0f / (dWidth * oversample);
    float const shmul = 1.0f / (dHeight * oversample);
    float const cs = 1.0f / (oversample * oversample);
    
    RB_ASSERT_TEXTURE2_VALID(pDestTex);
    RB_ASSERT_TEXTURE2_VALID(pSrcTex);
    
    for(size_t j = 0; j < dHeight; ++j) {
        for(size_t i = 0; i < dWidth; ++i) {
            RBColorTemp ct = colortempf(0.0f, 0.0f, 0.0f, 0.0f);
            
            for(size_t v = 0; v < oversample; ++v) {
                for(size_t u = 0; u < oversample; ++u) {
                    ct = ctadd(ct, ctscale(t2sampnc(pSrcTex,
                        vector2((float)((i * oversample + u) + 0.5f) * swmul,
                            (float)((j * oversample + v) + 0.5f) * shmul)),
                        cs));
                }
            }
            pDestTex->data[j * dStride + i] = colorct(ct);
        }
    }
}


void rbTexture2DebugTextF(RBTexture2 * pDestTex, int32_t du, int32_t dv,
    RBColor c, char const * format, ...)
{
    RBRect2I r = {.x = du, .y = dv, .h = RB_DEBUG_CHAR_HEIGHT};
    RBCoord2I o;
    char buf[1000];
    va_list va;
    int bufLen;
    
    va_start(va, format);
    bufLen = vsnprintf(buf, sizeof buf, format, va);
    va_end(va);
    
    if(bufLen < 0) {
        return;
    }
    
    if((size_t)bufLen > LENGTHOF(buf)) {
        bufLen = LENGTHOF(buf);
    }
    r.w = bufLen * RB_DEBUG_CHAR_WIDTH;
    
    rbClipRect(pDestTex, &r, &o);
    
    if(bufLen * RB_DEBUG_CHAR_WIDTH > r.w) {
        bufLen = (o.x + r.w + RB_DEBUG_CHAR_WIDTH - 1) / RB_DEBUG_CHAR_WIDTH;
    }
    
    if(bufLen == 0) {
        return;
    }
    
    // Convert chars in buf into indices into the font
    for(int i = 0; i < bufLen; ++i) {
        if((uint8_t)buf[i] < 32) {
            buf[i] = 0;
        }
        else if((uint8_t)buf[i] > 127) {
            buf[i] = '?' - 32;
        }
        else {
            buf[i] -= 32;
        }
    }
    
    for(int32_t j = 0; j < r.h; ++j) {
        int32_t row = o.y + j;
        for(int32_t i = 0; i < r.w; ++i) {
            size_t glyphIndex = (i + o.x) / RB_DEBUG_CHAR_WIDTH;
            size_t glyphPixel = (i + o.x) % RB_DEBUG_CHAR_WIDTH;
            uint8_t bits = g_rbFontDebug4x6[(size_t)buf[glyphIndex]][row];
            if((bits & (1 << glyphPixel)) != 0) {
                t2sett(pDestTex, r.x + i, r.y + j,
                    cmixi(t2gett(pDestTex, r.x + i, r.y + j), 255 - c.a, c,
                        255));
            }
        }
    }
}


void rbTexture2FillRect(RBTexture2 * pDestTex, int32_t du, int32_t dv,
    int32_t dw, int32_t dh, RBColor c)
{
    RBRect2I r = {.x = du, .y = dv, .w = dw, .h = dh};
    
    rbClipRect(pDestTex, &r, NULL);
    
    for(int32_t j = 0; j < r.h; ++j) {
        for(int32_t i = 0; i < r.w; ++i) {
            t2sett(pDestTex, i + r.x, j + r.y,
                cmixi(t2gett(pDestTex, i + r.x, j + r.y), 255 - c.a, c, 255));
        }
    }
}


void rbHarmonicPathGeneratorInitialize(RBHarmonicPathGenerator * pPathGen,
    float frequency, RBVector2 orientation,
    RBVector2 scale0, RBVector2 scale1, RBVector2 scale2, RBVector2 scale3)
{
    pPathGen->time = rbGetTime();
    pPathGen->frequency = frequency;
    pPathGen->phase = 0.0f;
    pPathGen->orientation = rbVector2Normalize(orientation);
    pPathGen->scale[0] = scale0;
    pPathGen->scale[1] = scale1;
    pPathGen->scale[2] = scale2;
    pPathGen->scale[3] = scale3;
    pPathGen->pos = rbVector2RotateScale(scale0, pPathGen->orientation);
}


void rbHarmonicPathGeneratorUpdate(RBHarmonicPathGenerator * pPathGen)
{
    float phase = pPathGen->phase;
    RBVector2 pos = {0.0f, 0.0f};
    RBTime dt = rbDiffTime(rbGetTime(), pPathGen->time);
    
    phase += rbMsFromTime(dt) * (pPathGen->frequency * 2.0f * RB_PI / 1000);
    phase = fmodf(phase, 2.0f * RB_PI);
    
    pPathGen->phase = phase;
    pPathGen->time += dt;
    
    for(size_t i = 0; i < LENGTHOF(pPathGen->scale); ++i) {
        RBVector2 scale = pPathGen->scale[i];
        if(scale.x != 0.0f) {
            pos.x += scale.x * cosf(phase * i);
        }
        if(scale.y != 0.0f) {
            pos.y += scale.y * sinf(phase * i);
        }
    }
    
    pPathGen->pos = rbVector2RotateScale(pos, pPathGen->orientation);
}


