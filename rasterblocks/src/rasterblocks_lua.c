#include "rasterblocks_lua.h"

#include "graphics_util.h"
#include "tle.h"


// Bindings for:
// - tgui types: point, size, rect, event, widgetid, fontid
// - widget functions
// - draw functions
// - event handling wrappers: call through to lua handlers?
// - draw handlers: widget toolkit
//   - button, label... table?


int rbLuaRegisterTypes(lua_State * l);

int rbLuaPrint(lua_State * l);

int rbLuaSet(lua_State * l);
int rbLuaPrint(lua_State * l);

int rbLuaLightGenerationCompositorAlloc(lua_State * l);
int rbLuaLightGenerationStaticImageAlloc(lua_State * l);
int rbLuaLightGenerationImageFilterAlloc(lua_State * l);
int rbLuaLightGenerationRescaleAlloc(lua_State * l);
int rbLuaLightGenerationTimedRotationAlloc(lua_State * l);
int rbLuaLightGenerationControllerSelectAlloc(lua_State * l);
int rbLuaLightGenerationControllerFadeAlloc(lua_State * l);
int rbLuaLightGenerationTriggerFlashAlloc(lua_State * l);
int rbLuaLightGenerationPlasmaAlloc(lua_State * l);
int rbLuaLightGenerationBeatFlashAlloc(lua_State * l);
int rbLuaLightGenerationPulsePlasmaAlloc(lua_State * l);
int rbLuaLightGenerationPulseGridAlloc(lua_State * l);
int rbLuaLightGenerationDashedCirclesAlloc(lua_State * l);
int rbLuaLightGenerationSmokeSignalsAlloc(lua_State * l);
int rbLuaLightGenerationFireworksAlloc(lua_State * l);
int rbLuaLightGenerationVerticalBarsAlloc(lua_State * l);
int rbLuaLightGenerationCriscrossAlloc(lua_State * l);
int rbLuaLightGenerationVolumeBarsAlloc(lua_State * l);
int rbLuaLightGenerationBeatStarsAlloc(lua_State * l);
int rbLuaLightGenerationIconCheckerboardAlloc(lua_State * l);
int rbLuaLightGenerationPulseCheckerboardAlloc(lua_State * l);
int rbLuaLightGenerationParticleLissajousAlloc(lua_State * l);
int rbLuaLightGenerationSignalLissajousAlloc(lua_State * l);
int rbLuaLightGenerationOscilloscopeAlloc(lua_State * l);


// RBLightGenerator

static int tle_RBColor_metatable_index;
static int tle_RBTime_metatable_index;
static int tle_RBTexture1_metatable_index;
static int tle_RBTexture2_metatable_index;

static int tle_RBColor_eq(lua_State * l);

static int tle_RBTime_eq(lua_State * l);
static int tle_RBTime_lt(lua_State * l);
static int tle_RBTime_le(lua_State * l);


void rbLuaInitialize(RBConfiguration * pConfig)
{
    tle_initialize(pConfig->luaPath);
    lua_pushcfunction(tle_state, rbLuaRegisterTypes);
    rbVerify(tle_pcall(tle_state, 0, 0, false) == 0);
    tle_dostring(tle_state, "include('display.lua')", true);
}


void rbLuaShutdown(void)
{
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
    lua_pushcfunction(l, rbLuaPrint);
    lua_setfield(l, -2, "print");
    //lua_pushcfunction(l, rb_);
    //lua_setfield(l, -2, "_");
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
    
    if(p == NULL) {
        luaL_error(l, "lua_touserdata returned NULL in "
            "tle_to_RBColor");
    }
    return *p;
}


void tle_push_RBColor(lua_State * l, RBColor value)
{
    RBColor * p = (RBColor *)lua_newuserdata(l,
        sizeof (RBColor));
    if(p == NULL) {
        luaL_error(l, "lua_newuserdata returned NULL in "
            "tle_push_RBColor");
    }
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
    
    if(p == NULL) {
        luaL_error(l, "lua_touserdata returned NULL in "
            "tle_to_RBTime");
    }
    return *p;
}


void tle_push_RBTime(lua_State * l, RBTime value)
{
    RBTime * p = (RBTime *)lua_newuserdata(l,
        sizeof (RBTime));
    if(p == NULL) {
        luaL_error(l, "lua_newuserdata returned NULL in "
            "tle_push_RBTime");
    }
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
    
    if(p == NULL) {
        luaL_error(l, "lua_touserdata returned NULL in "
            "tle_to_RBTexture1");
    }
    return p;
}


void tle_push_RBTexture1(lua_State * l, RBTexture1 * value)
{
    size_t size = rbTexture1ComputeSize(rbTexture1GetWidth(value));
    RBTexture1 * p = (RBTexture1 *)lua_newuserdata(l, size);
    if(p == NULL) {
        luaL_error(l, "lua_newuserdata returned NULL in "
            "tle_push_RBTexture1");
    }
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
    
    if(p == NULL) {
        luaL_error(l, "lua_touserdata returned NULL in "
            "tle_to_RBTexture2");
    }
    return p;
}


void tle_push_RBTexture2(lua_State * l, RBTexture2 * value)
{
    size_t size = rbTexture2ComputeSize(rbTexture2GetWidth(value),
        rbTexture2GetHeight(value));
    RBTexture2 * p = (RBTexture2 *)lua_newuserdata(l, size);
    if(p == NULL) {
        luaL_error(l, "lua_newuserdata returned NULL in "
            "tle_push_RBTexture2");
    }
    memcpy(p, value, size);
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_RBTexture2_metatable_index);
    lua_setmetatable(l, -2);
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
