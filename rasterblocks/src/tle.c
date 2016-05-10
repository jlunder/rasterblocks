#include "tle.h"


lua_State * tle_state;
int tle_traceback_index;
int tle_lua_module_table_index;
int tle_lua_globals_metatable_index;

jmp_buf tle_panic_target;
int tle_in_protected_function;
char tle_script_base[PATH_MAX];


int tle_lua_atpanic_handler(lua_State * l);
int tle_initialize_state(lua_State * l);
int tle_lua_include(lua_State * l);
int tle_lua_import(lua_State * l);


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

    rbStrlcpy(tle_script_base, script_base, sizeof tle_script_base);

    TLE_PROTECT_PROLOGUE

    lua_atpanic(tle_state, tle_lua_atpanic_handler);

    tle_initialize_state(tle_state);

    TLE_PROTECT_EPILOGUE
}


void tle_finalize(void)
{
    lua_close(tle_state);
}


int tle_lua_atpanic_handler(lua_State * l)
{
    printf("error during Lua setup: %s\n", lua_tostring(l, -1));
    longjmp(tle_panic_target, 1);
}


int tle_initialize_state(lua_State * l)
{
    int top = lua_gettop(l);
    
    luaL_openlibs(l);
    lua_getglobal(l, "debug");
    lua_getfield(l, -1, "traceback");
    tle_traceback_index = luaL_ref(l, LUA_REGISTRYINDEX);
    lua_pop(l, 1);
    
    lua_newtable(l);
    tle_lua_module_table_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_newtable(l);
    lua_rawgeti(l, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    lua_setfield(l, -2, "__index");
    tle_lua_globals_metatable_index = luaL_ref(l, LUA_REGISTRYINDEX);
    
    lua_pushcfunction(l, &tle_lua_include);
    lua_setglobal(l, "include");
    
    lua_pushcfunction(l, &tle_lua_import);
    lua_setglobal(l, "import");
    
    lua_settop(l, top);

    return 0;
}


int tle_dostring(lua_State * l, char const * code, bool newenv)
{
    int result;
    
    TLE_PROTECT_PROLOGUE

    luaL_loadstring(l, code);
    result = tle_pcall(l, 0, 0, newenv);

    TLE_PROTECT_EPILOGUE

    return result;
}


int tle_dofile(lua_State * l, char const * path, bool newenv)
{
    int result;
    
    TLE_PROTECT_PROLOGUE

    lua_pushcfunction(l, &tle_lua_include);
    lua_pushstring(l, path);
    result = tle_pcall(l, 1, 0, newenv);

    TLE_PROTECT_EPILOGUE

    return result;
}


int tle_pcall(lua_State * l, int nargs, int nresults, bool newenv)
{
    int result;

    TLE_PROTECT_PROLOGUE

    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_traceback_index);
    // traceback function has to go back past the arguments and function
    lua_insert(l, -2 - nargs);

    if(newenv) {
        lua_newtable(l);
        lua_rawgeti(l, LUA_REGISTRYINDEX, tle_lua_globals_metatable_index);
        lua_setmetatable(l, -2);
        lua_rawseti(l, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    }

    result = lua_pcall(l, nargs, nresults, -2 - nargs);
    if(result != 0) {
        printf("error: %d\n", result);
        printf("%s\n", lua_tostring(l, -1));
    }

    TLE_PROTECT_EPILOGUE

    return result;
}


int tle_lua_include(lua_State * l)
{
    int top = lua_gettop(l);
    int result = -1;
    
    if(top != 1) {
        luaL_error(l, "expected 1 parameter, string module name");
    }
    luaL_argcheck(l, lua_isstring(l, 1), 1, "expected string module name");
    
    // make the full path to the desired module
    lua_pushstring(l, tle_script_base);
    lua_pushvalue(l, 1);
    lua_concat(l, 2);
    
    // load the module!
    result = luaL_loadfile(l, lua_tostring(l, -1));
    if(result != 0) {
        // syntax error, file not found, or some such occurred
        lua_error(l);
    }
    
    // execute the module!
    lua_call(l, 0, 0);
    
    return 0;
}


int tle_lua_import(lua_State * l)
{
    int top = lua_gettop(l);
    int result = -1;
    int module_table_index = 0;
    int module_interface_index = 0;
    lua_Debug ar;
    
    if(top != 1) {
        luaL_error(l, "expected 1 parameter, string module name");
    }
    luaL_argcheck(l, lua_isstring(l, 1), 1, "expected string module name");
    
    // try to find the table in the loaded-modules table
    lua_rawgeti(l, LUA_REGISTRYINDEX, tle_lua_module_table_index);
    module_table_index = lua_gettop(l);
    lua_pushvalue(l, 1); // name of the module loaded
    lua_gettable(l, -2); // loaded-modules table
    module_interface_index = lua_gettop(l);
    if(lua_isboolean(l, -1)) {
        luaL_error(l, "circular import of %s (or import failed earlier)",
            lua_tostring(l, 1));
    }
    if(!lua_istable(l, -1)) {
        // module isn't loaded, load it
        int load_top = lua_gettop(l);
        
        // mark this module as loading
        lua_pushvalue(l, 1); // name of the module loaded
        lua_pushboolean(l, false);
        lua_settable(l, module_table_index);
        
        // make the full path to the desired module
        lua_pushstring(l, tle_script_base);
        lua_pushvalue(l, 1); // name of the module loaded
        lua_pushstring(l, ".lua");
        lua_concat(l, 3);
        
        // load the module!
        result = luaL_loadfile(l, lua_tostring(l, -1));
        if(result != 0) {
            // syntax error, file not found, or some such occurred
            lua_error(l);
        }
        
        // create the environment table -- it will exist only on the stack
        // until after the module is loaded, so that module load errors won't
        // create a global
        lua_newtable(l);
        
        // give the environment a metatable that refers back to the globals
        // table so that the default lua functions are accessible
        lua_rawgeti(l, LUA_REGISTRYINDEX,
            tle_lua_globals_metatable_index);
        lua_setmetatable(l, -2); // module environment table
        
        // use the fresh table as the default public interface
        lua_pushvalue(l, -1); // module environment table
        lua_replace(l, module_interface_index);
        // and the global environment for this module
        lua_setupvalue(l, -2, 1); // module chunk
        
        // execute the module!
        lua_call(l, 0, 1);
        
        // did the module specify a public interface?
        if(!lua_isnil(l, -1)) {
            // it did! make sure it's valid
            if(!lua_istable(l, -1)) {
                luaL_error(l, "module returned non-table environment");
            }
            lua_replace(l, module_interface_index);
        } else {
            // no useful return value, default to using the environment table
            // as the public interace
            lua_pop(l, 1);
        }
        
        // we're still here, so no errors -- commit the module environment to
        // the loaded-modules table
        lua_pushvalue(l, 1); // name of the module loaded
        lua_pushvalue(l, module_interface_index);
        lua_settable(l, module_table_index);
        
        lua_settop(l, load_top);
    }
    
    // add the module table to the caller's environment
    if(lua_getstack(l, 1, &ar) == 0) {
        luaL_error(l, "unable to determine caller on import");
    }
    lua_getinfo(l, "f", &ar);
    if(lua_isnil(l, -1)) {
        luaL_error(l, "no function environment in caller");
    }
    // get the caller's environment table
    lua_getupvalue(l, -1, 1); // caller function
    lua_pushvalue(l, 1); // name of the module loaded
    lua_pushvalue(l, module_interface_index);
    // add the imported module to the caller's environment
    lua_settable(l, -3); // caller's environment table
    
    lua_settop(l, top);
    
    return 1;
}


