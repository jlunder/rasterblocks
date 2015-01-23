#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
    RBTimer flashTimer;
} RBLightGeneratorPulseCheckerboard;


#define RB_PULSE_CHECKERBOARD_FLASH_TIME_MS 200

void rbLightGenerationPulseCheckerboardFree(void * pData);
void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(RBColor color)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)malloc(
            sizeof (RBLightGeneratorPulseCheckerboard));
    
    pPulseCheckerboard->genDef.pData = pPulseCheckerboard;
    pPulseCheckerboard->genDef.free = rbLightGenerationPulseCheckerboardFree;
    pPulseCheckerboard->genDef.generate = rbLightGenerationPulseCheckerboardGenerate;
    pPulseCheckerboard->color = color;
    rbStopTimer(&pPulseCheckerboard->flashTimer);
    
    return &pPulseCheckerboard->genDef;
}


void rbLightGenerationPulseCheckerboardFree(void * pData)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)pData;
    
    free(pPulseCheckerboard);
}


void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBTime const flashTime = rbTimeFromMs(RB_PULSE_CHECKERBOARD_FLASH_TIME_MS);
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)pData;
    float energy = pAnalysis->bassEnergy;//logf(pAnalysis->bassEnergy * (0.75f * RB_E)) * 0.5f + 1.0f;
    float squareSize =
        rbClampF(energy, 0.15f, 1.0f) * 4.0f;
    RBColor checkerboardColor = pPulseCheckerboard->color;
    
    if(pAnalysis->peakDetected) {
        rbStartTimer(&pPulseCheckerboard->flashTimer, flashTime);
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            bool inSquare = !(((i / 8) ^ (j / 8)) & 1);
            RBColor c = colori(0, 0, 0, 0);
            
            if(inSquare) {
                float x = fabsf(3.5f - (i % 8));
                float y = fabsf(3.5f - (j % 8));
                float p = squareSize - (x > y ? x : y);
                float a = (float)rbGetTimeLeft(
                    &pPulseCheckerboard->flashTimer) / (float)flashTime;
                
                a = a * a;
                
                if(p < 0) {
                }
                else if(p < 0.5f) {
                    a += p * 2.0f;
                }
                else if(p < 1.5f) {
                    a = 1.0f;
                }
                else if(p < 2.0f) {
                    a += (2.0f - p) * 2.0f;
                }
                c = cscalef(checkerboardColor, rbClampF(a, 0.0f, 1.0f));
            }
            t2sett(pFrame, i, j, c);
        }
    }
}


