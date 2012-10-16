#include "App.h"
#include "../Voxel/Voxel.h"
#include "../Voxel/Sphere.h"

/// @brief Iterates over a sphere volumetrically
/// @param center Sphere center, relative to space origin
/// @param radius Sphere radius
/// @param dx Extent of space cell in x-direction
/// @param dy Extent of space cell in y-direction
/// @param dz Extent of space cell in z-direction
/// @param func Function used to process spans
/// @param order Order used to obtain spans
/// @param context User-supplied context passed to span function
static void RenderSphereVolume (float center[3], float radius, float dx, float dy, float dz, void (*func)(int, int, int, int, void *), Voxel::Order order, void * context)
{
	if (radius < 0.0f) return;
	if (dx < 0.0f) return;
	if (dy < 0.0f) return;
	if (dz < 0.0f) return;

	// Render the sphere.
	Voxel::Sphere s(center, radius, dx, dy, dz, order);

	for (Voxel::ColumnIterF vci = s.begin(); vci != s.end(); ++vci)
	{
		for (Voxel::RowIterF vri = vci.begin(); vri != vci.end(); ++vri)
		{
			for (Voxel::SpanIterF vsi = vri.begin(); vsi != vri.end(); ++vsi)
			{
				func(*vci, *vri, vsi.I(), vsi.F(), context);
			}
		}
	}
}

///
/// BINDING
///
static void Callback (int y, int z, int x1, int x2, void * context)
{
	lua_State * L = static_cast<lua_State*>(context);

	lua_getglobal(L, "CFunc");	// CFunc
	lua_pushinteger(L, y);	// CFunc, z
	lua_pushinteger(L, z);	// CFunc, z, y
	lua_pushinteger(L, x1);	// CFunc, z, y, x1
	lua_pushinteger(L, x2);	// CFunc, z, y, x1, x2
	lua_call(L, 4, 0);
}

static int RenderSphereVolume (lua_State * L)
{
	lua_pushvalue(L, 6);// ..., func
	lua_setglobal(L, "CFunc");	// _G.CFunc = func

	RenderSphereVolume(static_cast<Lua::AppTypes::Vector*>(Lua::UD(L, 1))->m, Lua::F(L, 2), Lua::F(L, 3), Lua::F(L, 4), Lua::F(L, 5), Callback, Voxel::eYZX, L);

	return 0;
}

void luaopen_voxel (lua_State * L)
{
	lua_pushcfunction(L, RenderSphereVolume);
	lua_setglobal(L, "RenderSphereVolume");
}