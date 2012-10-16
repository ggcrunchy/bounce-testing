#include "App.h"
#include "../UserInt/UserInterface.h"

///
/// Lua UI helper functions
///
static lua_State * Method (UI::Widget * widget, char const * method)
{
	lua_State * L = static_cast<lua_State*>(widget->GetState()->GetContext());

	lua_pushlightuserdata(L, widget);	// widget
	lua_gettable(L, LUA_REGISTRYINDEX);	// w
	lua_getfield(L, -1, method);// w, M
	lua_insert(L, -2);	// M, w

	return L;
}

static void AddCoordinates (lua_State * L, UI::Widget * widget)
{
	lua_getfield(L, -1, "GetRect");	// M, w, GetRect
	lua_pushvalue(L, -2);	// M, w, GetRect, w
	lua_pushstring(L, "xywh");	// M, w, GetRect, w, "xywh"
	lua_pcall(L, 2, 4, 0);	// M, w, x, y, w, h
}

static void Invoke (UI::Widget * widget, char const * method)
{
	lua_State * L = Method(widget, method);	// Method, w

	AddCoordinates(L, widget);	// Method, w, x, y, w, h

	lua_pcall(L, 5, 0, 0);
}

///
/// Handlers
///
static void EventFunc (UI::Widget * widget, UI::Event event)
{
	lua_State * L = Method(widget, "Event");	// Event, w

	lua_pushnumber(L, event);	// Event, w, e
	lua_pcall(L, 2, 0, 0);
}

static void SignalFunc (UI::Widget * widget)
{
	Invoke(widget, "SignalTest");
}

static void UpdateFunc (UI::Widget * widget)
{
	Invoke(widget, "Update");
}

///
/// Type handlers
///
template<typename T> static T * UDT (lua_State * L, int index)
{
	return *static_cast<T**>(Lua::UD(L, index));
}

static inline UI::Widget * Uw (lua_State * L, int index)
{
	return UDT<UI::Widget>(L, index);
}

static inline UI::State * Us (lua_State * L, int index)
{
	return UDT<UI::State>(L, index);
}

/// @brief O * T::func (void); registry
template<typename T, typename O> static int T_T_r (lua_State * L, O * (T::* func)(void))
{
	O * object = (UDT<T>(L, 1)->*func)();
	
	if (object != 0)
	{
		lua_pushlightuserdata(L, object);	// object
		lua_gettable(L, LUA_REGISTRYINDEX);	// o

		return 1;
	}

	return 0;
}

/// @brief bool T::func (void); registry
template<typename T> static int B_T_r (lua_State * L, bool (T::* func)(void))
{
	lua_pushboolean(L, (UDT<T>(L, 1)->*func)());

	return 1;
}

/// @brief void T::func (void); registry
template<typename T> static int V_T_r (lua_State * L, void (T::* func)(void))
{
	(UDT<T>(L, 1)->*func)();

	return 0;
}

/// @brief void T::func (bool); registry
template<typename T> static int V_TB_r (lua_State * L, void (T::* func)(bool))
{
	(UDT<T>(L, 1)->*func)(Lua::B(L, 2));

	return 0;
}

///
/// Part functions
///
static int PartRemove (lua_State * L)
{
	lua_pushlightuserdata(L, UDT<UI::Part>(L, 1));	// part
	lua_pushnil(L);	// part, nil
	lua_settable(L, LUA_REGISTRYINDEX);	// R[part] = nil

	return V_T_r<UI::Part>(L, &UI::Part::Remove);
}

///
/// Signalee functions
///

static int SignaleeSignal (lua_State * L)
{
	return V_T_r<UI::Signalee>(L, &UI::Signalee::Signal);
}

static int SignaleeIsEntered (lua_State * L)
{
	return B_T_r<UI::Signalee>(L, &UI::Signalee::IsEntered);
}

static int SignaleeIsGrabbed (lua_State * L)
{
	return B_T_r<UI::Signalee>(L, &UI::Signalee::IsGrabbed);
}

static int SignaleeIsSignaled (lua_State * L)
{
	return B_T_r<UI::Signalee>(L, &UI::Signalee::IsSignaled);
}

///
/// State functions
///
static int StateClose (lua_State * L)
{
	lua_pushlightuserdata(L, Us(L, 1));	// state
	lua_pushnil(L);	// state, nil
	lua_settable(L, LUA_REGISTRYINDEX);	// R[state] = nil

	delete Us(L, 1);

	return 0;
}

static int StateAbortSignals (lua_State * L)
{
	return V_T_r<UI::State>(L, &UI::State::AbortSignals);
}

static int StateClearInput (lua_State * L)
{
	return V_T_r<UI::State>(L, &UI::State::ClearInput);
}

static int StatePropagateSignal (lua_State * L)
{
	return V_TB_r<UI::State>(L, &UI::State::PropagateSignal);
}

static int StateUnsignal (lua_State * L)
{
	return V_T_r<UI::State>(L, &UI::State::Unsignal);
}

static int StateUpdate (lua_State * L)
{
	return V_T_r<UI::State>(L, &UI::State::Update);
}

static int StateGetPress (lua_State * L)
{
	return B_T_r<UI::State>(L, &UI::State::GetPress);
}

static int StateGetFrameSize (lua_State * L)
{
	lua_pushnumber(L, Us(L, 1)->GetFrameSize());

	return 1;
}

static int StateGetEntered (lua_State * L)
{
	return T_T_r<UI::State, UI::Signalee>(L, &UI::State::GetEntered);
}

static int StateGetGrabbed (lua_State * L)
{
	return T_T_r<UI::State, UI::Signalee>(L, &UI::State::GetGrabbed);
}

static int StateGetSignaled (lua_State * L)
{
	return T_T_r<UI::State, UI::Signalee>(L, &UI::State::GetSignaled);
}

static int StateGetChoice (lua_State * L)
{
	return T_T_r<UI::State, UI::Widget>(L, &UI::State::GetChoice);
}

static int StateGetFrameHead (lua_State * L)
{
	return T_T_r<UI::State, UI::Widget>(L, &UI::State::GetFrameHead);
}

///
/// Widget functions
///
static int WidgetAddToFrame (lua_State * L)
{
	return V_T_r<UI::Widget>(L, &UI::Widget::AddToFrame);
}

static int WidgetAllowDockSignal (lua_State * L)
{
	return V_TB_r<UI::Widget>(L, &UI::Widget::AllowDockSignal);
}

static int WidgetAllowDockUpdate (lua_State * L)
{
	return V_TB_r<UI::Widget>(L, &UI::Widget::AllowDockUpdate);
}

static int WidgetAllowSignal (lua_State * L)
{
	return V_TB_r<UI::Widget>(L, &UI::Widget::AllowSignal);
}

static int WidgetAllowUpdate (lua_State * L)
{
	return V_TB_r<UI::Widget>(L, &UI::Widget::AllowUpdate);
}

static int WidgetDestroy (lua_State * L)
{
	lua_pushlightuserdata(L, Uw(L, 1));	// widget
	lua_pushnil(L);	// widget, nil
	lua_settable(L, LUA_REGISTRYINDEX);	// R[widget] = nil

	return V_T_r<UI::Widget>(L, &UI::Widget::Destroy);
}

static int WidgetDock (lua_State * L)
{
	Uw(L, 1)->Dock(Uw(L, 2));

	return 0;
}

static int WidgetPromoteToDockHead (lua_State * L)
{
	return V_T_r<UI::Widget>(L, &UI::Widget::PromoteToDockHead);
}

static int WidgetPromoteToFrameHead (lua_State * L)
{
	return V_T_r<UI::Widget>(L, &UI::Widget::PromoteToFrameHead);
}

static int WidgetUnload (lua_State * L)
{
	return V_T_r<UI::Widget>(L, &UI::Widget::Unload);
}

static int WidgetCreatePart (lua_State * L)
{
	lua_pushlightuserdata(L, Uw(L, 1)->CreatePart());

	Lua::class_New(L, "Part", 1);

	return 1;
}

static int WidgetGetState (lua_State * L)
{
	return T_T_r<UI::Widget, UI::State>(L, &UI::Widget::GetState);
}

static int WidgetGetDockHead (lua_State * L)
{
	return T_T_r<UI::Widget, UI::Widget>(L, &UI::Widget::GetDockHead);
}

static int WidgetGetNextDockLink (lua_State * L)
{
	return T_T_r<UI::Widget, UI::Widget>(L, &UI::Widget::GetNextDockLink);
}

static int WidgetGetNextFrameLink (lua_State * L)
{
	return T_T_r<UI::Widget, UI::Widget>(L, &UI::Widget::GetNextFrameLink);
}

static int WidgetGetParent (lua_State * L)
{
	return T_T_r<UI::Widget, UI::Widget>(L, &UI::Widget::GetParent);
}

static int WidgetIsDocked (lua_State * L)
{
	return B_T_r<UI::Widget>(L, &UI::Widget::IsDocked);
}

static int WidgetIsFramed (lua_State * L)
{
	return B_T_r<UI::Widget>(L, &UI::Widget::IsFramed);
}

static int WidgetIsLoaded (lua_State * L)
{
	return B_T_r<UI::Widget>(L, &UI::Widget::IsLoaded);
}

static int WidgetIsSignalAllowed (lua_State * L)
{
	return B_T_r<UI::Widget>(L, &UI::Widget::IsSignalAllowed);
}

static int WidgetIsUpdateAllowed (lua_State * L)
{
	return B_T_r<UI::Widget>(L, &UI::Widget::IsUpdateAllowed);
}

#define M_(w) { #w, Part##w }

///
/// Function tables
///
static const luaL_reg PartFuncs[] = {
	M_(Remove),
	{ 0, 0 }
};

#undef M_
#define M_(w) { #w, Signalee##w }

static const luaL_reg SignaleeFuncs[] = {
	M_(Signal),
	M_(IsEntered),
	M_(IsGrabbed),
	M_(IsSignaled),
	{ 0, 0 }
};

#undef M_
#define M_(w) { #w, State##w }

static const luaL_reg StateFuncs[] = {
	M_(Close),
	M_(AbortSignals),
	M_(ClearInput),
	M_(GetEntered),
	M_(GetGrabbed),
	M_(GetSignaled),
	M_(GetChoice),
	M_(GetFrameHead),
	M_(PropagateSignal),
	M_(Unsignal),
	M_(Update),
	M_(GetPress),
	M_(GetFrameSize),
	{ 0, 0 }
};

#undef M_
#define M_(w) { #w, Widget##w }

static const luaL_reg WidgetFuncs[] = {
	M_(AddToFrame),
	M_(AllowDockSignal),
	M_(AllowDockUpdate),
	M_(AllowSignal),
	M_(AllowUpdate),
	M_(Destroy),
	M_(Dock),
	M_(PromoteToDockHead),
	M_(PromoteToFrameHead),
	M_(Unload),
	M_(CreatePart),
	M_(GetState),
	M_(GetDockHead),
	M_(GetNextDockLink),
	M_(GetNextFrameLink),
	M_(GetParent),
	M_(IsDocked),
	M_(IsFramed),
	M_(IsLoaded),
	M_(IsSignalAllowed),
	M_(IsUpdateAllowed),
	{ 0, 0 }
};

#undef M_
#define E_(w) { #w, UI::e##w }

///
/// Constant tables
///
static const struct {
	char const * mName;	///< Name attached to constant
	UI::Event mEvent;	///< Event value constant
} WidgetEvents[] = {
	E_(PreChoose),
	E_(PostChoose),
	E_(Grab),
	E_(Drop),
	E_(Enter),
	E_(Leave),
	E_(PreUpkeep),
	E_(PostUpkeep),
	E_(Abandon),
	E_(GrabPart),
	E_(DropPart),
	E_(EnterPart),
	E_(LeavePart)
};

#undef E_

///
/// New functions
///
static int NewF (lua_State * L, size_t size)
{
	void * data = Lua::UD(L, 2);

	memcpy(Lua::UD(L, 1), &data, size);

	lua_insert(L, 1);	// data, D
	lua_settable(L, LUA_REGISTRYINDEX);	// R[data] = D

	return 0;
}

static int SignaleeNew (lua_State * L)
{
	return 0;
}

static int PartNew (lua_State * L)
{
	Lua::class_SCons(L, "Signalee", 0);

	return NewF(L, sizeof(UI::Part*));
}

static int StateNew (lua_State * L)
{
	return NewF(L, sizeof(UI::State*));
}

static int WidgetNew (lua_State * L)
{
	Lua::class_SCons(L, "Signalee", 0);

	lua_pushlightuserdata(L, Us(L, 2)->CreateWidget());	// W, state, widget
	lua_replace(L, 2);	// W, widget

	return NewF(L, sizeof(UI::Widget*));
}

static int UINew (lua_State * L)
{
	try {
		UI::State * state = new UI::State(EventFunc, SignalFunc, UpdateFunc);

		state->SetContext(L);

		lua_pushlightuserdata(L, state);// state

		Lua::class_New(L, "State", 1);	// s
	} catch (std::bad_alloc &) { return 0; }

	return 1;
}

/// @brief Binds the UI system to the Lua scripting system
void luaopen_ui (lua_State * L)
{
	Lua::class_Define(L, "Signalee", SignaleeFuncs, SignaleeNew, 0, 0);
	Lua::class_Define(L, "Part", PartFuncs, PartNew, "Signalee", sizeof(UI::Part*));
	Lua::class_Define(L, "Widget_", WidgetFuncs, WidgetNew, "Signalee", sizeof(UI::Widget*));
	Lua::class_Define(L, "State", StateFuncs, StateNew, 0, sizeof(UI::State*));

	// Install the event constants into the widget table.
	lua_newtable(L);// {}

	for (int index = 0; index < sizeof(WidgetEvents) / sizeof(WidgetEvents[0]); ++index)
	{
		lua_pushinteger(L, WidgetEvents[index].mEvent);	// { ... }, e
		lua_setfield(L, -2, WidgetEvents[index].mName);	// { ..., n = e }
	}

	lua_setglobal(L, "WE");

	// Add the initialization routine.
	lua_pushcfunction(L, UINew);
	lua_setglobal(L, "UINew");
}