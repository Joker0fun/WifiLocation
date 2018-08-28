#ifndef _DEF_VECTOR_H_
#define _DEF_VECTOR_H_

#include <math.h>

class CVector2
{
public:
	double x;
	double y;
	CVector2() {};
	CVector2(double xx, double yy)
	{
		x = xx;
		y = yy;
	}

	inline CVector2& operator += (const CVector2& in)
	{
		x += in.x;
		y += in.y;
		return *this;
	};

	inline CVector2& operator -=(const CVector2& in)
	{
		CVector2 out;
		x -= in.x;
		y -= in.y;
		return *this;
	};

	inline CVector2& operator *=(double &in)
	{
		x *= in;
		y *= in;
		return *this;
	};

	inline CVector2& operator /=(double &in)
	{
		x /= in;
		y /= in;
		return *this;
	};

	inline CVector2 operator + (const CVector2& v) const
	{
		return CVector2(x + v.x, y + v.y);
	}

	inline CVector2 operator -(const CVector2& v) const
	{
		return CVector2(x - v.x, y - v.y);
	};

	inline CVector2 operator * (double  in) const
	{
		return CVector2(x*in, y*in);
	};

	inline CVector2 operator /(double in) const
	{
		return CVector2(x / in, y / in);
	};

	inline bool operator == (const CVector2& in)
	{
		if (in.x == x && in.y == y) return true;
		return false;
	};

	inline double length()
	{
		return sqrt(x*x + y*y);
	}

	double Dot(const CVector2& v) const
	{
		return x*v.x + y*v.y;
	}
};


class CVector3
{
public:
	double x;
	double y;
	double z;
	CVector3() {};
	CVector3(double xx, double yy, double zz)
	{
		x = xx;
		y = yy;
		z = zz;
	}

	inline CVector3& operator += (const CVector3& in)
	{
		x += in.x;
		y += in.y;
		z += in.z;
		return *this;
	};

	inline CVector3& operator -=(const CVector3& in)
	{
		CVector3 out;
		x -= in.x;
		y -= in.y;
		z -= in.z;
		return *this;
	};

	inline CVector3& operator *=(double &in)
	{
		x *= in;
		y *= in;
		z *= in;
		return *this;
	};

	inline CVector3& operator /=(double &in)
	{
		x /= in;
		y /= in;
		z /= in;
		return *this;
	};

	inline CVector3 operator + (const CVector3& v) const
	{
		return CVector3(x + v.x, y + v.y, z + v.z);
	}

	inline CVector3 operator -(const CVector3& v) const
	{
		return CVector3(x - v.x, y - v.y, z - v.z);
	};

	inline CVector3 operator * (double  in) const
	{
		return CVector3(x*in, y*in, z*in);
	};

	inline CVector3 operator /(double in) const
	{
		return CVector3(x / in, y / in, z / in);
	};

	inline bool operator == (const CVector3& in)
	{
		if (in.x == x && in.y == y && in.z == z) return true;
		return false;
	};

	inline double length()
	{
		return sqrt(x*x + y*y + z*z);
	}

	double Dot(const CVector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}
};

#endif
