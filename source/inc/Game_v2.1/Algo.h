
#ifdef CALUA_COMPAT
#include <caLua.h> //CHANGED: Removed CaLua dependency
#include <caLua.h> //CHANGED: Removed CaLua dependency
#include <lualib.h>
#include <lauxlib.h>
#else
#include <lua.hpp>
#endif

lua_State* init_lua();
void exit_lua(lua_State* L);
int load_luc(lua_State* L, char const* fname);
