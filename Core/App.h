#ifndef APP_H
#define APP_H

#include <SDL/SDL_types.h>
#include <bitset>
#include <string>
#include <cstring>
extern "C"
{
#include "../Lua/include/lua.h"
#include "../Lua/include/lualib.h"
#include "../Lua/include/lauxlib.h"
}

void luaopen_class (lua_State * L);
void luaopen_dirent (lua_State * L);
void luaopen_graphics (lua_State * L);
void luaopen_misc (lua_State * L);
void luaopen_sequence (lua_State * L);
void luaopen_sdl (lua_State * L);
void luaopen_state (lua_State * L);
void luaopen_ui (lua_State * L);
void luaopen_voxel (lua_State * L);

namespace Lua
{
	typedef unsigned Uint;

	void Post (lua_State * L, char const * message, bool bError);

	Uint U (lua_State * L, int index);
	Uint8 U8 (lua_State * L, int index);
	Uint16 U16 (lua_State * L, int index);
	Sint32 S32 (lua_State * L, int index);
	Sint16 S16 (lua_State * L, int index);
	int I (lua_State * L, int index);
	long LI (lua_State * L, int index);
	float F (lua_State * L, int index);
	bool B (lua_State * L, int index);
	char const * S (lua_State * L, int index);
	void * UD (lua_State * L, int index);
	void * UT (lua_State * L, int index);
	Uint8 * PU8 (lua_State * L, int index);

	int V_V (lua_State * L, void (*func)(void));
	int V_U (lua_State * L, void (*func)(Uint));
	int I_V (lua_State * L, int (*func)(void));

	struct UserType_Reg {
		size_t mOffset;
		char * mName;
		char * mUserType;
		enum {
			eUserType,
			ePointer,
			eU8, eU16, eU32,
			eS8, eS16, eS32,
			eUChar, eUShort, eULong, eUInt,
			eSChar, eSShort, eSLong, eSInt,
			eString,
			eBoolean,
			eFloat
		} mType;
	};

	/// @brief Member specification
	struct Member_Reg {
		size_t mOffset;	///< Member offset
		std::string mName;	///< Member name
		enum Type {
			ePointer,	///< Pointer member
			eS8, eS16, eS32, eS64,	///< Sized members, signed
			eU8, eU16, eU32, eU64,	///< Sized members, unsigned
			eSChar, eSShort, eSLong, eSInt,	///< Integer primitives, signed
			eUChar, eUShort, eULong, eUInt,	///< Integer primitives, unsigned
			eString,///< String
			eBoolean,	///< Boolean
			eFSingle, eFDouble	///< Single- and double-precision floating point
		} mType;///< Member type
		enum Permissions {
			eFull,	///< Full permissions
			eRO, eWO	///< Read-only or write-only
		} mPermissions;	///< Member permissions

		Member_Reg (void) : mPermissions(eFull)
		{
		}

		void Set (size_t offset, std::string const & name, Type type)
		{
			mOffset = offset;
			mName = name;
			mType = type;
		}
	};

	/// @brief Means of member data lookup
	enum Members_Lookup {
		eThis,	///< The datum occupies the instance memory
		ePointerTo,	///< A pointer to the datum occupies the instance memory
		eKey,	///<  The datum is stored with a given key by the instance
		eRegistry	///< The datum is stored in the registry, with the instance as key
	};

	namespace AppTypes
	{
		struct Vector {
			float m[3];

			Vector (Vector const & v);
			Vector (float x, float y, float z);
			Vector (float v[3]);
			Vector (void);

			float angle (void);
			float length (void);

			Vector XZ (void);
			Vector Y (void);

			friend Vector operator + (Vector const & v, Vector const & w);
			friend Vector operator - (Vector const & v, Vector const & w);
			friend Vector operator * (Vector const & v, float k);
			friend Vector operator * (float k, Vector const & v);
			friend Vector operator / (Vector const & v, float k);
			friend Vector operator ^ (Vector const & v, Vector const & w);
			friend Vector operator - (Vector const & v);
			friend Vector operator ~ (Vector const & v);

			friend float operator * (Vector const & v, Vector const & w);
		};

		struct Quad {
			Vector mCorners[4];
			Vector mNormal;
			std::bitset<16> mETest;
			std::bitset<16> mVTest;

			Quad (Vector const & ul, Vector const & ur, Vector const & lr, Vector const & ll);
			Quad (float ul[3], float ur[3], float lr[3], float ll[3]);
		};

		struct Sphere {
			Vector mCenter;
			float mRadius;

			Sphere (Sphere const & sphere);
			Sphere (Vector const & center, float radius);
			Sphere (void);
		};
	}

	int Call (lua_State * L, char const * name, int retc, char const * params, ...);
	int Invoke (lua_State * L, int source, char const * name, int retc, char const * params, ...); 
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, lua_CFunction newf, char const * base, Uint size);
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, char const * base, Uint size);
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, char const * closures[], int count, lua_CFunction newf, char const * base, Uint size);
	void class_Define (lua_State * L, char const * name, luaL_Reg const * methods, char const * closures[], int count, char const * base, Uint size);
	void class_New (lua_State * L, char const * name, int count);
	void class_SCons (lua_State * L, char const * base, int count);
	void PushUserType (lua_State * L, void * data, char const * name);
	void RegisterUserType (lua_State * L, char const * name, UserType_Reg * reg, Uint count, size_t size, char * format, ...);

	void MemberBindFuncs (lua_State * L, luaL_Reg const * getters, luaL_Reg const * setters, Member_Reg const * members, int count, Members_Lookup lookup, char const * where = 0, bool bPropagateOnFail = false);
}

#endif // APP_H