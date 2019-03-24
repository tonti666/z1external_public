#pragma once
#include <initializer_list>
#include <functional>
#include <vector>
#include <Windows.h>

#include "Classes.h"

typedef float percentage_t;

#define DECIMAL_EPSILON 0.001

template <typename T>
constexpr __forceinline bool DecimalEnabled(const T& Value, const T& Threshold = T(DECIMAL_EPSILON))
{
	return Value > Threshold;
}

template <typename T>
constexpr __forceinline bool DecimalEqual(const T& a, const T& b, const T& Threshold = T(DECIMAL_EPSILON))
{
	return abs(a - b) < Threshold;
}

template <typename T>
constexpr __forceinline T Squared(const T& Value)
{
	return pow(Value, 2);
}

class CVector {
public:
	float x, y, z;

	explicit CVector(float X = 0.f, float Y = 0.f, float Z = 0.f);
	CVector(const CVector& rhs);
	CVector(const std::initializer_list<float>& List);
	void Init(float X = 0.f, float Y = 0.f, float Z = 0.f);

	CVector& operator=(const std::initializer_list<float>& List);
	CVector& operator=(const CVector& rhs);

	float Magnitude() const;
	float BaseMagnitude() const;
	float MagnitudeSquared() const;
	float DistanceTo(const CVector& V) const;
	float DotProduct(const CVector& V) const;

	void Overwrite(const float Value);
	float& operator[](const size_t Index);
	float operator[](const size_t Index) const;

	CVector operator+() const;
	CVector operator-() const;

	CVector operator+(const CVector& V) const;
	CVector operator-(const CVector& V) const;

	CVector& operator+=(const CVector& V);
	CVector& operator-=(const CVector& V);

	CVector operator*(const CVector& V) const;
	CVector operator/(const CVector& V) const;

	CVector operator*(float S) const;
	CVector operator/(float S) const;

	CVector& operator*=(const CVector& V);
	CVector& operator/=(const CVector& V);

	CVector& operator*=(float S);
	CVector& operator/=(float S);

	bool operator==(const CVector& V) const;
	bool operator!=(const CVector& V) const;

	PVOID Base();
	void Clamp();
	float BaseArg() const;
};

class CVector2D
{
public:
	float x, y;

	explicit CVector2D(float X = 0.f, float Y = 0.f);
	CVector2D(const CVector2D& rhs);
	CVector2D(const std::initializer_list<float>& List);
	void Init(float X, float Y = 0.f);

	CVector2D& operator=(const std::initializer_list<float>& List);
	CVector2D& operator=(const CVector2D& rhs);

	float Magnitude() const;
	float DistanceTo(const CVector2D& V) const;

	void Overwrite(const float Value);
	float& operator[](const size_t Index);
	float operator[](const size_t Index) const;

	CVector2D operator+() const;
	CVector2D operator-() const;

	CVector2D operator+(const CVector2D& V) const;
	CVector2D operator-(const CVector2D& V) const;

	CVector2D& operator+=(const CVector2D& V);
	CVector2D& operator-=(const CVector2D& V);

	CVector2D operator*(const CVector2D& V) const;
	CVector2D operator/(const CVector2D& V) const;

	CVector2D operator*(float S) const;
	CVector2D operator/(float S) const;

	CVector2D& operator*=(const CVector2D& V);
	CVector2D& operator/=(const CVector2D& V);

	CVector2D& operator*=(float S);
	CVector2D& operator/=(float S);

	bool operator==(const CVector2D& V) const;
	bool operator!=(const CVector2D& V) const;

	PVOID Base();
	void Clamp();
	float Arg() const;
};

class CPolarCoordinate2D
{
public:
	float ang, mag;

	explicit CPolarCoordinate2D(float _ang = 0.f, float _mag = 0.f);
	CPolarCoordinate2D(const CPolarCoordinate2D& rhs);
	CPolarCoordinate2D(const CVector2D& rhs);

	CPolarCoordinate2D& operator=(const CPolarCoordinate2D& rhs);
	CPolarCoordinate2D& operator=(const CVector2D& rhs);

	CPolarCoordinate2D operator+(const CPolarCoordinate2D& rhs) const;
	CPolarCoordinate2D operator-(const CPolarCoordinate2D& rhs) const;
	
	CPolarCoordinate2D operator*(const CPolarCoordinate2D& rhs) const;
	CPolarCoordinate2D operator/(const CPolarCoordinate2D& rhs) const;

	CPolarCoordinate2D operator+(const CVector2D& rhs) const;
	CPolarCoordinate2D operator-(const CVector2D& rhs) const;

	CPolarCoordinate2D operator*(const CVector2D& rhs) const;
	CPolarCoordinate2D operator/(const CVector2D& rhs) const;

	CPolarCoordinate2D operator+() const;
	CPolarCoordinate2D operator-() const;

	CPolarCoordinate2D& operator+=(const CPolarCoordinate2D& rhs);
	CPolarCoordinate2D& operator-=(const CPolarCoordinate2D& rhs);

	CPolarCoordinate2D& operator*=(const CPolarCoordinate2D& rhs);
	CPolarCoordinate2D& operator/=(const CPolarCoordinate2D& rhs);

	CPolarCoordinate2D& operator+=(const CVector2D& rhs);
	CPolarCoordinate2D& operator-=(const CVector2D& rhs);

	CPolarCoordinate2D& operator*=(const CVector2D& rhs);
	CPolarCoordinate2D& operator/=(const CVector2D& rhs);

	CPolarCoordinate2D& operator*=(float rhs);
	CPolarCoordinate2D& operator/=(float rhs);

	bool operator==(const CPolarCoordinate2D& rhs) const;
	bool operator!=(const CPolarCoordinate2D& rhs) const;

	bool operator==(const CVector2D& rhs) const;
	bool operator!=(const CVector2D& rhs) const;

	PVOID Base();
};

struct Matrix3x4_t
{
	float m_matrix[3][4];
};

struct Matrix4_t
{
	float m_matrix[4][4];
};

class IMath : public CBaseUtility
{
public:
	static constexpr float PI = 3.1415927f;
	static constexpr float TAU = 2.f * PI;
	static constexpr float PI_INV_2 = PI / 2.f;
	static constexpr float E = 2.718282f;
	static constexpr float PHI = 1.618034f;

	constexpr float RadiansToDegrees(const float Radians) const
	{
		return Radians * (180.f / PI);
	}

	constexpr float DegreesToRadians(const float Degrees) const
	{
		return Degrees * (PI / 180.f);
	}

	float TicksToTime(float Ticks) const;
	float TimeToTicks(float Time) const;
	int iTimeToTicks(float Time) const;

	void ClampAngle(CVector& InOut) const;
	void ClampAngle(CVector2D& InOut) const;

	float GetFOV(const CVector& View, const CVector& Aim) const;
	float GetFOV(const CVector2D& View, const CVector2D& Aim) const;

	void SmoothAngleLog(const CVector& View, CVector& Aim, float Smooth) const;
	void SmoothAngleLog(const CVector2D& View, CVector2D& Aim, float Smooth) const;

	void SmoothAngleLerp(const CVector& View, CVector& Aim, float Smooth, float Time) const;
	void SmoothAngleLerp(const CVector2D& View, CVector2D& Aim, float Smooth, float Time) const;

	void SmoothAngleExp(const CVector& View, CVector& Aim, float Smooth) const;
	void SmoothAngleExp(const CVector2D& View, CVector2D& Aim, float Smooth) const;

	void SmoothAngleRelative(const CVector& View, CVector& Aim, float Smooth) const;
	void SmoothAngleRelative(const CVector2D& View, CVector2D& Aim, float Smooth) const;

	void AngleVectors(const CVector& angles, CVector& forward) const;
	void AngleVectors(const CVector2D& angles, CVector& forward) const;

	void AngleVectors(const CVector& angles, CVector& forward, CVector& right, CVector& up) const;

	void VectorAngles(const CVector& forward, CVector& angles) const;
	void VectorAngles(const CVector& forward, CVector2D& angles) const;

	bool ProjectileAngles(float u, float g, const CVector& Source, const CVector& Destination, CVector2D& Angles) const;
	bool arctan(float o, float a, float& out) const;

	bool PassesActivationRate(percentage_t rate) const;

	template <typename T>
	T Lerp(const T& a, const T& b, percentage_t frac) const
	{
		return a + (b - a) * frac;
	}

	template <typename T>
	struct RootInterval_t
	{
		inline RootInterval_t(const T& _low, const T& _high) :
			low(_low), high(_high)
		{}

		T low, high;
	};

	template <typename T>
	void GetRootIntervals(std::vector<RootInterval_t<T>>& out, const T& start, const T& end, const T& jmp)
	{
		T firstneg = 0, firstpos = 0, it = start, fx = 0;

		while (it <= end)
		{
			fx = f(it);

			if (fx == 0.0)
			{
				out.emplace_back(it, it + jmp);
			}
			else
			{
				if (fx < 0.0)
				{
					firstneg = it;
				}
				else if (fx > 0.0)
				{
					firstpos = it;
				}

				if (firstneg != 0.0 && firstpos != 0.0)
				{
					out.emplace_back(min(firstneg, firstpos), max(firstpos, firstneg));
					firstneg = 0.0;
					firstpos = 0.0;
				}
			}

			it += jmp;
		}
	}

	/*
		T x = curroot.low, fx;

		for (size_t cur = 0; cur < PRECISION; cur++)
		{
			fx = f(x);
			x = linear_interpolation_solve(x, curroot.high, fx, fb);
		}
	*/

	//let b be the upper bound of a root's range; and a be that range's lower bound
//p(b, f(b)) and p(a, f(a))
//line between these two points and line from point to x axis at pi/2
//goal: find where line from points intersects x axis. let x be this point. take this point as new root and iterate for more precision
//we have two similar triangles: angle line between points makes with x axis is same because vert opp angles
//right angle from line from poitns to x axis; hence last angle in both triangles is 90 - angle line between points makes with x axis
//hecne similar triangles
//f(b) / -f(a) = (b - x) / (x - a)
//(scale factor hieghts) = (scale factor bases)
//x * (f(b) / -f(a)) + a * (f(b) / f(a)) + x = b
//x = ( b - a * (f(b) / f(a)) ) / ((f(b) / -f(a)) + 1)
//x = ( f(a) * b - a * f(b) ) / (-f(b) + f(a))
	template <typename T>
	T LinearInterpolationSolve(const std::function<T(void)>& f, const T& a, const T& b, const T& fa, const T& fb)
	{
		return ((fa * b - fb * a) / (fa - fb));
	}
	
	/*
		T x = curroot.high;

		for (size_t cur = 0; cur < PRECISION; cur++)
		{
			x = newton_raphson_solve(x);
		}
	*/

	//p(x0, f(x0))
//tangent: y - f(x0) = f'(x0)(x - x0)
//-f(x0) / f('x0) + x0 = x at zero.
//keep iteratin for more precis
	template <typename T>
	T NewtonRasphonSolve(const std::function<T(void)>& f, const std::function<T(void)>& fprime, const T& x)
	{
		return x - f(x) / fprime(x);
	}

	/*
		T curmov = (curroot.low + curroot.high) / 2;
		T x = curmov;

		for (size_t cur = 0; cur < PRECISION; cur++)
		{
			interval_bisection_solve(x, curmov);
		}
	*/
	template <typename T>
	void IntervalBisectionSolve(const std::function<T(void)>& f, T& x, T& curmov)
	{
		if (f(x) > 0)
		{
			x -= curmov / 2;
		}
		else
		{
			x += curmov / 2;
		}

		curmov /= 2;
	}

	template <typename T>
	T RandomNumber(T Base, percentage_t Deviation)
	{
		const T Low = Base * (1.f - Deviation);
		const T High = Base * (1.f + Deviation);

		return Low + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (High - Low)));
	}

	template <typename T>
	T RandomNumber(T Base, percentage_t Lowest, percentage_t Highest)
	{
		const T Low = Base * (1.f - Lowest);
		const T High = Base * (1.f + Highest);

		return Low + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (High - Low)));
	}

	CVector2D ToRectangular(const CPolarCoordinate2D& rhs);
	CPolarCoordinate2D FromRectangular(const CVector2D& rhs);
};

extern IMath g_Math;

class CTimer
{
private:
	float m_Start;

public:
	CTimer();
	~CTimer() = default;

	CTimer(const CTimer& Other) = default;
	CTimer& operator=(const CTimer&) = default;

	void Reset();

	float Elapsed() const;
	float Ticks() const;
	
	static void SleepThread(float Time);
};