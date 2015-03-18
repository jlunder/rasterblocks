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

static uint8_t g_rbIconsData[][8] = {
#include "icons.h"
};

static char const * g_rbAmericanFlagData =
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "bbwbbbwbbbwbwwwwwwwwwwwwwwww"
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "bbwbbbwbbbwbwwwwwwwwwwwwwwww"
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "bbwbbbwbbbwbwwwwwwwwwwwwwwww"
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "wwwwwwwwwwwwwwwwwwwwwwwwwwww"
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrr"
    "wwwwwwwwwwwwwwwwwwwwwwwwwwww"
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrr"
    "wwwwwwwwwwwwwwwwwwwwwwwwwwww"
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrr"
    ;

static char const * g_rbSeqCircLogoData =
    "                                                                                      "
    "                                                                                      "
    "                                                   XXXX                               "
    "                 XXXXXXXXXXXXXX                  XXXXXXXXX                            "
    "              XXXXXXXXXXXXXXXXXXXXXX           XXXXXXXXXXXXX                          "
    "           XXXXXXXXXXXXXXXXXXXXXXXXXXX       XXXXXXXXXXXXXXXX                         "
    "         XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXX                        "
    "        XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX                       "
    "      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                      "
    "     XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                      "
    "    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     "
    "   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     "
    "   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXX  "
    "  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXXX "
    "  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXX "
    " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  XXXXXXXXXXXXX        XXXXXXXXXX "
    " XXXXXXXXXXXXXXXXXX            XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXX       XXXXXXXXXX "
    " XXXXXXXXXXXXXXX                  XXXXXXXXXXXXXXXXX     XXXXXXXXXXXX       XXXXXXXXXX "
    " XXXXXXXXXXXXXX                    XXXXXXXXXXXXXXX       XXXXXXXXXXX       XXXXXXXXXX "
    " XXXXXXXXXXXXX                      XXXXXXXXXXXXXX       XXXXXXXXXXXX      XXXXXXXXXX "
    "XXXXXXXXXXXXX                        XXXXXXXXXXXXX       XXXXXXXXXXXX      XXXXXXXXXXX"
    "XXXXXXXXXXXX                         XXXXXXXXXXXXX        XXXXXXXXXXX      XXXXXXXXXXX"
    "XXXXXXXXXXXX                          XXXXXXXXXXXX        XXXXXXXXXXX      XXXXXXXXXXX"
    "XXXXXXXXXXX                           XXXXXXXXXXXX        XXXXXXXXXXXX     XXXXXXXXXXX"
    "XXXXXXXXXXX                           XXXXXXXXXXXX         XXXXXXXXXXX     XXXXXXXXXXX"
    "XXXXXXXXXXX                           XXXXXXXXXXXXX        XXXXXXXXXXXX   XXXXXXXXXXXX"
    "XXXXXXXXXXX                            XXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                            XXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                           XXXXXXXXXXXXX          XXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                           XXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                           XXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXX  "
    "XXXXXXXXXXX                           XXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXXXXXXX  "
    " XXXXXXXXXX                           XXXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXX   "
    " XXXXXXXXXX                           XXXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXXXXX   "
    " XXXXXXXXXXX                         XXXXXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXX    "
    " XXXXXXXXXXX                         XXXXXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXX     "
    "  XXXXXXXXXX                         XXXXXXXXXXXXXXXXXX           XXXXXXXXXXXXXX      "
    "  XXXXXXXXXXX                        XXXXXXXXXXXXXXXXXX            XXXXXXXXXXXX       "
    "  XXXXXXXXXXX                       XXXXXXXXXXXXXXXXXXXX             XXXXXXXXX        "
    "   XXXXXXXXXXX                     XXXXXXXXXXXXXXXXXXXXXX              XXXXX          "
    "                                                                                      "
    "                                                                                      "
    "                                                                                      "
    ;

static char const * g_rbVectorHarmonyLogoData =
    "   XXXXX          XXXX    XXXX        XXXX      "
    "    XXXXX         XXXX    XXXX        XXXX      "
    "     XXXXX        XXXX    XXXX        XXXX      "
    "      XXXXX       XXXX    XXXX        XXXX      "
    "       XXXXX      XXXX    XXXX        XXXX      "
    "        XXXXX     XXXX    XXXX        XXXX      "
    "         XXXXX    XXXX    XXXXXXXXXXXXXXXX      "
    "          XXXXX   XXXX    XXXXXXXXXXXXXXXX      "
    "           XXXXX  XXXX    XXXXXXXXXXXXXXXX      "
    "            XXXXX XXXX    XXXXXXXXXXXXXXXX      "
    "             XXXXXXXXX    XXXX        XXXX      "
    "              XXXXXXXX    XXXX        XXXX      "
    "               XXXXXXX    XXXX        XXXX      "
    "                XXXXXX    XXXX        XXXX      "
    "                 XXXXX    XXXX        XXXX      "
    "                  XXXX    XXXX        XXXX      "
    ;

static RBColor const g_rbInventorLiveLogoBulbData[] = {
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,  6},{255,255,255,143},{255,255,255,219},{255,255,255,118},{255,255,255, 11},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255, 70},{255,255,255,255},{255,255,255,171},{255,255,255, 78},{255,255,255,  3},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255, 27},{255,255,255,160},{255,255,255,173},{255,255,255,138},{255,255,255, 26},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255, 50},{255,255,255,209},{255,255,255,173},{255,255,255,245},{255,255,255, 50},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255, 87},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255, 95},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,217},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,213},{255,255,255,  2},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,191},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255, 79},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,174},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,198},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,146},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255, 41},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,226},{255,255,255,254},{255,255,255,255},{255,255,255,255},{255,255,255,245},{255,255,255, 75},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {255,255,255,  9},{255,255,255, 53},{255,255,255, 82},{255,255,255, 60},{255,255,255, 19},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
};

static RBColor const g_rbInventorLiveLogoTextData[] = {
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,171},{101,246,176,137},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 15},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,252},{101,246,176,149},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 68},{101,246,176,227},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,251},{101,246,176,128},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,130},{101,246,176,249},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,231},{101,246,176,127},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,152},{101,246,176,230},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,208},{101,246,176,136},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 48},{101,246,176,239},{101,246,176, 63},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,139},{101,246,176, 27},{101,246,176, 30},{101,246,176, 64},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,176},{101,246,176,184},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,144},{101,246,176,217},{101,246,176, 11},{101,246,176, 62},{101,246,176,  5},{101,246,176, 44},{101,246,176, 66},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 63},{101,246,176, 86},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,188},{101,246,176,146},{101,246,176,  6},{101,246,176,194},{101,246,176,237},{101,246,176,254},{101,246,176,155},{101,246,176, 22},{101,246,176,234},{  0,  0,  0,  0},{101,246,176, 64},{101,246,176,255},{101,246,176,100},{101,246,176,255},{101,246,176,255},{101,246,176,135},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 99},{101,246,176,220},{101,246,176,253},{101,246,176,255},{101,246,176, 63},{101,246,176,255},{101,246,176,255},{101,246,176,255},{101,246,176,254},{101,246,176,245},{101,246,176,184},{101,246,176,255},{101,246,176,255},{101,246,176,148},{101,246,176, 36},{101,246,176, 66},{101,246,176, 79},{101,246,176,255},{101,246,176,157},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,181},{101,246,176,154},{101,246,176, 11},{101,246,176,255},{101,246,176,170},{101,246,176,177},{101,246,176,197},{101,246,176,  2},{101,246,176,255},{101,246,176, 83},{101,246,176,145},{101,246,176,230},{101,246,176,135},{101,246,176,205},{101,246,176,108},{101,246,176,211},{101,246,176,  2},{101,246,176, 30},{101,246,176,224},{101,246,176,255},{101,246,176, 60},{101,246,176,249},{101,246,176, 71},{101,246,176, 48},{101,246,176,164},{101,246,176,249},{101,246,176, 21},{101,246,176, 24},{101,246,176,234},{101,246,176,134},{101,246,176,121},{101,246,176,208},{101,246,176,114},{101,246,176,221},{101,246,176,254},{101,246,176,103},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,183},{101,246,176,161},{101,246,176, 44},{101,246,176,255},{101,246,176, 36},{101,246,176,120},{101,246,176,223},{101,246,176,  4},{101,246,176,158},{101,246,176,230},{101,246,176,218},{101,246,176, 94},{101,246,176, 85},{101,246,176,255},{101,246,176,247},{101,246,176, 87},{101,246,176, 18},{101,246,176,220},{101,246,176,246},{101,246,176,169},{101,246,176,  6},{101,246,176,236},{101,246,176,110},{  0,  0,  0,  0},{101,246,176,134},{101,246,176,249},{  0,  0,  0,  0},{101,246,176, 17},{101,246,176,246},{101,246,176, 79},{101,246,176,126},{101,246,176,200},{101,246,176, 60},{101,246,176,255},{101,246,176,227},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,181},{101,246,176,193},{101,246,176, 52},{101,246,176,255},{  0,  0,  0,  0},{101,246,176, 56},{101,246,176,255},{101,246,176, 11},{101,246,176, 63},{101,246,176,255},{101,246,176,241},{243, 44,140,156},{243, 44,140, 42},{101,246,176,166},{101,246,176,248},{101,246,176,137},{101,246,176,245},{101,246,176, 81},{101,246,176,255},{101,246,176,123},{  0,  0,  0,  0},{101,246,176,220},{101,246,176,171},{  0,  0,  0,  0},{101,246,176,129},{101,246,176,251},{  0,  0,  0,  0},{101,246,176,  6},{101,246,176,221},{101,246,176,238},{101,246,176,255},{101,246,176,107},{  0,  0,  0,  0},{101,246,176,255},{101,246,176,114},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 54},{101,246,176,108},{101,246,176,  1},{101,246,176, 58},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,152},{101,246,176,  8},{  0,  0,  0,  0},{101,246,176,132},{101,246,176, 71},{243, 44,140,255},{243, 44,140, 50},{  0,  0,  0,  0},{101,246,176,114},{101,246,176,179},{101,246,176, 70},{  0,  0,  0,  0},{101,246,176, 60},{101,246,176, 18},{  0,  0,  0,  0},{101,246,176, 76},{101,246,176,122},{  0,  0,  0,  0},{101,246,176, 55},{101,246,176,166},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176, 43},{101,246,176,171},{101,246,176,107},{  0,  0,  0,  0},{  0,  0,  0,  0},{101,246,176,108},{101,246,176,  6},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140, 11},{243, 44,140,255},{243, 44,140, 28},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140,183},{243, 44,140, 90},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140, 15},{243, 44,140,251},{243, 44,140, 20},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140, 39},{243, 44,140, 16},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140, 15},{243, 44,140, 47},{  0,  0,  0,  0},{243, 44,140, 14},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140, 22},{243, 44,140,250},{243, 44,140, 21},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140,181},{243, 44,140, 83},{243, 44,140,232},{243, 44,140,  1},{243, 44,140,148},{243, 44,140,110},{243, 44,140,238},{243, 44,140,255},{243, 44,140, 41},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140, 21},{243, 44,140,252},{243, 44,140, 11},{  0,  0,  0,  0},{243, 44,140, 41},{243, 44,140, 35},{243, 44,140,185},{243, 44,140, 87},{243, 44,140,230},{243, 44,140, 67},{243, 44,140,244},{243, 44,140, 57},{243, 44,140,224},{243, 44,140,200},{243, 44,140,103},{243, 44,140, 31},{243, 44,140, 56},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140,255},{243, 44,140,240},{243, 44,140,255},{243, 44,140,249},{243, 44,140,243},{243, 44,140,253},{243, 44,140, 93},{243, 44,140, 74},{243, 44,140,250},{243, 44,140,208},{  0,  0,  0,  0},{243, 44,140,239},{243, 44,140,167},{243, 44,140, 27},{243, 44,140,242},{243, 44,140, 26},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
    {  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{243, 44,140,124},{243, 44,140, 92},{243, 44,140, 42},{243, 44,140, 21},{243, 44,140,  5},{243, 44,140,113},{243, 44,140, 60},{243, 44,140,  3},{243, 44,140,165},{243, 44,140, 44},{  0,  0,  0,  0},{243, 44,140, 25},{243, 44,140,158},{243, 44,140,160},{243, 44,140, 32},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},{  0,  0,  0,  0},
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

RBTexture2 * g_rbPAmericanFlagTex = NULL;
RBTexture2 * g_rbPSeqCircLogoTex = NULL;
RBTexture2 * g_rbPSeqCircLogoTex16x8 = NULL;
RBTexture2 * g_rbPSeqCircLogoTex24x12 = NULL;
RBTexture2 * g_rbPSeqCircLogoTex32x16 = NULL;
RBTexture2 * g_rbPVectorHarmonyLogoTex48x16 = NULL;
RBTexture2 * g_rbPVectorHarmonyLogoTex24x8 = NULL;
RBTexture2 * g_rbPInventorLiveLogoBulbTex40x16 = NULL;
RBTexture2 * g_rbPInventorLiveLogoTextTex40x16 = NULL;

RBTexture2 * g_rbPIconTexs[LENGTHOF(g_rbIconsData)];

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
#undef MAKE_PAL
    
    for(size_t k = 0; k < LENGTHOF(g_rbIconsData); ++k) {
        g_rbPIconTexs[k] = rbTexture2Alloc(8, 8);
        
        for(size_t j = 0; j < 8; ++j) {
            for(size_t i = 0; i < 8; ++i) {
                t2sett(g_rbPIconTexs[k], i, j,
                    ((g_rbIconsData[k][j] >> i) & 1) != 0 ?
                        colori(255, 255, 255, 255) : colori(0, 0, 0, 0));
            }
        }
    }
    
    g_rbPAmericanFlagTex = rbTexture2Alloc(28, 13);
    for(size_t j = 0; j < t2geth(g_rbPAmericanFlagTex); ++j) {
        for(size_t i = 0; i < t2getw(g_rbPAmericanFlagTex); ++i) {
            RBColor c;
            switch(g_rbAmericanFlagData[j * t2getw(g_rbPAmericanFlagTex) + i]) {
            case 'r': c = colori(63, 0, 0, 255); break;
            case 'w': c = colori(63, 63, 63, 255); break;
            case 'b': c = colori(0, 0, 63, 255); break;
            default: c = colori(0, 0, 0, 255); break;
            }
            t2sett(g_rbPAmericanFlagTex, i, j, c);
        }
    }
    
    g_rbPSeqCircLogoTex = rbTexture2Alloc(86, 43);
    for(size_t j = 0; j < t2geth(g_rbPSeqCircLogoTex); ++j) {
        for(size_t i = 0; i < t2getw(g_rbPSeqCircLogoTex); ++i) {
            RBColor c;
            switch(g_rbSeqCircLogoData[j * t2getw(g_rbPSeqCircLogoTex) + i]) {
            case ' ': c = colori(0, 0, 0, 0); break;
            default: c = colori(255, 255, 255, 255); break;
            }
            t2sett(g_rbPSeqCircLogoTex, i, j, c);
        }
    }
    
    g_rbPSeqCircLogoTex16x8 = rbTexture2Alloc(16, 8);
    rbTexture2Rescale(g_rbPSeqCircLogoTex16x8, g_rbPSeqCircLogoTex);
    g_rbPSeqCircLogoTex24x12 = rbTexture2Alloc(24, 12);
    rbTexture2Rescale(g_rbPSeqCircLogoTex24x12, g_rbPSeqCircLogoTex);
    g_rbPSeqCircLogoTex32x16 = rbTexture2Alloc(32, 16);
    rbTexture2Rescale(g_rbPSeqCircLogoTex32x16, g_rbPSeqCircLogoTex);
    
    g_rbPVectorHarmonyLogoTex48x16 = rbTexture2Alloc(48, 16);
    for(size_t j = 0; j < t2geth(g_rbPVectorHarmonyLogoTex48x16); ++j) {
        for(size_t i = 0; i < t2getw(g_rbPVectorHarmonyLogoTex48x16); ++i) {
            RBColor c;
            switch(g_rbVectorHarmonyLogoData[
                    j * t2getw(g_rbPVectorHarmonyLogoTex48x16) + i]) {
            case ' ': c = colori(0, 0, 0, 0); break;
            default: c = colori(255, 255, 255, 255); break;
            }
            t2sett(g_rbPVectorHarmonyLogoTex48x16, i, j, c);
        }
    }
    
    g_rbPVectorHarmonyLogoTex24x8 = rbTexture2Alloc(24, 8);
    rbTexture2Rescale(g_rbPVectorHarmonyLogoTex24x8,
        g_rbPVectorHarmonyLogoTex48x16);
    
    g_rbPInventorLiveLogoBulbTex40x16 = rbTexture2Alloc(40, 16);
    g_rbPInventorLiveLogoTextTex40x16 = rbTexture2Alloc(40, 16);
    for(size_t j = 0; j < 16; ++j) {
        for(size_t i = 0; i < 40; ++i) {
            t2sett(g_rbPInventorLiveLogoBulbTex40x16, i, j,
                colorct(rbColorTempPremultiplyAlpha(colortempc(
                    g_rbInventorLiveLogoBulbData[i + j * 40]))));
            t2sett(g_rbPInventorLiveLogoTextTex40x16, i, j,
                colorct(rbColorTempPremultiplyAlpha(colortempc(
                    g_rbInventorLiveLogoTextData[i + j * 40]))));
        }
    }
    
    {
        RBColor yellowGreen = colori(127, 191, 0, 255);
        RBLightGenerator * pGenerator;
    
        switch(pConfig->mode) {
        default:
        case 0:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationPlasmaAlloc(g_rbPBlackPurpleHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 1:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPBlackRedGoldHSPalTex,
                    g_rbPBluePurpleGreenHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 2:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVerticalBarsAlloc(
                    g_rbPRedPinkHSPalTex, g_rbPBlackWhiteHSPalTex,
                    40, rbTimeFromMs(10), rbTimeFromMs(250)),
                rbLightGenerationSignalLissajousAlloc(
                    colori(127, 63, 0, 0)));
            break;
        case 3:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationPulsePlasmaAlloc(
                    g_rbPBluePurpleGreenHSPalTex),
                rbLightGenerationPulseCheckerboardAlloc(yellowGreen));
            break;
        case 4:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 5:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 6:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 7:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 8:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 9:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 10:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 11:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        case 12:
            pGenerator = rbLightGenerationCompositor2Alloc(
                rbLightGenerationVolumeBarsAlloc(
                    g_rbPGreenLavenderHSPalTex,
                    g_rbPGreenLavenderHSPalTex),
                rbLightGenerationOscilloscopeAlloc(yellowGreen));
            break;
        }
        rbLightGenerationSetGenerator(pGenerator);
    }
    //rbLightGenerationInitializeGenerators();
}


/*
void rbLightGenerationInitializeGenerators(void)
{
    RBLightGenerator * pGenerators[] = {
        rbLightGenerationCreateGeneratorsFromTheme(
            g_rbPGreenLavenderHSPalTex,
            g_rbPGreenLavenderHSPalTex,
            //g_rbPBlackWhiteHSPalTex,
            g_rbPBlackGoldFSAlphaPalTex,
            colori(127, 191, 0, 255)),
        rbLightGenerationCreateGeneratorsFromTheme(
            g_rbPRedPinkHSPalTex,
            g_rbPBlueGoldHSPalTex,
            g_rbPBlackGoldFSAlphaPalTex,
            colori(127, 191, 0, 255)),
        rbLightGenerationCreateVUGeneratorsFromTheme(
            g_rbPBlackRedGoldHSPalTex,
            g_rbPBluePurpleGreenHSPalTex,
            g_rbPBlackGoldFSAlphaPalTex,
            colori(127, 191, 0, 255)),
    };
    
    UNUSED(pGenerators);
    rbLightGenerationSetGenerator(
        rbLightGenerationTimedRotationAlloc(pGenerators,
            LENGTHOF(pGenerators), rbTimeFromMs(180000))
        //rbLightGenerationCompositor2Alloc(
        //    rbLightGenerationPulsePlasmaAlloc(g_rbPGreenLavenderHSPalTex),
        //    rbLightGenerationOscilloscopeAlloc(colori(127, 191, 0, 255)))
        );
}


RBLightGenerator * rbLightGenerationCreateGeneratorsFromTheme(
    RBTexture1 * pBassPalTex, RBTexture1 * pTreblePalTex,
    RBTexture1 * pFGPalTex, RBColor fgColor)
{
    RBLightGenerator * pGenerators[] = {
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationVerticalBarsAlloc(pBassPalTex, pTreblePalTex,
                40, rbTimeFromMs(10), rbTimeFromMs(250)),
            rbLightGenerationOscilloscopeAlloc(fgColor)),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationVerticalBarsAlloc(pBassPalTex, pTreblePalTex,
                20, rbTimeFromMs(15), rbTimeFromMs(100)),
            rbLightGenerationSignalLissajousAlloc(fgColor)),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationVerticalBarsAlloc(pBassPalTex, pTreblePalTex,
                20, rbTimeFromMs(15), rbTimeFromMs(100)),
            rbLightGenerationDashedCirclesAlloc(pFGPalTex)),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationPulsePlasmaAlloc(pBassPalTex),
            rbLightGenerationOscilloscopeAlloc(fgColor)),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationPulsePlasmaAlloc(pBassPalTex),
            rbLightGenerationPulseCheckerboardAlloc(colori(63, 63, 63, 0))),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationDashedCirclesAlloc(pBassPalTex),
            rbLightGenerationPulseCheckerboardAlloc(colori(63, 63, 63, 0))),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationDashedCirclesAlloc(pBassPalTex),
            rbLightGenerationOscilloscopeAlloc(fgColor)),
    };
    
    return rbLightGenerationTimedRotationAlloc(pGenerators,
        LENGTHOF(pGenerators), rbTimeFromMs(60000));
}


RBLightGenerator * rbLightGenerationCreateVUGeneratorsFromTheme(
    RBTexture1 * pBassPalTex, RBTexture1 * pTreblePalTex,
    RBTexture1 * pFGPalTex, RBColor fgColor)
{
    RBLightGenerator * pGenerators[] = {
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationVolumeBarsAlloc(pBassPalTex, pTreblePalTex),
            rbLightGenerationSignalLissajousAlloc(fgColor)),
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationVolumeBarsAlloc(pBassPalTex, pTreblePalTex),
            rbLightGenerationOscilloscopeAlloc(fgColor)),
    };
    UNUSED(pFGPalTex);
    
    return rbLightGenerationTimedRotationAlloc(pGenerators,
        LENGTHOF(pGenerators), rbTimeFromMs(60000));
}
*/


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
    
    for(size_t k = 0; k < LENGTHOF(g_rbIconsData); ++k) {
        FREE_TEX2(g_rbPIconTexs[k]);
    }
    FREE_TEX2(g_rbPAmericanFlagTex);
    FREE_TEX2(g_rbPSeqCircLogoTex);
    FREE_TEX2(g_rbPSeqCircLogoTex16x8);
    FREE_TEX2(g_rbPSeqCircLogoTex24x12);
    FREE_TEX2(g_rbPSeqCircLogoTex32x16);
    FREE_TEX2(g_rbPVectorHarmonyLogoTex48x16);
    FREE_TEX2(g_rbPVectorHarmonyLogoTex24x8);
    FREE_TEX2(g_rbPInventorLiveLogoBulbTex40x16);
    FREE_TEX2(g_rbPInventorLiveLogoTextTex40x16);
#undef FREE_TEX1
#undef FREE_TEX2
}


void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBControls * pControls, RBTexture2 * pFrame)
{
    UNUSED(pControls);
    
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
