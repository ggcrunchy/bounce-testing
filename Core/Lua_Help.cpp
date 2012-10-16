#include "App.h"
#include <iostream>

namespace Lua
{
	// @brief Posts an error and prints a string
	void Post (lua_State * L, char const * message, bool bError)
	{
		for (int level = 0; ; ++level)
		{
			lua_Debug debug = { 0 };

			if (lua_getstack(L, level, &debug) == 0) break;

			lua_getinfo(L, "Sl", &debug);

			std::cerr << "Level " << level << std::endl;
			std::cerr << "\tsource: " << debug.source << std::endl;
			
			if (debug.currentline != -1) std::cerr << "\tcurrentline: " << debug.currentline << std::endl;
		}

		if (bError)
		{
			lua_pushstring(L, message);
			lua_error(L);
		}
	}

	/// @brief Validates and returns a Uint argument
	Uint U (lua_State * L, int index)
	{
		return static_cast<Uint>(luaL_checkint(L, index));
	}

	/// @brief Validates and returns a Uint8 argument
	Uint8 U8 (lua_State * L, int index)
	{
		return static_cast<Uint8>(luaL_checkint(L, index));
	}

	/// @brief Validates and returns a Uint16 argument
	Uint16 U16 (lua_State * L, int index)
	{
		return static_cast<Uint16>(luaL_checknumber(L, index));
	}

	/// @brief Validates and returns a Sint32 argument
	Sint32 S32 (lua_State * L, int index)
	{
		return static_cast<Sint32>(luaL_checknumber(L, index));
	}

	/// @brief Validates and returns a Sint16 argument
	Sint16 S16 (lua_State * L, int index)
	{
		return static_cast<Sint16>(luaL_checknumber(L, index));
	}

	/// @brief Validates and returns an int argument
	int I (lua_State * L, int index)
	{
		return luaL_checkint(L, index);
	}

	/// @brief Validates and returns a long int argument
	long LI (lua_State * L, int index)
	{
		return luaL_checklong(L, index);
	}

	/// @brief Validates and return a float argument
	float F (lua_State * L, int index)
	{
		return static_cast<float>(luaL_checknumber(L, index));
	}

	/// @brief Validates and returns a bool argument
	bool B (lua_State * L, int index)
	{
		luaL_checktype(L, index, LUA_TBOOLEAN);

		return lua_toboolean(L, index) != 0;
	}

	/// @brief Validates and returns a char const * argument
	char const * S (lua_State * L, int index)
	{
		return luaL_checkstring(L, index);
	}

	/// @brief Validates and returns a void * argument
	void * UD (lua_State * L, int index)
	{
		if (!lua_isuserdata(L, index)) luaL_error(L, "Argument %d is not a userdata", index);

		return lua_touserdata(L, index);
	}

	/// @brief Validates and returns a void * argument
	void * UT (lua_State * L, int index)
	{
		luaL_checktype(L, index, LUA_TUSERDATA);

		lua_getmetatable(L, index);	// ..., ud, ..., mt
		lua_getfield(L, -1, "size");// ..., ud, ..., mt, size

		int size = I(L, -1);

		lua_pop(L, 2);	// ..., ud, ...

		void * data = lua_touserdata(L, index);

		return size != 0 ? data : *(void**)data;
	}

	/// @brief Validates and returns a Uint8 * argument
	Uint8 * PU8 (lua_State * L, int index)
	{
		return static_cast<Uint8*>(UD(L, index));
	}

	/// @brief Void return; void argument
	int V_V (lua_State * L, void (*func)(void))
	{
		func();

		return 0;
	}

	/// @brief Void return; unsigned argument
	int V_U (lua_State * L, void (*func)(Uint))
	{
		func(U(L, 1));

		return 0;
	}

	/// @brief Int return; void argument
	int I_V (lua_State * L, int (*func)(void))
	{
		func();

		return 0;
	}
}