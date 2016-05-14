#include "light_generation.h"

#include "graphics_util.h"


static RBLightGenerator * g_rbPCurrentGenerator = NULL;
static RBLightGenerator * g_rbPDefaultGenerator = NULL;


void rbLightGenerationInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbLightGenerationShutdown();
    
    g_rbPDefaultGenerator = rbLightGenerationOscilloscopeAlloc(
        colori(127, 127, 127, 255));
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
    RBTexture2 * pFrame)
{
    if(g_rbPCurrentGenerator != NULL) {
        rbLightGenerationGeneratorGenerate(g_rbPCurrentGenerator, pAnalysis,
            pFrame);
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
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)pData;
}


*/
