#include "graphics_util.h"


RBColor RBColorMakeF(float r, float g, float b)
{
    RBColor res = {
        (uint8_t)rbClampF(r * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(g * 256.0f, 0.0f, 255.0f),
        (uint8_t)rbClampF(b * 256.0f, 0.0f, 255.0f),
        0
    };
    
    return res;
}


RBColor rbColorScaleF(RBColor a, float alpha)
{
    return rbColorMake(
        (uint8_t)rbClampF(roundf(a.r * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.g * alpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.b * alpha), 0.0f, 255.0f));
}


RBColor rbColorMixF(RBColor a, float aAlpha, RBColor b, float bAlpha)
{
    return rbColorMake(
        (uint8_t)rbClampF(roundf(a.r * aAlpha + b.r * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.g * aAlpha + b.g * bAlpha), 0.0f, 255.0f),
        (uint8_t)rbClampF(roundf(a.b * aAlpha + b.b * bAlpha), 0.0f, 255.0f));
}


RBColor rbPaletteLookupF(RBPalette const * pPal, float f)
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
        return rbColorMixF(pPal->c[i], b, pPal->c[i + 1], a);
    }
    else {
        return pPal->c[0];
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


float rbVector2Dot(RBVector2 a, RBVector2 b)
{
    return a.x * b.x + a.y * b.y;
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


