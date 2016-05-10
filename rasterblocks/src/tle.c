#include "tle.h"


lua_State * tle_state;
int tle_traceback_index;
int tle_lua_module_table_index;
int tle_lua_globals_metatable_index;

jmp_buf tle_panic_target;
int tle_in_protected_function;
char const * tle_script_base;


int tle_lua_atpanic_handler(lua_State * L);
int tle_initialize_state(lua_State * L);
int tle_lua_include(lua_State * L);
int tle_lua_import(lua_State * L);


#define TLE_PROTECT_PROLOGUE \
    ++tle_in_protected_function; \
    if(setjmp(tle_panic_target) != 0) { \
        exit(1); \
    }

#define TLE_PROTECT_EPILOGUE \
    --tle_in_protected_function;


void tle_initialize(char const * script_base)
{
    tle_state = luaL_newstate();

    tle_script_base = script_base;

    TLE_PROTECT_PROLOGUE

    lua_atpanic(tle_state, tle_lua_atpanic_handler);

    tle_initialize_state(tle_state);

    TLE_PROTECT_EPILOGUE
}


void tle_finalize(void)
{
    lua_close(tle_state);
}


int tle_lua_atpanic_handler(lua_State * L)
{
    printf("error during Lua setup: %s\n", lua_tostring(L, -1));
    longjmp(tle_panic_target, 1);
}


int tle_initialize_state(lua_State * L)
{
    int top = lua_gettop(L);
    
    luaL_openlibs(L);
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    tle_traceback_index = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);
    
    lua_newtable(L);
    tle_lua_module_table_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_newtable(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    lua_setfield(L, -2, "__index");
    tle_lua_globals_metatable_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushcfunction(L, &tle_lua_include);
    lua_setglobal(L, "include");
    
    lua_pushcfunction(L, &tle_lua_import);
    lua_setglobal(L, "import");
    
    lua_settop(L, top);

    return 0;
}


int tle_dostring(lua_State * L, char const * code, bool newenv)
{
    int result;
    
    TLE_PROTECT_PROLOGUE

    luaL_loadstring(L, code);
    result = tle_pcall(L, 0, 0, newenv);

    TLE_PROTECT_EPILOGUE

    return result;
}


int tle_dofile(lua_State * L, char const * path, bool newenv)
{
    int result;
    
    TLE_PROTECT_PROLOGUE

    lua_pushcfunction(L, &tle_lua_include);
    lua_pushstring(L, path);
    result = tle_pcall(L, 1, 0, newenv);

    TLE_PROTECT_EPILOGUE

    return result;
}


int tle_pcall(lua_State * L, int nargs, int nresults, bool newenv)
{
    int result;

    TLE_PROTECT_PROLOGUE

    lua_rawgeti(L, LUA_REGISTRYINDEX, tle_traceback_index);
    // traceback function has to go back past the arguments and function
    lua_insert(L, -2 - nargs);

    if(newenv) {
        lua_newtable(L);
        lua_rawgeti(L, LUA_REGISTRYINDEX, tle_lua_globals_metatable_index);
        lua_setmetatable(L, -2);
        lua_rawseti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    }

    result = lua_pcall(L, nargs, nresults, -2 - nargs);
    if(result != 0) {
        printf("error: %d\n", result);
        printf("%s\n", lua_tostring(L, -1));
    }

    TLE_PROTECT_EPILOGUE

    return result;
}


int tle_lua_include(lua_State * L)
{
    int top = lua_gettop(L);
    int result = -1;
    
    if(top != 1) {
        luaL_error(L, "expected 1 parameter, string module name");
    }
    luaL_argcheck(L, lua_isstring(L, 1), 1, "expected string module name");
    
    // make the full path to the desired module
    lua_pushstring(L, tle_script_base);
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
    
    // load the module!
    result = luaL_loadfile(L, lua_tostring(L, -1));
    if(result != 0) {
        // syntax error, file not found, or some such occurred
        lua_error(L);
    }
    
    // execute the module!
    lua_call(L, 0, 0);
    
    return 0;
}


int tle_lua_import(lua_State * L)
{
    int top = lua_gettop(L);
    int result = -1;
    int module_table_index = 0;
    int module_interface_index = 0;
    lua_Debug ar;
    
    if(top != 1) {
        luaL_error(L, "expected 1 parameter, string module name");
    }
    luaL_argcheck(L, lua_isstring(L, 1), 1, "expected string module name");
    
    // try to find the table in the loaded-modules table
    lua_rawgeti(L, LUA_REGISTRYINDEX, tle_lua_module_table_index);
    module_table_index = lua_gettop(L);
    lua_pushvalue(L, 1); // name of the module loaded
    lua_gettable(L, -2); // loaded-modules table
    module_interface_index = lua_gettop(L);
    if(lua_isboolean(L, -1)) {
        luaL_error(L, "circular import of %s (or import failed earlier)",
            lua_tostring(L, 1));
    }
    if(!lua_istable(L, -1)) {
        // module isn't loaded, load it
        int load_top = lua_gettop(L);
        
        // mark this module as loading
        lua_pushvalue(L, 1); // name of the module loaded
        lua_pushboolean(L, false);
        lua_settable(L, module_table_index);
        
        // make the full path to the desired module
        lua_pushstring(L, tle_script_base);
        lua_pushvalue(L, 1); // name of the module loaded
        lua_pushstring(L, ".lua");
        lua_concat(L, 3);
        
        // load the module!
        result = luaL_loadfile(L, lua_tostring(L, -1));
        if(result != 0) {
            // syntax error, file not found, or some such occurred
            lua_error(L);
        }
        
        // create the environment table -- it will exist only on the stack
        // until after the module is loaded, so that module load errors won't
        // create a global
        lua_newtable(L);
        
        // give the environment a metatable that refers back to the globals
        // table so that the default lua functions are accessible
        lua_rawgeti(L, LUA_REGISTRYINDEX,
            tle_lua_globals_metatable_index);
        lua_setmetatable(L, -2); // module environment table
        
        // use the fresh table as the default public interface
        lua_pushvalue(L, -1); // module environment table
        lua_replace(L, module_interface_index);
        // and the global environment for this module
        lua_setupvalue(L, -2, 1); // module chunk
        
        // execute the module!
        lua_call(L, 0, 1);
        
        // did the module specify a public interface?
        if(!lua_isnil(L, -1)) {
            // it did! make sure it's valid
            if(!lua_istable(L, -1)) {
                luaL_error(L, "module returned non-table environment");
            }
            lua_replace(L, module_interface_index);
        } else {
            // no useful return value, default to using the environment table
            // as the public interace
            lua_pop(L, 1);
        }
        
        // we're still here, so no errors -- commit the module environment to
        // the loaded-modules table
        lua_pushvalue(L, 1); // name of the module loaded
        lua_pushvalue(L, module_interface_index);
        lua_settable(L, module_table_index);
        
        lua_settop(L, load_top);
    }
    
    // add the module table to the caller's environment
    if(lua_getstack(L, 1, &ar) == 0) {
        luaL_error(L, "unable to determine caller on import");
    }
    lua_getinfo(L, "f", &ar);
    if(lua_isnil(L, -1)) {
        luaL_error(L, "no function environment in caller");
    }
    // get the caller's environment table
    lua_getupvalue(L, -1, 1); // caller function
    lua_pushvalue(L, 1); // name of the module loaded
    lua_pushvalue(L, module_interface_index);
    // add the imported module to the caller's environment
    lua_settable(L, -3); // caller's environment table
    
    lua_settop(L, top);
    
    return 1;
}


