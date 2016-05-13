#include "rasterblocks_lua.h"

#include "graphics_util.h"
#include "light_generation.h"
#include "tle.h"


typedef RBLightGenerator * PRBLightGenerator;


static int tle_RBColor_metatable_index;
static int tle_RBTime_metatable_index;
static int tle_RBTexture1_metatable_index;
static int tle_RBTexture2_metatable_index;
static int tle_PRBLightGenerator_metatable_index;


int rbLuaRegisterTypes(lua_State * l);

static int rbLuaPrint(lua_State * l);

static int rbLuaPaletteFromPwl(lua_State * l);
static int rbLuaColor(lua_State * l);

static int rbLuaLightGenerationSetGenerator(lua_State * l);

static int rbLuaLightGenerationCompositorAlloc(lua_State * l);
static int rbLuaLightGenerationStaticImageAlloc(lua_State * l);
static int rbLuaLightGenerationImageFilterAlloc(lua_State * l);
static int rbLuaLightGenerationRescaleAlloc(lua_State * l);
static int rbLuaLightGenerationTimedRotationAlloc(lua_State * l);
static int rbLuaLightGenerationControllerSelectAlloc(lua_State * l);
static int rbLuaLightGenerationControllerFadeAlloc(lua_State * l);
static int rbLuaLightGenerationTriggerFlashAlloc(lua_State * l);
/*
static int rbLuaLightGenerationPlasmaAlloc(lua_State * l);
static int rbLuaLightGenerationBeatFlashAlloc(lua_State * l);
static int rbLuaLightGenerationPulsePlasmaAlloc(lua_State * l);
static int rbLuaLightGenerationPulseGridAlloc(lua_State * l);
static int rbLuaLightGenerationDashedCirclesAlloc(lua_State * l);
*/
static int rbLuaLightGenerationSmokeSignalsAlloc(lua_State * l);
static int rbLuaLightGenerationFireworksAlloc(lua_State * l);
static int rbLuaLightGenerationVerticalBarsAlloc(lua_State * l);
static int rbLuaLightGenerationCriscrossAlloc(lua_State * l);
static int rbLuaLightGenerationVolumeBarsAlloc(lua_State * l);
static int rbLuaLightGenerationBeatStarsAlloc(lua_State * l);
static int rbLuaLightGenerationIconCheckerboardAlloc(lua_State * l);
static int rbLuaLightGenerationPulseCheckerboardAlloc(lua_State * l);
static int rbLuaLightGenerationParticleLissajousAlloc(lua_State * l);
static int rbLuaLightGenerationSignalLissajousAlloc(lua_State * l);
static int rbLuaLightGenerationOscilloscopeAlloc(lua_State * l);


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

static bool tle_is_RBTexture2(lua_State * l, int idx);
static RBTexture2 * tle_to_RBTexture2(lua_State * l, int idx);
static void tle_push_RBTexture2(lua_State * l, RBTexture2 * value);

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
}


void rbLuaShutdown(void)
{
    rbLightGenerationSetGenerator(NULL);
    
    tle_finalize();
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
    tle_RBTexture1_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    tle_RBTexture2_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, tle_PRBLightGenerator_gc);
    lua_setfield(l, -2, "__gc");
    tle_PRBLightGenerator_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_pushcfunction(l, rbLuaPrint);
    lua_setfield(l, -2, "print");
    lua_pushcfunction(l, rbLuaPaletteFromPwl);
    lua_setfield(l, -2, "palette_from_pwl");
    lua_pushcfunction(l, rbLuaColor);
    lua_setfield(l, -2, "color");
    lua_pushcfunction(l, rbLuaLightGenerationSetGenerator);
    lua_setfield(l, -2, "set_generator");
    lua_pushcfunction(l, rbLuaLightGenerationCompositorAlloc);
    lua_setfield(l, -2, "gen_compositor");
    lua_pushcfunction(l, rbLuaLightGenerationStaticImageAlloc);
    lua_setfield(l, -2, "gen_static_image");
    lua_pushcfunction(l, rbLuaLightGenerationImageFilterAlloc);
    lua_setfield(l, -2, "gen_image_filter");
    lua_pushcfunction(l, rbLuaLightGenerationRescaleAlloc);
    lua_setfield(l, -2, "gen_rescale");
    lua_pushcfunction(l, rbLuaLightGenerationTimedRotationAlloc);
    lua_setfield(l, -2, "gen_timed_rotation");
    lua_pushcfunction(l, rbLuaLightGenerationControllerSelectAlloc);
    lua_setfield(l, -2, "gen_controller_select");
    lua_pushcfunction(l, rbLuaLightGenerationControllerFadeAlloc);
    lua_setfield(l, -2, "gen_controller_fade");
    lua_pushcfunction(l, rbLuaLightGenerationTriggerFlashAlloc);
    lua_setfield(l, -2, "gen_trigger_flash");
    /*
    lua_pushcfunction(l, rbLuaLightGenerationPlasmaAlloc);
    lua_setfield(l, -2, "gen_plasma");
    lua_pushcfunction(l, rbLuaLightGenerationBeatFlashAlloc);
    lua_setfield(l, -2, "gen_beat_flash");
    lua_pushcfunction(l, rbLuaLightGenerationPulsePlasmaAlloc);
    lua_setfield(l, -2, "gen_pulse_plasma");
    lua_pushcfunction(l, rbLuaLightGenerationPulseGridAlloc);
    lua_setfield(l, -2, "gen_pulse_grid");
    lua_pushcfunction(l, rbLuaLightGenerationDashedCirclesAlloc);
    lua_setfield(l, -2, "gen_dashed_circles");
    */
    lua_pushcfunction(l, rbLuaLightGenerationSmokeSignalsAlloc);
    lua_setfield(l, -2, "gen_smoke_signals");
    lua_pushcfunction(l, rbLuaLightGenerationFireworksAlloc);
    lua_setfield(l, -2, "gen_fireworks");
    lua_pushcfunction(l, rbLuaLightGenerationVerticalBarsAlloc);
    lua_setfield(l, -2, "gen_vertical_bars");
    lua_pushcfunction(l, rbLuaLightGenerationCriscrossAlloc);
    lua_setfield(l, -2, "gen_criscross");
    lua_pushcfunction(l, rbLuaLightGenerationVolumeBarsAlloc);
    lua_setfield(l, -2, "gen_volume_bars");
    lua_pushcfunction(l, rbLuaLightGenerationBeatStarsAlloc);
    lua_setfield(l, -2, "gen_beat_stars");
    lua_pushcfunction(l, rbLuaLightGenerationIconCheckerboardAlloc);
    lua_setfield(l, -2, "gen_icon_checkerboard");
    lua_pushcfunction(l, rbLuaLightGenerationPulseCheckerboardAlloc);
    lua_setfield(l, -2, "gen_pulse_checkerboard");
    lua_pushcfunction(l, rbLuaLightGenerationParticleLissajousAlloc);
    lua_setfield(l, -2, "gen_particle_lissajous");
    lua_pushcfunction(l, rbLuaLightGenerationSignalLissajousAlloc);
    lua_setfield(l, -2, "gen_signal_lissajous");
    lua_pushcfunction(l, rbLuaLightGenerationOscilloscopeAlloc);
    lua_setfield(l, -2, "gen_oscilloscope");
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


int rbLuaPrint(lua_State * l)
{
    int top = lua_gettop(l);
    char const * str;

    str = luaL_checkstring(l, 1);
    printf("lua: %s\n", str);

    lua_settop(l, top);

    return 0;
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


int rbLuaColor(lua_State * l)
{
    int top = lua_gettop(l);
    lua_Number r, g, b, a;

    tle_verify(l, top == 3 || top == 4);
    r = (int)luaL_checknumber(l, 1);
    g = (int)luaL_checknumber(l, 2);
    b = (int)luaL_checknumber(l, 3);
    if(top == 4) {
        a = (int)luaL_checknumber(l, 4);
    }
    else {
        a = 1;
    }
    
    tle_push_RBColor(l, colorf(r, g, b, a));
    return 1;
}


static void rbLuaLightGenerationSetGenerator_PRBLightGenerator(lua_State * l,
    RBLightGenerator * pGen)
{
    (void)l;
    rbLightGenerationSetGenerator(pGen);
}


TLE_MAKE_WRAPPER_1_VOID(rbLuaLightGenerationSetGenerator,
    PRBLightGenerator, rbLuaLightGenerationSetGenerator_PRBLightGenerator)


int rbLuaLightGenerationCompositorAlloc(lua_State * l)
{
    int top = lua_gettop(l);
    RBLightGenerator * pGen0 = NULL;
    RBLightGenerator * pGen1 = NULL;
    RBLightGenerator * pGen2 = NULL;
    RBLightGenerator * pGen3 = NULL;
    RBLightGenerator * pGenRes;

    tle_verify(l, top <= 4);
    
    if(top >= 1) {
        luaL_argcheck(l, tle_is_PRBLightGenerator(l, 1), 1,
            "expected RBLightGenerator");
        pGen0 = tle_to_PRBLightGenerator(l, 1);
    }
    if(top >= 2) {
        luaL_argcheck(l, tle_is_PRBLightGenerator(l, 2), 2,
            "expected RBLightGenerator");
        pGen1 = tle_to_PRBLightGenerator(l, 2);
    }
    if(top >= 3) {
        luaL_argcheck(l, tle_is_PRBLightGenerator(l, 3), 3,
            "expected RBLightGenerator");
        pGen2 = tle_to_PRBLightGenerator(l, 3);
    }
    if(top >= 4) {
        luaL_argcheck(l, tle_is_PRBLightGenerator(l, 4), 4,
            "expected RBLightGenerator");
        pGen3 = tle_to_PRBLightGenerator(l, 4);
    }
    
    pGenRes = rbLightGenerationCompositor4Alloc(pGen0, pGen1, pGen2, pGen3);
    tle_verify(l, pGenRes != NULL);
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    if(top >= 1) { lua_pushvalue(l, 1); lua_rawseti(l, -2, 2); }
    if(top >= 2) { lua_pushvalue(l, 2); lua_rawseti(l, -2, 3); }
    if(top >= 3) { lua_pushvalue(l, 3); lua_rawseti(l, -2, 4); }
    if(top >= 4) { lua_pushvalue(l, 4); lua_rawseti(l, -2, 5); }
    
    return 1;
}


int rbLuaLightGenerationStaticImageAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBTexture2(l, 1), 1, "expected RBTexture2");
    pGenRes = rbLightGenerationStaticImageAlloc(tle_to_RBTexture2(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationImageFilterAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 2);
    luaL_argcheck(l, tle_is_PRBLightGenerator(l, 1), 1,
        "expected RBLightGenerator");
    luaL_argcheck(l, tle_is_RBTexture2(l, 2), 2, "expected RBTexture2");
    pGenRes = rbLightGenerationImageFilterAlloc(
        tle_to_PRBLightGenerator(l, 1), tle_to_RBTexture2(l, 2));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    lua_pushvalue(l, 2);
    lua_rawseti(l, -2, 3);
    
    return 1;
}


int rbLuaLightGenerationRescaleAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 2);
    luaL_argcheck(l, tle_is_PRBLightGenerator(l, 1), 1,
        "expected RBLightGenerator");
    luaL_argcheck(l, tle_is_RBTexture2(l, 2), 2, "expected number");
    luaL_argcheck(l, tle_is_RBTexture2(l, 3), 3, "expected number");
    pGenRes = rbLightGenerationRescaleAlloc(
        tle_to_PRBLightGenerator(l, 1), (size_t)lua_tonumber(l, 2),
        (size_t)lua_tonumber(l, 3));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationTimedRotationAlloc(lua_State * l)
{
    int top = lua_gettop(l);
    RBLightGenerator * pGenRes;
    size_t n;

    tle_verify(l, top == 3);
    luaL_argcheck(l, lua_istable(l, 1), 1, "expected table");
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    luaL_argcheck(l, tle_is_RBTime(l, 3), 3, "expected RBTime");
    
    lua_len(l, 1);
    n = (size_t)lua_tonumber(l, -1);
    lua_pop(l, 1);
    {
        RBLightGenerator * pGens[n];
        
        for(size_t i = 0; i < n; ++i) {
            lua_rawgeti(l, 1, i + 1);
            pGens[i] = tle_to_PRBLightGenerator(l, -1);
            lua_pop(l, 1);
        }
        pGenRes = rbLightGenerationTimedRotationAlloc(pGens, n,
            (int32_t)lua_tonumber(l, 2), tle_to_RBTime(l, 3));
    }
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1); // This is sketchy: we should make a new table
    lua_rawseti(l, -2, 2); // (instead of referencing the old one)
    
    return 1;
}


int rbLuaLightGenerationControllerSelectAlloc(lua_State * l)
{
    int top = lua_gettop(l);
    RBLightGenerator * pGenRes;
    size_t n;

    tle_verify(l, top == 2);
    luaL_argcheck(l, lua_istable(l, 1), 1, "expected table");
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    
    lua_len(l, 1);
    n = (size_t)lua_tonumber(l, -1);
    lua_pop(l, 1);
    {
        RBLightGenerator * pGens[n];
        
        for(size_t i = 0; i < n; ++i) {
            lua_rawgeti(l, 1, i + 1);
            pGens[i] = tle_to_PRBLightGenerator(l, -1);
            lua_pop(l, 1);
        }
        pGenRes = rbLightGenerationControllerSelectAlloc(pGens, n,
            (int32_t)lua_tonumber(l, 2));
    }
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1); // This is sketchy: we should make a new table
    lua_rawseti(l, -2, 2); // (instead of referencing the old one)
    
    return 1;
}


int rbLuaLightGenerationControllerFadeAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 2);
    luaL_argcheck(l, tle_is_PRBLightGenerator(l, 1), 1,
        "expected RBLightGenerator");
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    pGenRes = rbLightGenerationControllerFadeAlloc(
        tle_to_PRBLightGenerator(l, 1), (int32_t)lua_tonumber(l, 2));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationTriggerFlashAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 2);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    pGenRes = rbLightGenerationTriggerFlashAlloc(tle_to_RBTexture1(l, 1),
        (int32_t)lua_tonumber(l, 2));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationSmokeSignalsAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    pGenRes = rbLightGenerationSmokeSignalsAlloc(tle_to_RBTexture1(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationFireworksAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 2);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    pGenRes = rbLightGenerationFireworksAlloc(tle_to_RBTexture1(l, 1),
        (int32_t)lua_tonumber(l, 2));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationVerticalBarsAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 5);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    luaL_argcheck(l, tle_is_RBTexture1(l, 2), 2, "expected RBTexture1");
    luaL_argcheck(l, lua_isnumber(l, 3), 3, "expected number");
    luaL_argcheck(l, tle_is_RBTime(l, 4), 4, "expected RBTime");
    luaL_argcheck(l, tle_is_RBTime(l, 5), 5, "expected RBTime");
    
    pGenRes = rbLightGenerationVerticalBarsAlloc(tle_to_RBTexture1(l, 1),
        tle_to_RBTexture1(l, 2), (size_t)luaL_checknumber(l, 3),
        tle_to_RBTime(l, 4), tle_to_RBTime(l, 5));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    lua_pushvalue(l, 2);
    lua_rawseti(l, -2, 3);
    
    return 1;
}


int rbLuaLightGenerationCriscrossAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 4);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    luaL_argcheck(l, lua_isnumber(l, 2), 2, "expected number");
    luaL_argcheck(l, tle_is_RBTime(l, 3), 3, "expected RBTime");
    luaL_argcheck(l, tle_is_RBTime(l, 4), 4, "expected RBTime");
    
    pGenRes = rbLightGenerationCriscrossAlloc(tle_to_RBTexture1(l, 1),
        (size_t)luaL_checknumber(l, 2), tle_to_RBTime(l, 3),
        tle_to_RBTime(l, 4));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    
    return 1;
}


int rbLuaLightGenerationVolumeBarsAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 2);
    luaL_argcheck(l, tle_is_RBTexture1(l, 1), 1, "expected RBTexture1");
    luaL_argcheck(l, tle_is_RBTexture1(l, 2), 2, "expected RBTexture1");
    pGenRes = rbLightGenerationVolumeBarsAlloc(tle_to_RBTexture1(l, 1),
        tle_to_RBTexture1(l, 2));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    // Reference params so they're not collected out from under us
    lua_pushvalue(l, 1);
    lua_rawseti(l, -2, 2);
    lua_pushvalue(l, 2);
    lua_rawseti(l, -2, 3);
    
    return 1;
}


int rbLuaLightGenerationBeatStarsAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    tle_verify(l, tle_is_RBColor(l, 1));
    pGenRes = rbLightGenerationBeatStarsAlloc(tle_to_RBColor(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    return 1;
}


int rbLuaLightGenerationIconCheckerboardAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    tle_verify(l, tle_is_RBColor(l, 1));
    pGenRes = rbLightGenerationIconCheckerboardAlloc(tle_to_RBColor(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    return 1;
}



int rbLuaLightGenerationPulseCheckerboardAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBColor(l, 1), 1, "expected RBColor");
    pGenRes = rbLightGenerationPulseCheckerboardAlloc(tle_to_RBColor(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    return 1;
}



int rbLuaLightGenerationParticleLissajousAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBColor(l, 1), 1, "expected RBColor");
    pGenRes = rbLightGenerationParticleLissajousAlloc(tle_to_RBColor(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    return 1;
}



int rbLuaLightGenerationSignalLissajousAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBColor(l, 1), 1, "expected RBColor");
    pGenRes = rbLightGenerationSignalLissajousAlloc(tle_to_RBColor(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
    return 1;
}



int rbLuaLightGenerationOscilloscopeAlloc(lua_State * l)
{
    RBLightGenerator * pGenRes;
    (void)l;

    tle_verify(l, lua_gettop(l) == 1);
    luaL_argcheck(l, tle_is_RBColor(l, 1), 1, "expected RBColor");
    pGenRes = rbLightGenerationOscilloscopeAlloc(tle_to_RBColor(l, 1));
    tle_push_PRBLightGenerator(l, pGenRes);
    
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
void rotterbox_init_lua(lua_State * l)
{
    int stack_top = lua_gettop(l);

    rotterbox_lua_register_types(l);

    lua_newtable(l);
    lua_pushvalue(l, -1);
    lua_setglobal(l, "_sequencer");
    lua_pushcfunction(l, &rotterbox_lua_sequencer_enqueue_data);
    lua_setfield(l, -2, "_enqueue_data");
    lua_pushcfunction(l, &rotterbox_lua_sequencer_get_play_time_wrapper);
    lua_setfield(l, -2, "_get_play_time");

    lua_settop(l, stack_top);
}
*/

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
