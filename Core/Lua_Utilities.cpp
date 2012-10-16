#include "App.h"
#include <cassert>

namespace Lua
{
	/// @brief Data indices
	enum {
		eDescriptors = 1,	///< Descriptor table index
		eNoRead,///< Non-read permission table index
		eNoWrite,	///< Non-write permission table index
		eLookup	///< Lookup means index
	};

	/// @brief Descriptor indices
	enum {
		eDOffset = 1,	///< Offset index
		eDName,	///< Name index
		eDType	///< Type index
	};

	/// @brief Lookup indices
	enum {
		eLType = 1,	///< Lookup type index
		eLKey	///< Key index
	};

	/// @brief Gets member fields
	/// @note upvalue 1: Member descriptor, non-permissions, and lookup data
	static Uint8 * GetFields (lua_State * L)
	{
		lua_rawgeti(L, -1, eDOffset);	// data, key[, value], D, D[key], offset
		lua_rawgeti(L, -2, eDType);	// data, key[, value], D, D[key], offset, type

		return PU8(L, 1) + U(L, -2);
	}

	#define F_(t, p) lua_pushnumber(L, *(t*)p)
	#define I_(t, p) lua_pushinteger(L, *(t*)p)

	/// @brief Indexes a member
	/// @note upvalue 1: Member descriptor, non-permissions, and lookup data
	static void IndexMember (lua_State * L)
	{
		// Point to the requested member.
		Uint8 * pData = GetFields(L);	// data, key, D, D[key], offset, type

		// Return the appropriate type.
		switch (U(L, 6))
		{
		case Member_Reg::ePointer:
			lua_pushlightuserdata(L, *(void**)pData);	// data, key, D, D[key], offset, type, pointer
			break;
		case Member_Reg::eU8:
			I_(Uint8, pData);	// data, key, D, D[key], offset, type, u8
			break;
		case Member_Reg::eS8:
			I_(Sint8, pData);	// data, key, D, D[key], offset, type, s8
			break;
		case Member_Reg::eU16:
			I_(Uint16, pData);	// data, key, D, D[key], offset, type, u16
			break;
		case Member_Reg::eS16:
			I_(Sint16, pData);	// data, key, D, D[key], offset, type, s16
			break;
		case Member_Reg::eU32:
			I_(Uint32, pData);	// data, key, D, D[key], offset, type, u32
			break;
		case Member_Reg::eS32:
			I_(Sint32, pData);	// data, key, D, D[key], offset, type, s32
			break;
		case Member_Reg::eUChar:
			I_(unsigned char, pData);	// data, key, D, D[key], offset, type, uchar
			break;
		case Member_Reg::eSChar:
			I_(signed char, pData);	// data, key, D, D[key], offset, type, schar
			break;
		case Member_Reg::eUShort:
			I_(unsigned short, pData);	// data, key, D, D[key], offset, type, ushort
			break;
		case Member_Reg::eSShort:
			I_(signed short, pData);	// data, key, D, D[key], offset, type, sshort
			break;
		case Member_Reg::eULong:
			I_(unsigned long, pData);	// data, key, D, D[key], offset, type, ulong
			break;
		case Member_Reg::eSLong:
			I_(signed long, pData);	// data, key, D, D[key], offset, type, slong
			break;
		case Member_Reg::eUInt:
			I_(unsigned int, pData);// data, key, D, D[key], offset, type, uint
			break;
		case Member_Reg::eSInt:
			I_(signed int, pData);	// data, key, D, D[key], offset, type, sint
			break;
		case Member_Reg::eFSingle:	
			F_(float, pData);	// data, key, D, D[key], offset, type, fsingle
			break;
		case Member_Reg::eFDouble:
			F_(double, pData);	// data, key, D, D[key], offset, type, fdouble
			break;
		case Member_Reg::eString:
			lua_pushstring(L, *(char**)pData);	// data, key, D, D[key], offset, type, string
			break;
		case Member_Reg::eBoolean:
			lua_pushboolean(L, *(bool*)pData);	// data, key, D, D[key], offset, type, boolean
			break;
		default:
			lua_pushnil(L);	// data, key, D, D[key], offset, type, nil
		}
	}

	#undef F_
	#undef I_

	/// @brief Assigns to a member
	/// @note upvalue 1: Member descriptor, non-permissions, and lookup data
	static void NewIndexMember (lua_State * L)
	{
		// Point to the requested member.
		Uint8 * pData = GetFields(L);	// data, key, value, D, D[key], offset, type

		// Assign the appropriate type.
		switch (U(L, 7))
		{
		case Member_Reg::ePointer:
			*(void**)pData = UD(L, 3);
			break;
		case Member_Reg::eU8:
		case Member_Reg::eUChar:
			*(Uint8*)pData = U8(L, 3);
			break;
		case Member_Reg::eS8:
		case Member_Reg::eSChar:
		//	*(Sint8*)pData = S8(L, 3);
			break;
		case Member_Reg::eU16:
		case Member_Reg::eUShort:
			*(Uint16*)pData = U16(L, 3);
			break;
		case Member_Reg::eS16:
		case Member_Reg::eSShort:
			*(Sint16*)pData = S16(L, 3);
			break;
		case Member_Reg::eU32:
			*(Uint32*)pData = U(L, 3);
			break;
		case Member_Reg::eS32:
			*(Sint32*)pData = S32(L, 3);
			break;
		case Member_Reg::eULong:
			break;
		case Member_Reg::eSLong:
			*(long*)pData = LI(L, 3);
			break;
		case Member_Reg::eUInt:
			break;
		case Member_Reg::eSInt:
			*(Sint32*)pData = S32(L, 3);
			break;
		case Member_Reg::eFSingle:
			*(float*)pData = F(L, 3);
			break;
		case Member_Reg::eFDouble:
			break;
		case Member_Reg::eString:
			break;
		case Member_Reg::eBoolean:
			*(bool*)pData = B(L, 3);
			break;
		}
	}

	/// @brief Looks up member data
	/// @note upvalue 1: Member descriptor, non-permissions, and lookup data
	static void Lookup (lua_State * L)
	{
		lua_rawgeti(L, lua_upvalueindex(1), eLookup);	// object, key[, value], lookup
		lua_rawgeti(L, -1, eLType);	// object, key[, value], lookup, type

		lua_Integer type = lua_tointeger(L, -1);

		lua_pop(L, 1);	// object, key[, value], lookup

		switch (type)
		{
		case eThis:
		case eRegistry:
			lua_pushvalue(L, 1);// object, key[, value], lookup, object

			if (eRegistry == type) lua_gettable(L, LUA_REGISTRYINDEX);	// object, key[, value], lookup, data;
			break;
		case ePointerTo:
			lua_pushlightuserdata(L, *(void**)UD(L, 1));// object, key[, value], lookup, data
			break;
		case eKey:
			lua_rawgeti(L, -2, eLKey);	// object, key[, value], lookup, key
			lua_gettable(L, 1);	// object, key[, value], lookup, data
			break;
		}

		lua_replace(L, -2);	// object, key[, value], data
	}

	/// @brief __index closure
	/// @note upvalue 1: Member descriptor, non-permissions, and lookup data
	/// @note upvalue 2: Member getter table
	/// @note object: Object being accessed
	/// @note key: Lookup key
	static int Index (lua_State * L)
	{
		Lookup(L);	// object, key, data

		//
		lua_pushvalue(L, 2);// object, key, data, key
		lua_gettable(L, lua_upvalueindex(2));	// object, key, data, getter

		if (!lua_isnil(L, 4))
		{
			lua_insert(L, 1);	// getter, object, key, data
			lua_call(L, 3, 1);	// result
		}

		//
		else
		{
			lua_pop(L, 1);	// object, key, data
			lua_replace(L, 1);	// data, key
			
			lua_rawgeti(L, lua_upvalueindex(1), eNoRead);	// data, key, noread
			lua_pushvalue(L, 2);// data, key, noread, key
			lua_gettable(L, 3);	// data, key, noread, bNoRead

			if (!lua_toboolean(L, 4))
			{
				//
				lua_pop(L, 2);	// data, key
				lua_rawgeti(L, lua_upvalueindex(1), eDescriptors);	// data, key, D
				lua_pushvalue(L, 2);// data, key, D, key
				lua_gettable(L, 3);	// data, key, D, D[key]

				if (!lua_isnil(L, 4)) IndexMember(L);	// result
			}
		}

		return 1;
	}

	/// @brief __newindex closure
	/// @note upvalue 1: Member descriptor, non-permissions, and lookup data
	/// @note upvalue 2: Member setter table
	/// @note upvalue 3: Propagation boolean
	/// @note object: Object being accessed
	/// @note key: Lookup key
	/// @note value: Value to assign
	static int NewIndex (lua_State * L)
	{
		Lookup(L);	// object, key, value, data

		//
		lua_pushvalue(L, 2);// object, key, value, data, key
		lua_gettable(L, lua_upvalueindex(2));	// object, key, value, data, setter

		if (!lua_isnil(L, 5))
		{
			lua_insert(L, 1);	// setter, object, key, value, data
			lua_call(L, 4, 1);	// result

			if (!lua_isnil(L, 1) && lua_toboolean(L, lua_upvalueindex(3))) return 1;
		}

		//
		else
		{
			lua_pop(L, 1);	// object, key, value, data
			lua_replace(L, 1);	// data, key, value
			lua_rawgeti(L, lua_upvalueindex(1), eNoWrite);	// data, key, value, nowrite
			lua_pushvalue(L, 2);// data, key, value, nowrite, key
			lua_gettable(L, 4);	// data, key, value, nowrite, bNoWrite

			if (!lua_toboolean(L, 5))
			{
				//
				lua_pop(L, 2);	// data, key, value
				lua_rawgeti(L, lua_upvalueindex(1), eDescriptors);	// data, key, value, D
				lua_pushvalue(L, 2);// data, key, value, D, key
				lua_gettable(L, 4);	// data, key, value, D, D[key]

				if (!lua_isnil(L, 5)) NewIndexMember(L);

				else if (lua_toboolean(L, lua_upvalueindex(3))) return 1;
			}
		}

		return 0;
	}

	/// @brief Pushes __index and __newindex member binding closures onto stack
	/// @param L Lua state
	/// @param getters [optional] Member getter functions
	/// @param setters [optional] Member setter functions
	/// @param members [optional] Member descriptors
	/// @param count Count of member descriptors
	/// @param lookup Means of datum lookup
	/// @param where [optional] If lookup is eKey, used as lookup key
	/// @param bPropagateOnFail If true, __newindex calls will propagate if no setter or member exists
	/// @note At least one of getters, setters, or members must be non-0 (if members, count must also be non-0)
	void MemberBindFuncs (lua_State * L, luaL_Reg const * getters, luaL_Reg const * setters, Member_Reg const * members, int count, Members_Lookup lookup, char const * where, bool bPropagateOnFail)
	{
		assert(0 == count || members != 0);
		assert(where != 0 || lookup != eKey);
		assert(getters != 0 || setters != 0 || (members != 0 && count > 0));

		lua_createtable(L, 4, 0);	// data

		//
		lua_createtable(L, 0, count);	// data, M
		lua_newtable(L);// data, M, RP
		lua_newtable(L);// data, M, RP, WP

		for (int index = 0; index < count; ++index)
		{
			// Install any permission information.
			switch (members[index].mPermissions)
			{
			case Member_Reg::eRO:
			case Member_Reg::eWO:
				lua_pushstring(L, members[index].mName.c_str());// data, M, RP, WP, name
				lua_pushboolean(L, true);	// data, M, RP, WP, name, true
				lua_settable(L, Member_Reg::eRO == members[index].mPermissions ? -4 : -3);	// data, M, RP = { ..., name = true }, WP / data, M, RP, WP = { ..., name = true }
				break;
			}

			//
			lua_pushstring(L, members[index].mName.c_str());	// data, M, RP, WP, name
			lua_createtable(L, 2, 0);	// data, M, RP, WP, name, {}
			lua_pushinteger(L, members[index].mOffset);	// data, M, RP, WP, name, D, offset
			lua_pushinteger(L, members[index].mType);	// data, M, RP, WP, name, D, offset, type
			lua_rawseti(L, -3, eDType);	// data, M, RP, WP, name, D = { type = type }, offset
			lua_rawseti(L, -2, eDOffset);	// data, M, RP, WP, name, D = { type, offset = offset }
			lua_settable(L, -5);// data, M = { ..., name = D }, RP, WP
		}

		lua_rawseti(L, -4, eNoWrite);	// data = { nowrite = WP }, M, RP
		lua_rawseti(L, -3, eNoRead);// data = { nowrite, noread = RP }, M
		lua_rawseti(L, -2, eDescriptors);	// data = { nowrite, noread, descriptors = M }

		//
		lua_createtable(L, 2, 0);	// data, L
		lua_pushinteger(L, lookup);	// data, L, lookup
		lua_rawseti(L, -2, eLType);	// data, L = { type = lookup }

		if (eKey == lookup)
		{
			lua_pushstring(L, where);	// data, L, where
			lua_rawseti(L, -2, eLKey);	// data, L = { type, where = where }
		}

		lua_rawseti(L, -2, eLookup);// data = { nowrite, noread, descriptors, lookup = L }

		// Build __index closure.
		lua_pushvalue(L, -1);	// data, data
		lua_newtable(L);// data, data, {}

		if (getters != 0) luaL_register(L, 0, getters);

		lua_pushcclosure(L, Index, 2);	// data, __index

		// Build __newindex closure.
		lua_insert(L, -2);	// __index, data
		lua_newtable(L);// __index, data, {}

		if (setters != 0) luaL_register(L, 0, setters);

		lua_pushboolean(L, bPropagateOnFail);	// __index, data, setters, bPropagateOnFail
		lua_pushcclosure(L, NewIndex, 3);	// __index, __newindex
	}
}