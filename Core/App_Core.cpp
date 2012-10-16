#include <SDL/SDL_main.h>
#include "App.h"

int main (int argc, char * argv[])
{
	// Create a Lua state and load its standard libraries.
	lua_State * L = luaL_newstate();

	luaL_openlibs(L);

	// Give Lua some useful tools.
	luaopen_class(L);
	luaopen_dirent(L);
	luaopen_graphics(L);
	luaopen_misc(L);
	luaopen_sequence(L);
	luaopen_sdl(L);
	luaopen_state(L);
	luaopen_ui(L);
	luaopen_voxel(L);

	// Boot and run the application, and close Lua when done.
	luaL_loadfile(L, "Scripts/Boot.txt");	// boot
	lua_call(L, 0, 0);
	lua_close(L);

	return 0;
}