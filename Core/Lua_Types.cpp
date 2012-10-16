#include "App.h"
#include <cassert>
#include <cctype>
#include <cstdarg>

namespace Lua
{
	/// @brief Gets the metatable fields
	static Uint8 * GetMetaFields (lua_State * L)
	{
		Uint8 * pData = static_cast<Uint8*>(UT(L, -2));	// ..., u, k

		lua_getmetatable(L, -2);// ..., u, k, mt
		lua_insert(L, -2);	// ..., u, mt, k
		lua_gettable(L, -2);// ..., u, mt, dt
		lua_getfield(L, -1, "usertype");// ..., u, mt, dt, ut
		lua_getfield(L, -2, "offset");	// ..., u, mt, dt, ut, o
		lua_getfield(L, -3, "type");// ..., u, mt, dt, ut, o, t
		lua_insert(L, -6);	// ..., t, u, mt, dt, ut, o
		lua_insert(L, -6);	// ..., o, t, u, mt, dt, ut
		lua_insert(L, -6);	// ..., ut, o, t, u, mt, dt

		return pData;
	}

	#define N_(t, p) lua_pushnumber(L, *(t*)p)

	/// @brief Index access method
	static int Index (lua_State * L)
	{
		// Point to the requested member.
		Uint8 * pData = GetMetaFields(L);	// ut, o, t

		pData += U(L, 2);

		// Return the appropriate type.
		switch (U(L, 3))
		{
		case UserType_Reg::eUserType:
			PushUserType(L, pData, S(L, 1));
			break;
		case UserType_Reg::ePointer:
			lua_pushlightuserdata(L, *(void**)pData);
			break;
		case UserType_Reg::eU8:
			N_(Uint8, pData);
			break;
		case UserType_Reg::eS8:
			N_(Sint8, pData);
			break;
		case UserType_Reg::eU16:
			N_(Uint16, pData);
			break;
		case UserType_Reg::eS16:
			N_(Sint16, pData);
			break;
		case UserType_Reg::eU32:
			N_(Uint32, pData);
			break;
		case UserType_Reg::eS32:
			N_(Sint32, pData);
			break;
		case UserType_Reg::eUChar:
			N_(unsigned char, pData);
			break;
		case UserType_Reg::eSChar:
			N_(signed char, pData);
			break;
		case UserType_Reg::eUShort:
			N_(unsigned short, pData);
			break;
		case UserType_Reg::eSShort:
			N_(signed short, pData);
			break;
		case UserType_Reg::eULong:
			N_(unsigned long, pData);
			break;
		case UserType_Reg::eSLong:
			N_(signed long, pData);
			break;
		case UserType_Reg::eUInt:
			N_(unsigned int, pData);
			break;
		case UserType_Reg::eSInt:
			N_(signed int, pData);
			break;
		case UserType_Reg::eFloat:
			N_(float, pData);
			break;
		case UserType_Reg::eString:
			lua_pushstring(L, *(char**)pData);
			break;
		case UserType_Reg::eBoolean:
			lua_pushboolean(L, *(bool*)pData);
			break;
		}

		return 1;
	}

	#undef N_

	/// @brief Index assignment method
	static int NewIndex (lua_State * L)
	{
		// Point to the requested member.
		lua_insert(L, 1);	// v, u, k

		Uint8 * pData = GetMetaFields(L);	// v, ut, o, t

		pData += U(L, 3);

		// Assign the appropriate type.
		switch (U(L, 4))
		{
		case UserType_Reg::eUserType:
			break;
		case UserType_Reg::ePointer:
			*(void**)pData = UD(L, 1);
			break;
		case UserType_Reg::eU8:
			*(Uint8*)pData = U8(L, 1);
			break;
		case UserType_Reg::eS8:
			break;
		case UserType_Reg::eU16:
		case UserType_Reg::eUShort:
			*(Uint16*)pData = U16(L, 1);
			break;
		case UserType_Reg::eS16:
		case UserType_Reg::eSShort:
			*(Sint16*)pData = S16(L, 1);
			break;
		case UserType_Reg::eU32:
			*(Uint32*)pData = U(L, 1);
			break;
		case UserType_Reg::eS32:
			*(Sint32*)pData = S32(L, 1);
			break;
		case UserType_Reg::eUChar:
			break;
		case UserType_Reg::eSChar:
			break;
		case UserType_Reg::eULong:
			break;
		case UserType_Reg::eSLong:
			*(long*)pData = LI(L, 1);
			break;
		case UserType_Reg::eUInt:
			break;
		case UserType_Reg::eSInt:
			break;
		case UserType_Reg::eFloat:
			*(float*)pData = F(L, 1);
			break;
		case UserType_Reg::eString:
			break;
		case UserType_Reg::eBoolean:
			*(bool*)pData = B(L, 1);
			break;
		}

		return 0;
	}

	#define L_(type, key, value) lua_push##type(L, value),	\
								 lua_setfield(L, -2, key)

	/// @brief
	/// @param L Lua state
	/// @param count Count of arguments already added to stack
	/// @param retc Result count (may be MULT_RET)
	/// @param params Parameter descriptors
	///		   b Boolean
	///		   f Function
	///		   i Integer
	///		   n Number
	///		   s String
	/// @param args Variable argument list
	/// @return Number of results of call
	static int CallCore (lua_State * L, int count, int retc, char const * params, va_list & args)
	{
		std::string ps = params;

		int arg, top = lua_gettop(L) - count;

		for (size_t offset = 0; ; ++offset, ++count)
		{
			while (offset < ps.size() && isspace(ps[offset])) ++offset;

			if (offset == ps.size()) break;

			switch (ps[offset])
			{
			case 'a':	// Add value from lower in the stack
				arg = va_arg(args, int);

				lua_pushvalue(L, arg > 0 ? arg : top - arg - 1);/// ..., func[, ...], arg
				break;
			case 'b':	// Add boolean
				lua_pushboolean(L, va_arg(args, bool));	/// ..., func[, ...], boolean
				break;
			case 'f':	// Add function
				lua_pushcfunction(L, va_arg(args, lua_CFunction));	/// ..., func[, ...], function
				break;
			case 'i':	// Add integer
				lua_pushinteger(L, va_arg(args, lua_Integer));	/// ..., func[, ...], integer
				break;
			case 'n':	// Add number
				lua_pushnumber(L, va_arg(args, lua_Number));/// ..., func[, ...], number
				break;
			case 's':	// Add string
				lua_pushstring(L, va_arg(args, char const *));	/// ..., func[, ...], string
				break;
			case 'u':	// Add userdata
				lua_pushlightuserdata(L, va_arg(args, void *));	/// ..., func[, ...], userdata
				break;
			default:
				assert(!"Lua::Call or Lua::Invoke: Bad type");
			}
		}
	   
		va_end(args);

		// Invoke the function.
		int after = lua_gettop(L) - count - 1;

		if (lua_pcall(L, count, retc, 0) != 0) luaL_error(L, "Lua::Call or Lua::Invoke: not callable");

		return lua_gettop(L) - after;
	}

	/// @brief Calls a Lua routine from C/C++
	/// @param L Lua state
	/// @param name Routine name
	/// @param retc Result count (q.v. CallCore)
	/// @param params Parameter descriptors (q.v. CallCore)
	/// @return Number of results of call
	/// @note Vararg parameters are the arguments
	int Call (lua_State * L, char const * name, int retc, char const * params, ...)
	{
		lua_getglobal(L, name);	// ..., func

		va_list args;	va_start(args, params);

		return CallCore(L, 0, retc, params, args);
	}

	/// @brief Calls a Lua method from C/C++
	/// @param L Lua state
	/// @param name Routine name
	/// @param source Source argument stack index
	/// @param retc Result count (q.v. CallCore)
	/// @param params Parameter descriptors (q.v. CallCore)
	/// @return Number of results of call
	/// @note Vararg parameters are the arguments
	int Invoke (lua_State * L, int source, char const * name, int retc, char const * params, ...)
	{
		if (source < 0) source = lua_gettop(L) - source - 1;

		lua_pushstring(L, name);// ..., name
		lua_gettable(L, source);// ..., source[name]
		lua_pushvalue(L, source);	// ..., source[name], source

		va_list args;	va_start(args, params);

		return CallCore(L, 1, retc, params, args);
	}

	/// @brief Defines a class
	/// @param L Lua state
	/// @param name Type name
	/// @param methods Methods to associate with class
	/// @param newf Function used to instantiate class
	/// @param base [optional] Base type name
	/// @param size [optional] Userdatum size
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, lua_CFunction newf, char const * base, Uint size)
	{
		class_Define(L, name, methods, 0, 0, newf, base, size);
	}

	/// @brief Defines a class, with new function on the stack top
	/// @param L Lua state
	/// @param name Type name
	/// @param methods Methods to associate with class
	/// @param base [optional] Base type name
	/// @param size [optional] Userdatum size
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, char const * base, Uint size)
	{
		class_Define(L, name, methods, 0, 0, base, size);
	}

	/// @brief Defines a class, with closures on the stack
	/// @param L Lua state
	/// @param name Type name
	/// @param methods Methods to associate with class
	/// @param closures Closure names
	/// @param count Count of closures at top of stack
	/// @param newf Function used to instantiate class
	/// @param base [optional] Base type name
	/// @param size [optional] Userdatum size
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, char const * closures[], int count, lua_CFunction newf, char const * base, Uint size)
	{
		lua_pushcfunction(L, newf);	// ..., newf

		class_Define(L, name, methods, closures, count, base, size);

		lua_pop(L, 1);
	}

	/// @brief Defines a class, with closures on the stack and new function at the top
	/// @param L Lua state
	/// @param name Type name
	/// @param methods Methods to associate with class
	/// @param closures Closure names
	/// @param count Count of closures at top of stack
	/// @param base [optional] Base type name
	/// @param size [optional] Userdatum size
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, char const * closures[], int count, char const * base, Uint size)
	{
		assert(count >= 0);
		assert(0 == closures || count > 0);
		assert(methods != 0 || closures != 0);

		// Install the new function.
		lua_insert(L, -count - 1);	// new, ...

		// Load methods.
		lua_newtable(L);// new, ..., M
		
		if (methods != 0) luaL_register(L, 0, methods);

		// Load closures.
		for (int index = 0; index < count; ++index)
		{
			lua_pushstring(L, closures[index]);	// new, ..., M, name
			lua_pushvalue(L, -count - 2 + index);	// new, ..., M, name, closure
			lua_settable(L, -3);// new, ..., M = { ..., name = closure }
		}

		lua_insert(L, -count - 1);	// new, M, ...
		lua_pop(L, count);	// new, M
		lua_insert(L, -2);	// M, new

		// Prepare the define call.
		lua_getglobal(L, "class");	// M, new, class
		lua_getfield(L, -1, "define");	// M, new, class, class.define
		lua_insert(L, -4);	// class.define, M, new, class
		lua_pushstring(L, name);// class.define, M, new, class, name
		lua_insert(L, -4);	// class.define, name, M, new, class
		lua_pop(L, 1);	// class.define, name, M, new

		// Assign any parameters.
		bool bParams = base != 0 || size != 0;

		if (bParams)
		{
			lua_createtable(L, 0, 2);	// class.define, name, M, new, P

			if (base != 0)
			{
				lua_pushstring(L, base);	// class.define, name, M, new, P, base
				lua_setfield(L, -2, "base");// class.define, name, M, new, P = { base = base }
			}

			if (size != 0)
			{
				lua_pushinteger(L, size);	// class.define, name, M, new, P, size
				lua_setfield(L, -2, "size");// class.define, name, M, new, P = { [base, ]size = size }
			}
		}

		lua_call(L, bParams ? 4 : 3, 0);
	}

	/// @brief Instantiates a class
	/// @param L Lua state
	/// @param name Type name
	/// @param count Count of parameters on stack
	void class_New (lua_State * L, char const * name, int count)
	{
		lua_getglobal(L, "class");	// ..., class
		lua_getfield(L, -1, "new");	// ..., class, class.new
		lua_replace(L, -2);	// ..., class.new
		lua_pushstring(L, name);// ..., class.new, name
		lua_insert(L, -2 - count);	// class.new, ..., name
		lua_insert(L, -2 - count);	// class.new, name, ...
		lua_call(L, count + 1, 1);	// I
	}

	/// @brief Invokes a base type constructor
	/// @param L Lua state
	/// @param base Base type name
	/// @param count Count of parameters on stack
	void class_SCons (lua_State * L, char const * base, int count)
	{
		lua_getglobal(L, "class");	// ..., class
		lua_getfield(L, -1, "scons");	// ..., class, class.scons
		lua_replace(L, -2);	// ..., class.scons
		lua_pushstring(L, base);// ..., class.scons, base
		lua_insert(L, -2 - count);	// class.scons, ..., base
		lua_insert(L, -2 - count);	// class.scons, base, ...
		lua_call(L, count + 1, 0);
	}

	/// @brief Pushes a user type onto the stack
	/// @param L Lua state
	/// @param data User type data
	/// @param name Type name
	void PushUserType (lua_State * L, void * data, char const * name)
	{
		// Get the metatable and acquire the instance size.
		lua_getfield(L, LUA_REGISTRYINDEX, name);	// mt
		lua_getfield(L, -1, "size");// mt size

		// If the size is 0, pass the data by reference. Otherwise, copy it into the buffer.
		int size = I(L, -1);

		lua_pop(L, 1);	// mt

		void * udata = lua_newuserdata(L, size != 0 ? size : sizeof(void*));// mt u

		memcpy(udata, size != 0 ? data : &data, size != 0 ? size : sizeof(void*));

		// Bind the userdata to its type's metatable.
		lua_insert(L, -2);	// u mt
		lua_setmetatable(L, -2);// u
	}

	/// @brief Registers a user type with Lua
	/// @param L Lua state
	/// @param name Type name
	/// @param reg Type bindings
	/// @param count Count of bindings
	/// @param size [optional] If non-0, the size of the datum to copy
	/// @param format List of metamethod names
	/// @note Vararg parameters are metamethod functions
	void RegisterUserType (lua_State * L, char const * name, UserType_Reg * reg, Uint count, size_t size, char * format, ...)
	{
		// Install the usertype metatable and install variable metamethods.
		luaL_newmetatable(L, name);	// {}

		L_(cfunction, "__index", Index);// { __index = Index }
		L_(cfunction, "__newindex", NewIndex);// { __index, __newindex = NewIndex }

		// Store the instance size.
		L_(number, "size", size);	// { __index, __newindex, size }

		// Add metamethods.
		va_list function;	va_start(function, format);

		char buffer[256]; strcpy(buffer, format);

		for (char * key = strtok(buffer, " "); key != 0; key = strtok(0, " "))
		{
			L_(cfunction, key, va_arg(function, lua_CFunction));// { __index, __newindex, size, [..., method ] }
		}
	   
		va_end(function);

		// Register variable addresses.
		for (Uint index = 0; index < count; ++index)
		{
			lua_newtable(L);// { __index, __newindex, size, [...] }, {}

			L_(string, "usertype", reg[index].mUserType);	// { __index, __newindex, size, [...] }, { usertype = ut }
			L_(number, "offset", reg[index].mOffset);	// { __index, __newindex, size, [...] }, { usertype, offset = o }
			L_(number, "type", reg[index].mType);	// { __index, __newindex, size, [...] }, { usertype, offset, type = t }

			lua_setfield(L, -2, reg[index].mName);	// { __index, __newindex, size, [...], n = { usertype, offset, type } }
		}

		// Restore stack.
		lua_pop(L, 1);
	}
}