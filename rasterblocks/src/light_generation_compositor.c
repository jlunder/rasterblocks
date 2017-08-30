#include "light_generation.h"

#include "graphics_util.h"


#define RB_COMPOSITOR_NUM_LAYERS 32


typedef struct
{
    RBLightGenerator * pGenerator;
    RBTexture2 * pDestTexture; // not owned by the generator
    RBLightGenerationBlendMode blendMode;
    size_t alphaIndex;
    size_t transformPosIndex;
    size_t transformScaleIndex;
} RBLightGeneratorCompositorLayer;


typedef struct
{
    RBLightGenerator genDef;
    size_t numLayers;
    RBLightGeneratorCompositorLayer layers[RB_COMPOSITOR_NUM_LAYERS];
} RBLightGeneratorCompositor;


void rbLightGenerationCompositorFree(void * pData);
void rbLightGenerationCompositorGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationCompositorAlloc(void)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)malloc(
            sizeof (RBLightGeneratorCompositor));
    
    pCompositor->genDef.pData = pCompositor;
    pCompositor->genDef.free = rbLightGenerationCompositorFree;
    pCompositor->genDef.generate = rbLightGenerationCompositorGenerate;
    
    pCompositor->numLayers = 0;
    
    return &pCompositor->genDef;
}


void rbLightGenerationCompositorAddLayer(RBLightGenerator * pCompositorGen,
    RBLightGenerator * pGenerator, RBTexture2 * pDestTexture,
    RBLightGenerationBlendMode blendMode, size_t alphaIndex,
    size_t transformPosIndex, size_t transformScaleIndex)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pCompositorGen->pData;
    size_t layer = pCompositor->numLayers;
    
    rbAssert(pCompositor->genDef.generate ==
        rbLightGenerationCompositorGenerate);
    
    if(layer < RB_COMPOSITOR_NUM_LAYERS) {
        pCompositor->layers[layer].pGenerator = pGenerator;
        pCompositor->layers[layer].pDestTexture = pDestTexture;
        pCompositor->layers[layer].blendMode = blendMode;
        pCompositor->layers[layer].alphaIndex = alphaIndex;
        pCompositor->layers[layer].transformPosIndex = transformPosIndex;
        pCompositor->layers[layer].transformScaleIndex = transformScaleIndex;
        pCompositor->numLayers = layer + 1;
    }
    else {
        rbWarning("Tried to add layer to compositor, but max count reached");
    }
}


void rbLightGenerationCompositorFree(void * pData)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pData;
    
    free(pCompositor);
}


void rbLightGenerationCompositorGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    RBColorTemp compositionBuf[fHeight][fWidth];
    
    UNUSED(pAnalysis);
    
    rbZero(compositionBuf, sizeof compositionBuf);
    
    for(size_t k = 0; k < pCompositor->numLayers; ++k) {
        float alpha = rbParameterGetF(pParameters,
            pCompositor->layers[k].alphaIndex, 1.0f);
        RBTexture2 * pDestTexture = pCompositor->layers[k].pDestTexture;
        
        rbLightGenerationGeneratorGenerate(pCompositor->layers[k].pGenerator,
            pAnalysis, pParameters, pDestTexture);
        
        if(alpha > 0.0f) {
            RBVector2 pos = rbParameterGetV2(pParameters,
                pCompositor->layers[k].transformPosIndex, vector2(0.0f, 0.0f));
            RBVector2 scale = rbParameterGetV2(pParameters,
                pCompositor->layers[k].transformScaleIndex,
                vector2(1.0f, 0.0f));
            float invScaleLenSqr = 1.0f / v2lensq(scale);
            float uMult = invScaleLenSqr / (t2getw(pDestTexture) - 1);
            float vMult = invScaleLenSqr / (t2geth(pDestTexture) - 1);
            RBVector2 xInc = vector2(scale.x * uMult, scale.y * vMult);
            RBVector2 yInc = vector2(-scale.y * uMult, scale.x * vMult);
            RBVector2 lineTc =
                v2add(v2scale(xInc, -pos.x),
                    v2scale(yInc, -pos.y));
            RBLightGenerationBlendMode blendMode =
                pCompositor->layers[k].blendMode;
            for(size_t j = 0; j < t2geth(pFrame); ++j) {
                RBVector2 tc = lineTc;
                for(size_t i = 0; i < t2getw(pFrame); ++i) {
                    RBColorTemp sample = t2samplc(pDestTexture, tc);
                    switch(blendMode) {
                    default:
                    case RBLGBM_SRC_ALPHA:
                        compositionBuf[j][i] = ctadd(ctscale(
                            compositionBuf[j][i], 1.0f - ctgeta(sample)),
                            sample);
                        break;
                    case RBLGBM_DEST_ALPHA:
                        compositionBuf[j][i] = ctadd(ctscale(
                            sample, 1.0f - ctgeta(compositionBuf[j][i])),
                            compositionBuf[j][i]);
                        break;
                    case RBLGBM_STENCIL:
                        compositionBuf[j][i] = ctscale(compositionBuf[j][i],
                            rbClampF(ctgeta(sample), 0.0f, 1.0f));
                        break;
                    case RBLGBM_INV_STENCIL:
                        compositionBuf[j][i] = ctscale(compositionBuf[j][i],
                            1.0f - rbClampF(ctgeta(sample), 0.0f, 1.0f));
                        break;
                    case RBLGBM_ADD:
                        compositionBuf[j][i] =
                            ctadd(sample, compositionBuf[j][i]);
                        break;
                    case RBLGBM_SUBTRACT:
                        compositionBuf[j][i] =
                            ctsub(sample, compositionBuf[j][i]);
                        break;
                    case RBLGBM_MULTIPLY:
                        compositionBuf[j][i] =
                            ctmul(sample, compositionBuf[j][i]);
                        break;
                    }
                    tc = v2add(tc, xInc);
                }
                lineTc = v2add(lineTc, yInc);
            }
        }
    }
    
    for(size_t j = 0; j < fHeight; ++j) {
        for(size_t i = 0; i < fWidth; ++i) {
            t2sett(pFrame, i, j, colorct(compositionBuf[j][i]));
        }
    }
}


