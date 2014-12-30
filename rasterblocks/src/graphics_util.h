#ifndef GRAPHICS_UTIL_H_INCLUDED
#define GRAPHICS_UTIL_H_INCLUDED


#include "rasterblocks.h"


#ifdef RB_USE_NEON
#endif


typedef uint32_t RBIntAlpha;


typedef struct {
    RBTime time;
    float frequency;
    float phase;
    RBVector2 orientation;
    RBVector2 scale[4];
    RBVector2 pos;
} RBHarmonicPathGenerator;


RBColor rbPaletteLookupI(RBPalette const * pPal, RBIntAlpha alpha);
RBColor rbPaletteLookupF(RBPalette const * pPal, float alpha);
void rbPaletteMixF(RBPalette * pOut, RBPalette const * pA, float aAlpha,
    RBPalette const * pB, float bAlpha);

static inline RBColor rbColorMake(uint8_t r, uint8_t g, uint8_t b)
{
    RBColor res = {r, g, b, 0};
    return res;
}

RBColor rbColorMakeF(float r, float g, float b);
RBColor rbColorScaleI(RBColor a, RBIntAlpha alpha);
RBColor rbColorScaleF(RBColor a, float alpha);
RBColor rbColorMixI(RBColor a, RBIntAlpha aAlpha, RBColor b,
    RBIntAlpha bAlpha);
RBColor rbColorMixF(RBColor a, float aAlpha, RBColor b, float bAlpha);

static inline RBVector2 rbVector2Make(float x, float y)
{
    RBVector2 res = {x, y};
    return res;
}

static inline RBVector2 rbVector2Add(RBVector2 a, RBVector2 b)
{
    RBVector2 res = {a.x + b.x, a.y + b.y};
    return res;
}

static inline RBVector2 rbVector2Sub(RBVector2 a, RBVector2 b)
{
    RBVector2 res = {a.x + b.x, a.y + b.y};
    return res;
}

static inline RBVector2 rbVector2Scale(RBVector2 a, float scale)
{
    RBVector2 res = {a.x * scale, a.y * scale};
    return res;
}

static inline float rbVector2LengthSqr(RBVector2 a)
{
    return a.x * a.x + a.y * a.y;
}

static inline float rbVector2Length(RBVector2 a)
{
    return sqrtf(rbVector2LengthSqr(a));
}

RBVector2 rbVector2Normalize(RBVector2 a);

static inline RBVector2 rbVector2Cross(RBVector2 a)
{
    RBVector2 res = {-a.y, a.x};
    return res;
}

static inline RBVector2 rbVector2Rotate(RBVector2 a, float r)
{
    float s = sinf(r);
    float c = cosf(r);
    RBVector2 res = {a.x * c + a.y * s, a.x * -s + a.y * c};
    return res;
}

static inline RBVector2 rbVector2RotateScale(RBVector2 a, RBVector2 rs)
{
    RBVector2 res = {a.x * rs.x + a.y * rs.y, a.x * -rs.y + a.y * rs.x};
    return res;
}

RBVector2 rbVector2RotateScale(RBVector2 a, RBVector2 rs);

RBMatrix2 rbMatrix2Identity(void);
RBMatrix2 rbMatrix2Scale(RBVector2 s);
RBMatrix2 rbMatrix2Rotate(float r);
RBMatrix2 rbMatrix2RotateScale(RBVector2 rs);
RBMatrix2 rbMatrix2Multiply(RBMatrix2 a, RBMatrix2 b);
RBVector2 rbMatrix2Transform(RBMatrix2 m, RBVector2 v);

void rbHarmonicPathGeneratorInitialize(RBHarmonicPathGenerator * pPathGen,
    float frequency, RBVector2 orientation,
    RBVector2 scale0, RBVector2 scale1, RBVector2 scale2, RBVector2 scale3);
void rbHarmonicPathGeneratorUpdate(RBHarmonicPathGenerator * pPathGen);

static inline RBVector2 rbHarmonicPathGeneratorPos(
    RBHarmonicPathGenerator const * pPathGen)
{
    return pPathGen->pos;
}


#define plooki rbPaletteLookupI
#define plookf rbPaletteLookupF

#define color rbColorMake
#define cscalei rbColorScaleI
#define cscalef rbColorScaleF
#define cmixi rbColorMixI
#define cmixf rbColorMixF

#define vector2 rbVector2Make
#define v2add rbVector2Add
#define v2sub rbVector2Sub
#define v2scale rbVector2Scale
#define v2lensq rbVector2LengthSqr
#define v2len rbVector2Length
#define v2norm rbVector2Normalize
#define v2dot rbVector2Dot
#define v2cross rbVector2Cross
#define v2rot rbVector2Rotate
#define v2rotscale rbVector2RotateScale

#define m2ident rbMatrix2Identity
#define m2scale rbMatrix2Scale
#define m2rot rbMatrix2Rotate
#define m2rotscale rbMatrix2RotateScale
#define m2mul rbMatrix2Multiply
#define m2xform rbMatrix2Transform


#endif // GRAPHICS_UTIL_H_INCLUDED

