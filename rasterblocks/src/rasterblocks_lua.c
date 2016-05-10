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


int rbLuaRegisterTypes(lua_State * L);

int rbLuaPrint(lua_State * L);

int rbLuaSet(lua_State * L);
int rbLuaPrint(lua_State * L);

int rbLuaLightGenerationCompositorAlloc(lua_State * L);
int rbLuaLightGenerationStaticImageAlloc(lua_State * L);
int rbLuaLightGenerationImageFilterAlloc(lua_State * L);
int rbLuaLightGenerationRescaleAlloc(lua_State * L);
int rbLuaLightGenerationTimedRotationAlloc(lua_State * L);
int rbLuaLightGenerationControllerSelectAlloc(lua_State * L);
int rbLuaLightGenerationControllerFadeAlloc(lua_State * L);
int rbLuaLightGenerationTriggerFlashAlloc(lua_State * L);
int rbLuaLightGenerationPlasmaAlloc(lua_State * L);
int rbLuaLightGenerationBeatFlashAlloc(lua_State * L);
int rbLuaLightGenerationPulsePlasmaAlloc(lua_State * L);
int rbLuaLightGenerationPulseGridAlloc(lua_State * L);
int rbLuaLightGenerationDashedCirclesAlloc(lua_State * L);
int rbLuaLightGenerationSmokeSignalsAlloc(lua_State * L);
int rbLuaLightGenerationFireworksAlloc(lua_State * L);
int rbLuaLightGenerationVerticalBarsAlloc(lua_State * L);
int rbLuaLightGenerationCriscrossAlloc(lua_State * L);
int rbLuaLightGenerationVolumeBarsAlloc(lua_State * L);
int rbLuaLightGenerationBeatStarsAlloc(lua_State * L);
int rbLuaLightGenerationIconCheckerboardAlloc(lua_State * L);
int rbLuaLightGenerationPulseCheckerboardAlloc(lua_State * L);
int rbLuaLightGenerationParticleLissajousAlloc(lua_State * L);
int rbLuaLightGenerationSignalLissajousAlloc(lua_State * L);
int rbLuaLightGenerationOscilloscopeAlloc(lua_State * L);


// RBLightGenerator

static int tle_RBColor_metatable_index;
static int tle_RBTime_metatable_index;
static int tle_RBTexture1_metatable_index;
static int tle_RBTexture2_metatable_index;

static int tle_RBColor_eq(lua_State * L);

static int tle_RBTime_eq(lua_State * L);
static int tle_RBTime_lt(lua_State * L);
static int tle_RBTime_le(lua_State * L);


void rbLuaInitialize(void)
{
    lua_pushcfunction(tle_state, rbLuaRegisterTypes);
    rbVerify(tle_pcall(tle_state, 0, 0, false) == 0);
}


int rbLuaRegisterTypes(lua_State * L)
{
    int top = lua_gettop(L);

    lua_newtable(L);
    lua_pushcfunction(L, tle_RBColor_eq);
    lua_setfield(L, -2, "__eq");
    tle_RBColor_metatable_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_newtable(L);
    lua_pushcfunction(L, tle_RBTime_eq);
    lua_setfield(L, -2, "__eq");
    lua_pushcfunction(L, tle_RBTime_lt);
    lua_setfield(L, -2, "__lt");
    lua_pushcfunction(L, tle_RBTime_le);
    lua_setfield(L, -2, "__le");
    tle_RBTime_metatable_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_newtable(L);
    tle_RBTexture1_metatable_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_newtable(L);
    tle_RBTexture2_metatable_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_newtable(L);
    lua_pushcfunction(L, rbLuaPrint);
    lua_setfield(L, -2, "print");
    //lua_pushcfunction(L, rb_);
    //lua_setfield(L, -2, "_");
    lua_setglobal(L, "rb");

    lua_settop(L, top);

    return 0;
}


int rbLuaPrint(lua_State * L)
{
    int top = lua_gettop(L);
    char const * str;

    str = luaL_checkstring(L, 1);
    printf("lua: %s\n", str);

    lua_settop(L, top);

    return 0;
}


// RBColor -------------------------------------------------------------


bool tle_is_RBColor(lua_State * L, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(L, idx) && lua_getmetatable(L, idx)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX,
            tle_RBColor_metatable_index);
        if(lua_rawequal(L, -1, -2)) {
            result = true;
        }
        lua_pop(L, 2);
    }
    
    return result;
}


RBColor tle_to_RBColor(lua_State * L, int idx)
{
    RBColor * p = (RBColor *)lua_touserdata(L, idx);
    
    if(p == NULL) {
        luaL_error(L, "lua_touserdata returned NULL in "
            "tle_to_RBColor");
    }
    return *p;
}


void tle_push_RBColor(lua_State * L, RBColor value)
{
    RBColor * p = (RBColor *)lua_newuserdata(L,
        sizeof (RBColor));
    if(p == NULL) {
        luaL_error(L, "lua_newuserdata returned NULL in "
            "tle_push_RBColor");
    }
    *p = value;
    lua_rawgeti(L, LUA_REGISTRYINDEX, tle_RBColor_metatable_index);
    lua_setmetatable(L, -2);
}


static bool tle_eq_RBColor_RBColor(lua_State * L,
    RBColor x, RBColor y)
{
    (void)L;

    return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a;
}


TLE_MAKE_WRAPPER_2(tle_RBColor_eq,
    bool, RBColor, RBColor,
    tle_eq_RBColor_RBColor)


// RBTime -------------------------------------------------------------


bool tle_is_RBTime(lua_State * L, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(L, idx) && lua_getmetatable(L, idx)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX,
            tle_RBTime_metatable_index);
        if(lua_rawequal(L, -1, -2)) {
            result = true;
        }
        lua_pop(L, 2);
    }
    
    return result;
}


RBTime tle_to_RBTime(lua_State * L, int idx)
{
    RBTime * p = (RBTime *)lua_touserdata(L, idx);
    
    if(p == NULL) {
        luaL_error(L, "lua_touserdata returned NULL in "
            "tle_to_RBTime");
    }
    return *p;
}


void tle_push_RBTime(lua_State * L, RBTime value)
{
    RBTime * p = (RBTime *)lua_newuserdata(L,
        sizeof (RBTime));
    if(p == NULL) {
        luaL_error(L, "lua_newuserdata returned NULL in "
            "tle_push_RBTime");
    }
    *p = value;
    lua_rawgeti(L, LUA_REGISTRYINDEX, tle_RBTime_metatable_index);
    lua_setmetatable(L, -2);
}


static bool tle_eq_RBTime_RBTime(lua_State * L,
    RBTime x, RBTime y)
{
    (void)L;

    return x == y;
}


TLE_MAKE_WRAPPER_2(tle_RBTime_eq,
    bool, RBTime, RBTime,
    tle_eq_RBTime_RBTime)


static bool tle_lt_RBTime_RBTime(lua_State * L,
    RBTime x, RBTime y)
{
    (void)L;

    return rbDiffTime(x, y) < 0;
}


TLE_MAKE_WRAPPER_2(tle_RBTime_lt,
    bool, RBTime, RBTime,
    tle_lt_RBTime_RBTime)


static bool tle_le_RBTime_RBTime(lua_State * L,
    RBTime x, RBTime y)
{
    (void)L;

    return rbDiffTime(x, y) <= 0;
}


TLE_MAKE_WRAPPER_2(tle_RBTime_le,
    bool, RBTime, RBTime,
    tle_le_RBTime_RBTime)


// RBTexture1 ----------------------------------------------------------


bool tle_is_RBTexture1(lua_State * L, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(L, idx) && lua_getmetatable(L, idx)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX,
            tle_RBTexture1_metatable_index);
        if(lua_rawequal(L, -1, -2)) {
            result = true;
        }
        lua_pop(L, 2);
    }
    
    return result;
}


RBTexture1 * tle_to_RBTexture1(lua_State * L, int idx)
{
    RBTexture1 * p = (RBTexture1 *)lua_touserdata(L, idx);
    
    if(p == NULL) {
        luaL_error(L, "lua_touserdata returned NULL in "
            "tle_to_RBTexture1");
    }
    return p;
}


void tle_push_RBTexture1(lua_State * L, RBTexture1 * value)
{
    size_t size = rbTexture1ComputeSize(rbTexture1GetWidth(value));
    RBTexture1 * p = (RBTexture1 *)lua_newuserdata(L, size);
    if(p == NULL) {
        luaL_error(L, "lua_newuserdata returned NULL in "
            "tle_push_RBTexture1");
    }
    memcpy(p, value, size);
    lua_rawgeti(L, LUA_REGISTRYINDEX, tle_RBTexture1_metatable_index);
    lua_setmetatable(L, -2);
}


// RBTexture2 ----------------------------------------------------------


bool tle_is_RBTexture2(lua_State * L, int idx)
{
    bool result = false;
    
    if(lua_isuserdata(L, idx) && lua_getmetatable(L, idx)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX,
            tle_RBTexture2_metatable_index);
        if(lua_rawequal(L, -1, -2)) {
            result = true;
        }
        lua_pop(L, 2);
    }
    
    return result;
}


RBTexture2 * tle_to_RBTexture2(lua_State * L, int idx)
{
    RBTexture2 * p = (RBTexture2 *)lua_touserdata(L, idx);
    
    if(p == NULL) {
        luaL_error(L, "lua_touserdata returned NULL in "
            "tle_to_RBTexture2");
    }
    return p;
}


void tle_push_RBTexture2(lua_State * L, RBTexture2 * value)
{
    size_t size = rbTexture2ComputeSize(rbTexture2GetWidth(value),
        rbTexture2GetHeight(value));
    RBTexture2 * p = (RBTexture2 *)lua_newuserdata(L, size);
    if(p == NULL) {
        luaL_error(L, "lua_newuserdata returned NULL in "
            "tle_push_RBTexture2");
    }
    memcpy(p, value, size);
    lua_rawgeti(L, LUA_REGISTRYINDEX, tle_RBTexture2_metatable_index);
    lua_setmetatable(L, -2);
}



/*
void rotterbox_init_lua(lua_State * L)
{
    int stack_top = lua_gettop(L);

    rotterbox_lua_register_types(L);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "_sequencer");
    lua_pushcfunction(L, &rotterbox_lua_sequencer_enqueue_data);
    lua_setfield(L, -2, "_enqueue_data");
    lua_pushcfunction(L, &rotterbox_lua_sequencer_get_play_time_wrapper);
    lua_setfield(L, -2, "_get_play_time");

    lua_settop(L, stack_top);
}
*/

/*
static inline sequencer_timestamp_t tle_to_sequencer_timespan_t(
    lua_State * L, int idx)
{
    lua_Number as_number = floor(lua_tonumber(L, idx));
    luaL_argcheck(L, (as_number <= SEQUENCER_TIMESPAN_MAX)
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
