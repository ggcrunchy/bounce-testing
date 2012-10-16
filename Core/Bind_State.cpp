#include "App.h"
#include <cassert>

/// @brief Binds the state library to the Lua scripting system
void luaopen_state (lua_State * L)
{
	assert(L != 0);

	luaL_loadfile(L, "Scripts/State.txt");
	lua_call(L, 0, 0);
}