#ifndef GRAPHICS_UTIL_H_INCLUDED
#define GRAPHICS_UTIL_H_INCLUDED


#include "stage_lights.h"


#ifdef SL_USE_NEON
#endif


typedef uint32_t SLIntAlpha;


typedef struct {
    float x, y;
} SLVector2;


typedef struct {
    float x, y, z;
} SLVector3;


typedef struct {
    float x, y, z, w;
} SLVector4;


typedef struct {
    float m00, m01, m10, m11;
} SLMatrix2;


typedef struct {
    SLTime time;
    float frequency;
    float phase;
    SLVector2 orientation;
    SLVector2 scale[4];
    SLVector2 pos;
} SLHarmonicPathGenerator;


SLColor slPaletteLookupI(SLPalette const * pPal, SLIntAlpha alpha);
SLColor slPaletteLookupF(SLPalette const * pPal, float alpha);
void slPaletteMixF(SLPalette * pOut, SLPalette const * pA, float aAlpha,
    SLPalette const * pB, float bAlpha);

static inline SLColor slColorMake(uint8_t r, uint8_t g, uint8_t b)
{
    SLColor res = {r, g, b, 0};
    return res;
}

SLColor slColorMakeF(float r, float g, float b);
SLColor slColorScaleI(SLColor a, SLIntAlpha alpha);
SLColor slColorScaleF(SLColor a, float alpha);
SLColor slColorMixI(SLColor a, SLIntAlpha aAlpha, SLColor b,
    SLIntAlpha bAlpha);
SLColor slColorMixF(SLColor a, float aAlpha, SLColor b, float bAlpha);

static inline SLVector2 slVector2Make(float x, float y)
{
    SLVector2 res = {x, y};
    return res;
}

static inline SLVector2 slVector2Add(SLVector2 a, SLVector2 b)
{
    SLVector2 res = {a.x + b.x, a.y + b.y};
    return res;
}

static inline SLVector2 slVector2Sub(SLVector2 a, SLVector2 b)
{
    SLVector2 res = {a.x + b.x, a.y + b.y};
    return res;
}

static inline SLVector2 slVector2Scale(SLVector2 a, float scale)
{
    SLVector2 res = {a.x * scale, a.y * scale};
    return res;
}

static inline float slVector2LengthSqr(SLVector2 a)
{
    return a.x * a.x + a.y * a.y;
}

static inline float slVector2Length(SLVector2 a)
{
    return sqrtf(slVector2LengthSqr(a));
}

SLVector2 slVector2Normalize(SLVector2 a);

static inline SLVector2 slVector2Cross(SLVector2 a)
{
    SLVector2 res = {-a.y, a.x};
    return res;
}

static inline SLVector2 slVector2Rotate(SLVector2 a, float r)
{
    float s = sinf(r);
    float c = cosf(r);
    SLVector2 res = {a.x * c + a.y * s, a.x * -s + a.y * c};
    return res;
}

static inline SLVector2 slVector2RotateScale(SLVector2 a, SLVector2 rs)
{
    SLVector2 res = {a.x * rs.x + a.y * rs.y, a.x * -rs.y + a.y * rs.x};
    return res;
}

SLVector2 slVector2RotateScale(SLVector2 a, SLVector2 rs);

SLMatrix2 slMatrix2Identity(void);
SLMatrix2 slMatrix2Scale(SLVector2 s);
SLMatrix2 slMatrix2Rotate(float r);
SLMatrix2 slMatrix2RotateScale(SLVector2 rs);
SLMatrix2 slMatrix2Multiply(SLMatrix2 a, SLMatrix2 b);
SLVector2 slMatrix2Transform(SLMatrix2 m, SLVector2 v);

void slHarmonicPathGeneratorInitialize(SLHarmonicPathGenerator * pPathGen,
    float frequency, SLVector2 orientation,
    SLVector2 scale0, SLVector2 scale1, SLVector2 scale2, SLVector2 scale3);
void slHarmonicPathGeneratorUpdate(SLHarmonicPathGenerator * pPathGen);

static inline SLVector2 slHarmonicPathGeneratorPos(
    SLHarmonicPathGenerator const * pPathGen)
{
    return pPathGen->pos;
}


#define plooki slPaletteLookupI
#define plookf slPaletteLookupF

#define color slColorMake
#define cscalei slColorScaleI
#define cscalef slColorScaleF
#define cmixi slColorMixI
#define cmixf slColorMixF

#define v2 slVector2Make
#define v2add slVector2Add
#define v2sub slVector2Sub
#define v2scale slVector2Scale
#define v2lensq slVector2LengthSqr
#define v2len slVector2Length
#define v2norm slVector2Normalize
#define v2dot slVector2Dot
#define v2cross slVector2Cross
#define v2rot slVector2Rotate
#define v2rotscale slVector2RotateScale

#define m2ident slMatrix2Identity
#define m2scale slMatrix2Scale
#define m2rot slMatrix2Rotate
#define m2rotscale slMatrix2RotateScale
#define m2mul slMatrix2Multiply
#define m2xform slMatrix2Transform


#endif // GRAPHICS_UTIL_H_INCLUDED

