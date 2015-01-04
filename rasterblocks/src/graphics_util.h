#ifndef GRAPHICS_UTIL_H_INCLUDED
#define GRAPHICS_UTIL_H_INCLUDED


#include "rasterblocks.h"


#ifdef RB_USE_NEON
#endif


typedef RBVector4 RBColorTemp;


typedef uint32_t RBIntAlpha;


typedef struct {
    RBTime time;
    float frequency;
    float phase;
    RBVector2 orientation;
    RBVector2 scale[4];
    RBVector2 pos;
} RBHarmonicPathGenerator;


typedef struct {
    size_t width;
    size_t size;
    RBColor data[0];
} RBTexture1;


typedef struct {
    size_t width;
    size_t height;
    size_t stride;
    size_t size;
    RBColor data[0];
} RBTexture2;


typedef struct {
    RBColor color;
    size_t length;
} RBPiecewiseLinearColorSegment;


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
    RBVector2 res = {a.x - b.x, a.y - b.y};
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

static inline float rbVector2Dot(RBVector2 a, RBVector2 b)
{
    return a.x * b.x + a.y * b.y;
}

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

static inline RBVector4 rbVector4Make(float x, float y, float z, float w)
{
    RBVector4 res = {x, y, z, w};
    return res;
}

static inline RBVector4 rbVector4Add(RBVector4 a, RBVector4 b)
{
    RBVector4 res = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    return res;
}

static inline RBVector4 rbVector4Sub(RBVector4 a, RBVector4 b)
{
    RBVector4 res = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    return res;
}

static inline RBVector4 rbVector4Scale(RBVector4 a, float scale)
{
    RBVector4 res = {a.x * scale, a.y * scale, a.z * scale, a.w * scale};
    return res;
}

static inline float rbVector4LengthSqr(RBVector4 a)
{
    return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

static inline float rbVector4Length(RBVector4 a)
{
    return sqrtf(rbVector4LengthSqr(a));
}

static inline float rbVector4Dot(RBVector4 a, RBVector4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

RBVector4 rbVector4Normalize(RBVector4 a);

RBMatrix2 rbMatrix2Identity(void);
RBMatrix2 rbMatrix2Scale(RBVector2 s);
RBMatrix2 rbMatrix2Rotate(float r);
RBMatrix2 rbMatrix2RotateScale(RBVector2 rs);
RBMatrix2 rbMatrix2Multiply(RBMatrix2 a, RBMatrix2 b);
RBVector2 rbMatrix2Transform(RBMatrix2 m, RBVector2 v);

static inline RBColor rbColorMakeI(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    RBColor res = {r, g, b, a};
    return res;
}

RBColor rbColorMakeF(float r, float g, float b, float a);
RBColor rbColorMakeCT(RBColorTemp ct);
RBColor rbColorScaleI(RBColor a, RBIntAlpha alpha);
RBColor rbColorScaleF(RBColor a, float alpha);
RBColor rbColorMixI(RBColor a, RBIntAlpha aAlpha, RBColor b,
    RBIntAlpha bAlpha);
RBColor rbColorMixF(RBColor a, float aAlpha, RBColor b, float bAlpha);

static inline RBColorTemp rbColorTempMakeI(uint8_t r, uint8_t g, uint8_t b,
    uint8_t a)
{
    return rbVector4Make(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

#define rbColorTempMakeF rbVector4Make

static inline RBColorTemp rbColorTempMakeC(RBColor c)
{
    return rbColorTempMakeF(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f,
        c.a / 255.0f);
}

static inline RBColorTemp rbColorTempSetA(RBColorTemp c, float a)
{
    return rbColorTempMakeF(c.x, c.y, c.z, a);
}

static inline float rbColorTempGetR(RBColorTemp c) { return c.x; }
static inline float rbColorTempGetG(RBColorTemp c) { return c.y; }
static inline float rbColorTempGetB(RBColorTemp c) { return c.z; }
static inline float rbColorTempGetA(RBColorTemp c) { return c.w; }

static inline RBColorTemp rbColorTempPremultiplyAlpha(RBColorTemp c)
{
    return rbVector4Scale(rbColorTempSetA(c, 1.0f), rbColorTempGetA(c));
}

static inline RBColorTemp rbColorTempMix(RBColorTemp a, float aAlpha,
    RBColorTemp b, float bAlpha)
{
    return rbVector4Add(rbVector4Scale(a, aAlpha), rbVector4Scale(b, bAlpha));
}

RBTexture1 * rbTexture1Alloc(size_t width);
void rbTexture1Free(RBTexture1 * pTex);
static inline size_t rbTexture1GetWidth(RBTexture1 * pTex)
    { return pTex->width; }

static inline RBColor rbTexture1GetTexel(RBTexture1 * pTex, size_t u)
{
    rbAssert(u < pTex->width);
    return pTex->data[u];
}

static inline void rbTexture1SetTexel(RBTexture1 * pTex, size_t u, RBColor c)
{
    rbAssert(u < pTex->width);
    pTex->data[u] = c;
}

void rbTexture1FillFromPiecewiseLinear(RBTexture1 * pTex,
    RBPiecewiseLinearColorSegment * pSegments, size_t count, bool repeat);
void rbTexture1PrepareForSampling(RBTexture1 * pTex);
RBColorTemp rbTexture1SampleNearestRepeat(RBTexture1 * pTex, float tc);
RBColorTemp rbTexture1SampleNearestClamp(RBTexture1 * pTex, float tc);
RBColorTemp rbTexture1SampleLinearRepeat(RBTexture1 * pTex, float tc);
RBColorTemp rbTexture1SampleLinearClamp(RBTexture1 * pTex, float tc);

RBTexture2 * rbTexture2Alloc(size_t width, size_t height);
void rbTexture2Free(RBTexture2 * pTex);
static inline size_t rbTexture2GetWidth(RBTexture2 * pTex)
    { return pTex->width; }
static inline size_t rbTexture2GetHeight(RBTexture2 * pTex)
    { return pTex->height; }

static inline RBColor rbTexture2GetTexel(RBTexture2 * pTex, size_t u, size_t v)
{
    rbAssert(pTex->stride >= pTex->width);
    rbAssert(u < pTex->width);
    rbAssert(v < pTex->height);
    return pTex->data[u + pTex->stride * v];
}

static inline void rbTexture2SetTexel(RBTexture2 * pTex, size_t u, size_t v,
    RBColor c)
{
    rbAssert(pTex->stride >= pTex->width);
    rbAssert(u < pTex->width);
    rbAssert(v < pTex->height);
    pTex->data[u + pTex->stride * v] = c;
}

void rbTexture2PrepareForSampling(RBTexture2 * pTex);
RBColorTemp rbTexture2SampleNearestRepeat(RBTexture2 * pTex, RBVector2 tc);
RBColorTemp rbTexture2SampleNearestClamp(RBTexture2 * pTex, RBVector2 tc);
RBColorTemp rbTexture2SampleLinearRepeat(RBTexture2 * pTex, RBVector2 tc);
RBColorTemp rbTexture2SampleLinearClamp(RBTexture2 * pTex, RBVector2 tc);

void rbHarmonicPathGeneratorInitialize(RBHarmonicPathGenerator * pPathGen,
    float frequency, RBVector2 orientation,
    RBVector2 scale0, RBVector2 scale1, RBVector2 scale2, RBVector2 scale3);
void rbHarmonicPathGeneratorUpdate(RBHarmonicPathGenerator * pPathGen);

static inline RBVector2 rbHarmonicPathGeneratorPos(
    RBHarmonicPathGenerator const * pPathGen)
{
    return pPathGen->pos;
}


#define colori rbColorMakeI
#define colorf rbColorMakeI
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

