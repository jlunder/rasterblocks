#include "light_generation.h"

#include "graphics_util.h"


static RBLightGenerator * g_rbPCurrentGenerator = NULL;
static RBLightGenerator * g_rbPDefaultGenerator = NULL;
static union
{
    uint8_t allocation[sizeof (RBTexture1) + 2 * sizeof (RBColor)];
    RBTexture1 texture;
} g_rbGrayscaleAlphaPalette;


void rbLightGenerationInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbLightGenerationShutdown();
    
    rbAssert(rbTexture1ComputeSize(2) <= sizeof g_rbGrayscaleAlphaPalette);
    rbTexture1Construct(&g_rbGrayscaleAlphaPalette.texture, 2);
    t1sett(&g_rbGrayscaleAlphaPalette.texture, 0, colori(0, 0, 0, 0));
    t1sett(&g_rbGrayscaleAlphaPalette.texture, 1, colori(255, 255, 255, 255));
    
    g_rbPDefaultGenerator =
        rbLightGenerationOscilloscopeAlloc(&g_rbGrayscaleAlphaPalette.texture);
    rbLightGenerationSetGenerator(g_rbPDefaultGenerator);
}


void rbLightGenerationShutdown(void)
{
    rbLightGenerationSetGenerator(NULL);
    if(g_rbPDefaultGenerator != NULL) {
        rbLightGenerationGeneratorFree(g_rbPDefaultGenerator);
    }
    g_rbPDefaultGenerator = NULL;
}


void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBParameters const * pParameters, RBTexture2 * pFrame)
{
    if(g_rbPCurrentGenerator != NULL) {
        rbLightGenerationGeneratorGenerate(g_rbPCurrentGenerator, pAnalysis,
            pParameters, pFrame);
    }
    else {
        for(size_t j = 0; j < t2geth(pFrame); ++j) {
            for(size_t i = 0; i < t2getw(pFrame); ++i) {
                t2sett(pFrame, i, j, colori(255, 0, 255, 255));
            }
        }
    }
}


void rbLightGenerationSetGenerator(RBLightGenerator * pGenerator)
{
    g_rbPCurrentGenerator = pGenerator;
}


/*
typedef struct
{
    RBLightGenerator genDef;
} RBLightGenerator<**>;


void rbLightGeneration<**>Free(void * pData);
void rbLightGeneration<**>Generate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGeneration<**>Alloc(void)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)malloc(
            sizeof (RBLightGenerator<**>));
    
    p<**>->genDef.pData = p<**>;
    p<**>->genDef.free = rbLightGeneration<**>Free;
    p<**>->genDef.generate = rbLightGeneration<**>Generate;
    
    return &p<**>->genDef;
}


void rbLightGeneration<**>Free(void * pData)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)pData;
}


void rbLightGeneration<**>Generate(void * pData,
    RBParameters const * pParameters, RBTexture2 * pFrame)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)pData;
}


*/
