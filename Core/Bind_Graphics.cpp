#include "App.h"
#include "../Graphics/Graphics.h"
#include <string>

static bool bInit;

///
/// Type handlers
///
template<typename T> static T * UTT (lua_State * L, int index)
{
	return static_cast<T*>(Lua::UT(L, index));
}

static inline Graphics::Picture * UP_ (lua_State * L, int index)
{
	return UTT<Graphics::Picture>(L, index);
}

static inline Lua::AppTypes::Sphere * US_ (lua_State * L, int index)
{
	return UTT<Lua::AppTypes::Sphere>(L, index);
}

static inline float * UV_ (lua_State * L, int index)
{
	return UTT<Lua::AppTypes::Vector>(L, index)->m;
}

///
/// Utility methods
///
static int F3 (lua_State * L, void (*func)(float[3]))
{
	func(UV_(L, 1));

	return 0;
}

static int F4 (lua_State * L, void (*func)(float, float, float, float))
{
	func(Lua::F(L, 1), Lua::F(L, 2), Lua::F(L, 3), Lua::F(L, 4));

	return 0;
}

static int I4 (lua_State * L, void (*func)(int, int, int, int))
{
	func(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::I(L, 4));

	return 0;
}

static int PushR (lua_State * L, float R[3])
{
	Lua::PushUserType(L, &Lua::AppTypes::Vector(R), "Vector");

	return 1;
}

template<typename T> int I_UT (lua_State * L, int (*func)(T *))
{
	func(UTT<T>(L, 1));

	return 0;
}

template<typename T> static int UT_F4 (lua_State * L, int (*func)(T *, float, float, float, float))
{
	func(UTT<T>(L, 1), Lua::F(L, 2), Lua::F(L, 3), Lua::F(L, 4), Lua::F(L, 5));

	return 0;
}

template<typename T> static int UT_I4 (lua_State * L, int (*func)(T *, int, int, int, int))
{
	func(UTT<T>(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::I(L, 4), Lua::I(L, 5));

	return 0;
}

template<typename T> static int Load (lua_State * L, std::string const & name, T datum)
{
	if (datum != 0)
	{
		Lua::PushUserType(L, datum, name.c_str());

		return 1;
	}

	return 0;
}

/* Font */
static int GetTextSize (lua_State * L)
{
	int width, height;	Graphics::GetTextSize(Lua::UT(L, 1), Lua::S(L, 2), width, height);

	lua_pushnumber(L, width);
	lua_pushnumber(L, height);

	return 2;
}

static int UnloadFont (lua_State * L)
{
	return I_UT(L, Graphics::UnloadFont);
}

static int LoadFont (lua_State * L)
{
	return Load<Graphics::Font_h>(L, "Font", Graphics::LoadFont(Lua::S(L, 1), Lua::I(L, 2)));
}

/* Picture */
static int DrawPicture (lua_State * L)
{
	return UT_I4(L, Graphics::DrawPicture);
}

static int GetPictureTexels (lua_State * L)
{
	float fS0, fT0, fS1, fT1;	Graphics::GetPictureTexels(UP_(L, 1), fS0, fT0, fS1, fT1);

	lua_pushnumber(L, fS0);
	lua_pushnumber(L, fT0);
	lua_pushnumber(L, fS1);
	lua_pushnumber(L, fT1);

	return 4;
}

static int SetPictureTexels (lua_State * L)
{
	return UT_F4(L, Graphics::SetPictureTexels);
}

static int UnloadPicture (lua_State * L)
{
	return I_UT(L, Graphics::UnloadPicture);
}

static int LoadPicture (lua_State * L)
{
	return Load<Graphics::Picture*>(L, "Picture", Graphics::LoadPicture(Lua::S(L, 1), Lua::F(L, 2), Lua::F(L, 3), Lua::F(L, 4), Lua::F(L, 5)));
}

/* TextImage */
static int DrawTextImage (lua_State * L)
{
	return UT_I4(L, Graphics::DrawTextImage);
}

static int UnloadTextImage (lua_State * L)
{
	return I_UT(L, Graphics::UnloadTextImage);
}

static int LoadTextImage (lua_State * L)
{
	SDL_Color color;

	color.r = Lua::U8(L, 3);
	color.g = Lua::U8(L, 4);
	color.b = Lua::U8(L, 5);

	return Load<Graphics::TextImage*>(L, "TextImage", Graphics::LoadTextImage(Lua::UT(L, 1), Lua::S(L, 2), color));
}

/* Main */
static int Close (lua_State * L)
{
	bInit = false;

	return Lua::V_V(L, Graphics::Close);
}

static int DrawBox (lua_State * L)
{
	Graphics::DrawBox(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::I(L, 4), Lua::B(L, 5));

	return 0;
}

static int DrawDisk (lua_State * L)
{
	Graphics::DrawDisk(UV_(L, 1), Lua::F(L, 2), Lua::F(L, 3), Lua::F(L, 4), Lua::I(L, 5), Lua::I(L, 6));

	return 0;
}

static int DrawGrid (lua_State * L)
{
	Graphics::DrawGrid(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::I(L, 4), Lua::U(L, 5), Lua::U(L, 6));

	return 0;
}

static int DrawLine (lua_State * L)
{
	return I4(L, Graphics::DrawLine);
}

static int DrawLine3D (lua_State * L)
{
	Graphics::DrawLine3D(UV_(L, 1), UV_(L, 2));

	return 0;
}

static int DrawQuad (lua_State * L)
{
	Graphics::DrawQuad(UV_(L, 1), UV_(L, 2), UV_(L, 3), UV_(L, 4), UV_(L, 5));

	return 0;
}

static int DrawSphere (lua_State * L)
{
	Graphics::DrawSphere(UV_(L, 1), Lua::F(L, 2), Lua::I(L, 3), Lua::I(L, 4));

	return 0;
}

static int Enter2D (lua_State * L)
{
	return Lua::V_V(L, Graphics::Enter2D);
}

static int Enter3D (lua_State * L)
{
	return Lua::V_V(L, Graphics::Enter3D);
}

static int GetColor (lua_State * L)
{
	Graphics::Color color;	Graphics::GetColor(color);

	return PushR(L, color);
}

static int GetVideoSize (lua_State * L)
{
	Uint32 width, height;	Graphics::GetVideoSize(width, height);

	lua_pushnumber(L, width);
	lua_pushnumber(L, height);

	return 2;
}

static int Prepare (lua_State * L)
{
	return Lua::V_V(L, Graphics::Prepare);
}

static int Render (lua_State * L)
{
	return Lua::V_V(L, Graphics::Render);
}

static int SetBounds (lua_State * L)
{
	return I4(L, Graphics::SetBounds);
}

static int SetClipPlanes (lua_State * L)
{
	Graphics::SetClipPlanes(Lua::F(L, 1), Lua::F(L, 2));

	return 0;
}

static int SetColor (lua_State * L)
{
	return F3(L, Graphics::SetColor);
}

static int SetEye (lua_State * L)
{
	return F3(L, Graphics::SetEye);
}

static int SetFOV (lua_State * L)
{
	Graphics::SetFOV(Lua::F(L, 1));

	return 0;
}

static int SetPicture (lua_State * L)
{
	Graphics::SetPicture(UP_(L, 1));

	return 0;
}

static int SetTarget (lua_State * L)
{
	return F3(L, Graphics::SetTarget);
}

static int Setup (lua_State * L)
{
	Graphics::Setup(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::B(L, 4));

	bInit = true;

	return 0;
}

static int SetUpVector (lua_State * L)
{
	return F3(L, Graphics::SetUpVector);
}

static int SetVideoMode (lua_State * L)
{
	Graphics::SetVideoMode(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::B(L, 4));

	return 0;
}

static int Set3DProjection (lua_State * L)
{
	Graphics::Set3DProjection(Lua::B(L, 1));

	return 0;
}

///
/// Garbage collectors
///
template<typename T> static int Unload (lua_State * L, int (*func)(T))
{
	return bInit ? I_UT(L, func) : 0;
}

static int GC_Font (lua_State * L)
{
	return Unload(L, Graphics::UnloadFont);
}

static int GC_Picture (lua_State * L)
{
	return Unload(L, Graphics::UnloadPicture);
}

static int GC_TextImage (lua_State * L)
{
	return Unload(L, Graphics::UnloadTextImage);
}

#undef UP_
#undef UQ_
#undef US_
#undef UV_
#define M_(w) { #w, w }

///
/// Function tables
///
static const luaL_reg GraphicsFuncs[] = {
	/* Font */
	M_(GetTextSize),
	M_(UnloadFont),
	M_(LoadFont),
	/* Picture */
	M_(DrawPicture),
	M_(GetPictureTexels),
	M_(SetPictureTexels),
	M_(UnloadPicture),
	M_(LoadPicture),
	/* TextImage */
	M_(DrawTextImage),
	M_(UnloadTextImage),
	M_(LoadTextImage),
	/* Main */
	M_(Close),
	M_(DrawBox),
	M_(DrawDisk),
	M_(DrawGrid),
	M_(DrawLine),
	M_(DrawLine3D),
	M_(DrawQuad),
	M_(DrawSphere),
	M_(Enter2D),
	M_(Enter3D),
	M_(GetColor),
	M_(GetVideoSize),
	M_(Prepare),
	M_(Render),
	M_(SetBounds),
	M_(SetClipPlanes),
	M_(SetColor),
	M_(SetEye),
	M_(SetFOV),
	M_(SetPicture),
	M_(SetTarget),
	M_(Setup),
	M_(SetUpVector),
	M_(SetVideoMode),
	M_(Set3DProjection),
	{ 0, 0 }
};

#undef M_

/// @brief Binds the graphics system to the Lua scripting system
void luaopen_graphics (lua_State * L)
{
	Lua::RegisterUserType(L, "Font", 0, 0, 0, "__gc", GC_Font);
	Lua::RegisterUserType(L, "Picture", 0, 0, 0, "__gc", GC_Picture);
	Lua::RegisterUserType(L, "TextImage", 0, 0, 0, "__gc", GC_TextImage);

	// Install tables.
	luaL_register(L, "Graphics", GraphicsFuncs);
}