#include <SDL/SDL.h>
#include "App.h"

static SDL_Event _Event;
static SDL_Rect _Rect;

#undef SDL_SaveBMP
#undef SDL_LoadBMP

#define U_(t, i) static_cast<t>(Lua::UT(L, i))
#define T_(t, i) static_cast<SDL_##t*>(Lua::UT(L, i))
#define I_(t, i) static_cast<SDL_##t>(Lua::I(L, i))
#define J_(t, i) static_cast<SDL##t>(Lua::I(L, i))
#define F_(n) { #n, SDL_##n }
#define C_(n) { #n, SDL_##n }
#define K_(n) { "K_"#n, SDLK_##n }
#define M_(n) { #n, n }
#define RT_(r, n, c, t) (r).mName = #n, (r).mUserType = 0, (r).mOffset = offsetof(SDL_##c, n), (r).mType = t
#define RU_(r, n, c, u) (r).mName = #n, (r).mUserType = u, (r).mOffset = offsetof(SDL_##c, n), (r).mType = Lua::UserType_Reg::eUserType

static int SDL_GetError (lua_State * L)
{
	lua_pushstring(L, SDL_GetError());

	return 1;
}

static int SDL_ClearError (lua_State * L)
{
	return Lua::V_V(L, SDL_ClearError);
}

static int SDL_GetTicks (lua_State * L)
{
	lua_pushnumber(L, SDL_GetTicks());

	return 1;
}

static int SDL_Delay (lua_State * L)
{
	return Lua::V_U(L, SDL_Delay);
}

static int SDL_NumJoysticks (lua_State * L)
{
	lua_pushnumber(L, SDL_NumJoysticks());

	return 1;
}

static int SDL_JoystickName (lua_State * L)
{
	lua_pushstring(L, SDL_JoystickName(Lua::I(L, 1)));

	return 1;
}

static int SDL_JoystickOpen (lua_State * L)
{
	Lua::PushUserType(L, SDL_JoystickOpen(Lua::I(L, 1)), "Joystick");

	return 1;
}

static int SDL_JoystickOpened (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickOpened(Lua::I(L, 1)));

	return 1;
}

static int SDL_JoystickIndex (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickIndex(T_(Joystick, 1)));

	return 1;
}

static int SDL_JoystickNumAxes (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickNumAxes(T_(Joystick, 1)));

	return 1;
}

static int SDL_JoystickNumBalls (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickNumBalls(T_(Joystick, 1)));

	return 1;
}

static int SDL_JoystickNumHats (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickNumHats(T_(Joystick, 1)));

	return 1;
}

static int SDL_JoystickNumButtons (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickNumButtons(T_(Joystick, 1)));

	return 1;
}

static int SDL_JoystickUpdate (lua_State * L)
{
	return Lua::V_V(L, SDL_JoystickUpdate);
}

static int SDL_JoystickEventState (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickEventState(Lua::I(L, 1)));

	return 1;
}

static int SDL_JoystickGetAxis (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickGetAxis(T_(Joystick, 1), Lua::I(L, 2)));

	return 1;
}

static int SDL_JoystickGetHat (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickGetHat(T_(Joystick, 1), Lua::I(L, 2)));

	return 1;
}

static int SDL_JoystickGetBall (lua_State * L)
{
	int dx = Lua::I(L, 3), dy = Lua::I(L, 4);

	lua_pushnumber(L, SDL_JoystickGetBall(T_(Joystick, 1), Lua::I(L, 2), &dx, &dy));
	lua_pushnumber(L, dx);
	lua_pushnumber(L, dy);

	return 3;
}

static int SDL_JoystickGetButton (lua_State * L)
{
	lua_pushnumber(L, SDL_JoystickGetButton(T_(Joystick, 1), Lua::I(L, 2)));

	return 1;
}

static int SDL_JoystickClose (lua_State * L)
{
	SDL_JoystickClose(T_(Joystick, 1));

	return 0;
}

static int SDL_EnableUNICODE (lua_State * L)
{
	lua_pushnumber(L, SDL_EnableUNICODE(Lua::I(L, 1)));

	return 1;
}

static int SDL_EnableKeyRepeat (lua_State * L)
{
	lua_pushnumber(L, SDL_EnableKeyRepeat(Lua::I(L, 1), Lua::I(L, 2)));

	return 1;
}

static int SDL_GetModState (lua_State * L)
{
	lua_pushnumber(L, SDL_GetModState());

	return 1;
}

static int SDL_SetModState (lua_State * L)
{
	SDL_SetModState(J_(Mod, 1));

	return 0;
}

static int SDL_GetKeyName (lua_State * L)
{
	lua_pushstring(L, SDL_GetKeyName(J_(Key, 1)));

	return 1;
}

static int SDL_GetMouseState (lua_State * L)
{
	int x, y;

	lua_pushnumber(L, SDL_GetMouseState(&x, &y));
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	return 3;
}

static int SDL_GetRelativeMouseState (lua_State * L)
{
	int x = Lua::I(L, 1);
	int y = Lua::I(L, 2);

	lua_pushnumber(L, SDL_GetRelativeMouseState(&x, &y));
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	return 3;
}

static int SDL_WarpMouse (lua_State * L)
{
	SDL_WarpMouse(Lua::U16(L, 1), Lua::U16(L, 2));

	return 0;
}

static int SDL_CreateCursor (lua_State * L)
{
	Lua::PushUserType(L, SDL_CreateCursor(Lua::PU8(L, 1), Lua::PU8(L, 2), Lua::I(L, 3), Lua::I(L, 4), Lua::I(L, 5), Lua::I(L, 6)), "Cursor");

	return 1;
}

static int SDL_SetCursor (lua_State * L)
{
	SDL_SetCursor(T_(Cursor, 1));

	return 0;
}

static int SDL_GetCursor (lua_State * L)
{
	Lua::PushUserType(L, SDL_GetCursor(), "Cursor");

	return 1;
}

static int SDL_FreeCursor (lua_State * L)
{
	SDL_FreeCursor(T_(Cursor, 1));

	return 0;
}

static int SDL_ShowCursor (lua_State * L)
{
	lua_pushnumber(L, SDL_ShowCursor(Lua::I(L, 1)));

	return 1;
}

static int SDL_PumpEvents (lua_State * L)
{
	return Lua::V_V(L, SDL_PumpEvents);
}

static int SDL_PeepEvents (lua_State * L)
{
	lua_pushnumber(L, SDL_PeepEvents(T_(Event, 1), Lua::I(L, 2), I_(eventaction, 3), Lua::U(L, 4)));

	return 1;
}

static int SDL_PollEvent (lua_State * L)
{
	lua_pushnumber(L, SDL_PollEvent(T_(Event, 1)));

	return 1;
}

static int SDL_WaitEvent (lua_State * L)
{
	lua_pushnumber(L, SDL_WaitEvent(T_(Event, 1)));

	return 1;
}

static int SDL_PushEvent (lua_State * L)
{
	lua_pushnumber(L, SDL_PushEvent(T_(Event, 1)));

	return 1;
}

static int SDL_VideoInit (lua_State * L)
{
	lua_pushnumber(L, SDL_VideoInit(Lua::S(L, 1), Lua::U(L, 2)));

	return 1;
}

static int SDL_VideoQuit (lua_State * L)
{
	return Lua::V_V(L, SDL_VideoQuit);
}

static int SDL_VideoDriverName (lua_State * L)
{
	lua_pushstring(L, SDL_VideoDriverName(const_cast<char*>(Lua::S(L, 1)), Lua::I(L, 2)));

	return 1;
}

static int SDL_GetVideoSurface (lua_State * L)
{
	Lua::PushUserType(L, SDL_GetVideoSurface(), "Surface");

	return 1;
}

static int SDL_GetVideoInfo (lua_State * L)
{
	Lua::PushUserType(L, (void*)SDL_GetVideoInfo(), "SDL_VideoInfo");

	return 1;
}

static int SDL_VideoModeOK (lua_State * L)
{
	lua_pushnumber(L, SDL_VideoModeOK(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::U(L, 4)));

	return 1;
}

static int SDL_SetVideoMode (lua_State * L)
{
	Lua::PushUserType(L, SDL_SetVideoMode(Lua::I(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::U(L, 4)), "Surface");

	return 1;
}

static int SDL_UpdateRects (lua_State * L)
{
	SDL_UpdateRects(T_(Surface, 1), Lua::I(L, 2), T_(Rect, 3));

	return 0;
}

static int SDL_UpdateRect (lua_State * L)
{
	SDL_UpdateRect(T_(Surface, 1), Lua::S32(L, 2), Lua::S32(L, 3), Lua::S32(L, 4), Lua::S32(L, 5));

	return 0;
}

static int SDL_Flip (lua_State * L)
{
	lua_pushnumber(L, SDL_Flip(T_(Surface, 1)));

	return 1;
}

static int SDL_SetGamma (lua_State * L)
{
	lua_pushnumber(L, SDL_SetGamma(Lua::F(L, 1), Lua::F(L, 2), Lua::F(L, 3)));

	return 1;
}

static int SDL_SetGammaRamp (lua_State * L)
{
	Uint16 r = Lua::U16(L, 1);
	Uint16 g = Lua::U16(L, 2);
	Uint16 b = Lua::U16(L, 3);

	lua_pushnumber(L, SDL_SetGammaRamp(&r, &g, &b));
	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);

	return 4;
}

static int SDL_GetGammaRamp (lua_State * L)
{
	Uint16 r = Lua::U16(L, 1);
	Uint16 g = Lua::U16(L, 2);
	Uint16 b = Lua::U16(L, 3);

	lua_pushnumber(L, SDL_GetGammaRamp(&r, &g, &b));
	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);

	return 4;
}

static int SDL_SetColors (lua_State * L)
{
	lua_pushnumber(L, SDL_SetColors(T_(Surface, 1), T_(Color, 2), Lua::I(L, 3), Lua::I(L, 4)));

	return 1;
}

static int SDL_SetPalette (lua_State * L)
{
	lua_pushnumber(L, SDL_SetPalette(T_(Surface, 1), Lua::I(L, 2), T_(Color, 3), Lua::I(L, 4), Lua::I(L, 5)));

	return 1;
}

static int SDL_MapRGB (lua_State * L)
{
	lua_pushnumber(L, SDL_MapRGB(T_(PixelFormat, 1), Lua::U8(L, 2), Lua::U8(L, 3), Lua::U8(L, 4)));

	return 1;
}

static int SDL_MapRGBA (lua_State * L)
{
	lua_pushnumber(L, SDL_MapRGBA(T_(PixelFormat, 1), Lua::U8(L, 2), Lua::U8(L, 3), Lua::U8(L, 4), Lua::U8(L, 5)));

	return 1;
}

static int SDL_GetRGB (lua_State * L)
{
	Uint8 r = Lua::U8(L, 3);
	Uint8 g = Lua::U8(L, 4);
	Uint8 b = Lua::U8(L, 5);

	SDL_GetRGB(Lua::U(L, 1), T_(PixelFormat, 2), &r, &g, &b);

	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);

	return 3;
}

static int SDL_GetRGBA (lua_State * L)
{
	Uint8 r = Lua::U8(L, 3);
	Uint8 g = Lua::U8(L, 4);
	Uint8 b = Lua::U8(L, 5);
	Uint8 a = Lua::U8(L, 6);

	SDL_GetRGBA(Lua::U(L, 1), T_(PixelFormat, 2), &r, &g, &b, &a);

	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);
	lua_pushnumber(L, a);

	return 4;
}

static int SDL_CreateRGBSurface (lua_State * L)
{
	Lua::PushUserType(L, SDL_CreateRGBSurface(Lua::U(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::I(L, 4), Lua::U(L, 5), Lua::U(L, 6), Lua::U(L, 7), Lua::U(L, 8)), "Surface");

	return 1;
}

static int SDL_CreateRGBSurfaceFrom (lua_State * L)
{
	Lua::PushUserType(L, SDL_CreateRGBSurfaceFrom(Lua::UD(L, 1), Lua::I(L, 2), Lua::I(L, 3), Lua::I(L, 4), Lua::I(L, 5), Lua::U(L, 6), Lua::U(L, 7), Lua::U(L, 8), Lua::U(L, 9)), "Surface");

	return 1;
}

static int SDL_FreeSurface (lua_State * L)
{
	SDL_FreeSurface(T_(Surface, 1));

	return 0;
}

static int SDL_LockSurface (lua_State * L)
{
	lua_pushnumber(L, SDL_LockSurface(T_(Surface, 1)));

	return 1;
}

static int SDL_UnlockSurface (lua_State * L)
{
	SDL_UnlockSurface(T_(Surface, 1));

	return 0;
}

static int SDL_LoadBMP_RW (lua_State * L)
{
	Lua::PushUserType(L, SDL_LoadBMP_RW(T_(RWops, 1), Lua::I(L, 2)), "Surface");

	return 1;
}

static int SDL_LoadBMP (lua_State * L)
{
	SDL_RWops * RWops = SDL_RWFromFile(Lua::S(L, 1), "rb");

	Lua::PushUserType(L, SDL_LoadBMP_RW(RWops, 1), "Surface");

	return 1;
}

static int SDL_SaveBMP_RW (lua_State * L)
{
	lua_pushnumber(L, SDL_SaveBMP_RW(T_(Surface, 1), T_(RWops, 2), Lua::I(L, 3)));

	return 1;
}

static int SDL_SaveBMP (lua_State * L)
{
	SDL_RWops * RWops = SDL_RWFromFile(Lua::S(L, 2), "wb");

	lua_pushnumber(L, SDL_SaveBMP_RW(T_(Surface, 1), RWops, 1));

	return 1;
}

static int SDL_SetColorKey (lua_State * L)
{
	lua_pushnumber(L, SDL_SetColorKey(T_(Surface, 1), Lua::U(L, 2), Lua::U(L, 3)));

	return 1;
}

static int SDL_SetAlpha (lua_State * L)
{
	lua_pushnumber(L, SDL_SetAlpha(T_(Surface, 1), Lua::U(L, 2), Lua::U8(L, 3)));

	return 1;
}

static int SDL_SetClipRect (lua_State * L)
{
	lua_pushnumber(L, SDL_SetClipRect(T_(Surface, 1), T_(Rect, 2)));

	return 1;
}

static int SDL_GetClipRect (lua_State * L)
{
	SDL_GetClipRect(T_(Surface, 1), T_(Rect, 2));

	return 0;
}

static int SDL_ConvertSurface (lua_State * L)
{
	Lua::PushUserType(L, SDL_ConvertSurface(T_(Surface, 1), T_(PixelFormat, 2), Lua::U(L, 3)), "Surface");

	return 1;
}

static int SDL_UpperBlit (lua_State * L)
{
	lua_pushnumber(L, SDL_UpperBlit(T_(Surface, 1), T_(Rect, 2), T_(Surface, 3), T_(Rect, 4)));

	return 1;
}

static int SDL_LowerBlit (lua_State * L)
{
	lua_pushnumber(L, SDL_LowerBlit(T_(Surface, 1), T_(Rect, 2), T_(Surface, 3), T_(Rect, 4)));

	return 1;
}

static int SDL_FillRect (lua_State * L)
{
	lua_pushnumber(L, SDL_FillRect(T_(Surface, 1), T_(Rect, 2), Lua::U(L, 3)));

	return 1;
}

static int SDL_DisplayFormat (lua_State * L)
{
	Lua::PushUserType(L, SDL_DisplayFormat(T_(Surface, 1)), "Surface");

	return 1;
}

static int SDL_DisplayFormatAlpha (lua_State * L)
{
	Lua::PushUserType(L, SDL_DisplayFormatAlpha(T_(Surface, 1)), "Surface");

	return 1;
}

static int SDL_CreateYUVOverlay (lua_State * L)
{
	Lua::PushUserType(L, SDL_CreateYUVOverlay(Lua::I(L, 1), Lua::I(L, 2), Lua::U(L, 3), T_(Surface, 4)), "Overlay");

	return 1;
}

static int SDL_LockYUVOverlay (lua_State * L)
{
	lua_pushnumber(L, SDL_LockYUVOverlay(T_(Overlay, 1)));

	return 1;
}

static int SDL_UnlockYUVOverlay (lua_State * L)
{
	SDL_UnlockYUVOverlay(T_(Overlay, 1));

	return 0;
}

static int SDL_DisplayYUVOverlay (lua_State * L)
{
	lua_pushnumber(L, SDL_DisplayYUVOverlay(T_(Overlay, 1), T_(Rect, 2)));

	return 1;
}

static int SDL_FreeYUVOverlay (lua_State * L)
{
	SDL_FreeYUVOverlay(T_(Overlay, 1));

	return 0;
}

static int SDL_GL_LoadLibrary (lua_State * L)
{
	lua_pushnumber(L, SDL_GL_LoadLibrary(Lua::S(L, 1)));

	return 1;
}

static int SDL_GL_GetProcAddress (lua_State * L)
{
	lua_pushlightuserdata(L, SDL_GL_GetProcAddress(Lua::S(L, 1)));

	return 1;
}

static int SDL_GL_SetAttribute (lua_State * L)
{
	lua_pushnumber(L, SDL_GL_SetAttribute(I_(GLattr, 1), Lua::I(L, 2)));

	return 1;
}

static int SDL_GL_GetAttribute (lua_State * L)
{
	int value = Lua::I(L, 2);

	lua_pushnumber(L, SDL_GL_GetAttribute(I_(GLattr, 1), &value));
	lua_pushnumber(L, value);

	return 2;
}

static int SDL_GL_SwapBuffers (lua_State * L)
{
	return Lua::V_V(L, SDL_GL_SwapBuffers);
}

static int SDL_GL_UpdateRects (lua_State * L)
{
	SDL_GL_UpdateRects(Lua::I(L, 1), T_(Rect, 2));

	return 0;
}

static int SDL_GL_Lock (lua_State * L)
{
	return Lua::V_V(L, SDL_GL_Lock);
}

static int SDL_GL_Unlock (lua_State * L)
{
	return Lua::V_V(L, SDL_GL_Unlock);
}

static int SDL_WM_SetCaption (lua_State * L)
{
	SDL_WM_SetCaption(Lua::S(L, 1), Lua::S(L, 2));

	return 0;
}

static int SDL_WM_GetCaption (lua_State * L)
{
	char Buf[512], * pTitle = Buf + 0, * pIcon = Buf + 256;
	
	SDL_WM_GetCaption(&pTitle, &pIcon);

	lua_pushstring(L, pTitle);
	lua_pushstring(L, pIcon);

	return 2;
}

static int SDL_WM_SetIcon (lua_State * L)
{
	Uint8 mask = Lua::U8(L, 2);

	SDL_WM_SetIcon(T_(Surface, 1), &mask);

	lua_pushnumber(L, mask);

	return 1;
}

static int SDL_WM_IconifyWindow (lua_State * L)
{
	lua_pushnumber(L, SDL_WM_IconifyWindow());

	return 1;
}

static int SDL_WM_ToggleFullScreen (lua_State * L)
{
	lua_pushnumber(L, SDL_WM_ToggleFullScreen(T_(Surface, 1)));

	return 1;
}

static int SDL_WM_GrabInput (lua_State * L)
{
	lua_pushnumber(L, SDL_WM_GrabInput(I_(GrabMode, 1)));

	return 1;
}

static int SDL_RWFromFile (lua_State * L)
{
	Lua::PushUserType(L, SDL_RWFromFile(Lua::S(L, 1), Lua::S(L, 2)), "RWops");

	return 1;
}

static int SDL_RWFromFP (lua_State * L)
{
	Lua::PushUserType(L, SDL_RWFromFP(U_(FILE*, 1), Lua::I(L, 2)), "RWops");

	return 1;
}

static int SDL_RWFromMem (lua_State * L)
{
	Lua::PushUserType(L, SDL_RWFromMem(Lua::UD(L, 1), Lua::I(L, 2)), "RWops");

	return 1;
}

static int SDL_AllocRW (lua_State * L)
{
	Lua::PushUserType(L, SDL_AllocRW(), "RWops");

	return 1;
}

static int SDL_FreeRW (lua_State * L)
{
	SDL_FreeRW(T_(RWops, 1));

	return 0;
}

static int SDL_Init (lua_State * L)
{
	lua_pushnumber(L, SDL_Init(Lua::U(L, 1)));

	return 1;
}

static int SDL_InitSubSystem (lua_State * L)
{
	lua_pushnumber(L, SDL_InitSubSystem(Lua::U(L, 1)));

	return 1;
}

static int SDL_QuitSubSystem (lua_State * L)
{
	return Lua::V_U(L, SDL_QuitSubSystem);
}

static int SDL_WasInit (lua_State * L)
{
	lua_pushnumber(L, SDL_WasInit(Lua::U(L, 1)));

	return 1;
}

static int SDL_Quit (lua_State * L)
{
	return Lua::V_V(L, SDL_Quit);
}

///
/// Garbage collectors
///
static int GC_Cursor (lua_State * L)
{
	if (SDL_WasInit(SDL_INIT_VIDEO)) SDL_FreeCursor(T_(Cursor, 1));

	return 1;
}

///
/// Function table
///
static const luaL_reg SDLFuncs[] = {
	F_(GetError), F_(ClearError), F_(GetTicks), F_(Delay),
	F_(NumJoysticks), F_(JoystickName), F_(JoystickOpen), F_(JoystickOpened), F_(JoystickIndex),
	F_(JoystickNumAxes),  F_(JoystickNumBalls), F_(JoystickNumHats), F_(JoystickNumButtons),
	F_(JoystickUpdate), F_(JoystickEventState),
	F_(JoystickGetAxis), F_(JoystickGetHat), F_(JoystickGetBall), F_(JoystickGetButton),
	F_(JoystickClose),
	F_(EnableUNICODE), F_(EnableKeyRepeat),
	F_(GetModState), F_(SetModState), F_(GetKeyName),
	F_(GetMouseState), F_(GetRelativeMouseState), F_(WarpMouse),
	F_(CreateCursor), F_(SetCursor), F_(GetCursor), F_(FreeCursor), F_(ShowCursor),
	F_(PumpEvents), F_(PeepEvents), F_(PollEvent), F_(WaitEvent), F_(PushEvent),
	F_(VideoInit), F_(VideoQuit), F_(VideoDriverName), F_(GetVideoSurface), F_(GetVideoInfo),
	F_(VideoModeOK), F_(SetVideoMode),
	F_(UpdateRects), F_(UpdateRect), F_(Flip),
	F_(SetGamma), F_(SetGammaRamp), F_(GetGammaRamp),
	F_(SetColors), F_(SetPalette), F_(MapRGB), F_(MapRGBA), F_(GetRGB), F_(GetRGBA),
	F_(CreateRGBSurface), F_(CreateRGBSurfaceFrom), F_(FreeSurface), F_(LockSurface), F_(UnlockSurface),
	F_(LoadBMP_RW), F_(LoadBMP), F_(SaveBMP_RW), F_(SaveBMP),
	F_(SetColorKey), F_(SetAlpha),
	F_(SetClipRect), F_(GetClipRect),
	F_(ConvertSurface),
	F_(UpperBlit), F_(LowerBlit), F_(FillRect),
	F_(DisplayFormat), F_(DisplayFormatAlpha),
	F_(CreateYUVOverlay), F_(LockYUVOverlay), F_(UnlockYUVOverlay), F_(DisplayYUVOverlay), F_(FreeYUVOverlay),
	F_(GL_LoadLibrary), F_(GL_GetProcAddress), F_(GL_SetAttribute), F_(GL_GetAttribute),
	F_(GL_SwapBuffers), F_(GL_UpdateRects), F_(GL_Lock),
	F_(WM_SetCaption), F_(WM_GetCaption), F_(WM_SetIcon), F_(WM_IconifyWindow),
	F_(WM_ToggleFullScreen), F_(WM_GrabInput),
	F_(RWFromFile), F_(RWFromFP), F_(RWFromMem), F_(AllocRW), F_(FreeRW),
	F_(Init), F_(InitSubSystem), F_(QuitSubSystem), F_(WasInit), F_(Quit),
	{ 0, 0 }
};

///
/// Constant table
///
static const struct {
	char const * mName;	///< Name attached to constant
	int mConstant;	///< Value of constant
} SDLConsts[] = {
	C_(FALSE), C_(TRUE), C_(PRESSED), C_(RELEASED), C_(TIMESLICE),
	C_(HAT_CENTERED), C_(HAT_UP), C_(HAT_DOWN), C_(HAT_RIGHT), C_(HAT_LEFT),
	C_(HAT_RIGHTUP), C_(HAT_RIGHTDOWN), C_(HAT_LEFTUP), C_(HAT_LEFTDOWN),
	C_(ALL_HOTKEYS), C_(DEFAULT_REPEAT_DELAY), C_(DEFAULT_REPEAT_INTERVAL),
	C_(BUTTON_LEFT), C_(BUTTON_MIDDLE), C_(BUTTON_RIGHT),
	C_(BUTTON_LMASK), C_(BUTTON_MMASK), C_(BUTTON_RMASK),
	C_(NOEVENT),
	C_(ACTIVEEVENT),
	C_(KEYDOWN), C_(KEYUP),
	C_(MOUSEMOTION), C_(MOUSEBUTTONDOWN), C_(MOUSEBUTTONUP),
	C_(JOYAXISMOTION), C_(JOYBALLMOTION), C_(JOYHATMOTION),
	C_(JOYBUTTONDOWN), C_(JOYBUTTONUP),
	C_(QUIT),
	C_(SYSWMEVENT),
	C_(EVENT_RESERVEDA), C_(EVENT_RESERVEDB),
	C_(VIDEORESIZE), C_(VIDEOEXPOSE),
	C_(EVENT_RESERVED2), C_(EVENT_RESERVED3), C_(EVENT_RESERVED4), C_(EVENT_RESERVED5), C_(EVENT_RESERVED6), C_(EVENT_RESERVED7),
	C_(USEREVENT),
	C_(NUMEVENTS),
	C_(ACTIVEEVENTMASK), C_(KEYDOWNMASK), C_(KEYUPMASK), C_(MOUSEMOTIONMASK),
	C_(MOUSEBUTTONDOWNMASK), C_(MOUSEBUTTONUPMASK), C_(MOUSEEVENTMASK), C_(JOYAXISMOTIONMASK),
	C_(JOYBALLMOTIONMASK), C_(JOYHATMOTIONMASK), C_(JOYBUTTONDOWNMASK), C_(JOYBUTTONUPMASK),
	C_(JOYEVENTMASK), C_(VIDEORESIZEMASK), C_(VIDEOEXPOSEMASK), C_(QUITMASK), C_(SYSWMEVENTMASK),
	C_(ALLEVENTS), C_(ADDEVENT), C_(PEEKEVENT), C_(GETEVENT),
	C_(QUERY), C_(IGNORE), C_(DISABLE), C_(ENABLE),
	C_(ALPHA_OPAQUE), C_(ALPHA_TRANSPARENT),
	C_(SWSURFACE), C_(HWSURFACE),
	C_(ASYNCBLIT), C_(ANYFORMAT), C_(HWPALETTE), C_(DOUBLEBUF), C_(FULLSCREEN),
	C_(OPENGL), C_(OPENGLBLIT), C_(RESIZABLE), C_(NOFRAME), C_(HWACCEL), C_(SRCCOLORKEY),
	C_(RLEACCELOK), C_(RLEACCEL), C_(SRCALPHA), C_(PREALLOC),
	C_(YV12_OVERLAY), C_(IYUV_OVERLAY), C_(YUY2_OVERLAY), C_(UYVY_OVERLAY), C_(YVYU_OVERLAY),
	C_(GL_RED_SIZE), C_(GL_GREEN_SIZE), C_(GL_BLUE_SIZE), C_(GL_ALPHA_SIZE),
	C_(GL_BUFFER_SIZE),  C_(GL_DOUBLEBUFFER), C_(GL_DEPTH_SIZE), C_(GL_STENCIL_SIZE),
	C_(GL_ACCUM_RED_SIZE), C_(GL_ACCUM_GREEN_SIZE), C_(GL_ACCUM_BLUE_SIZE), C_(GL_ACCUM_ALPHA_SIZE),
	C_(LOGPAL), C_(PHYSPAL),
	C_(GRAB_QUERY), C_(GRAB_OFF), C_(GRAB_ON), C_(GRAB_FULLSCREEN),
	C_(MAJOR_VERSION), C_(MINOR_VERSION), C_(PATCHLEVEL),
	C_(INIT_TIMER), C_(INIT_AUDIO), C_(INIT_VIDEO), C_(INIT_CDROM), C_(INIT_JOYSTICK),
	C_(INIT_NOPARACHUTE), C_(INIT_EVENTTHREAD), C_(INIT_EVERYTHING),
	K_(UNKNOWN), K_(FIRST), K_(LAST),
	K_(BACKSPACE), K_(TAB), K_(CLEAR), K_(RETURN), K_(PAUSE), K_(ESCAPE), K_(SPACE),
	K_(EXCLAIM), K_(QUOTEDBL), K_(HASH), K_(DOLLAR), K_(AMPERSAND), K_(QUOTE),
	K_(LEFTPAREN), K_(RIGHTPAREN), K_(ASTERISK), K_(PLUS), K_(COMMA), K_(DELETE),
	K_(MINUS), K_(PERIOD), K_(SLASH),
	K_(0), K_(1), K_(2), K_(3), K_(4), K_(5), K_(6), K_(7), K_(8), K_(9),
	K_(COLON), K_(SEMICOLON), K_(LESS), K_(EQUALS), K_(GREATER), K_(QUESTION), K_(AT),
	K_(LEFTBRACKET), K_(BACKSLASH), K_(RIGHTBRACKET), K_(CARET), K_(UNDERSCORE), K_(BACKQUOTE),
	K_(a), K_(b), K_(c), K_(d), K_(e), K_(f), K_(g), K_(h), K_(i), K_(j), K_(k), K_(l), K_(m),
	K_(n), K_(o), K_(p), K_(q), K_(r), K_(s), K_(t), K_(u), K_(v), K_(w), K_(x), K_(y), K_(z),
	K_(WORLD_0), K_(WORLD_1), K_(WORLD_2), K_(WORLD_3), K_(WORLD_4), K_(WORLD_5), K_(WORLD_6),
	K_(WORLD_7), K_(WORLD_8), K_(WORLD_9), K_(WORLD_10), K_(WORLD_11), K_(WORLD_12), K_(WORLD_13),
	K_(WORLD_14), K_(WORLD_15), K_(WORLD_16), K_(WORLD_17),	K_(WORLD_18), K_(WORLD_19), K_(WORLD_20),
	K_(WORLD_21), K_(WORLD_22), K_(WORLD_23), K_(WORLD_24), K_(WORLD_25), K_(WORLD_26),	K_(WORLD_27),
	K_(WORLD_28), K_(WORLD_29), K_(WORLD_30), K_(WORLD_31), K_(WORLD_32), K_(WORLD_33), K_(WORLD_34),
	K_(WORLD_35), K_(WORLD_36), K_(WORLD_37), K_(WORLD_38), K_(WORLD_39), K_(WORLD_40), K_(WORLD_41),
	K_(WORLD_42), K_(WORLD_43), K_(WORLD_44), K_(WORLD_45), K_(WORLD_46), K_(WORLD_47), K_(WORLD_48),
	K_(WORLD_49), K_(WORLD_50), K_(WORLD_51), K_(WORLD_52), K_(WORLD_53), K_(WORLD_54), K_(WORLD_55),
	K_(WORLD_56), K_(WORLD_57), K_(WORLD_58), K_(WORLD_59), K_(WORLD_60), K_(WORLD_61), K_(WORLD_62),
	K_(WORLD_63), K_(WORLD_64), K_(WORLD_65), K_(WORLD_66), K_(WORLD_67), K_(WORLD_68), K_(WORLD_69),
	K_(WORLD_70), K_(WORLD_71),	K_(WORLD_72), K_(WORLD_73), K_(WORLD_74), K_(WORLD_75), K_(WORLD_76),
	K_(WORLD_77), K_(WORLD_78), K_(WORLD_79), K_(WORLD_80),	K_(WORLD_81), K_(WORLD_82), K_(WORLD_83),
	K_(WORLD_84), K_(WORLD_85), K_(WORLD_86), K_(WORLD_87), K_(WORLD_88), K_(WORLD_89),	K_(WORLD_90),
	K_(WORLD_91), K_(WORLD_92), K_(WORLD_93), K_(WORLD_94), K_(WORLD_95),
	K_(KP0), K_(KP1), K_(KP2), K_(KP3), K_(KP4), K_(KP5), K_(KP6), K_(KP7), K_(KP8), K_(KP9),
	K_(KP_PERIOD), K_(KP_DIVIDE), K_(KP_MULTIPLY), K_(KP_MINUS), K_(KP_PLUS),
	K_(KP_ENTER), K_(KP_EQUALS),
	K_(UP), K_(DOWN), K_(RIGHT), K_(LEFT),
	K_(INSERT), K_(HOME), K_(END), K_(PAGEUP), K_(PAGEDOWN),
	K_(F1), K_(F2), K_(F3), K_(F4), K_(F5), K_(F6), K_(F7), K_(F8),
	K_(F9), K_(F10), K_(F11), K_(F12), K_(F13), K_(F14), K_(F15),
	K_(NUMLOCK), K_(CAPSLOCK), K_(SCROLLOCK),
	K_(RSHIFT), K_(LSHIFT), K_(RCTRL), K_(LCTRL), K_(RALT), K_(LALT),
	K_(RMETA), K_(LMETA), K_(LSUPER), K_(RSUPER),
	K_(MODE), K_(COMPOSE), K_(HELP), K_(PRINT), K_(SYSREQ), K_(BREAK),
	K_(MENU), K_(POWER), K_(EURO),
	M_(KMOD_NONE), M_(TIMER_RESOLUTION),
	M_(KMOD_LSHIFT), M_(KMOD_RSHIFT), M_(KMOD_LCTRL), M_(KMOD_RCTRL),
	M_(KMOD_LALT), M_(KMOD_RALT), M_(KMOD_LMETA), M_(KMOD_RMETA),
	M_(KMOD_NUM), M_(KMOD_CAPS), M_(KMOD_MODE), M_(KMOD_RESERVED),
	M_(KMOD_CTRL), M_(KMOD_SHIFT), M_(KMOD_ALT), M_(KMOD_META)
};

/// @brief Binds SDL to the Lua scripting system
void luaopen_sdl (lua_State * L)
{
	Lua::UserType_Reg R[16];

	// Register Joystick type.
	Lua::RegisterUserType(L, "Joystick", 0, 0, 0, "");

	// Register keysym type.
	RT_(R[0], scancode, keysym, Lua::UserType_Reg::eU8);
	RT_(R[1], sym, keysym, Lua::UserType_Reg::eSInt);
	RT_(R[2], mod, keysym, Lua::UserType_Reg::eSInt);
	RT_(R[3], unicode, keysym, Lua::UserType_Reg::eU16);
	Lua::RegisterUserType(L, "keysym", R, 4, 0, "");

	// Register WMcursor and Cursor types.
	Lua::RegisterUserType(L, "WMcursor", 0, 0, 0, "");
	Lua::RegisterUserType(L, "Cursor", 0, 0, 0, "__gc", GC_Cursor);

	// Register ActiveEvent type.
	RT_(R[0], type, ActiveEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], gain, ActiveEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], state, ActiveEvent, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "ActiveEvent", R, 3, 0, "");

	// Register KeyboardEvent type.
	RT_(R[0], type, KeyboardEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, KeyboardEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], state, KeyboardEvent, Lua::UserType_Reg::eU8);
	RU_(R[3], keysym, KeyboardEvent, "keysym");
	RT_(R[4], state, KeyboardEvent, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "KeyboardEvent", R, 5, 0, "");

	// Register MouseMotionEvent type.
	RT_(R[0], type, MouseMotionEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, MouseMotionEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], state, MouseMotionEvent, Lua::UserType_Reg::eU8);
	RT_(R[3], x, MouseMotionEvent, Lua::UserType_Reg::eU16);
	RT_(R[4], y, MouseMotionEvent, Lua::UserType_Reg::eU16);
	RT_(R[5], xrel, MouseMotionEvent, Lua::UserType_Reg::eS16);
	RT_(R[6], yrel, MouseMotionEvent, Lua::UserType_Reg::eS16);
	Lua::RegisterUserType(L, "MouseMotionEvent", R, 7, 0, "");

	// Register MouseButtonEvent type.
	RT_(R[0], type, MouseButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, MouseButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], button, MouseButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[3], state, MouseButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[4], x, MouseButtonEvent, Lua::UserType_Reg::eU16);
	RT_(R[5], y, MouseButtonEvent, Lua::UserType_Reg::eU16);
	Lua::RegisterUserType(L, "MouseButtonEvent", R, 6, 0, "");

	// Register JoyAxisEvent type.
	RT_(R[0], type, JoyAxisEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, JoyAxisEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], axis, JoyAxisEvent, Lua::UserType_Reg::eU8);
	RT_(R[3], value, JoyAxisEvent, Lua::UserType_Reg::eS16);
	Lua::RegisterUserType(L, "JoyAxisEvent", R, 4, 0, "");

	// Register JoyBallEvent type.
	RT_(R[0], type, JoyBallEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, JoyBallEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], ball, JoyBallEvent, Lua::UserType_Reg::eU8);
	RT_(R[3], xrel, JoyBallEvent, Lua::UserType_Reg::eS16);
	RT_(R[4], yrel, JoyBallEvent, Lua::UserType_Reg::eS16);
	Lua::RegisterUserType(L, "JoyBallEvent", R, 5, 0, "");

	// Register JoyHatEvent type.
	RT_(R[0], type, JoyHatEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, JoyHatEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], hat, JoyHatEvent, Lua::UserType_Reg::eU8);
	RT_(R[3], value, JoyHatEvent, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "JoyHatEvent", R, 4, 0, "");

	// Register JoyButtonEvent type.
	RT_(R[0], type, JoyButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], which, JoyButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[2], button, JoyButtonEvent, Lua::UserType_Reg::eU8);
	RT_(R[3], state, JoyButtonEvent, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "JoyButtonEvent", R, 4, 0, "");

	// Register ResizeEvent type.
	RT_(R[0], type, ResizeEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], w, ResizeEvent, Lua::UserType_Reg::eSInt);
	RT_(R[2], h, ResizeEvent, Lua::UserType_Reg::eSInt);
	Lua::RegisterUserType(L, "ResizeEvent", R, 3, 0, "");

	// Register ExposeEvent type.
	RT_(R[0], type, ExposeEvent, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "ExposeEvent", R, 1, 0, "");

	// Register QuitEvent type.
	RT_(R[0], type, QuitEvent, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "QuitEvent", R, 1, 0, "");

	// Register UserEvent type.
	RT_(R[0], type, UserEvent, Lua::UserType_Reg::eU8);
	RT_(R[1], code, UserEvent, Lua::UserType_Reg::eSInt);
	RT_(R[2], data1, UserEvent, Lua::UserType_Reg::ePointer);
	RT_(R[3], data2, UserEvent, Lua::UserType_Reg::ePointer);
	Lua::RegisterUserType(L, "UserEvent", R, 4, 0, "");

	// Register SysWMEvent type.
	Lua::RegisterUserType(L, "SysWMEvent", 0, 0, 0, "");

	// Register Event type.
	RT_(R[0], type, Event, Lua::UserType_Reg::eU8);
	RU_(R[1], active, Event, "ActiveEvent");
	RU_(R[2], key, Event, "KeyboardEvent");
	RU_(R[3], motion, Event, "MouseMotionEvent");
	RU_(R[4], button, Event, "MouseButtonEvent");
	RU_(R[5], jaxis, Event, "JoyAxisEvent");
	RU_(R[6], jball, Event, "JoyBallEvent");
	RU_(R[7], jhat, Event, "JoyHatEvent");
	RU_(R[8], jbutton, Event, "JoyButtonEvent");
	RU_(R[9], resize, Event, "ResizeEvent");
	RU_(R[10], expose, Event, "ExposeEvent");
	RU_(R[11], quit, Event, "QuitEvent");
	RU_(R[12], user, Event, "UserEvent");
	RU_(R[13], syswm, Event, "SysWMEvent");
	Lua::RegisterUserType(L, "Event", R, 14, 0, "");

	// Register Rect type.
	RT_(R[0], x, Rect, Lua::UserType_Reg::eS16);
	RT_(R[1], y, Rect, Lua::UserType_Reg::eS16);
	RT_(R[2], w, Rect, Lua::UserType_Reg::eU16);
	RT_(R[3], h, Rect, Lua::UserType_Reg::eU16);
	Lua::RegisterUserType(L, "Rect", R, 4, 0, "");

	// Register Color type.
	RT_(R[0], r, Color, Lua::UserType_Reg::eSInt);
	RT_(R[1], g, Color, Lua::UserType_Reg::eU8);
	RT_(R[2], b, Color, Lua::UserType_Reg::eU8);
	RT_(R[3], unused, Color, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "Color", R, 4, 0, "");

	// Register Palette type.
	RT_(R[0], ncolors, Palette, Lua::UserType_Reg::eU8);
	RU_(R[1], colors, Palette, "Color");
	Lua::RegisterUserType(L, "Palette", R, 2, 0, "");

	// Register PixelFormat type.
	RU_(R[0], palette, PixelFormat, "Palette");
	RT_(R[1], BitsPerPixel, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[2], Rloss, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[3], Gloss, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[4], Bloss, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[5], Aloss, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[6], Rshift, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[7], Gshift, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[8], Bshift, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[9], Ashift, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[10], Rmask, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[11], Gmask, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[12], Bmask, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[13], Amask, PixelFormat, Lua::UserType_Reg::eU8);
	RT_(R[14], colorkey, PixelFormat, Lua::UserType_Reg::eU32);
	RT_(R[15], alpha, PixelFormat, Lua::UserType_Reg::eU8);
	Lua::RegisterUserType(L, "PixelFormat", R, 16, 0, "");

	// Register Surface type.
	RT_(R[0], flags, Surface, Lua::UserType_Reg::eU32);
	RU_(R[1], format, Surface, "PixelFormat");
	RT_(R[2], w, Surface, Lua::UserType_Reg::eSInt);
	RT_(R[3], h, Surface, Lua::UserType_Reg::eSInt);
	RT_(R[4], pitch, Surface, Lua::UserType_Reg::eU16);
	RT_(R[5], pixels, Surface, Lua::UserType_Reg::ePointer);
	Lua::RegisterUserType(L, "Surface", R, 6, 0, "");

	// Register VideoInfo type.
	RT_(R[0], video_mem, VideoInfo, Lua::UserType_Reg::eU32);
	RU_(R[1], vfmt, VideoInfo, "PixelFormat");
	Lua::RegisterUserType(L, "VideoInfo", R, 2, 0, "");

	// Register Overlay, FILE, and RWops types.
	Lua::RegisterUserType(L, "Overlay", 0, 0, 0, "");
	Lua::RegisterUserType(L, "FILE", 0, 0, 0, "");
	Lua::RegisterUserType(L, "RWops", 0, 0, 0, "");

	// Install the SDL interface and its constants.
	luaL_register(L, "SDL", SDLFuncs);	// SDL

	for (Uint32 index = 0; index < sizeof(SDLConsts) / sizeof(SDLConsts[0]); ++index)
	{
		lua_pushstring(L, SDLConsts[index].mName);	// SDL n
		lua_pushnumber(L, SDLConsts[index].mConstant);	// SDL n c

		lua_settable(L, -3);// SDL = { ..., n = c, ... }
	}

	lua_pushliteral(L, "_Event");	// SDL "_Event"
	Lua::PushUserType(L, &_Event, "Event");	// SDL "_Event" E

	lua_settable(L, -3);// SDL = { ..., _Event, ... }

	lua_pushliteral(L, "_Rect");// SDL "_Rect"
	Lua::PushUserType(L, &_Rect, "Rect");// SDL "_Rect" R

	lua_settable(L, -3);// SDL = { ..., _Rect, ... }
}

#undef U_
#undef T_
#undef I_
#undef J_
#undef F_
#undef C_
#undef K_
#undef M_
#undef RT_
#undef RU_