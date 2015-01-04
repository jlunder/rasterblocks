#include "graphics_util.h"


#define RB_MAX_REASONABLE_SIZE 10000


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


RBColor rbColorScaleF(RBColor a, float alpha)
{
    return rbColorMakeI(
        (uint8_t)rbClampF(roundf(a.r * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.g * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.b * alpha), 0.0f, 255.0f),
        0);
}


RBColor rbColorMixF(RBColor a, float aAlpha, RBColor b, float bAlpha)
{
    return rbColorMakeI(
        (uint8_t)rbClampF(roundf(a.r * aAlpha + b.r * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.g * aAlpha + b.g * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.b * aAlpha + b.b * bAlpha), 0.0f, 255.0f),
        0);
}


RBTexture1 * rbTexture1Alloc(size_t width)
{
    size_t size = sizeof (RBTexture1) + (width + 1) * sizeof (RBColor);
    RBTexture1 * pTex = malloc(size);
    
    rbAssert(width <= RB_MAX_REASONABLE_SIZE);
    rbAssert(width > 0);
    
    pTex->width = width;
    pTex->size = width + 1;
    memset(pTex->data, 0, pTex->size * sizeof (RBColor));
    
    return pTex;
}


void rbTexture1Free(RBTexture1 * pTex)
{
    rbAssert(pTex->width > 0);
    
    pTex->width = 0;
    pTex->size = 0;
    free(pTex);
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


void rbTexture1PrepareForSampling(RBTexture1 * pTex)
{
    rbAssert(pTex->width > 0);
    rbAssert(pTex->size > pTex->width);
    
    pTex->data[pTex->width] = pTex->data[0];
}


RBColorTemp rbTexture1SampleNearestRepeat(RBTexture1 * pTex, float tc)
{
    size_t sampleIndex = (size_t)(pTex->width * (tc - floorf(tc)));
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->size >= pTex->width);
    rbAssert(!isnan(tc));
    rbAssert(sampleIndex < pTex->width);
    
    return rbColorTempMakeC(pTex->data[sampleIndex]);
}


RBColorTemp rbTexture1SampleNearestClamp(RBTexture1 * pTex, float tc)
{
    size_t sampleIndex;
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->size >= pTex->width);
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


RBColorTemp rbTexture1SampleLinearRepeat(RBTexture1 * pTex, float tc)
{
    float sampleIndexF = pTex->width * (tc - floorf(tc));
    size_t sampleIndex = (size_t)sampleIndexF;
    float alpha = sampleIndexF - floorf(sampleIndexF);
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->size > pTex->width);
    rbAssert(!isnan(tc));
    
    return rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndex]),
        1.0f - alpha, rbColorTempMakeC(pTex->data[sampleIndex + 1]), alpha);
}


RBColorTemp rbTexture1SampleLinearClamp(RBTexture1 * pTex, float tc)
{
    rbAssert(pTex->width > 0);
    rbAssert(pTex->size > pTex->width);
    rbAssert(!isnan(tc));
    
    if(tc > 0) {
        if(tc < 1.0f) {
            float sampleIndexF = (pTex->width - 1) * (tc - floorf(tc));
            size_t sampleIndex = (size_t)sampleIndexF;
            float alpha = sampleIndexF - floorf(sampleIndexF);
            
            rbAssert(sampleIndex < pTex->width);
            
            return rbColorTempMix(rbColorTempMakeC(pTex->data[sampleIndex]),
                1.0f - alpha, rbColorTempMakeC(pTex->data[sampleIndex + 1]),
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


RBTexture2 * rbTexture2Alloc(size_t width, size_t height)
{
    size_t stride = (width + 1 + 3) & (~3);
    size_t size = sizeof (RBTexture2) +
        (width + 1) * (height + 1) * sizeof (RBColor);
    RBTexture2 * pTex = malloc(size);
    
    rbAssert(width <= RB_MAX_REASONABLE_SIZE);
    rbAssert(width > 0);
    rbAssert(height <= RB_MAX_REASONABLE_SIZE);
    rbAssert(height > 0);
    
    pTex->width = width;
    pTex->height = height;
    pTex->stride = stride;
    pTex->size = width + 1;
    memset(pTex->data, 0, pTex->size * sizeof (RBColor));
    
    return pTex;
}


void rbTexture2Free(RBTexture2 * pTex)
{
    rbAssert(pTex->width > 0);
    rbAssert(pTex->height > 0);
    
    pTex->width = 0;
    pTex->size = 0;
    free(pTex);
}


void rbTexture2PrepareForSampling(RBTexture2 * pTex)
{
    size_t const stride = pTex->stride;
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->height > 0);
    rbAssert(pTex->size >= (pTex->height + 1) * pTex->stride);
    
    for(size_t i = 0; i < pTex->height; ++i) {
        pTex->data[pTex->width + i * stride] = pTex->data[0 + i * stride];
    }
    for(size_t i = 0; i < pTex->width + 1; ++i) {
        pTex->data[i + pTex->height * stride] = pTex->data[i + 0 * stride];
    }
}


RBColorTemp rbTexture2SampleNearestRepeat(RBTexture2 * pTex, RBVector2 tc)
{
    size_t sampleIndexU = (size_t)(pTex->width * (tc.x - floorf(tc.x)));
    size_t sampleIndexV = (size_t)(pTex->height * (tc.y - floorf(tc.y)));
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->height > 0);
    rbAssert(pTex->stride >= pTex->width);
    rbAssert(pTex->size >= pTex->stride * pTex->height);
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    rbAssert(sampleIndexU < pTex->width);
    rbAssert(sampleIndexV < pTex->height);
    
    return rbColorTempMakeC(pTex->data[sampleIndexU +
        sampleIndexV * pTex->stride]);
}


RBColorTemp rbTexture2SampleNearestClamp(RBTexture2 * pTex, RBVector2 tc)
{
    size_t sampleIndexU;
    size_t sampleIndexV;
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->height > 0);
    rbAssert(pTex->stride >= pTex->width);
    rbAssert(pTex->size >= pTex->stride * pTex->height);
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


RBColorTemp rbTexture2SampleLinearRepeat(RBTexture2 * pTex, RBVector2 tc)
{
    float sampleIndexUF = tc.x - floorf(tc.x);
    float sampleIndexVF = tc.y - floorf(tc.y);
    size_t sampleIndexU = (size_t)sampleIndexUF;
    size_t sampleIndexV = (size_t)sampleIndexVF;
    float alphaU = sampleIndexUF - floorf(sampleIndexUF);
    float alphaV = sampleIndexVF - floorf(sampleIndexVF);
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->height > 0);
    rbAssert(pTex->stride > pTex->width);
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    rbAssert(sampleIndexU < pTex->width);
    rbAssert(sampleIndexV < pTex->height);
    
    return rbColorTempMix(
        rbColorTempMix(rbColorTempMakeC(pTex->data[(sampleIndexU + 0) +
                pTex->stride * (sampleIndexV + 0)]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[(sampleIndexU + 1) +
                pTex->stride * (sampleIndexV + 0)]), alphaU),
        1.0f - alphaV,
        rbColorTempMix(rbColorTempMakeC(pTex->data[(sampleIndexU + 0) +
                pTex->stride * (sampleIndexV + 1)]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[(sampleIndexU + 1) +
                pTex->stride * (sampleIndexV + 1)]), alphaU),
        alphaV);
}


RBColorTemp rbTexture2SampleLinearClamp(RBTexture2 * pTex, RBVector2 tc)
{
    float sampleIndexUF;
    float sampleIndexVF;
    size_t sampleIndexU;
    size_t sampleIndexV;
    float alphaU;
    float alphaV;
    
    rbAssert(pTex->width > 0);
    rbAssert(pTex->height > 0);
    rbAssert(pTex->stride > pTex->width);
    rbAssert(pTex->size >= pTex->stride * (pTex->height + 1));
    rbAssert(!isnan(tc.x));
    rbAssert(!isnan(tc.y));
    
    if(tc.x > 0) {
        if(tc.x < 1.0f) {
            sampleIndexUF = (pTex->width - 1) * tc.x;
        }
        else {
            sampleIndexUF = pTex->width - 1;
        }
    }
    else {
        sampleIndexUF = 0;
    }
    
    if(tc.y > 0) {
        if(tc.y < 1.0f) {
            sampleIndexVF = (pTex->height - 1) * tc.y;
        }
        else {
            sampleIndexVF = pTex->height - 1;
        }
    }
    else {
        sampleIndexVF = 0;
    }
    
    sampleIndexU = (size_t)sampleIndexUF;
    alphaU = sampleIndexUF - floorf(sampleIndexUF);
    sampleIndexV = (size_t)sampleIndexVF;
    alphaV = sampleIndexVF - floorf(sampleIndexVF);
    
    rbAssert(sampleIndexU < pTex->width);
    rbAssert(sampleIndexV < pTex->height);
    
    return rbColorTempMix(
        rbColorTempMix(rbColorTempMakeC(pTex->data[(sampleIndexU + 0) +
                pTex->stride * (sampleIndexV + 0)]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[(sampleIndexU + 1) +
                pTex->stride * (sampleIndexV + 0)]), alphaU),
        1.0f - alphaV,
        rbColorTempMix(rbColorTempMakeC(pTex->data[(sampleIndexU + 0) +
                pTex->stride * (sampleIndexV + 1)]), 1.0f - alphaU,
            rbColorTempMakeC(pTex->data[(sampleIndexU + 1) +
                pTex->stride * (sampleIndexV + 1)]), alphaU),
        alphaV);
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
    float phase = 0.0f;
    RBVector2 pos = {0.0f, 0.0f};
    
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


