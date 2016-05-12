#ifndef TLE_H
#define TLE_H


#include "rasterblocks.h"


extern lua_State * tle_state;
extern int tle_lua_module_table_index;
extern int tle_lua_globals_metatable_index;

extern void tle_initialize(char const * script_base);
extern void tle_finalize();
extern int tle_dostring(lua_State * l, char const * code, bool newenv);
extern int tle_dofile(lua_State * l, char const * path, bool newenv);
extern int tle_pcall(lua_State * l, int nargs, int nresults, bool newenv);

#define tle_verify(l, expr) \
    do { \
        if(!(expr)) { \
            luaL_error(l, "tle_verify fail: expected " #expr); \
        } \
    } while(0)


// witness my macro-fu. this is terrifying and yet compellingly convenient...


#define TLE_MAKE_WRAPPER_0(wrapper_name, return_type, call_through)\
static int wrapper_name(lua_State * l)\
{\
    return_type result;\
    result = call_through(l);\
    tle_push_##return_type(l, result);\
    return 1;\
}\
/* end define TLE_MAKE_WRAPPER_0 */


#define TLE_MAKE_WRAPPER_0_VOID(wrapper_name, call_through)\
static int wrapper_name(lua_State * l)\
{\
    call_through(l);\
    return 0;\
}\
/* end define TLE_MAKE_WRAPPER_0_VOID */


#define TLE_MAKE_WRAPPER_1(wrapper_name, return_type, parameter_1_type,\
    call_through)\
static int wrapper_name(lua_State * l)\
{\
    return_type result;\
    parameter_1_type parameter_1;\
    luaL_argcheck(l, tle_is_##parameter_1_type(l, 1), 1,\
        "expected " #parameter_1_type);\
    parameter_1 = tle_to_##parameter_1_type(l, 1);\
    result = call_through(l, parameter_1);\
    tle_push_##return_type(l, result);\
    return 1;\
}\
/* end define TLE_MAKE_WRAPPER_1 */


#define TLE_MAKE_WRAPPER_1_VOID(wrapper_name, parameter_1_type,\
    call_through)\
static int wrapper_name(lua_State * l)\
{\
    parameter_1_type parameter_1;\
    luaL_argcheck(l, tle_is_##parameter_1_type(l, 1), 1,\
        "expected " #parameter_1_type);\
    parameter_1 = tle_to_##parameter_1_type(l, 1);\
    call_through(l, parameter_1);\
    return 0;\
}\
/* end define TLE_MAKE_WRAPPER_1_VOID */


#define TLE_MAKE_WRAPPER_2(wrapper_name, return_type, parameter_1_type,\
    parameter_2_type, call_through)\
static int wrapper_name(lua_State * l)\
{\
    return_type result;\
    parameter_1_type parameter_1;\
    parameter_2_type parameter_2;\
    luaL_argcheck(l, tle_is_##parameter_1_type(l, 1), 1,\
        "expected " #parameter_1_type);\
    luaL_argcheck(l, tle_is_##parameter_2_type(l, 2), 2,\
        "expected " #parameter_2_type);\
    parameter_1 = tle_to_##parameter_1_type(l, 1);\
    parameter_2 = tle_to_##parameter_2_type(l, 2);\
    result = call_through(l, parameter_1, parameter_2);\
    tle_push_##return_type(l, result);\
    return 1;\
}\
/* end define TLE_MAKE_WRAPPER_2 */


#define TLE_MAKE_WRAPPER_2_VOID(wrapper_name, return_type, parameter_1_type,\
    parameter_2_type, call_through)\
static int wrapper_name(lua_State * l)\
{\
    parameter_1_type parameter_1;\
    parameter_2_type parameter_2;\
    luaL_argcheck(l, tle_is_##parameter_1_type(l, 1), 1,\
        "expected " #parameter_1_type);\
    luaL_argcheck(l, tle_is_##parameter_2_type(l, 2), 2,\
        "expected " #parameter_2_type);\
    parameter_1 = tle_to_##parameter_1_type(l, 1);\
    parameter_2 = tle_to_##parameter_2_type(l, 2);\
    call_through(l, parameter_1, parameter_2);\
    tle_push_##return_type(l, result);\
    return 0;\
}\
/* end define TLE_MAKE_WRAPPER_2 */


#define TLE_START_OVERLOAD_WRAPPER(wrapper_name)\
static int wrapper_name(lua_State * l)\
{\
    int failed_match_count = 0;\
    int num_params = lua_gettop(l);\
    lua_pushstring(l, "overloaded function failed to find parameter match");\
/* end define TLE_START_OVERLOAD_WRAPPER */

#define TLE_MAKE_OVERLOAD_1(return_type, parameter_1_type, call_through)\
    if((num_params == 1) && tle_is_##parameter_1_type(l, 1)) {\
        return_type result;\
        parameter_1_type parameter_1;\
        parameter_1 = tle_to_##parameter_1_type(l, 1);\
        result = call_through(l, parameter_1);\
        tle_push_##return_type(l, result);\
        return 1;\
    }\
    lua_pushstring(l, "\n  failed match: " #parameter_1_type);\
    ++failed_match_count;\
/* end define TLE_MAKE_OVERLOAD_1 */

#define TLE_MAKE_OVERLOAD_2(return_type, parameter_1_type,\
    parameter_2_type, call_through)\
\
    if((num_params == 2) && tle_is_##parameter_1_type(l, 1)\
        && tle_is_##parameter_2_type(l, 2))\
    {\
        return_type result;\
        parameter_1_type parameter_1;\
        parameter_2_type parameter_2;\
        parameter_1 = tle_to_##parameter_1_type(l, 1);\
        parameter_2 = tle_to_##parameter_2_type(l, 2);\
        result = call_through(l, parameter_1, parameter_2);\
        tle_push_##return_type(l, result);\
        return 1;\
    }\
    lua_pushstring(l, "\n  failed match: " #parameter_1_type ", "\
        #parameter_2_type);\
    ++failed_match_count;\
/* end define TLE_MAKE_OVERLOAD_2 */

#define TLE_END_OVERLOAD_WRAPPER\
    if(failed_match_count == 0) {\
        lua_pushstring(l, "  no matches attempted: error in wrapper");\
        ++failed_match_count;\
    }\
    lua_concat(l, failed_match_count + 1);\
    lua_error(l);\
    /* we should never get here */\
    return 0;\
}\
/* end define TLE_END_OVERLOAD_WRAPPER */


static inline bool tle_is_int32_t(lua_State * l, int idx)
{
    return lua_isnumber(l, idx);
}


static inline int32_t tle_to_int32_t(lua_State * l, int idx)
{
    return (int32_t)lua_tonumber(l, idx);
}


static inline void tle_push_int32_t(lua_State * l, int32_t value)
{
    lua_pushnumber(l, (lua_Number)value);
}


static inline bool tle_is_bool(lua_State * l, int idx)
{
    return lua_isboolean(l, idx);
}


static inline bool tle_to_bool(lua_State * l, int idx)
{
    return lua_toboolean(l, idx);
}


static inline void tle_push_bool(lua_State * l, bool value)
{
    lua_pushboolean(l, value);
}


#endif // TLE_H


