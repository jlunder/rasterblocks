#include "graphics_util.h"


SLColor SLColorMakeF(float r, float g, float b)
{
    SLColor res = {
        (uint8_t)slClampF(r * 256.0f, 0.0f, 255.0f),
        (uint8_t)slClampF(g * 256.0f, 0.0f, 255.0f),
        (uint8_t)slClampF(b * 256.0f, 0.0f, 255.0f),
        0
    };
    
    return res;
}


SLColor slColorScaleF(SLColor a, float alpha)
{
    return slColorMake(
        (uint8_t)slClampF(roundf(a.r * alpha), 0.0f, 255.0f),
        (uint8_t)slClampF(roundf(a.g * alpha), 0.0f, 255.0f),
        (uint8_t)slClampF(roundf(a.b * alpha), 0.0f, 255.0f));
}


SLColor slColorMixF(SLColor a, float aAlpha, SLColor b, float bAlpha)
{
    return slColorMake(
        (uint8_t)slClampF(roundf(a.r * aAlpha + b.r * bAlpha), 0.0f, 255.0f),
        (uint8_t)slClampF(roundf(a.g * aAlpha + b.g * bAlpha), 0.0f, 255.0f),
        (uint8_t)slClampF(roundf(a.b * aAlpha + b.b * bAlpha), 0.0f, 255.0f));
}


SLColor slPaletteLookupF(SLPalette const * pPal, float f)
{
    // Conditional arrangement carefully ensures a NaN f will result in
    // selecting palette entry 0
    if(f >= 1.0f) {
        return pPal->c[LENGTHOF(pPal->c) - 1];
    }
    else if(f >= 0.0f) {
        float f4 = f * (float)(LENGTHOF(pPal->c) - 1);
        float floorf4 = floorf(f4);
        size_t i = (size_t)floorf4;
        float a = f4 - floorf4;
        float b = 1.0f - a;
        return slColorMixF(pPal->c[i], b, pPal->c[i + 1], a);
    }
    else {
        return pPal->c[0];
    }
}


SLVector2 slVector2Normalize(SLVector2 a)
{
    float l = slVector2Length(a);
    if(l != 0.0f) {
        SLVector2 res = {a.x / l, a.y / l};
        return res;
    }
    else {
        // Putting this in the "else" case should catch NaN
        SLVector2 res = {1.0f, 0.0f};
        return res;
    }
}


float slVector2Dot(SLVector2 a, SLVector2 b)
{
    return a.x * b.x + a.y * b.y;
}


SLMatrix2 slMatrix2Identity(void)
{
    SLMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    return res;
}


SLMatrix2 slMatrix2Scale(SLVector2 s)
{
    SLMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(s);
    return res;
}


SLMatrix2 slMatrix2Rotate(float r)
{
    SLMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(r);
    return res;
}


SLMatrix2 slMatrix2RotateScale(SLVector2 rs)
{
    SLMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(rs);
    return res;
}


SLMatrix2 slMatrix2Multiply(SLMatrix2 a, SLMatrix2 b)
{
    SLMatrix2 res = {0.0f, 0.0f, 0.0f, 0.0f};
    UNUSED(a);
    UNUSED(b);
    return res;
}


SLVector2 slMatrix2Transform(SLMatrix2 m, SLVector2 v)
{
    UNUSED(m);
    return v;
}


void slHarmonicPathGeneratorInitialize(SLHarmonicPathGenerator * pPathGen,
    float frequency, SLVector2 orientation,
    SLVector2 scale0, SLVector2 scale1, SLVector2 scale2, SLVector2 scale3)
{
    pPathGen->time = slGetTime();
    pPathGen->frequency = frequency;
    pPathGen->phase = 0.0f;
    pPathGen->orientation = slVector2Normalize(orientation);
    pPathGen->scale[0] = scale0;
    pPathGen->scale[1] = scale1;
    pPathGen->scale[2] = scale2;
    pPathGen->scale[3] = scale3;
    pPathGen->pos = slVector2RotateScale(scale0, pPathGen->orientation);
}


void slHarmonicPathGeneratorUpdate(SLHarmonicPathGenerator * pPathGen)
{
    float phase = 0.0f;
    SLVector2 pos = {0.0f, 0.0f};
    
    for(size_t i = 0; i < LENGTHOF(pPathGen->scale); ++i) {
        SLVector2 scale = pPathGen->scale[i];
        if(scale.x != 0.0f) {
            pos.x += scale.x * cosf(phase * i);
        }
        if(scale.y != 0.0f) {
            pos.y += scale.y * sinf(phase * i);
        }
    }
    
    pPathGen->pos = slVector2RotateScale(pos, pPathGen->orientation);
}


