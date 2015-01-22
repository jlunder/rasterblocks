#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor hColor;
    RBColor vColor;
    float hEnergy;
    float vEnergy;
} RBLightGeneratorPulseGrid;


void rbLightGenerationPulseGridFree(void * pData);
void rbLightGenerationPulseGridGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulseGridAlloc(RBColor hColor,
    RBColor vColor)
{
    RBLightGeneratorPulseGrid * pPulseGrid =
        (RBLightGeneratorPulseGrid *)malloc(
            sizeof (RBLightGeneratorPulseGrid));
    
    pPulseGrid->genDef.pData = pPulseGrid;
    pPulseGrid->genDef.free = rbLightGenerationPulseGridFree;
    pPulseGrid->genDef.generate = rbLightGenerationPulseGridGenerate;
    pPulseGrid->hColor = hColor;
    pPulseGrid->vColor = vColor;
    pPulseGrid->hEnergy = 0.0f;
    pPulseGrid->vEnergy = 0.0f;
    
    return &pPulseGrid->genDef;
}


void rbLightGenerationPulseGridFree(void * pData)
{
    RBLightGeneratorPulseGrid * pPulseGrid =
        (RBLightGeneratorPulseGrid *)pData;
    
    free(pPulseGrid);
}


void rbLightGenerationPulseGridGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulseGrid * pPulseGrid =
        (RBLightGeneratorPulseGrid *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    float centerX = (float)(fWidth - 1) * 0.5f;
    float centerY = (float)(fHeight - 1) * 0.5f;
    RBColor const hColor = pPulseGrid->hColor;
    RBColor const vColor = pPulseGrid->vColor;
    float hSpacing;
    float vSpacing;
    
    pPulseGrid->hEnergy = 0.5f * pPulseGrid->hEnergy +
        0.5f * pAnalysis->bassEnergy;
    pPulseGrid->vEnergy = 0.5f * pPulseGrid->vEnergy +
        0.5f * pAnalysis->trebleEnergy;
    
    hSpacing = 4.0f * (1.0f + 0.5f *
        rbClampF(pPulseGrid->hEnergy, 0.0f, 4.0f));
    vSpacing = 4.0f * (1.0f + 0.5f *
        rbClampF(pPulseGrid->vEnergy, 0.0f, 4.0f));
    
    for(size_t j = 0; j < fHeight; ++j) {
        for(size_t i = 0; i < fWidth; ++i) {
            float u = fmodf(fabsf((float)i - centerX) + hSpacing * 0.75f,
                hSpacing);
            float v = fmodf(fabsf((float)j - centerY) + vSpacing * 0.75f,
                vSpacing);
            RBColor hc;
            RBColor vc;
            
            if(u < 0.5f) {
                hc = cscalef(hColor, u * 2.0f);
            }
            else if(u < 1.5f) {
                hc = hColor;
            }
            else if(u < 2.0f) {
                hc = cscalef(hColor, (2.0f - u) * 2.0f);
            }
            else {
                hc = colori(0, 0, 0, 0);
            }
            
            if(v < 1.0f) {
                vc = cscalef(vColor, v * 1.0f);
            }
            else if(v < 1.5f) {
                vc = vColor;
            }
            else if(v < 2.0f) {
                vc = cscalef(vColor, (2.0f - v) * 2.0f);
            }
            else {
                vc = colori(0, 0, 0, 0);
            }
            
            t2sett(pFrame, i, j, cadd(hc, vc));
        }
    }
}


