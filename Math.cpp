#include "Math.h"
#include "Structs.h"

#include <thread>
#include <chrono>
#include <algorithm>

IMath g_Math;

CVector::CVector(float X, float Y, float Z)
	: x(X), y(Y), z(Z)
{
}

CVector::CVector(const CVector& rhs)
	: x(rhs.x), y(rhs.y), z(rhs.z)
{
}

CVector::CVector(const std::initializer_list<float>& List)
{
	size_t Count = 0;
	for (const auto i : List)
	{
		((float*)this)[Count] = i;
		Count++;
	}

	for (auto i = Count; i < 3; i++)
	{
		((float*)this)[i] = 0.f;
	}
}

void CVector::Init(float X, float Y, float Z)
{
	x = X; y = Y; z = Z;
}

CVector& CVector::operator=(const std::initializer_list<float>& List)
{
	size_t Count = 0;
	for (const auto i : List)
	{
		((float*)this)[Count] = i;
		Count++;
	}

	for (auto i = Count; i < 3; i++)
	{
		((float*)this)[i] = 0.f;
	}

	return *this;
}

CVector& CVector::operator=(const CVector& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

float CVector::Magnitude() const
{
	return sqrtf(Squared(x) + Squared(y) + Squared(z));
}

float CVector::BaseMagnitude() const
{
	return sqrtf(Squared(x) + Squared(y));
}

float CVector::MagnitudeSquared() const
{
	return Squared(x) + Squared(y) + Squared(z);
}

float CVector::DistanceTo(const CVector& V) const
{
	return ((*this) - V).Magnitude();
}

float CVector::DotProduct(const CVector& V) const
{
	return x * V.x + y * V.y + z * V.z;
}

void CVector::Overwrite(const float Value)
{
	const CVector ValueVector(Value, Value, Value);
	(*this) = ValueVector;
}

float& CVector::operator[](const size_t Index)
{
	return ((float*)this)[Index];
}

float CVector::operator[](const size_t Index) const
{
	return ((float*)this)[Index];
}

CVector CVector::operator+() const
{
	return CVector(+x, +y, +z);
}

CVector CVector::operator-() const
{
	return CVector(-x, -y, -z);
}

CVector CVector::operator+(const CVector& V) const
{
	return CVector(x + V.x, y + V.y, z + V.z);
}

CVector CVector::operator-(const CVector& V) const
{
	return CVector(x - V.x, y - V.y, z - V.z);
}

CVector& CVector::operator+=(const CVector& V)
{
	x += V.x;
	y += V.y;
	z += V.z;

	return *this;
}

CVector& CVector::operator-=(const CVector& V)
{
	x -= V.x;
	y -= V.y;
	z -= V.z;

	return *this;
}

CVector CVector::operator*(const CVector& V) const
{
	return CVector(x * V.x, y * V.y, z * V.z);
}

CVector CVector::operator/(const CVector& V) const
{
	return CVector(x / V.x, y / V.y, z / V.z);
}

CVector CVector::operator*(float S) const
{
	return CVector(x * S, y * S, z * S);
}

CVector CVector::operator/(float S) const
{
	return CVector(x / S, y / S, z / S);
}

CVector& CVector::operator*=(const CVector& V)
{
	x *= V.x;
	y *= V.y;
	z *= V.z;
	return *this;
}

CVector& CVector::operator/=(const CVector& V)
{
	x /= V.x;
	y /= V.y;
	z /= V.z;
	return *this;
}

CVector & CVector::operator*=(float S)
{
	x *= S;
	y *= S;
	z *= S;
	return *this;
}

CVector & CVector::operator/=(float S)
{
	x /= S;
	y /= S;
	z /= S;
	return *this;
}

bool CVector::operator==(const CVector& V) const
{
	return DecimalEqual(x, V.x) && DecimalEqual(y, V.y) && DecimalEqual(z, V.z);
}

bool CVector::operator!=(const CVector& V) const
{
	return !((*this) == V);
}

PVOID CVector::Base()
{
	return (PVOID)this;
}

void CVector::Clamp()
{
	g_Math.ClampAngle(*this);
}

float CVector::BaseArg() const
{
	float ang = atanf(y / x);

	if (x < 0.f && y < 0.f)
	{
		ang -= g_Math.PI;
	}
	else if (x < 0.f && y > 0.f)
	{
		ang += g_Math.PI;
	}

	return ang;
}

CVector2D::CVector2D(float X, float Y)
	: x(X), y(Y)
{
}

CVector2D::CVector2D(const CVector2D& rhs)
	: x(rhs.x), y(rhs.y)
{
}

CVector2D::CVector2D(const std::initializer_list<float>& List)
{
	size_t Count = 0;
	for (const auto i : List)
	{
		((float*)this)[Count] = i;
		Count++;
	}

	for (auto i = Count; i < 2; i++)
	{
		((float*)this)[i] = 0.f;
	}
}

void CVector2D::Init(float X, float Y)
{
	x = X; y = Y;
}

CVector2D& CVector2D::operator=(const std::initializer_list<float>& List)
{
	size_t Count = 0;
	for (const auto i : List)
	{
		((float*)this)[Count] = i;
		Count++;
	}

	for (auto i = Count; i < 2; i++)
	{
		((float*)this)[i] = 0.f;
	}

	return *this;
}

CVector2D& CVector2D::operator=(const CVector2D& rhs)
{
	x = rhs.x;
	y = rhs.y;

	return *this;
}

float CVector2D::Magnitude() const
{
	return sqrtf(Squared(x) + Squared(y));
}

float CVector2D::DistanceTo(const CVector2D& V) const
{
	return ((*this) - V).Magnitude();
}

void CVector2D::Overwrite(const float Value)
{
	const CVector2D ValueVector2D(Value, Value);
	(*this) = ValueVector2D;
}

float& CVector2D::operator[](const size_t Index)
{
	return ((float*)this)[Index];
}

float CVector2D::operator[](const size_t Index) const
{
	return ((float*)this)[Index];
}

CVector2D CVector2D::operator+() const
{
	return CVector2D(+x, +y);
}

CVector2D CVector2D::operator-() const
{
	return CVector2D(-x, -y);
}

CVector2D CVector2D::operator+(const CVector2D& V) const
{
	return CVector2D(x + V.x, y + V.y);
}

CVector2D CVector2D::operator-(const CVector2D& V) const
{
	return CVector2D(x - V.x, y - V.y);
}

CVector2D& CVector2D::operator+=(const CVector2D& V)
{
	x += V.x;
	y += V.y;

	return *this;
}

CVector2D& CVector2D::operator-=(const CVector2D& V)
{
	x -= V.x;
	y -= V.y;

	return *this;
}

CVector2D CVector2D::operator*(const CVector2D& V) const
{
	return CVector2D(x * V.x, y * V.y);
}

CVector2D CVector2D::operator/(const CVector2D& V) const
{
	return CVector2D(x / V.x, y / V.y);
}

CVector2D CVector2D::operator*(float S) const
{
	return CVector2D(x * S, y * S);
}

CVector2D CVector2D::operator/(float S) const
{
	return CVector2D(x / S, y / S);
}

CVector2D& CVector2D::operator*=(const CVector2D& V)
{
	x *= V.x;
	y *= V.y;
	return *this;
}

CVector2D& CVector2D::operator/=(const CVector2D& V)
{
	x /= V.x;
	y /= V.y;
	return *this;
}

CVector2D & CVector2D::operator*=(float S)
{
	x *= S;
	y *= S;
	return *this;
}

CVector2D & CVector2D::operator/=(float S)
{
	x /= S;
	y /= S;
	return *this;
}

bool CVector2D::operator==(const CVector2D& V) const
{
	return DecimalEqual(x, V.x) && DecimalEqual(y, V.y);
}

bool CVector2D::operator!=(const CVector2D& V) const
{
	return !((*this) == V);
}

PVOID CVector2D::Base()
{
	return (PVOID)this;
}

void CVector2D::Clamp()
{
	g_Math.ClampAngle(*this);
}

float CVector2D::Arg() const
{
	float ang = atanf(y / x);

	if (x < 0.f && y < 0.f)
	{
		ang -= g_Math.PI;
	}
	else if (x < 0.f && y > 0.f)
	{
		ang += g_Math.PI;
	}

	return ang;
}

static CVector ClampAngleCopy(CVector In) 
{
	if (In.x < -89.0f)
		In.x = -89.0f;

	if (In.x > 89.0f)
		In.x = 89.0f;

	while (In.y < -180.0f)
		In.y += 360.0f;

	while (In.y > 180.0f)
		In.y -= 360.0f;

	In.z = 0.0f;
	return In;
}

static CVector2D ClampAngleCopy(CVector2D In) 
{
	if (In.x < -89.0f)
		In.x = -89.0f;

	if (In.x > 89.0f)
		In.x = 89.0f;

	while (In.y < -180.0f)
		In.y += 360.0f;

	while (In.y > 180.0f)
		In.y -= 360.0f;

	return In;
}

float IMath::TicksToTime(float Ticks) const
{
	return Ticks * g_GlobalVars.m_flIntervalPerTick;
}

float IMath::TimeToTicks(float Time) const
{
	return Time / g_GlobalVars.m_flIntervalPerTick;
}

int IMath::iTimeToTicks(float Time) const
{
	return int((Time / g_GlobalVars.m_flIntervalPerTick) + 0.5f);
}

void IMath::ClampAngle(CVector& InOut) const
{
	if (InOut.x < -89.0f)
		InOut.x = -89.0f;

	if (InOut.x > 89.0f)
		InOut.x = 89.0f;

	while (InOut.y < -180.0f)
		InOut.y += 360.0f;

	while (InOut.y > 180.0f)
		InOut.y -= 360.0f;

	InOut.z = 0.0f;
}

void IMath::ClampAngle(CVector2D& InOut) const
{
	if (InOut.x < -89.0f)
		InOut.x = -89.0f;

	if (InOut.x > 89.0f)
		InOut.x = 89.0f;

	while (InOut.y < -180.0f)
		InOut.y += 360.0f;

	while (InOut.y > 180.0f)
		InOut.y -= 360.0f;
}

float IMath::GetFOV(const CVector& View, const CVector& Aim) const
{
	return ClampAngleCopy(View - Aim).Magnitude();
}

float IMath::GetFOV(const CVector2D & View, const CVector2D & Aim) const
{
	return ClampAngleCopy(View - Aim).Magnitude();
}

void IMath::SmoothAngleLog(const CVector& View, CVector& Aim, float Smooth) const
{
	if (!DecimalEnabled(Smooth))
	{
		return;
	}

	Smooth = std::clamp(Smooth, 0.f, 49.99f);

	auto Delta = ClampAngleCopy(View - Aim);

	Aim = CVector(
		View.x - (Delta.x / 100.f) * (50.f - Smooth),
		View.y - (Delta.y / 100.f) * (50.f - Smooth)
	);

	ClampAngle(Aim);
}

void IMath::SmoothAngleLog(const CVector2D & View, CVector2D & Aim, float Smooth) const
{
	if (!DecimalEnabled(Smooth))
	{
		return;
	}

	Smooth = std::clamp(Smooth, 0.f, 49.99f);

	auto Delta = ClampAngleCopy(View - Aim);

	Aim = CVector2D(
		View.x - (Delta.x / 100.f) * (50.f - Smooth),
		View.y - (Delta.y / 100.f) * (50.f - Smooth)
	);

	ClampAngle(Aim);
}

void IMath::SmoothAngleLerp(const CVector & View, CVector & Aim, float Smooth, float Time) const
{
	Aim = View + ((Aim - View) * (Time / (Smooth / 10.f)));

	ClampAngle(Aim);
}

void IMath::SmoothAngleLerp(const CVector2D & View, CVector2D & Aim, float Smooth, float Time) const
{
	Aim = View + ((Aim - View) * (Time / (Smooth / 10.f)));

	ClampAngle(Aim);
}

void IMath::SmoothAngleExp(const CVector & View, CVector & Aim, float Smooth) const
{
	if (!DecimalEnabled(Smooth))
	{
		return;
	}

	Smooth = std::clamp(Smooth, 0.f, 49.99f);

	auto Delta = ClampAngleCopy(View - Aim);

	Delta.x *= 1.f / expf(fabsf(Delta.x));
	Delta.y *= 1.f / expf(fabsf(Delta.y));

	Aim = CVector(
		View.x - (Delta.x / 10.f) * (50.f - Smooth),
		View.y - (Delta.y / 10.f) * (50.f - Smooth)
	);

	ClampAngle(Aim);
}

void IMath::SmoothAngleExp(const CVector2D & View, CVector2D & Aim, float Smooth) const
{
	if (!DecimalEnabled(Smooth))
	{
		return;
	}

	Smooth = std::clamp(Smooth, 0.f, 49.99f);

	auto Delta = ClampAngleCopy(View - Aim);

	Delta.x *= 1.f / expf(fabsf(Delta.x));
	Delta.y *= 1.f / expf(fabsf(Delta.y));

	Aim = CVector2D(
		View.x - (Delta.x / 10.f) * (50.f - Smooth),
		View.y - (Delta.y / 10.f) * (50.f - Smooth)
	);

	ClampAngle(Aim);
}

void IMath::SmoothAngleRelative(const CVector& View, CVector& Aim, float Smooth) const
{
	if (!DecimalEnabled(Smooth))
	{
		return;
	}

	CVector delta(
		View.x - (View.x - Aim.x),
		View.y - (View.y - Aim.y)
	);

	ClampAngle(delta);

	Aim = CVector(
		Aim.x - delta.x / 100.f * Smooth,
		Aim.y - delta.y / 100.f * Smooth
	);
}

void IMath::SmoothAngleRelative(const CVector2D & View, CVector2D & Aim, float Smooth) const
{
	if (!DecimalEnabled(Smooth))
	{
		return;
	}

	CVector delta(
		View.x - (View.x - Aim.x),
		View.y - (View.y - Aim.y)
	);

	ClampAngle(delta);

	Aim = CVector2D(
		Aim.x - delta.x / 100.f * Smooth,
		Aim.y - delta.y / 100.f * Smooth
	);
}

static inline void SinCos(float* pSin, float* pCos, float Value)
{
	*pSin = sinf(Value);
	*pCos = cosf(Value);
}

void IMath::AngleVectors(const CVector &angles, CVector& forward) const
{
	float	sp, sy, cp, cy;

	SinCos(&sp, &cp, g_Math.DegreesToRadians(angles[0]));
	SinCos(&sy, &cy, g_Math.DegreesToRadians(angles[1]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void IMath::AngleVectors(const CVector2D & angles, CVector & forward) const
{
	float	sp, sy, cp, cy;

	SinCos(&sp, &cp, g_Math.DegreesToRadians(angles[0]));
	SinCos(&sy, &cy, g_Math.DegreesToRadians(angles[1]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void IMath::AngleVectors(const CVector &angles, CVector& forward, CVector& right, CVector& up) const
{
	float sr, sp, sy, cr, cp, cy;

	SinCos(&sp, &cp, g_Math.DegreesToRadians(angles[0]));
	SinCos(&sy, &cy, g_Math.DegreesToRadians(angles[1]));
	SinCos(&sr, &cr, g_Math.DegreesToRadians(angles[2]));

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

void IMath::VectorAngles(const CVector& forward, CVector &angles) const
{
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

void IMath::VectorAngles(const CVector & forward, CVector2D & angles) const
{
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);
	}

	angles[0] = pitch;
	angles[1] = yaw;
}

bool IMath::ProjectileAngles(float u, float g, const CVector & Source, const CVector & Destination, CVector2D& Angles) const
{
	const float s = (Source - Destination).BaseMagnitude();
	const float h = Source.z - Destination.z;
	const float bsquared = Squared(s) + (2 * g * s) / u - Squared((g * s) / u);

	if (bsquared < 0.f)
	{
		return false;
	}

	const float coeff = (u / (g * s));
	const float sola = atanf(coeff * (s - sqrtf(bsquared)));
	const float solb = atanf(coeff * (s + sqrtf(bsquared)));

	Angles.x = sola;
	const bool bres = arctan(Source.y - Destination.y, Source.x - Destination.x, Angles.y);

	return bres;
}

bool IMath::arctan(float o, float a, float & out) const
{
	const auto mag = sqrt(Squared(o) + Squared(a));
	if (DecimalEqual(mag, 0.f, (float)DECIMAL_EPSILON / 100.f))
	{
		return false;
	}

	const auto sine = sin(o / mag);
	const auto cosine = cos(a / mag);
	if (DecimalEqual(cosine, 0.f, (float)DECIMAL_EPSILON / 100.f))
	{
		return false;
	}

	out = sine / cosine;

	if (o < 0.f)
	{
		if (a < 0.f)
		{
			out -= g_Math.PI;
		}
		else if (a > 0.f)
		{
			out += g_Math.PI;
		}
	}

	return true;
}

bool IMath::PassesActivationRate(percentage_t rate) const
{
	return (rand() % 1000) < int(rate * 1000.f + 0.5f);
}

CTimer::CTimer()
{
	Reset();
}

void CTimer::Reset()
{
	m_Start = g_GlobalVars.m_flCurTime;
}

float CTimer::Elapsed() const
{
	return g_GlobalVars.m_flCurTime - m_Start;
}

float CTimer::Ticks() const
{
	return g_Math.TimeToTicks(this->Elapsed());
}

void CTimer::SleepThread(float Time)
{
	std::this_thread::sleep_for(std::chrono::duration<long, std::micro>((long)(Time * 1000000.f)));
}

CPolarCoordinate2D::CPolarCoordinate2D(float _ang, float _mag) :
	ang(_ang), mag(_mag)
{
	;
}

CPolarCoordinate2D::CPolarCoordinate2D(const CPolarCoordinate2D & rhs)
{
	*this = rhs;
}

CPolarCoordinate2D::CPolarCoordinate2D(const CVector2D & rhs)
{
	*this = rhs;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator=(const CPolarCoordinate2D & rhs)
{
	ang = rhs.ang;
	mag = rhs.mag;
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator=(const CVector2D & rhs)
{
	*this = g_Math.FromRectangular(rhs);
	return *this;
}

CVector2D IMath::ToRectangular(const CPolarCoordinate2D & rhs)
{
	return CVector2D(cosf(rhs.ang) * rhs.mag, sinf(rhs.ang) * rhs.mag);
}

CPolarCoordinate2D IMath::FromRectangular(const CVector2D & rhs)
{
	const float _mag = rhs.Magnitude();

	if (rhs.x < rhs.y)
	{
		return CPolarCoordinate2D(acosf(rhs.x / _mag), _mag);
	}
	else
	{
		return CPolarCoordinate2D(asinf(rhs.y / _mag), _mag);
	}
}

CPolarCoordinate2D CPolarCoordinate2D::operator+(const CPolarCoordinate2D & rhs) const
{
	return g_Math.FromRectangular((g_Math.ToRectangular(*this) + g_Math.ToRectangular(rhs)));
}

CPolarCoordinate2D CPolarCoordinate2D::operator-(const CPolarCoordinate2D & rhs) const
{
	return g_Math.FromRectangular((g_Math.ToRectangular(*this) - g_Math.ToRectangular(rhs)));
}

CPolarCoordinate2D CPolarCoordinate2D::operator*(const CPolarCoordinate2D & rhs) const
{
	return g_Math.FromRectangular((g_Math.ToRectangular(*this) * g_Math.ToRectangular(rhs)));
}

CPolarCoordinate2D CPolarCoordinate2D::operator/(const CPolarCoordinate2D & rhs) const
{
	return g_Math.FromRectangular((g_Math.ToRectangular(*this) / g_Math.ToRectangular(rhs)));
}

CPolarCoordinate2D CPolarCoordinate2D::operator+(const CVector2D & rhs) const
{
	return *this + g_Math.FromRectangular(rhs);
}

CPolarCoordinate2D CPolarCoordinate2D::operator-(const CVector2D & rhs) const
{
	return *this - g_Math.FromRectangular(rhs);
}

CPolarCoordinate2D CPolarCoordinate2D::operator*(const CVector2D & rhs) const
{
	return *this * g_Math.FromRectangular(rhs);
}

CPolarCoordinate2D CPolarCoordinate2D::operator/(const CVector2D & rhs) const
{
	return *this / g_Math.FromRectangular(rhs);
}

CPolarCoordinate2D CPolarCoordinate2D::operator+() const
{
	return CPolarCoordinate2D(+ang, +mag);
}

CPolarCoordinate2D CPolarCoordinate2D::operator-() const
{
	return CPolarCoordinate2D(-ang, -mag);
}

CPolarCoordinate2D& CPolarCoordinate2D::operator+=(const CPolarCoordinate2D & rhs) 
{
	*this = g_Math.ToRectangular(*this) + g_Math.ToRectangular(rhs);
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator-=(const CPolarCoordinate2D & rhs)
{
	*this = g_Math.ToRectangular(*this) - g_Math.ToRectangular(rhs);
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator*=(const CPolarCoordinate2D & rhs)
{
	ang += rhs.ang;
	mag *= rhs.mag;
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator/=(const CPolarCoordinate2D & rhs)
{
	ang -= rhs.ang;
	mag /= rhs.mag;
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator+=(const CVector2D & rhs)
{
	*this += g_Math.FromRectangular(rhs);
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator-=(const CVector2D & rhs)
{
	*this -= g_Math.FromRectangular(rhs);
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator*=(const CVector2D & rhs)
{
	*this *= g_Math.FromRectangular(rhs);
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator/=(const CVector2D & rhs)
{
	*this /= g_Math.FromRectangular(rhs);
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator*=(float rhs)
{
	mag *= rhs;
	return *this;
}

CPolarCoordinate2D & CPolarCoordinate2D::operator/=(float rhs)
{
	mag /= rhs;
	return *this;
}

bool CPolarCoordinate2D::operator==(const CPolarCoordinate2D & rhs) const
{
	return DecimalEqual(ang, rhs.ang) && DecimalEqual(mag, rhs.mag);
}

bool CPolarCoordinate2D::operator!=(const CPolarCoordinate2D & rhs) const
{
	return !(*this == rhs);
}

bool CPolarCoordinate2D::operator==(const CVector2D & rhs) const
{
	return *this == g_Math.FromRectangular(rhs);
}

bool CPolarCoordinate2D::operator!=(const CVector2D & rhs) const
{
	return *this != g_Math.FromRectangular(rhs);
}

PVOID CPolarCoordinate2D::Base()
{
	return this;
}
