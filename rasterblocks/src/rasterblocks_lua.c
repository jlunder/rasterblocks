#include "rasterblocks_lua.h"

#include "graphics_util.h"
#include "light_generation.h"
#include "parameter_generation.h"
#include "tle.h"


typedef RBLightGenerator * PRBLightGenerator;


static int tle_RBColor_metatable_index;
static int tle_RBTime_metatable_index;
static int tle_RBTexture1_metatable_index;
static int tle_RBTexture2_metatable_index;
static int tle_PRBLightGenerator_metatable_index;
static int rbLuaCurrentGeneratorIndex;

static RBParameterGenerator g_rbLuaParameterGenerator;

static RBAnalyzedAudio const * g_rbLuaPCurrentAnalysis;
static float * g_rbLuaPCurrentParameters;
static size_t g_rbLuaCurrentNumParameters;


static int rbLuaRegisterTypes(lua_State * l);

static void rbLuaParametersGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis,
    float * pParameters, size_t numParameters);

static int rbLuaRunLuaParameterGeneration(lua_State * l);

static int rbLuaInfo(lua_State * l);
static int rbLuaWarning(lua_State * l);

static int rbLuaColor(lua_State * l);

static int rbLuaTime(lua_State * l);

static int rbLuaPaletteFromPwl(lua_State * l);
static int rbLuaTexture1Alloc(lua_State * l);
static int rbLuaTexture2Alloc(lua_State * l);

static int rbLuaLightGenerationSetGenerator(lua_State * l);

static int rbLuaLightGenerationCompositorAlloc(lua_State * l);
static int rbLuaLightGenerationCompositorAddLayer(lua_State * l);
static int rbLuaLightGenerationDashedCirclesAlloc(lua_State * l);
static int rbLuaLightGenerationFillAlloc(lua_State * l);
static int rbLuaLightGenerationOscilloscopeAlloc(lua_State * l);
static int rbLuaLightGenerationPlasmaAlloc(lua_State * l);
static int rbLuaLightGenerationSelectorAlloc(lua_State * l);
static int rbLuaLightGenerationSignalLissajousAlloc(lua_State * l);
static int rbLuaLightGenerationStaticImageAlloc(lua_State * l);
static int rbLuaLightGenerationVerticalBarsAlloc(lua_State * l);

static bool tle_is_RBColor(lua_State * l, int idx);
static RBColor tle_to_RBColor(lua_State * l, int idx);
static void tle_push_RBColor(lua_State * l, RBColor value);
static bool tle_eq_RBColor_RBColor(lua_State * l, RBColor x, RBColor y);

static int tle_RBColor_eq(lua_State * l);

static bool tle_is_RBTime(lua_State * l, int idx);
static RBTime tle_to_RBTime(lua_State * l, int idx);
static void tle_push_RBTime(lua_State * l, RBTime value);
static bool tle_eq_RBTime_RBTime(lua_State * l, RBTime x, RBTime y);
static bool tle_lt_RBTime_RBTime(lua_State * l, RBTime x, RBTime y);
static bool tle_le_RBTime_RBTime(lua_State * l, RBTime x, RBTime y);

static int tle_RBTime_eq(lua_State * l);
static int tle_RBTime_lt(lua_State * l);
static int tle_RBTime_le(lua_State * l);

static bool tle_is_RBTexture1(lua_State * l, int idx);
static RBTexture1 * tle_to_RBTexture1(lua_State * l, int idx);
static void tle_push_RBTexture1(lua_State * l, RBTexture1 * value);

static int rbLuaTexture1SampleNearestRepeat(lua_State * l);
static int rbLuaTexture1SampleNearestClamp(lua_State * l);
static int rbLuaTexture1SampleLinearRepeat(lua_State * l);
static int rbLuaTexture1SampleLinearClamp(lua_State * l);
static int rbLuaTexture1GetTexel(lua_State * l);
static int rbLuaTexture1SetTexel(lua_State * l);

static bool tle_is_RBTexture2(lua_State * l, int idx);
static RBTexture2 * tle_to_RBTexture2(lua_State * l, int idx);
static void tle_push_RBTexture2(lua_State * l, RBTexture2 * value);

static int rbLuaTexture2SampleNearestRepeat(lua_State * l);
static int rbLuaTexture2SampleNearestClamp(lua_State * l);
static int rbLuaTexture2SampleLinearRepeat(lua_State * l);
static int rbLuaTexture2SampleLinearClamp(lua_State * l);
static int rbLuaTexture2GetTexel(lua_State * l);
static int rbLuaTexture2SetTexel(lua_State * l);

static bool tle_is_PRBLightGenerator(lua_State * l, int idx);
static RBLightGenerator * tle_to_PRBLightGenerator(lua_State * l, int idx);
static void tle_push_PRBLightGenerator(lua_State * l, RBLightGenerator * value);

static int tle_PRBLightGenerator_gc(lua_State * l);

void rbLuaInitialize(RBConfiguration * pConfig)
{
    tle_initialize(pConfig->luaPath);
    lua_pushcfunction(tle_state, rbLuaRegisterTypes);
    rbVerify(tle_pcall(tle_state, 0, 0, false) == 0);
    tle_dostring(tle_state, "include('display.lua')", true);
    
    (void)tle_push_RBColor;
    (void)tle_push_RBTime;
    (void)tle_is_RBTexture1;
    (void)tle_to_RBTexture1;
    (void)tle_push_RBTexture1;
    (void)tle_is_RBTexture2;
    (void)tle_to_RBTexture2;
    (void)tle_push_RBTexture2;
    
    g_rbLuaParameterGenerator.free = NULL;
    g_rbLuaParameterGenerator.generate = rbLuaParametersGenerate;
    
    rbParameterGenerationSetGenerator(&g_rbLuaParameterGenerator, 0,
        RB_NUM_PARAMETERS);
}


int rbLuaRegisterTypes(lua_State * l)
{
    int top = lua_gettop(l);

    lua_newtable(l);
    lua_pushcfunction(l, tle_RBColor_eq);
    lua_setfield(l, -2, "__eq");
    tle_RBColor_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, tle_RBTime_eq);
    lua_setfield(l, -2, "__eq");
    lua_pushcfunction(l, tle_RBTime_lt);
    lua_setfield(l, -2, "__lt");
    lua_pushcfunction(l, tle_RBTime_le);
    lua_setfield(l, -2, "__le");
    tle_RBTime_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, rbLuaTexture1SampleNearestRepeat);
    lua_setfield(l, -2, "sampnr");
    lua_pushcfunction(l, rbLuaTexture1SampleNearestClamp);
    lua_setfield(l, -2, "sampnc");
    lua_pushcfunction(l, rbLuaTexture1SampleLinearRepeat);
    lua_setfield(l, -2, "samplr");
    lua_pushcfunction(l, rbLuaTexture1SampleLinearClamp);
    lua_setfield(l, -2, "samplc");
    lua_pushcfunction(l, rbLuaTexture1GetTexel);
    lua_setfield(l, -2, "gett");
    lua_pushcfunction(l, rbLuaTexture1SetTexel);
    lua_setfield(l, -2, "sett");
    lua_pushvalue(l, -1);
    lua_setfield(l, -2, "__index");
    tle_RBTexture1_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, rbLuaTexture2SampleNearestRepeat);
    lua_setfield(l, -2, "sampnr");
    lua_pushcfunction(l, rbLuaTexture2SampleNearestClamp);
    lua_setfield(l, -2, "sampnc");
    lua_pushcfunction(l, rbLuaTexture2SampleLinearRepeat);
    lua_setfield(l, -2, "samplr");
    lua_pushcfunction(l, rbLuaTexture2SampleLinearClamp);
    lua_setfield(l, -2, "samplc");
    lua_pushcfunction(l, rbLuaTexture2GetTexel);
    lua_setfield(l, -2, "gett");
    lua_pushcfunction(l, rbLuaTexture2SetTexel);
    lua_setfield(l, -2, "sett");
    lua_pushvalue(l, -1);
    lua_setfield(l, -2, "__index");
    tle_RBTexture2_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, tle_PRBLightGenerator_gc);
    lua_setfield(l, -2, "__gc");
    tle_PRBLightGenerator_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_pushnil(l);
    rbLuaCurrentGeneratorIndex = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, rbLuaInfo);
    lua_setfield(l, -2, "info");
    lua_pushcfunction(l, rbLuaWarning);
    lua_setfield(l, -2, "warning");
    
    lua_pushcfunction(l, rbLuaColor);
    lua_setfield(l, -2, "color");
    
    lua_pushcfunction(l, rbLuaTime);
    lua_setfield(l, -2, "time");
    
    lua_pushcfunction(l, rbLuaTexture1Alloc);
    lua_setfield(l, -2, "texture1");
    lua_pushcfunction(l, rbLuaPaletteFromPwl);
    lua_setfield(l, -2, "palette_from_pwl");
    
    lua_pushcfunction(l, rbLuaTexture2Alloc);
    lua_setfield(l, -2, "texture2");
    
    lua_pushcfunction(l, rbLuaLightGenerationSetGenerator);
    lua_setfield(l, -2, "set_generator");
    
    lua_pushcfunction(l, rbLuaLightGenerationCompositorAlloc);
    lua_setfield(l, -2, "gen_compositor");
    lua_pushcfunction(l, rbLuaLightGenerationCompositorAddLayer);
    lua_setfield(l, -2, "compositor_add_layer");
    lua_pushcfunction(l, rbLuaLightGenerationDashedCirclesAlloc);
    lua_setfield(l, -2, "gen_dashed_circles");
    lua_pushcfunction(l, rbLuaLightGenerationFillAlloc);
    lua_setfield(l, -2, "gen_fill");
    lua_pushcfunction(l, rbLuaLightGenerationOscilloscopeAlloc);
    lua_setfield(l, -2, "gen_oscilloscope");
    lua_pushcfunction(l, rbLuaLightGenerationPlasmaAlloc);
    lua_setfield(l, -2, "gen_plasma");
    lua_pushcfunction(l, rbLuaLightGenerationSelectorAlloc);
    lua_setfield(l, -2, "gen_selector");
    lua_pushcfunction(l, rbLuaLightGenerationSignalLissajousAlloc);
    lua_setfield(l, -2, "gen_signal_lissajous");
    lua_pushcfunction(l, rbLuaLightGenerationStaticImageAlloc);
    lua_setfield(l, -2, "gen_static_image");
    lua_pushcfunction(l, rbLuaLightGenerationVerticalBarsAlloc);
    lua_setfield(l, -2, "gen_vertical_bars");
    /*
    lua_pushcfunction(l, rbLua);
    lua_setfield(l, -2, "");
    lua_pushcfunction(l, rbLua);
    lua_setfield(l, -2, "");
    lua_pushcfunction(l, rbLua);
    lua_setfield(l, -2, "");
    lua_pushcfunction(l, rbLua);
    lua_setfield(l, -2, "");
    lua_pushcfunction(l, rbLua);
    lua_setfield(l, -2, "");
    lua_pushcfunction(l, rbLua);
    lua_setfield(l, -2, "");
    */
    lua_setglobal(l, "rb");

    lua_settop(l, top);

    return 0;
}


void rbLuaShutdown(void)
{
    rbParameterGenerationSetGenerator(NULL, 0, RB_NUM_PARAMETERS);
    rbLightGenerationSetGenerator(NULL);
    
    tle_finalize();
}


void rbLuaParametersGenerate(void * pData, RBAnalyzedAudio const * pAnalysis,
    float * pParameters, size_t numParameters)
{
    (void)pData;

    g_rbLuaPCurrentAnalysis = pAnalysis;
    g_rbLuaPCurrentParameters = pParameters;
    g_rbLuaCurrentNumParameters = numParameters;

    lua_pushcfunction(tle_state, rbLuaRunLuaParameterGeneration);
    rbVerify(tle_pcall(tle_state, 0, 0, false) == 0);

    g_rbLuaPCurrentAnalysis = NULL;
    g_rbLuaPCurrentParameters = NULL;
    g_rbLuaCurrentNumParameters = 0;
}


int rbLuaRunLuaParameterGeneration(lua_State * l)
{
    int top = lua_gettop(l);

    // parameters table
    lua_newtable(l);
    for(size_t i = 0; i < g_rbLuaCurrentNumParameters; ++i) {
        lua_pushnumber(l, g_rbLuaPCurrentParameters[i]);
        lua_rawseti(l, -2, i + 1);
    }

    lua_getglobal(l, "generate_parameters");

    if(!lua_isfunction(l, -1)) {
        rbWarning("Lua does not define generate_parameters\n");
        lua_settop(l, top);
        return 0;
    }

    // first parameter, "analysis"
    lua_newtable(l);

    lua_pushinteger(l, g_rbLuaPCurrentAnalysis->frameNum);
    lua_setfield(l, -2, "frameNum");

    lua_newtable(l);
    lua_newtable(l);
    for(size_t i = 0; i < RB_NUM_CONTROLLERS; ++i) {
        lua_pushnumber(l, g_rbLuaPCurrentAnalysis->controls.controllers[i]);
        lua_rawseti(l, -2, i + 1);
    }
    lua_setfield(l, -2, "controllers");
    lua_newtable(l);
    for(size_t i = 0; i < RB_NUM_TRIGGERS; ++i) {
        lua_pushboolean(l, g_rbLuaPCurrentAnalysis->controls.triggers[i]);
        lua_rawseti(l, -2, i + 1);
    }
    lua_setfield(l, -2, "triggers");
    lua_pushboolean(l, g_rbLuaPCurrentAnalysis->controls.debugDisplayReset);
    lua_setfield(l, -2, "debugDisplayReset");
    lua_pushinteger(l, g_rbLuaPCurrentAnalysis->controls.debugDisplayMode);
    lua_setfield(l, -2, "debugDisplayMode");
    lua_setfield(l, -2, "controls");

    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->bassEnergy);
    lua_setfield(l, -2, "bassEnergy");
    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->midEnergy);
    lua_setfield(l, -2, "midEnergy");
    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->trebleEnergy);
    lua_setfield(l, -2, "trebleEnergy");
    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->totalEnergy);
    lua_setfield(l, -2, "totalEnergy");
    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->leftRightBalance);
    lua_setfield(l, -2, "leftRightBalance");
    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->bassEnergy);
    lua_setfield(l, -2, "bassEnergy");
    lua_pushnumber(l, g_rbLuaPCurrentAnalysis->agcValue);
    lua_setfield(l, -2, "agcValue");
    lua_pushboolean(l, g_rbLuaPCurrentAnalysis->sourceOverdriven);
    lua_setfield(l, -2, "sourceOverdriven");
    lua_pushboolean(l, g_rbLuaPCurrentAnalysis->sourceLargeDc);
    lua_setfield(l, -2, "sourceLargeDc");
    lua_pushboolean(l, g_rbLuaPCurrentAnalysis->peakDetected);
    lua_setfield(l, -2, "peakDetected");
    // end construction of first parameter

    // second parameter, "parameters"
    lua_pushvalue(l, -3);

    lua_call(l, 2, 0);

    // extract modified parameter values
    for(size_t i = 0; i < g_rbLuaCurrentNumParameters; ++i) {
        lua_rawgeti(l, -1, i + 1);
        g_rbLuaPCurrentParameters[i] = lua_tonumber(l, -1);
        lua_pop(l, 1);
    }

    lua_settop(l, top);

    return 0;
}


int rbLuaInfo(lua_State * l)
{
    int top = lua_gettop(l);
    char const * str;

    str = luaL_checkstring(l, 1);
    rbInfo("%s\n", str);

    lua_settop(l, top);

    return 0;
}


int rbLuaWarning(lua_State * l)
{
    int top = lua_gettop(l);
    char const * str;

    str = luaL_checkstring(l, 1);
    rbWarning("%s\n", str);

    lua_settop(l, top);

    return 0;
}


int rbLuaColor(lua_State * l)
{
    int top = lua_gettop(l);
    lua_Number r, g, b, a;

    tle_verify(l, top == 3 || top == 4);
    r = luaL_checknumber(l, 1);
    g = luaL_checknumber(l, 2);
    b = luaL_checknumber(l, 3);
    if(top == 4) {
        a = luaL_checknumber(l, 4);
    }
    else {
        a = 1;
    }
    
    tle_push_RBColor(l, colorf(r, g, b, a));
    return 1;
}


int rbLuaTime(lua_State * l)
{
    int top = lua_gettop(l);
    lua_Number t;

    tle_verify(l, top == 1);
    t = luaL_checknumber(l, 1);
    tle_push_RBTime(l, rbTimeFromMs((int32_t)(t * 1000 + 0.5f)));
    return 1;
}


int rbLuaTexture1Alloc(lua_State * l)
{
    size_t texWidth;
    RBColor clearColor;
    size_t size;
    RBTexture1 * pTex;

    texWidth = (size_t)luaL_checkinteger(l, 1);
    clearColor = tle_to_RBColor(l, 2);
    
    size = rbTexture1ComputeSize(texWidth);
    
    // Create and fill the texture
    pTex = (RBTexture1 *)lua_newuserdata(l, size);
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTexture1_metatable_index);
    lua_setmetatable(l, -2);
    rbTexture1Construct(pTex, texWidth);
    rbTexture1Clear(pTex, clearColor);
    
    return 1;
}


int rbLuaPaletteFromPwl(lua_State * l)
{
    int n;

    luaL_checktype(l, 1, LUA_TTABLE);
    lua_len(l, 1);
    n = (int)luaL_checknumber(l, -1);
    lua_pop(l, 1);
    
    {
        size_t texWidth = 256;
        RBPiecewiseLinearColorSegment pwl[n];
        size_t size = rbTexture1ComputeSize(texWidth);
        RBTexture1 * pTex;
        
        // Iterate array, pull elements into PWL array
        for(int i = 1; i <= n; ++i) {
            int top2 = lua_gettop(l);
            int pwlTable;
            
            lua_rawgeti(l, 1, i); // get PWL entry table from param
            pwlTable = lua_gettop(l);
            luaL_checktype(l, pwlTable, LUA_TTABLE);
            
            lua_rawgeti(l, pwlTable, 1); // get color table from PWL
            luaL_argcheck(l, tle_is_RBColor(l, -1), 1,
                "expected table of {color, length} (element not color)");
            pwl[i - 1].color = tle_to_RBColor(l, -1);
            
            lua_rawgeti(l, pwlTable, 2); // get length of pwl element
            pwl[i - 1].length = (size_t)luaL_checknumber(l, -1);
            
            lua_settop(l, top2);
        }
        
        // Create and fill the texture
        pTex = (RBTexture1 *)lua_newuserdata(l, size);
        lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTexture1_metatable_index);
        lua_setmetatable(l, -2);
        rbTexture1Construct(pTex, texWidth);
        rbTexture1FillFromPiecewiseLinear(pTex, pwl, LENGTHOF(pwl), false);
        
        return 1;
    }
}


int rbLuaTexture2Alloc(lua_State * l)
{
    size_t texWidth, texHeight;
    RBColor clearColor = colori(0, 0, 0, 0);
    size_t size;
    RBTexture2 * pTex;

    tle_verify(l, lua_gettop(l) >= 2);
    texWidth = (size_t)luaL_checkinteger(l, 1);
    texHeight = (size_t)luaL_checkinteger(l, 2);
    if(lua_gettop(l) >= 3) {
        clearColor = tle_to_RBColor(l, 3);
    }
    
    size = rbTexture2ComputeSize(texWidth, texHeight);
    
    // Create and fill the texture
    pTex = (RBTexture2 *)lua_newuserdata(l, size);
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTexture2_metatable_index);
    lua_setmetatable(l, -2);
    rbTexture2Construct(pTex, texWidth, texHeight);
    rbTexture2Clear(pTex, clearColor);
    
    return 1;
}


int rbLuaLightGenerationSetGenerator(lua_State * l)
{
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    if(lua_isnil(l, 1)) {
        rbLightGenerationSetGenerator(NULL);
    }
    else {
        luaL_argcheck(l, tle_is_PRBLightGenerator(l, 1), 1,
            "expected RBLightGenerator");
        rbLightGenerationSetGenerator(tle_to_PRBLightGenerator(l, 1));
    }
    
    // Reference param (or unref, if nil)
    lua_pushvalue(l, 1);
    lua_rawseti(l, LUA_REGISTRYINDEX, rbLuaCurrentGeneratorIndex);
    
    return 1;
}


int rbLuaLightGenerationCompositorAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;

    pGenRes = rbLightGenerationCompositorAlloc();
    tle_verify(l, pGenRes != NULL);
    tle_push_PRBLightGenerator(l, pGenRes);
    lua_pushcfunction(l, rbLuaLightGenerationCompositorAddLayer);
    lua_setfield(l, -2, "add_layer");
    
    return 1;
}


int rbLuaLightGenerationCompositorAddLayer(lua_State * l)
{
    int top = lua_gettop(l);
    RBLightGenerator * pGen;
    RBLightGenerator * pLayerGen;
    RBTexture2 * pDestTexture = NULL;
    RBLightGenerationBlendMode blendMode = RBLGBM_SRC_ALPHA;
    size_t alphaIndex = RB_PARAMETER_NONE;
    size_t transformPosIndex = RB_PARAMETER_NONE;
    size_t transformScaleIndex = RB_PARAMETER_NONE;

    tle_verify(l, top >= 2);
    
    luaL_argcheck(l, tle_is_PRBLightGenerator(l, 1), 1,
        "expected RBLightGenerator");
    pGen = tle_to_PRBLightGenerator(l, 1);
    tle_verify(l, pGen != NULL);
    
    luaL_argcheck(l, tle_is_PRBLightGenerator(l, 2), 2,
        "expected RBLightGenerator");
    pLayerGen = tle_to_PRBLightGenerator(l, 2);
    tle_verify(l, pLayerGen != NULL);
    
    if(top >= 3) {
        luaL_argcheck(l, tle_is_RBTexture2(l, 3), 3,
            "expected RBTexture2");
        pDestTexture = tle_to_RBTexture2(l, 3);
    }
    
    if(top >= 4) {
        luaL_argcheck(l, lua_isnumber(l, 4), 4,
            "expected blend mode");
        blendMode = (RBLightGenerationBlendMode)lua_tointeger(l, 4);
        
    }
    
    if(top >= 5) {
        luaL_argcheck(l, lua_isnumber(l, 5), 5,
            "expected parameter index");
        alphaIndex = lua_tointeger(l, 5) - 1;
    }
    
    if(top >= 6) {
        luaL_argcheck(l, lua_isnumber(l, 6), 6,
            "expected parameter index");
        transformPosIndex = lua_tointeger(l, 6) - 1;
    }
    
    if(top >= 7) {
        luaL_argcheck(l, lua_isnumber(l, 7), 7,
            "expected parameter index");
        transformScaleIndex = lua_tointeger(l, 7) - 1;
    }
    
    rbLightGenerationCompositorAddLayer(pGen, pLayerGen, pDestTexture,
        blendMode, alphaIndex, transformPosIndex, transformScaleIndex);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 2);
    lua_rawseti(l, 1, lua_rawlen(l, 1) + 1);
    if(pDestTexture != NULL) {
        lua_pushvalue(l, 3);
        lua_rawseti(l, 1, lua_rawlen(l, 1) + 1);
    }
    return 1;
}


static int rbLuaLightGenerationDashedCirclesAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    RBTexture1 * pPalette;
    size_t scaleIndex = RB_PARAMETER_NONE;
    size_t dashScaleIndex = RB_PARAMETER_NONE;
    size_t rotationIndex = RB_PARAMETER_NONE;

    tle_verify(l, lua_gettop(l) == 4);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    pPalette = tle_to_RBTexture1(l, 1);
    
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected parameter index");
    scaleIndex = lua_tointeger(l, 2) - 1;
    
    luaL_argcheck(l, lua_isnumber(l, 3), 3, "expected parameter index");
    dashScaleIndex = lua_tointeger(l, 3) - 1;
    
    luaL_argcheck(l, lua_isnumber(l, 4), 4, "expected parameter index");
    rotationIndex = lua_tointeger(l, 4) - 1;
    
    pGenRes = rbLightGenerationDashedCirclesAlloc(pPalette, scaleIndex,
        dashScaleIndex, rotationIndex);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference param so it's not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
    
    return 1;
}


static int rbLuaLightGenerationFillAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    size_t colorIndex;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, lua_isnumber(l, 1), 1, "expected parameter index");
    colorIndex = lua_tointeger(l, 1) - 1;
    
    pGenRes = rbLightGenerationFillAlloc(colorIndex);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    return 1;
}


int rbLuaLightGenerationOscilloscopeAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    RBTexture1 * pPalette;
    
    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    pPalette = tle_to_RBTexture1(l, 1);
    
    pGenRes = rbLightGenerationOscilloscopeAlloc(pPalette);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference param so it's not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
    
    return 1;
}


static int rbLuaLightGenerationPlasmaAlloc(lua_State * l)
{
    int top = lua_gettop(l);
    RBLightGenerator * pGenRes;
    RBTexture1 * pPalette;
    size_t scaleIndex = RB_PARAMETER_NONE;
    
    tle_verify(l, top >= 1);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    pPalette = tle_to_RBTexture1(l, 1);
    
    if(top >= 2) {
        luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected parameter index");
        scaleIndex = lua_tointeger(l, 2) - 1;
    }
    
    pGenRes = rbLightGenerationPlasmaAlloc(pPalette, scaleIndex);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference param so it's not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
    
    return 1;
}


int rbLuaLightGenerationSelectorAlloc(lua_State * l)
{
    int top = lua_gettop(l);
    RBLightGenerator * pGenRes;
    size_t numGenerators;
    RBTime transitionTime;
    size_t selectIndex;

    tle_verify(l, top == 4);
    luaL_argcheck(l, lua_istable(l, 1), 1, "expected table of generators");
    numGenerators = lua_rawlen(l, 1);
    {
        RBLightGenerator * pGenerators[numGenerators];
        
        for(size_t i = 0; i < numGenerators; ++i) {
            lua_rawgeti(l, 1, i + 1);
            pGenerators[i] = tle_to_PRBLightGenerator(l, -1);
            lua_pop(l, 1);
        }
        
        luaL_argcheck(l, tle_is_RBTime(l, 2), 2, "expected RBTime");
        transitionTime = tle_to_RBTime(l, 2);
        
        luaL_argcheck(l, lua_isnumber(l, 3), 3, "expected parameter index");
        selectIndex = lua_tointeger(l, 3) - 1;
        
        pGenRes = rbLightGenerationSelectorAlloc(pGenerators, numGenerators,
            transitionTime, selectIndex);
        tle_push_PRBLightGenerator(l, pGenRes);
        
        // Reference params so they're not collected out from under us
        for(size_t i = 0; i < numGenerators; ++i) {
            lua_rawgeti(l, 1, i + 1);
            lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
        }
    }
    
    return 1;
}


int rbLuaLightGenerationSignalLissajousAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    RBTexture1 * pPalette;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    pPalette = tle_to_RBTexture1(l, 1);
    
    pGenRes = rbLightGenerationSignalLissajousAlloc(pPalette);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference param so it's not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
    
    return 1;
}



int rbLuaLightGenerationStaticImageAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    RBTexture2 * pTexture;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBTexture2(l, 1), 1, "expected RBTexture2");
    pTexture = tle_to_RBTexture2(l, 1);
    
    pGenRes = rbLightGenerationStaticImageAlloc(pTexture);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference param so it's not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
    
    return 1;
}


int rbLuaLightGenerationVerticalBarsAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    RBTexture1 * pPalette;
    size_t numBars;
    RBTime spawnInterval;
    RBTime fadeTime;
    size_t intensityIndex;
    
    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    pPalette = tle_to_RBTexture1(l, 1);
    
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    numBars = lua_tointeger(l, 2);
    
    luaL_argcheck(l, tle_is_RBTime(l, 3), 3, "expected RBTime");
    spawnInterval = lua_tointeger(l, 2);
    
    luaL_argcheck(l, tle_is_RBTime(l, 4), 4, "expected RBTime");
    fadeTime = lua_tointeger(l, 2);
    
    luaL_argcheck(l, lua_isnumber(l, 5), 5, "expected number");
    intensityIndex = lua_tointeger(l, 2) - 1;
    
    pGenRes = rbLightGenerationVerticalBarsAlloc(pPalette, numBars,
        spawnInterval, fadeTime, intensityIndex);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference param so it's not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, lua_rawlen(l, -2) + 1);
    
    return 1;
}


// RBColor -------------------------------------------------------------


bool tle_is_RBColor(lua_State * l, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(l, idx) && lua_getmetatable(l, idx)) {
        lua_rawgeti(l, LUA_REGISTRYINDEX,
            tle_RBColor_metatable_index);
        if(lua_rawequal(l, -1, -2)) {
            result = true;
        }
        lua_pop(l, 2);
    }
    
    return result;
}


RBColor tle_to_RBColor(lua_State * l, int idx)
{
    RBColor * p = (RBColor *)lua_touserdata(l, idx);
    tle_verify(l, p != NULL);
    return *p;
}


void tle_push_RBColor(lua_State * l, RBColor value)
{
    RBColor * p = (RBColor *)lua_newuserdata(l,
        sizeof (RBColor));
    *p = value;
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBColor_metatable_index);
    lua_setmetatable(l, -2);
}


static bool tle_eq_RBColor_RBColor(lua_State * l,
    RBColor x, RBColor y)
{
    (void)l;

    return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a;
}


TLE_MAKE_WRAPPER_2(tle_RBColor_eq,
    bool, RBColor, RBColor,
    tle_eq_RBColor_RBColor)


// RBTime -------------------------------------------------------------


bool tle_is_RBTime(lua_State * l, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(l, idx) && lua_getmetatable(l, idx)) {
        lua_rawgeti(l, LUA_REGISTRYINDEX,
            tle_RBTime_metatable_index);
        if(lua_rawequal(l, -1, -2)) {
            result = true;
        }
        lua_pop(l, 2);
    }
    
    return result;
}


RBTime tle_to_RBTime(lua_State * l, int idx)
{
    RBTime * p = (RBTime *)lua_touserdata(l, idx);
    tle_verify(l, p != NULL);
    return *p;
}


void tle_push_RBTime(lua_State * l, RBTime value)
{
    RBTime * p = (RBTime *)lua_newuserdata(l,
        sizeof (RBTime));
    *p = value;
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTime_metatable_index);
    lua_setmetatable(l, -2);
}


static bool tle_eq_RBTime_RBTime(lua_State * l,
    RBTime x, RBTime y)
{
    (void)l;

    return x == y;
}


TLE_MAKE_WRAPPER_2(tle_RBTime_eq,
    bool, RBTime, RBTime,
    tle_eq_RBTime_RBTime)


static bool tle_lt_RBTime_RBTime(lua_State * l,
    RBTime x, RBTime y)
{
    (void)l;

    return rbDiffTime(x, y) < 0;
}


TLE_MAKE_WRAPPER_2(tle_RBTime_lt,
    bool, RBTime, RBTime,
    tle_lt_RBTime_RBTime)


static bool tle_le_RBTime_RBTime(lua_State * l,
    RBTime x, RBTime y)
{
    (void)l;

    return rbDiffTime(x, y) <= 0;
}


TLE_MAKE_WRAPPER_2(tle_RBTime_le,
    bool, RBTime, RBTime,
    tle_le_RBTime_RBTime)


// RBTexture1 ----------------------------------------------------------


bool tle_is_RBTexture1(lua_State * l, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(l, idx) && lua_getmetatable(l, idx)) {
        lua_rawgeti(l, LUA_REGISTRYINDEX,
            tle_RBTexture1_metatable_index);
        if(lua_rawequal(l, -1, -2)) {
            result = true;
        }
        lua_pop(l, 2);
    }
    
    return result;
}


RBTexture1 * tle_to_RBTexture1(lua_State * l, int idx)
{
    RBTexture1 * p = (RBTexture1 *)lua_touserdata(l, idx);
    tle_verify(l, p != NULL);
    return p;
}


void tle_push_RBTexture1(lua_State * l, RBTexture1 * value)
{
    size_t size = rbTexture1ComputeSize(rbTexture1GetWidth(value));
    RBTexture1 * p = (RBTexture1 *)lua_newuserdata(l, size);
    memcpy(p, value, size);
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTexture1_metatable_index);
    lua_setmetatable(l, -2);
}


int rbLuaTexture1SampleNearestRepeat(lua_State * l)
{
    RBTexture1 * p;
    float u;
    RBColor c;
    
    p = tle_to_RBTexture1(l, 1);
    u = (float)lua_tonumber(l, 2);
    c = rbColorMakeCT(rbTexture1SampleNearestRepeat(p, u));
    tle_push_RBColor(l, c);
    
    return 1;
}



int rbLuaTexture1SampleNearestClamp(lua_State * l)
{
    RBTexture1 * p;
    float u;
    RBColor c;
    
    p = tle_to_RBTexture1(l, 1);
    u = (float)lua_tonumber(l, 2);
    c = rbColorMakeCT(rbTexture1SampleNearestClamp(p, u));
    tle_push_RBColor(l, c);
    
    return 1;
}



int rbLuaTexture1SampleLinearRepeat(lua_State * l)
{
    RBTexture1 * p;
    float u;
    RBColor c;
    
    p = tle_to_RBTexture1(l, 1);
    u = (float)lua_tonumber(l, 2);
    c = rbColorMakeCT(rbTexture1SampleLinearRepeat(p, u));
    tle_push_RBColor(l, c);
    
    return 1;
}



int rbLuaTexture1SampleLinearClamp(lua_State * l)
{
    RBTexture1 * p;
    float u;
    RBColor c;
    
    p = tle_to_RBTexture1(l, 1);
    u = (float)lua_tonumber(l, 2);
    c = rbColorMakeCT(rbTexture1SampleLinearClamp(p, u));
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture1GetTexel(lua_State * l)
{
    RBTexture1 * p;
    size_t u;
    RBColor c;
    
    p = tle_to_RBTexture1(l, 1);
    u = (size_t)lua_tointeger(l, 2);
    c = rbTexture1GetTexel(p, u);
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture1SetTexel(lua_State * l)
{
    RBTexture1 * p;
    size_t u;
    RBColor c;
    
    p = tle_to_RBTexture1(l, 1);
    u = (size_t)lua_tointeger(l, 2);
    c = tle_to_RBColor(l, 3);
    
    rbTexture1SetTexel(p, u, c);
    
    return 0;
}


// RBTexture2 ----------------------------------------------------------


bool tle_is_RBTexture2(lua_State * l, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(l, idx) && lua_getmetatable(l, idx)) {
        lua_rawgeti(l, LUA_REGISTRYINDEX,
            tle_RBTexture2_metatable_index);
        if(lua_rawequal(l, -1, -2)) {
            result = true;
        }
        lua_pop(l, 2);
    }
    
    return result;
}


RBTexture2 * tle_to_RBTexture2(lua_State * l, int idx)
{
    RBTexture2 * p = (RBTexture2 *)lua_touserdata(l, idx);
    tle_verify(l, p != NULL);
    return p;
}


void tle_push_RBTexture2(lua_State * l, RBTexture2 * value)
{
    size_t size = rbTexture2ComputeSize(rbTexture2GetWidth(value),
        rbTexture2GetHeight(value));
    RBTexture2 * p = (RBTexture2 *)lua_newuserdata(l, size);
    memcpy(p, value, size);
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTexture2_metatable_index);
    lua_setmetatable(l, -2);
}


int rbLuaTexture2SampleNearestRepeat(lua_State * l)
{
    RBTexture2 * p;
    float u, v;
    RBColor c;
    
    p = tle_to_RBTexture2(l, 1);
    u = (float)lua_tonumber(l, 2);
    v = (float)lua_tonumber(l, 3);
    c = rbColorMakeCT(rbTexture2SampleNearestRepeat(p, rbVector2Make(u, v)));
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture2SampleNearestClamp(lua_State * l)
{
    RBTexture2 * p;
    float u, v;
    RBColor c;
    
    p = tle_to_RBTexture2(l, 1);
    u = (float)lua_tonumber(l, 2);
    v = (float)lua_tonumber(l, 3);
    c = rbColorMakeCT(rbTexture2SampleNearestClamp(p, rbVector2Make(u, v)));
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture2SampleLinearRepeat(lua_State * l)
{
    RBTexture2 * p;
    float u, v;
    RBColor c;
    
    p = tle_to_RBTexture2(l, 1);
    u = (float)lua_tonumber(l, 2);
    v = (float)lua_tonumber(l, 3);
    c = rbColorMakeCT(rbTexture2SampleLinearRepeat(p, rbVector2Make(u, v)));
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture2SampleLinearClamp(lua_State * l)
{
    RBTexture2 * p;
    float u, v;
    RBColor c;
    
    lua_rawgeti(l, 1, 1);
    p = tle_to_RBTexture2(l, -1);
    lua_pop(l, 1);
    
    u = (float)lua_tonumber(l, 2);
    v = (float)lua_tonumber(l, 3);
    c = rbColorMakeCT(rbTexture2SampleLinearClamp(p, rbVector2Make(u, v)));
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture2GetTexel(lua_State * l)
{
    RBTexture2 * p;
    size_t u, v;
    RBColor c;
    
    p = tle_to_RBTexture2(l, 1);
    u = (size_t)lua_tointeger(l, 2);
    v = (size_t)lua_tointeger(l, 3);
    c = rbTexture2GetTexel(p, u, v);
    tle_push_RBColor(l, c);
    
    return 1;
}


int rbLuaTexture2SetTexel(lua_State * l)
{
    RBTexture2 * p;
    size_t u, v;
    RBColor c;
    
    p = tle_to_RBTexture2(l, 1);
    u = (size_t)lua_tointeger(l, 2);
    v = (size_t)lua_tointeger(l, 3);
    c = tle_to_RBColor(l, 4);
    
    rbTexture2SetTexel(p, u, v, c);
    
    return 0;
}


// RBLightGenerator ----------------------------------------------------


bool tle_is_PRBLightGenerator(lua_State * l, int idx)
{
    bool result = false;
    
    if(lua_istable(l, idx) && lua_getmetatable(l, idx)) {
        lua_rawgeti(l, LUA_REGISTRYINDEX,
            tle_PRBLightGenerator_metatable_index);
        if(lua_rawequal(l, -1, -2)) {
            result = true;
        }
        lua_pop(l, 2);
    }
    
    return result;
}


RBLightGenerator * tle_to_PRBLightGenerator(lua_State * l, int idx)
{
    RBLightGenerator * p;
    lua_rawgeti(l, idx, 1);
    p = (RBLightGenerator *)lua_touserdata(l, -1);
    lua_pop(l, 1);
    return p;
}


void tle_push_PRBLightGenerator(lua_State * l, RBLightGenerator * value)
{
    lua_createtable(l, 5, 0);
    lua_pushlightuserdata(l, value);
    lua_rawseti(l, -2, 1);
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_PRBLightGenerator_metatable_index);
    lua_setmetatable(l, -2);
}


int tle_PRBLightGenerator_gc(lua_State * l)
{
    RBLightGenerator * p;
    
    lua_rawgeti(l, 1, 1);
    p = (RBLightGenerator *)lua_touserdata(l, -1);
    lua_pop(l, 1);
    
    // Clear the Lua lightuserdata so we don't double-free ever
    lua_pushnil(l);
    lua_rawseti(l, 1, 1);
    
    rbLightGenerationGeneratorFree(p);
    
    return 0;
}


/*
static inline sequencer_timestamp_t tle_to_sequencer_timespan_t(
    lua_State * l, int idx)
{
    lua_Number as_number = floor(lua_tonumber(l, idx));
    luaL_argcheck(l, (as_number <= SEQUENCER_TIMESPAN_MAX)
        && (as_number >= -SEQUENCER_TIMESPAN_MAX), idx,
        "timespan out of range");
    return (sequencer_timestamp_t)as_number;
}

TLE_START_OVERLOAD_WRAPPER(tle_timestamp_sub)
TLE_MAKE_OVERLOAD_2(sequencer_timespan_t, sequencer_timestamp_t,
    sequencer_timestamp_t, tle_sub_timestamp_timestamp)
TLE_MAKE_OVERLOAD_2(sequencer_timestamp_t, sequencer_timestamp_t,
    sequencer_timespan_t, tle_sub_timestamp_timespan)
TLE_END_OVERLOAD_WRAPPER
*/
