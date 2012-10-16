#include "App.h"
#include <algorithm>
#include <cmath>

/// @brief Hit storage
struct Hit {
	Lua::AppTypes::Vector mContact;	///< Point of collision contact
	Lua::AppTypes::Vector mNormal;	///< Normal at collision point
	float mT;	///< Time of hit
	bool mInit;	///< Initialization flag

	// Methods
	Hit (float fLimit) : mT(fLimit), mInit(false) {};

	void AddHit (Lua::AppTypes::Vector const & contact, Lua::AppTypes::Vector const & normal, float fT);
	int LoadHit (lua_State * L);
};

static bool _BQF (float a, float b, float c, float & t)
{
	if (fabsf(a) < 1e-6f) return false;

	float term = b * b - a * c;

	if (term < 0.0f) return false;

	float discriminant = sqrtf(term);

	t = (-b - discriminant) / a;

	if (t < 0.0f) t = (-b + discriminant) / a;

	return t >= 0.0f;
}

void Hit::AddHit (Lua::AppTypes::Vector const & contact, Lua::AppTypes::Vector const & normal, float fT)
{
	if (fT >= mT) return;

	mContact = contact;
	mNormal = normal;
	mT = fT;

	mInit = true;
}

int Hit::LoadHit (lua_State * L)
{
	if (!mInit) return 0;

	lua_pushnumber(L, mT);	// t
	Lua::PushUserType(L, &mContact, "Vector");	// t (x y z)
	Lua::PushUserType(L, &(~mNormal), "Vector");// t (x y z) (nx ny nz)

	return 3;
}

///
/// Type handlers
///
template<typename T> static T * UTT (lua_State * L, int index)
{
	return static_cast<T*>(Lua::UT(L, index));
}


static inline Lua::AppTypes::Sphere * US_ (lua_State * L, int index)
{
	return UTT<Lua::AppTypes::Sphere>(L, index);
}

static inline Lua::AppTypes::Quad * UQ_ (lua_State * L, int index)
{
	return UTT<Lua::AppTypes::Quad>(L, index);
}

static inline Lua::AppTypes::Vector UV_ (lua_State * L, int index)
{
	return (*UTT<Lua::AppTypes::Vector>(L, index));
}

///
/// Collision
///
static int Reflect (lua_State * L)
{
	Lua::AppTypes::Vector N = UV_(L, 1), I = UV_(L, 2);

	Lua::PushUserType(L, &(I - 2.0f * (N * I) * N), "Vector");

	return 1;
}

static int Slide (lua_State * L)
{
	Lua::AppTypes::Vector N = UV_(L, 1), V = UV_(L, 2);

	float fVN = V * N;

	Lua::AppTypes::Vector T = V - fVN * N;

	float fLength = T.length();

	T.m[1] *= fLength > 1e-6f ? 1.0f / fLength : 0.0f;

	if (fVN > 0.0f && fVN <= 0.5f * Lua::F(L, 4) * Lua::F(L, 3) * sqrtf(1.0f - T.m[1] * T.m[1]))
	{
		Lua::PushUserType(L, &Lua::AppTypes::Vector(T.m[0], 0.0f, T.m[2]), "Vector");

		return 1;
	}

	return 0;
}

static int SphereQuad (lua_State * L)
{
	float fLimit = Lua::F(L, 4);

	Hit hit(fLimit);

	Lua::AppTypes::Sphere * pS = US_(L, 1);
	Lua::AppTypes::Quad * pQ = UQ_(L, 2);

	Lua::AppTypes::Vector V = UV_(L, 3), N = pQ->mNormal, P = pQ->mCorners[0];

	// Switch the normal's sense if the sphere is on the opposite side of the quad.
	Lua::AppTypes::Vector D = pS->mCenter - P;

	if (D * N < 0.0f) N = -N;

	// If the sphere's motion is parallel to / away from the plane, or will not switch
	// sides before hitting its goal, trivially reject it if it is not near the plane.
	float fDN = D * N, fVN = V * N;

	bool bPlaneHit = fVN < -1e-6f;

	if (!bPlaneHit && fDN > pS->mRadius + 1e-3f) return 0;
	if (fDN + fLimit * fVN > pS->mRadius + 1e-3f) return 0;

	// Determine if and when the sphere will intersect the quad plane in this step. If the
	// sphere is penetrating or touching, this is instant.
	float fT = bPlaneHit && pS->mRadius < fDN ? (pS->mRadius - fDN) / fVN : 0.0f;

	bPlaneHit &= fT >= 0.0f;

	float fV2 = V * V, fR2 = pS->mRadius * pS->mRadius;

	// If there is nothing to test, quit now.
	if (!bPlaneHit && pQ->mETest.none() && pQ->mVTest.none()) return 0;

	// Move the center along its heading up to the collision time. From here, step from the
	// center to the point of intersection on the sphere.
	Lua::AppTypes::Vector C = pS->mCenter + fT * V - pS->mRadius * N;

	// Iterate through the four corners, forming an edge with the corner prior to each.
	for (Uint32 index = 0; index < 4; ++index)
	{
		Lua::AppTypes::Vector Q = pQ->mCorners[(index + 1) % 4];
		Lua::AppTypes::Vector edge = Q - P;

		// Form a ray from the corner to the point on the plane.  If its cross product with
		// the edge points away from the surface normal, the point is outside the quad.
		bPlaneHit &= ((Q - C) ^ edge) * N > 0.0f;

		// Test the sphere against the current edge.
		float fDV = D * V, fD2 = D * D, fT;

		if (pQ->mETest[index])
		{
			Lua::AppTypes::Vector EU = ~edge;

			float fEV = EU * V, fED = EU * D;

			if (_BQF(fV2 - fEV * fEV, V * D - fED * fEV, fD2 - fED * fED - fR2, fT))
			{
				Lua::AppTypes::Vector C = pS->mCenter + fT * V;

				float fOffset = (C - P) * EU;

				// Verify that the contact lies along the edge.
				if (fOffset > 0.0f && fOffset < edge.length())
				{
					Lua::AppTypes::Vector contact = P + fOffset * EU;

					hit.AddHit(contact, C - contact, fT);
				}
			}
		}

		// Test the sphere against the current vertex.
		if (pQ->mVTest[index] && _BQF(fV2, fDV, fD2 - fR2, fT)) hit.AddHit(P, D + fT * V, fT);

		P = Q;
		D = pS->mCenter - P;
	}

	// Load the point on the plane if it lies within the quad.
	if (bPlaneHit) hit.AddHit(C, N, fT);

	// Return the first hit.
	return hit.LoadHit(L);
}

static int Spheres (lua_State * L)
{
	// Compute the coefficients for the sphere collision equation, which is a binary
	// quadratic form. Check whether the spheres will intersect in this time step.
	Lua::AppTypes::Sphere * pS1 = US_(L, 1), * pS2 = US_(L, 2);

	Lua::AppTypes::Vector V1 = UV_(L, 3), V2 = UV_(L, 4);

	Lua::AppTypes::Vector dC = pS1->mCenter - pS2->mCenter, dV = V1 - V2;

	float fT, rSum = pS1->mRadius + pS2->mRadius;

	if (_BQF(dV * dV, dV * dC, dC * dC - rSum * rSum, fT) && fT < Lua::F(L, 5))
	{
		lua_pushnumber(L, fT);	// t

		// Compute the vector between the new centers, following it to the collision point.
		// Normalize it to acquire the collision normal.
		dC = pS1->mRadius / rSum * (dC + fT * dV);

		Lua::PushUserType(L, &(pS1->mCenter + fT * V1 - dC), "Vector");	// t (x y z)
		Lua::PushUserType(L, &(dC / pS1->mRadius), "Vector");// t (x y z) (nx ny nz)

		return 3;
	}

	return 0;
}

///
/// Math
///
static int BQF (lua_State * L)
{
	float t;

	if (_BQF(Lua::F(L, 1), Lua::F(L, 2), Lua::F(L, 3), t))
	{
		lua_pushnumber(L, t);

		return 1;
	}

	return 0;
}

static int Quad (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Quad(UV_(L, 1), UV_(L, 2), UV_(L, 3), UV_(L, 4)), "Quad");

	return 1;
}

static int Sphere (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Sphere(UV_(L, 1), Lua::F(L, 2)), "Sphere");

	return 1;
}

static int Vector (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Vector(Lua::F(L, 1), Lua::F(L, 2), Lua::F(L, 3)), "Vector");

	return 1;
}

static int vXZ (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Vector(Lua::F(L, 1), 0.0f, Lua::F(L, 2)), "Vector");

	return 1;
}

static int vY (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Vector(0.0f, Lua::F(L, 1), 0.0f), "Vector");

	return 1;
}

static int v0 (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Vector(0.0f, 0.0f, 0.0f), "Vector");

	return 1;
}

///
/// Miscellany
///
static int BufferU8 (lua_State * L)
{								
	lua_getglobal(L, "table");	// d = { ..., d[x], ... }, "table" -> d, table
	lua_pushliteral(L, "maxn");	// d, table, "maxn"
	lua_gettable(L, -2);// d, table, table.maxn
	lua_pushvalue(L, -3);	// d, table, table.maxn, d
	lua_call(L, 1, 1);	// d, table, N

	int count = Lua::I(L, 3);

	lua_pop(L, 2);	// d

	Uint8 * pBuffer = static_cast<Uint8*>(lua_newuserdata(L, count));	// d, b = {}

	lua_insert(L, -2);	// b, d

	for (int index = 1; index <= count; ++index)
	{
		lua_pushinteger(L, index);	// b, d, i
		lua_gettable(L, -2);// b, d, d[i]

		pBuffer[index - 1] = Lua::U8(L, -1);

		lua_pop(L, 1);	// b, d
	}

	lua_pop(L, 1);	// b

	return 1;
}

static int Dump (lua_State * L)
{
	Lua::Post(L, "Dump", Lua::B(L, 1));

	return 0;
}

static int Partition (lua_State * L)
{
	int base = Lua::I(L, 1), dim = Lua::I(L, 2), count = Lua::I(L, 3) + 1, range = 0;

	lua_createtable(L, count, 0);	// {}

	for (int index = 1; index <= count; ++index)
	{
		int oldr = range;

		lua_pushinteger(L, index);	// { ... } index {}
		lua_newtable(L);// { ... } index {}
		lua_pushinteger(L, base + range);	// { ... } index {} v
		lua_setfield(L, -2, "value");	// { ... } index { value = v }

		range = index * dim / count;

		lua_pushinteger(L, range - oldr);	// { ... } index { value = v } d
		lua_setfield(L, -2, "dim");	// { ... } index { value = v, dim = d }
		lua_settable(L, -3);// { ..., [index] = { value = v, dim = d } }
	}

	return 1;
}

///
/// Ops
///
static int AND (lua_State * L)
{
	lua_pushinteger(L, Lua::I(L, 1) & Lua::I(L, 2));

	return 1;
}

static int ANDS (lua_State * L)
{
	int test = Lua::I(L, 2);

	lua_pushboolean(L, (Lua::I(L, 1) & test) == test);

	return 1;
}

static int MOD (lua_State * L)
{
	lua_pushinteger(L, Lua::I(L, 1) % Lua::U(L, 2));
	
	return 1;
}

static int NOT (lua_State * L)
{
	lua_pushinteger(L, ~Lua::I(L, 1));

	return 1;
}

static int OR (lua_State * L)
{
	lua_pushinteger(L, Lua::I(L, 1) | Lua::I(L, 2));

	return 1;
}

static int SHL (lua_State * L)
{
	lua_pushinteger(L, Lua::I(L, 1) << Lua::U(L, 2));

	return 1;
}

static int SHR (lua_State * L)
{
	lua_pushinteger(L, Lua::I(L, 1) >> Lua::U(L, 2));

	return 1;
}

static int XOR (lua_State * L)
{
	lua_pushinteger(L, Lua::I(L, 1) ^ Lua::I(L, 2));

	return 1;
}

///
/// Trajectory
///
static const float fPi = 3.14159f;
static const float fHalfPi = fPi / 2;

static Lua::AppTypes::Vector _Y (float fY)
{
	return Lua::AppTypes::Vector(0.0f, fY, 0.0f);
}

// x, y: Target point
// v: Initial speed
// g: Gravity force
static int GetAngles (lua_State * L)
{
	float fX = Lua::F(L, 1), fY = Lua::F(L, 2), fV = Lua::F(L, 3), fG = Lua::F(L, 4);

	if (fX < 0.0f) return 0;
	if (fV <= 0.0f) return 0;
	if (fG <= 0.0f) return 0;

	// Ensure that angles will lie in the bounding parabola.
	// radius: Distance to target
	// factor: (y + g * x^2 / v^2) / radius
	// Within parabola: factor <= 1
	float fX2 = fX * fX;
	float fNumerator = fY + fG * fX2 / (fV * fV), fRadius = sqrtf(fX2 + fY * fY);

	int count = 0;

	if (fNumerator <= fRadius)
	{
		// Angle to (x, y): theta = arcsin(y / radius)
		// Solution A: sin(2 * A - theta) = factor
		// A = (arcsin(factor) + theta) / 2
		// sin(a) = sin(pi - a): solution B satisfies sin(theta + pi - 2 * B) = factor
		// B = (theta + pi - arcsin(factor)) / 2
		float fIncline = asinf(fY / fRadius), fTerm = asinf(fNumerator / fRadius);
		float fAngles[2], fValues[2] = { +0.5f * (fTerm + fIncline), -0.5f * (fTerm - fIncline - fPi) };

		// Keep angles found in the right semicircle. If this includes both, sort them by
		// increasing radian. Return these angles.
		for (Uint32 index = 0; index < 2; ++index)
		{
			if (fValues[index] <= -fHalfPi) continue;
			if (fValues[index] >= +fHalfPi) continue;
			
			fAngles[count++] = fValues[index];
		}

		if (count == 2 && fAngles[1] < fAngles[0]) std::swap(fAngles[0], fAngles[1]);

		for (int index = 0; index < count; ++index) lua_pushnumber(L, fAngles[index]);
	}

	return count;
}

// v: Initial speed
// a: Launch angle
// g: Gravity force
static int GetMaxHeight (lua_State * L)
{
	float fV = Lua::F(L, 1), fA = Lua::F(L, 2), fG = Lua::F(L, 3);

	if (fV <= 0.0f) return 0;
	if (fA <= -fHalfPi || fA >= +fHalfPi) return 0;
	if (fG <= 0.0f) return 0;

	// If the angle is negative, the trajectory will never be upward, and thus the peak is
	// the start of the curve. Otherwise, compute the height when gravity matches ascent.
	if (fA < 0.0f) lua_pushnumber(L, 0);

	else
	{
		// dy/dt = 0: v * sin(a) = g * t
		// y(v * sin(a) / g) = v^2 * sin(a)^2 / g - v^2 * sin(a)^2 / (g * 2)
		// y = v^2 * sin(a)^2 / (g * 2)
		float fSin = sinf(fA);

		lua_pushnumber(L, 0.5f * fV * fV * fSin * fSin / fG);
	}

	return 1;
}

// begin, end: Trajectory curve bounds
// v: Initial speed
// a: Launch angle
// g: Gravity force
// t: Time along curve
static int GetPosition (lua_State * L)
{
	Lua::AppTypes::Vector begin = UV_(L, 1);

	float fV = Lua::F(L, 3), fA = Lua::F(L, 4), fG = Lua::F(L, 5), fT = Lua::F(L, 6);

	if (fV <= 0.0f) return 0;
	if (fA <= -fHalfPi || fA >= +fHalfPi) return 0;
	if (fG <= 0.0f) return 0;
	if (fT < 0.0f) return 0;

	// d = end - begin
	// d, unit tangential part: normalize(d - {0, d.y, 0})
	// d, unit normal part: {0, 1, 0}
	// x(t) = v * cos(a) * t
	// y(t) = v * sin(a) * t - g * t^2 / 2
	// P(t) = x(t) * d,utp + y(t) * d,unp
	float fSin = sinf(fA);

	Lua::AppTypes::Vector vT = ~(UV_(L, 2) - begin).XZ() * fV * sqrtf(1.0f - fSin * fSin);
	Lua::AppTypes::Vector vN = _Y(fV * fSin - 0.5f * fG * fT);

	Lua::PushUserType(L, &(begin + fT * (vT + vN)), "Vector");

	return 1;
}

// x, y: Target point
// a: Launch angle
// g: Gravity force
static int GetSpeed (lua_State * L)
{
	float fX = Lua::F(L, 1), fA = Lua::F(L, 3), fG = Lua::F(L, 4);

	if (fX < 0.0f) return 0;
	if (fA <= -fHalfPi || fA >= +fHalfPi) return 0;
	if (fG <= 0.0f) return 0;

	// x(t) = v * cos(a) * t
	// y(x / v * cos(a)) = x * tan(a) - g * (x / v * cos(a))^2 / 2
	// v^2 = g * (x / cos(a))^2 / ((x * tan(a) - y) * 2)
	// v = x * sec(a) * (g / ((x * tan(a) - y) * 2))^.5
	float fSin = sinf(fA);
	float fCos = sqrtf(1.0f - fSin * fSin);
	float fDisc = fX * fSin - Lua::F(L, 2) * fCos;

	if (fDisc < 0.0f) return 0;

	lua_pushnumber(L, fX * sqrtf(0.5f * fG / (fDisc * fCos)));

	return 1;
}

// x: Tangential distance along curve
// v: Initial speed
// a: Launch angle
static int GetTime (lua_State * L)
{
	float fX = Lua::F(L, 1), fV = Lua::F(L, 2), fA = Lua::F(L, 3);

	// x(t) = v * cos(a) * t
	// t = x / (v * cos(a))
	if (fX < 0.0f) return 0;
	if (fV <= 0.0f) return 0;
	if (fA <= -fHalfPi || fA >= +fHalfPi) return 0;

	lua_pushnumber(L, fX / (fV * cosf(fA)));

	return 1;
}

// begin, end: Trajectory curve bounds
// v: Initial speed
// a: Launch angle
// g: Gravity force
// t: Time along curve
static int GetVelocity (lua_State * L)
{
	float fV = Lua::F(L, 3), fA = Lua::F(L, 4), fG = Lua::F(L, 5), fT = Lua::F(L, 6);

	if (fV <= 0.0f) return 0;
	if (fA <= -fHalfPi || fA >= +fHalfPi) return 0;
	if (fG <= 0.0f) return 0;
	if (fT < 0.0f) return 0;

	// d = end - begin
	// d, unit tangential part: normalize(d - {0, d.y, 0})
	// d, unit normal part: {0, 1, 0}
	// x'(t) = v * cos(a)
	// y'(t) = v * sin(a) - g * t
	// P(t) = x'(t) * d,utp + y'(t) * d,unp
	float fSin = sinf(fA);

	Lua::AppTypes::Vector vT = ~(UV_(L, 2) - UV_(L, 1)).XZ() * fV * sqrtf(1.0f - fSin * fSin);
	Lua::AppTypes::Vector vN = _Y(fV * fSin - fG * fT);

	Lua::PushUserType(L, &(vT + vN), "Vector");

	return 1;
}

// y: Normal distance along curve
// v: Initial speed
// a: Launch angle
// g: Gravity force
static int GetYTimes (lua_State * L)
{
	float fV = Lua::F(L, 2), fA = Lua::F(L, 3), fG = Lua::F(L, 4);

	if (fV <= 0.0f) return 0;
	if (fA <= -fHalfPi || fA >= +fHalfPi) return 0;
	if (fG <= 0.0f) return 0;

	// y(t) = v * sin(a) * t - g * t^2 / 2
	// a: -g / 2
	// b: v * sin(a)
	// c: -y
	// t = (v * sin(a) -+ (v^2 * sin(a)^2 - 2 * g * y)^.5) / g
	float fB = fV * sinf(fA);
	float fTerm = fB * fB - 2.0f * fG * Lua::F(L, 1);

	int count = 0;

	if (fTerm >= 0.0f)
	{
		fTerm = sqrtf(fTerm);

		// Find the lesser and greater times that satisfy the height parabola. Choose the
		// non-negative solutions.
		for (Uint32 index = 0; index < 2; ++index)
		{
			fTerm = -fTerm;

			if (fB < -fTerm) continue;

			lua_pushnumber(L, (fB + fTerm) / fG);

			++count;
		}
	}

	return count;
}

///
/// Vector
///
static int Add (lua_State * L)
{
	Lua::PushUserType(L, &(UV_(L, 1) + UV_(L, 2)), "Vector");

	return 1;
}

static int Angle (lua_State * L)
{
	lua_pushnumber(L, UV_(L, 1).angle());

	return 1;
}

static int ClampToMax (lua_State * L)
{
	Lua::AppTypes::Vector V = UV_(L, 1);

	float fMax = Lua::F(L, 2);

	Lua::PushUserType(L, &(V.length() > fMax ? fMax * ~V : V), "Vector");

	return 1;
}

static int Copy (lua_State * L)
{
	Lua::PushUserType(L, &Lua::AppTypes::Vector(UV_(L, 1)), "Vector");

	return 1;
}

static int Cross (lua_State * L)
{
	Lua::PushUserType(L, &(UV_(L, 1) ^ UV_(L, 2)), "Vector");

	return 1;
}

static int Div (lua_State * L)
{
	Lua::PushUserType(L, &(UV_(L, 1) / Lua::F(L, 2)), "Vector");

	return 1;
}

static int Len (lua_State * L)
{
	lua_pushnumber(L, UV_(L, 1).length());

	return 1;
}

static int Mul (lua_State * L)
{
	if (lua_isnumber(L, 2)) Lua::PushUserType(L, &(UV_(L, 1) * Lua::F(L, 2)), "Vector");

	else lua_pushnumber(L, UV_(L, 1) * UV_(L, 2));

	return 1;
}

static int Neg (lua_State * L)
{
	Lua::PushUserType(L, &(-UV_(L, 1)), "Vector");

	return 1;
}

static int ScaleTo (lua_State * L)
{
	Lua::PushUserType(L, &(~UV_(L, 1) * Lua::F(L, 2)), "Vector");

	return 1;
}

static int SphereInvert (lua_State * L)
{
	Lua::AppTypes::Vector V = UV_(L, 1);

	Lua::PushUserType(L, &(V / (V * V)), "Vector");

	return 1;
}

static int Sub (lua_State * L)
{
	Lua::PushUserType(L, &(UV_(L, 1) - UV_(L, 2)), "Vector");

	return 1;
}

static int TLen (lua_State * L)
{
	Lua::AppTypes::Vector V = UV_(L, 1);

	lua_pushnumber(L, sqrtf(V.m[0] * V.m[0] + V.m[2] * V.m[2]));

	return 1;
}

static int Unit (lua_State * L)
{
	Lua::PushUserType(L, &(~UV_(L, 1)), "Vector");

	return 1;
}

static int UnitF (lua_State * L)
{
	Lua::PushUserType(L, &(~Lua::AppTypes::Vector(Lua::F(L, 1), Lua::F(L, 2), Lua::F(L, 3))), "Vector");

	return 1;
}

static int xz (lua_State * L)
{
	Lua::PushUserType(L, &(UV_(L, 1).XZ()), "Vector");

	return 1;
}

static int y (lua_State * L)
{
	Lua::PushUserType(L, &(UV_(L, 1).Y()), "Vector");

	return 1;
}

#define M_(w) { #w, w }

static const luaL_reg CollisionFuncs[] = {
	M_(Reflect),
	M_(Slide),
	M_(SphereQuad),
	M_(Spheres),
	{ 0, 0 }
};

static const luaL_reg MathFuncs[] = {
	M_(BQF),
	M_(Quad),
	M_(Sphere),
	M_(Vector),
	M_(vXZ),
	M_(vY),
	M_(v0),
	{ 0, 0 }
};

static const luaL_reg MiscFuncs[] = {
	M_(BufferU8),
	M_(Dump),
	M_(Partition),
	{ 0, 0 }
};

static const luaL_reg OpsFuncs[] = {
	M_(AND),
	M_(ANDS),
	M_(MOD),
	M_(NOT),
	M_(OR),
	M_(SHL),
	M_(SHR),
	M_(XOR),
	{ 0, 0 }
};

static const luaL_reg TrajectoryFuncs[] = {
	M_(GetAngles),
	M_(GetMaxHeight),
	M_(GetPosition),
	M_(GetSpeed),
	M_(GetTime),
	M_(GetVelocity),
	M_(GetYTimes),
	{ 0, 0 }
};

static const luaL_reg VectorFuncs[] = {
	M_(Add),
	M_(Angle),
	M_(ClampToMax),
	M_(Copy),
	M_(Cross),
	M_(Div),
	M_(Len),
	M_(Mul),
	M_(Neg),
	M_(ScaleTo),
	M_(SphereInvert),
	M_(Sub),
	M_(TLen),
	M_(Unit),
	M_(UnitF),
	M_(xz),
	M_(y),
	{ 0, 0 }
};

#undef M_
#define RT_(r, n, c, m, t) (r).mName = #n, (r).mUserType = 0, (r).mOffset = offsetof(c, m), (r).mType = t
#define RU_(r, n, c, m, u) (r).mName = #n, (r).mUserType = u, (r).mOffset = offsetof(c, m), (r).mType = Lua::UserType_Reg::eUserType

/// @brief Binds miscellaneous utilities to the Lua scripting system
void luaopen_misc (lua_State * L)
{
	Lua::UserType_Reg R[5];

	// Register Sphere type.
	RT_(R[0], x, Lua::AppTypes::Sphere, mCenter.m[0], Lua::UserType_Reg::eFloat);
	RT_(R[1], y, Lua::AppTypes::Sphere, mCenter.m[1], Lua::UserType_Reg::eFloat);
	RT_(R[2], z, Lua::AppTypes::Sphere, mCenter.m[2], Lua::UserType_Reg::eFloat);
	RU_(R[3], center, Lua::AppTypes::Sphere, mCenter, "Vector");
	RT_(R[4], radius, Lua::AppTypes::Sphere, mRadius, Lua::UserType_Reg::eFloat);
	Lua::RegisterUserType(L, "Sphere", R, 5, sizeof(Lua::AppTypes::Sphere), "");

	RU_(R[0], ul, Lua::AppTypes::Quad, mCorners[0], "Vector");
	RU_(R[1], ur, Lua::AppTypes::Quad, mCorners[1], "Vector");
	RU_(R[2], lr, Lua::AppTypes::Quad, mCorners[2], "Vector");
	RU_(R[3], ll, Lua::AppTypes::Quad, mCorners[3], "Vector");
	RU_(R[4], normal, Lua::AppTypes::Quad, mNormal, "Vector");
	Lua::RegisterUserType(L, "Quad", R, 5, sizeof(Lua::AppTypes::Quad), "");

	// Register Vector type.
	RT_(R[0], x, Lua::AppTypes::Vector, m[0], Lua::UserType_Reg::eFloat);
	RT_(R[1], y, Lua::AppTypes::Vector, m[1], Lua::UserType_Reg::eFloat);
	RT_(R[2], z, Lua::AppTypes::Vector, m[2], Lua::UserType_Reg::eFloat);
	Lua::RegisterUserType(L, "Vector", R, 3, sizeof(Lua::AppTypes::Vector), "__add __sub __mul __div __pow __unm __len", Add, Sub, Mul, Div, Cross, Neg, Len);

	// Install tables.
	luaL_register(L, "Collision", CollisionFuncs);
	luaL_register(L, "Math", MathFuncs);
	luaL_register(L, "Misc", MiscFuncs);
	luaL_register(L, "Ops", OpsFuncs);
	luaL_register(L, "Trajectory", TrajectoryFuncs);
	luaL_register(L, "Vec", VectorFuncs);
}

#undef RT_
#undef RU_