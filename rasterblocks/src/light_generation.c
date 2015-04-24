#include "light_generation.h"

#include "graphics_util.h"


static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldWhiteFS[] = {
    {{  0,   0,   0, 255}, 2},
    {{ 63,   0,   0, 255}, 2},
    {{127,  15,   0, 255}, 2},
    {{255,  63,   0, 255}, 1},
    {{255, 127,   0, 255}, 1},
    {{255, 255,   0, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldWhiteFSAlpha[] = {
    {{  0,   0,   0,   0}, 2},
    {{ 63,   0,   0,  57}, 2},
    {{127,  15,   0, 113}, 2},
    {{255,  63,   0, 170}, 1},
    {{255, 127,   0, 198}, 1},
    {{255, 255,   0, 227}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackBlueGreenWhiteFS[] = {
    {{  0,   0,   0, 255}, 1},
    {{  0,  63, 255, 255}, 1},
    {{127,   0,  63, 255}, 1},
    {{  0, 255,  63, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackBlueGreenWhiteFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{  0,  63, 255,  64}, 1},
    {{127,   0,  63, 128}, 1},
    {{  0, 255,  63, 192}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteFS[] = {
    {{  0,   0,   0, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalRainbowFS[] = {
    {{255,   0,   0, 255}, 1},
    {{  0, 255,   0, 255}, 1},
    {{  0,   0, 255, 255}, 1},
    {{255,   0,   0, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackGoldFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{255, 255,  63, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPaleBlueFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{ 63,  63, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPaleGreenFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{ 63, 255,  63, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{127,   0, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalGreenLavenderHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{  7,  31,   7, 255}, 1},
    {{ 95,  31, 127, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleRedHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{ 31,   0,  63, 255}, 1},
    {{127,  15,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{ 15,   0,  31, 255}, 1},
    {{ 95,   0, 127, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBluePurpleGreenHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{  0,  31, 127, 255}, 1},
    {{ 63,   0,  31, 255}, 1},
    {{  0, 127,  31, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlueGoldHS[] = {
    {{  0,  15,  31, 255}, 1},
    {{  0,  63, 127, 255}, 1},
    {{127, 127,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalRedPinkHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{ 91,   0,   0, 255}, 1},
    {{127,  63,  63, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldHS[] = {
    {{  0,   0,   0, 255}, 2},
    {{ 31,   0,   0, 255}, 2},
    {{ 63,   7,   0, 255}, 2},
    {{127,  31,   0, 255}, 1},
    {{127,  63,   0, 255}, 1},
    {{127, 127,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{127, 127, 127, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalImageOverlay[] = {
    {{128, 128, 128, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBColor const g_rbSfuCsSurreyData[] = {
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{254,254,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{254,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 54, 58,255},{253,253,254,255},{253,253,254,255},{183, 52, 57,255},{253,253,254,255},{253,253,254,255},{253,253,254,255},{182, 52, 58,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{184, 53, 57,255},{253,253,254,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{182, 54, 56,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,255,255},{181, 53, 58,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{181, 54, 56,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{254,252,251,255},{183, 52, 57,255},{183, 52, 57,255},{254,255,255,255},{255,254,254,255},{253,253,254,255},{185, 53, 57,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{179, 53, 58,255},{253,253,254,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{184, 52, 57,255},{184, 52, 56,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 53, 58,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{184, 52, 57,255},{253,253,254,255},{253,254,251,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{183, 52, 57,255},{253,253,254,255},{253,253,254,255},{183, 52, 57,255},{183, 52, 57,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{254,253,254,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{184, 52, 56,255},{254,254,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    {255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{246,246,246,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{250,250,250,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},
    { 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 94, 94, 94,255},{255,255,255,255},{104,104,104,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 84, 84, 84,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{252,252,252,255},{255,255,255,255},{254,254,254,255},{253,253,253,255},{251,251,251,255},{247,247,247,255},{252,252,252,255},{248,248,248,255},{254,254,254,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{251,251,251,255},{255,255,255,255},{255,255,255,255},{251,251,251,255},
    { 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{248,248,248,255},{253,253,253,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{248,248,248,255},{255,255,255,255},{255,255,255,255},{ 98, 98, 98,255},{ 90, 90, 90,255},{255,255,255,255},{251,251,251,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 95, 95, 95,255},{104,104,104,255},{ 90, 90, 90,255},{ 92, 92, 92,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},
    { 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 84, 84, 84,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},
    { 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{105,105,105,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{251,251,251,255},{247,247,247,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{248,248,248,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},
    {255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{243,243,243,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{ 90, 90, 90,255},{255,255,255,255},{255,255,255,255},
};

RBTexture1 * g_rbPBlackRedGoldWhiteFSPalTex = NULL;
RBTexture1 * g_rbPBlackRedGoldWhiteFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackBlueGreenWhiteFSPalTex = NULL;
RBTexture1 * g_rbPBlackBlueGreenWhiteFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackWhiteFSPalTex = NULL;
RBTexture1 * g_rbPBlackWhiteFSAlphaPalTex = NULL;
// This palette is meant to be used with repeat sampling
RBTexture1 * g_rbPRainbowFSPalTex = NULL;
RBTexture1 * g_rbPBlackGoldFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackPaleBlueFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackPaleGreenFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackPurpleFSAlphaPalTex = NULL;
RBTexture1 * g_rbPGreenLavenderHSPalTex = NULL;
RBTexture1 * g_rbPBlackPurpleRedHSPalTex = NULL;
RBTexture1 * g_rbPBlackPurpleHSPalTex = NULL;
RBTexture1 * g_rbPBluePurpleGreenHSPalTex = NULL;
RBTexture1 * g_rbPBlueGoldHSPalTex = NULL;
RBTexture1 * g_rbPRedPinkHSPalTex = NULL;
RBTexture1 * g_rbPBlackRedGoldHSPalTex = NULL;
RBTexture1 * g_rbPBlackWhiteHSPalTex = NULL;
RBTexture1 * g_rbPImageOverlayPalTex = NULL;

RBTexture2 * g_rbPSfuCsSurreyTex = NULL;

static RBLightGenerator * g_rbPCurrentGenerator = NULL;


/*
static void rbLightGenerationInitializeGenerators(void);
static RBLightGenerator * rbLightGenerationCreateGeneratorsFromTheme(
    RBTexture1 * pBassPalTex, RBTexture1 * pTreblePalTex,
    RBTexture1 * pFGPalTex, RBColor fgColor);
static RBLightGenerator * rbLightGenerationCreateVUGeneratorsFromTheme(
    RBTexture1 * pBassPalTex, RBTexture1 * pTreblePalTex,
    RBTexture1 * pFGPalTex, RBColor fgColor);
*/
static RBLightGenerator * rbLightGenerationCreateForegroundGenerators(
    RBColor fgColor);


void rbLightGenerationInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbLightGenerationShutdown();

#define MAKE_PAL(name) \
    do { \
        g_rbP##name##PalTex = rbTexture1Alloc(256); \
        rbTexture1FillFromPiecewiseLinear(g_rbP##name##PalTex, \
            g_rbPal##name, LENGTHOF(g_rbPal##name), false); \
    } while(false)
    
    MAKE_PAL(BlackRedGoldWhiteFS);
    MAKE_PAL(BlackRedGoldWhiteFSAlpha);
    MAKE_PAL(BlackBlueGreenWhiteFS);
    MAKE_PAL(BlackBlueGreenWhiteFSAlpha);
    MAKE_PAL(BlackWhiteFS);
    MAKE_PAL(BlackWhiteFSAlpha);
    //MAKE_PAL(RainbowFS);
    g_rbPRainbowFSPalTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPRainbowFSPalTex, g_rbPalRainbowFS,
        LENGTHOF(g_rbPalRainbowFS), true);
    MAKE_PAL(BlackGoldFSAlpha);
    MAKE_PAL(BlackPaleBlueFSAlpha);
    MAKE_PAL(BlackPaleGreenFSAlpha);
    MAKE_PAL(BlackPurpleFSAlpha);
    MAKE_PAL(GreenLavenderHS);
    MAKE_PAL(BlackPurpleRedHS);
    MAKE_PAL(BlackPurpleHS);
    MAKE_PAL(BluePurpleGreenHS);
    MAKE_PAL(BlueGoldHS);
    MAKE_PAL(RedPinkHS);
    MAKE_PAL(BlackRedGoldHS);
    MAKE_PAL(BlackWhiteHS);
    MAKE_PAL(ImageOverlay);
#undef MAKE_PAL
    
    g_rbPSfuCsSurreyTex = rbTexture2Alloc(32, 24);
    for(size_t j = 0; j < 24; ++j) {
        for(size_t i = 0; i < 32; ++i) {
            t2sett(g_rbPSfuCsSurreyTex, i, j,
                colorct(rbColorTempPremultiplyAlpha(colortempc(
                    g_rbSfuCsSurreyData[i + j * 32]))));
        }
    }
    
    {
        RBColor yellow = colori(127, 127, 0, 127);
        RBColor orange = colori(127, 63, 0, 127);
        RBColor white = colori(127, 127, 127, 127);
        RBColor pink = colori(127, 63, 63, 127);
        RBColor black = colori(0, 0, 0, 255);
        RBLightGenerator * pBackgroundGenerators[] = {
            rbLightGenerationImageFilterAlloc(
                rbLightGenerationPlasmaAlloc(g_rbPImageOverlayPalTex),
                g_rbPSfuCsSurreyTex),
            rbLightGenerationVolumeBarsAlloc(
                g_rbPBlackRedGoldHSPalTex,
                g_rbPBluePurpleGreenHSPalTex),
            rbLightGenerationVolumeBarsAlloc(
                g_rbPGreenLavenderHSPalTex,
                g_rbPGreenLavenderHSPalTex),
            rbLightGenerationVerticalBarsAlloc(
                g_rbPRedPinkHSPalTex, g_rbPBlackWhiteHSPalTex,
                40, rbTimeFromMs(10), rbTimeFromMs(250)),
            rbLightGenerationPulsePlasmaAlloc(
                g_rbPBluePurpleGreenHSPalTex),
        };
        RBLightGenerator * pFGColorGenerators[] = {
            rbLightGenerationCreateForegroundGenerators(yellow),
            rbLightGenerationCreateForegroundGenerators(orange),
            rbLightGenerationCreateForegroundGenerators(white),
            rbLightGenerationCreateForegroundGenerators(pink),
            rbLightGenerationCreateForegroundGenerators(black),
        };
        RBLightGenerator * pGenerators[] = {
            rbLightGenerationCompositor2Alloc(
                rbLightGenerationControllerSelectAlloc(pBackgroundGenerators,
                    LENGTHOF(pBackgroundGenerators), 1),
                rbLightGenerationControllerSelectAlloc(pFGColorGenerators,
                    LENGTHOF(pFGColorGenerators), 3)),
        };
        rbLightGenerationSetGenerator(
            rbLightGenerationControllerSelectAlloc(pGenerators,
                LENGTHOF(pGenerators), 0)
            );
    }
}


static RBLightGenerator * rbLightGenerationCreateForegroundGenerators(
    RBColor fgColor)
{
    RBLightGenerator * pForegroundGenerators[] = {
        rbLightGenerationSignalLissajousAlloc(fgColor),
        rbLightGenerationPulseCheckerboardAlloc(fgColor),
        rbLightGenerationOscilloscopeAlloc(fgColor),
    };
    
    return rbLightGenerationControllerSelectAlloc(pForegroundGenerators,
        LENGTHOF(pForegroundGenerators), 4);
}


void rbLightGenerationShutdown(void)
{
    rbLightGenerationSetGenerator(NULL);
#define FREE_TEX1(tex) \
    do { \
        if(tex != NULL) { \
            rbTexture1Free(tex); \
            tex = NULL; \
        } \
    } while(false)
#define FREE_TEX2(tex) \
    do { \
        if(tex != NULL) { \
            rbTexture2Free(tex); \
            tex = NULL; \
        } \
    } while(false)
    
    FREE_TEX1(g_rbPBlackRedGoldWhiteFSPalTex);
    FREE_TEX1(g_rbPBlackRedGoldWhiteFSAlphaPalTex);
    FREE_TEX1(g_rbPBlackBlueGreenWhiteFSPalTex);
    FREE_TEX1(g_rbPBlackBlueGreenWhiteFSAlphaPalTex);
    FREE_TEX1(g_rbPBlackWhiteFSPalTex);
    FREE_TEX1(g_rbPBlackWhiteFSAlphaPalTex);
    FREE_TEX1(g_rbPRainbowFSPalTex);
    FREE_TEX1(g_rbPBlackGoldFSAlphaPalTex);
    FREE_TEX1(g_rbPBlackPaleBlueFSAlphaPalTex);
    FREE_TEX1(g_rbPBlackPaleGreenFSAlphaPalTex);
    FREE_TEX1(g_rbPBlackPurpleFSAlphaPalTex);
    FREE_TEX1(g_rbPGreenLavenderHSPalTex);
    FREE_TEX1(g_rbPBlackPurpleRedHSPalTex);
    FREE_TEX1(g_rbPBlackPurpleHSPalTex);
    FREE_TEX1(g_rbPBluePurpleGreenHSPalTex);
    FREE_TEX1(g_rbPBlueGoldHSPalTex);
    FREE_TEX1(g_rbPRedPinkHSPalTex);
    FREE_TEX1(g_rbPBlackRedGoldHSPalTex);
    FREE_TEX1(g_rbPBlackWhiteHSPalTex);
    
    FREE_TEX2(g_rbPSfuCsSurreyTex);
#undef FREE_TEX1
#undef FREE_TEX2
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
    if(g_rbPCurrentGenerator != NULL) {
        rbLightGenerationGeneratorFree(g_rbPCurrentGenerator);
    }
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
