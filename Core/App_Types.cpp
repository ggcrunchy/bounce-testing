#include "App.h"
#include <cmath>

namespace Lua
{
	namespace AppTypes
	{	
		Vector::Vector (Vector const & v)
		{
			m[0] = v.m[0];
			m[1] = v.m[1];
			m[2] = v.m[2];
		}

		Vector::Vector (float x, float y, float z)
		{
			m[0] = x;
			m[1] = y;
			m[2] = z;
		}

		Vector::Vector (float v[3])
		{
			m[0] = v[0];
			m[1] = v[1];
			m[2] = v[2];
		}

		Vector::Vector (void)
		{
		}

		float Vector::angle (void)
		{
			return atan2f(m[1], sqrtf(m[0] * m[0] + m[2] * m[2]));
		}

		float Vector::length (void)
		{
			return sqrtf(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
		}

		Vector Vector::XZ (void)
		{
			return Vector(m[0], 0.0f, m[2]);
		}

		Vector Vector::Y (void)
		{
			return Vector(0.0f, m[1], 0.0f);
		}

		Vector operator + (Vector const & v, Vector const & w)
		{
			return Vector(v.m[0] + w.m[0], v.m[1] + w.m[1], v.m[2] + w.m[2]);
		}

		Vector operator - (Vector const & v, Vector const & w)
		{
			return Vector(v.m[0] - w.m[0], v.m[1] - w.m[1], v.m[2] - w.m[2]);
		}

		Vector operator * (Vector const & v, float k)
		{
			return Vector(k * v.m[0], k * v.m[1], k * v.m[2]);
		}

		Vector operator * (float k, Vector const & v)
		{
			return v * k;
		}

		Vector operator / (Vector const & v, float k)
		{
			return v * (1.0f / k);
		}

		Vector operator ^ (Vector const & v, Vector const & w)
		{
			return Vector(v.m[1] * w.m[2] - v.m[2] * w.m[1],
						  v.m[2] * w.m[0] - v.m[0] * w.m[2],
						  v.m[0] * w.m[1] - v.m[1] * w.m[0]);
		}

		Vector operator - (Vector const & v)
		{
			return Vector(-v.m[0], -v.m[1], -v.m[2]);
		}

		Vector operator ~ (Vector const & v)
		{
			AppTypes::Vector w(v);

			return w / w.length();
		}

		float operator * (Vector const & v, Vector const & w)
		{
			return v.m[0] * w.m[0] + v.m[1] * w.m[1] + v.m[2] * w.m[2];
		}

		Quad::Quad (Vector const & ul, Vector const & ur, Vector const & lr, Vector const & ll)
		{
			// Load the corners.
			mCorners[0] = ul;
			mCorners[1] = ur;
			mCorners[2] = lr;
			mCorners[3] = ll;

			// Compute the normal.
			mNormal = ~((ur - ul) ^ (ll - ul));
		}

		Quad::Quad (float ul[3], float ur[3], float lr[3], float ll[3])
		{
			// Load the corners.
			mCorners[0] = AppTypes::Vector(ul);
			mCorners[1] = AppTypes::Vector(ur);
			mCorners[2] = AppTypes::Vector(lr);
			mCorners[3] = AppTypes::Vector(ll);

			// Compute the normal.
			mNormal = ~((mCorners[1] - mCorners[0]) ^ (mCorners[3] - mCorners[0]));
		}

		Sphere::Sphere (Sphere const & sphere) : mCenter(sphere.mCenter), mRadius(sphere.mRadius)
		{
		}

		Sphere::Sphere (Vector const & center, float radius) : mCenter(center), mRadius(radius)
		{
		}

		Sphere::Sphere (void)
		{
		}
	}
}