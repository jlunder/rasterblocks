#ifndef GRAPHICS_UTIL_H_INCLUDED
#define GRAPHICS_UTIL_H_INCLUDED


#include "rasterblocks.h"


#define RB_DEBUG_CHAR_WIDTH 4
#define RB_DEBUG_CHAR_HEIGHT 6


#ifdef RB_USE_NEON
#endif


typedef RBVector4 RBColorTemp;


typedef struct {
    int32_t x;
    int32_t y;
} RBCoord2I;


typedef struct {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
} RBRect2I;


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
    RBTime time;
    float frequency;
    float phase;
    RBVector2 orientation;
    RBVector2 scale[4];
    RBVector2 pos;
} RBHarmonicPathGenerator;


typedef struct {
    RBColor color;
    size_t length;
} RBPiecewiseLinearColorSegment;


// This is a CIE even intensity table for converting linear perceived
// brightness values to PWM values (which are linear power output).
// This is basically a correction table to correct for the way the eye
// perceives color.
uint8_t const g_rbCieTable[256];


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

static inline RBVector4 rbVector4EntrywiseMul(RBVector4 a, RBVector4 b)
{
    RBVector4 res = {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
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
RBColor rbColorScaleI(RBColor a, uint8_t alpha);
RBColor rbColorScaleF(RBColor a, float alpha);
RBColor rbColorAdd(RBColor a, RBColor b);
RBColor rbColorMul(RBColor a, RBColor b);
RBColor rbColorMixI(RBColor a, uint8_t aAlpha, RBColor b, uint8_t bAlpha);
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

#define rbColorTempScale rbVector4Scale
#define rbColorTempAdd rbVector4Add
#define rbColorTempMul rbVector4EntrywiseMul

static inline RBColorTemp rbColorTempClamp(RBColorTemp c)
{
    return rbColorTempMakeF(
        rbClampF(c.x, 0.0f, 1.0f), rbClampF(c.y, 0.0f, 1.0f),
        rbClampF(c.z, 0.0f, 1.0f), rbClampF(c.w, 0.0f, 1.0f));
}

RBTexture1 * rbTexture1Alloc(size_t width);
void rbTexture1Free(RBTexture1 * pTex);
static inline size_t rbTexture1GetWidth(RBTexture1 const * pTex)
    { return pTex->width; }

static inline RBColor rbTexture1GetTexel(RBTexture1 const * pTex, size_t u)
{
    rbAssert(u < pTex->width);
    return pTex->data[u];
}

static inline void rbTexture1SetTexel(RBTexture1 * pTex, size_t u, RBColor c)
{
    rbAssert(u < pTex->width);
    pTex->data[u] = c;
}

RBColorTemp rbTexture1SampleNearestRepeat(RBTexture1 const * pTex, float tc);
RBColorTemp rbTexture1SampleNearestClamp(RBTexture1 const * pTex, float tc);
RBColorTemp rbTexture1SampleLinearRepeat(RBTexture1 const * pTex, float tc);
RBColorTemp rbTexture1SampleLinearClamp(RBTexture1 const * pTex, float tc);

void rbTexture1FillFromPiecewiseLinear(RBTexture1 * pTex,
    RBPiecewiseLinearColorSegment * pSegments, size_t count, bool repeat);

RBTexture2 * rbTexture2Alloc(size_t width, size_t height);
void rbTexture2Free(RBTexture2 * pTex);
static inline size_t rbTexture2GetWidth(RBTexture2 const * pTex)
    { return pTex->width; }
static inline size_t rbTexture2GetHeight(RBTexture2 const * pTex)
    { return pTex->height; }

static inline RBColor rbTexture2GetTexel(RBTexture2 const * pTex, size_t u,
    size_t v)
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

RBColorTemp rbTexture2SampleNearestRepeat(RBTexture2 const * pTex,
    RBVector2 tc);
RBColorTemp rbTexture2SampleNearestClamp(RBTexture2 const * pTex,
    RBVector2 tc);
RBColorTemp rbTexture2SampleLinearRepeat(RBTexture2 const * pTex,
    RBVector2 tc);
RBColorTemp rbTexture2SampleLinearClamp(RBTexture2 const * pTex, RBVector2 tc);

void rbTexture2Clear(RBTexture2 * pDestTex, RBColor clearColor);

void rbTexture2Blt(RBTexture2 * pDestTex, int32_t du, int32_t dv, int32_t dw,
    int32_t dh, RBTexture2 const * pSrcTex, int32_t su, int32_t sv);
void rbTexture2BltSrcAlpha(RBTexture2 * pDestTex, int32_t du, int32_t dv,
    int32_t dw, int32_t dh, RBTexture2 const * pSrcTex, int32_t su,
    int32_t sv);
    
void rbTexture2Mix(RBTexture2 * pDestTex, RBTexture2 const * pSrcTexA,
    uint8_t alphaA, RBTexture2 const * pSrcTexB, uint8_t alphaB);
void rbTexture2Rescale(RBTexture2 * pDestTex, RBTexture2 const * pSrcTex);

void rbTexture2DebugTextF(RBTexture2 * pDestTex, int32_t du, int32_t dv,
    RBColor c, char const * format, ...);
void rbTexture2FillRect(RBTexture2 * pDestTex, int32_t du, int32_t dv,
    int32_t dw, int32_t dh, RBColor c);

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
#define colorf rbColorMakeF
#define colorct rbColorMakeCT
#define cscalei rbColorScaleI
#define cscalef rbColorScaleF
#define cadd rbColorAdd
#define cmul rbColorMul
#define cmixi rbColorMixI
#define cmixf rbColorMixF

#define colortempf rbColorTempMakeF
#define colortempc rbColorTempMakeC
#define ctseta rbColorTempSetA
#define ctgetr rbColorTempGetR
#define ctgetg rbColorTempGetG
#define ctgetb rbColorTempGetB
#define ctgeta rbColorTempGetA
#define ctmix rbColorTempMix
#define ctscale rbColorTempScale
#define ctadd rbColorTempAdd
#define ctmul rbColorTempMul
#define ctclamp rbColorTempClamp

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

#define t1getw rbTexture1GetWidth
#define t1gett rbTexture1GetTexel
#define t1sett rbTexture1SetTexel
#define t1sampnr rbTexture1SampleNearestRepeat
#define t1sampnc rbTexture1SampleNearestClamp
#define t1samplr rbTexture1SampleLinearRepeat
#define t1samplc rbTexture1SampleLinearClamp

#define t2getw rbTexture2GetWidth
#define t2geth rbTexture2GetHeight
#define t2gett rbTexture2GetTexel
#define t2sett rbTexture2SetTexel
#define t2sampnr rbTexture2SampleNearestRepeat
#define t2sampnc rbTexture2SampleNearestClamp
#define t2samplr rbTexture2SampleLinearRepeat
#define t2samplc rbTexture2SampleLinearClamp

#define t2clear rbTexture2Clear

#define t2blt rbTexture2Blt
#define t2bltsa rbTexture2BltSrcAlpha

#define t2mix rbTexture2Mix

#define t2dtextf rbTexture2DebugTextF
#define t2rect rbTexture2FillRect


#endif // GRAPHICS_UTIL_H_INCLUDED

