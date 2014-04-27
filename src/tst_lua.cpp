extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <stdio.h>

    // expose c's API
    int barfunc(int foo)
    {
        /* a dummy function to test with FFI */
        return foo + 1;
    }
}

int
main(int argc, char* argv[])
{
    int status, result;
    lua_State *L;
    L = luaL_newstate();

    luaL_openlibs(L);

    /* Load the file containing the script we are going to run */
    status = luaL_loadfile(L, argv[1]);
    if (status) {
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    /* Ask Lua to run our little script */
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    lua_close(L);   /* Cya, Lua */

    return 0;
}
