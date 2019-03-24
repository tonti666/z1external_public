#include "Color.h"

CColor CColor::Red = CColor(225, 15, 15);
CColor CColor::Orange = CColor(255, 165, 0);
CColor CColor::Yellow = CColor(255, 255, 0);
CColor CColor::Green = CColor(50, 205, 50);
CColor CColor::Blue = CColor(0, 191, 255);
CColor CColor::Indigo = CColor(75, 0, 130);
CColor CColor::Violet = CColor(238, 130, 238);
CColor CColor::White = CColor(255, 255, 255);
CColor CColor::Silver = CColor(192, 192, 192);
CColor CColor::Black = CColor(31, 31, 31);

HBRUSH CColor::RedBrush = CreateSolidBrush(CColor(225, 15, 15).GetRawColor());
HBRUSH CColor::OrangeBrush = CreateSolidBrush(CColor(255, 165, 0).GetRawColor());
HBRUSH CColor::YellowBrush = CreateSolidBrush(CColor(255, 255, 0).GetRawColor());
HBRUSH CColor::GreenBrush = CreateSolidBrush(CColor(50, 205, 50).GetRawColor());
HBRUSH CColor::BlueBrush = CreateSolidBrush(CColor(0, 191, 255).GetRawColor());
HBRUSH CColor::IndigoBrush = CreateSolidBrush(CColor(75, 0, 130).GetRawColor());
HBRUSH CColor::VioletBrush = CreateSolidBrush(CColor(238, 130, 238).GetRawColor());
HBRUSH CColor::WhiteBrush = CreateSolidBrush(CColor(255, 255, 255).GetRawColor());
HBRUSH CColor::SilverBrush = CreateSolidBrush(CColor(192, 192, 192).GetRawColor());
HBRUSH CColor::BlackBrush = CreateSolidBrush(CColor(31, 31, 31).GetRawColor());

CColor::CColor(COLORREF raw)
{
	SetRawColor(raw);
}

CColor::CColor()
{
	*((int *)this) = 0;
}

CColor::CColor(int r, int g, int b)
{
	SetColor(r, g, b, 255);
}

CColor::CColor(int r, int g, int b, int a)
{
	SetColor(r, g, b, a);
}

void CColor::SetColor(int r, int g, int b, int a)
{
	m_Color[0] = (unsigned char)r;
	m_Color[1] = (unsigned char)g;
	m_Color[2] = (unsigned char)b;
	m_Color[3] = (unsigned char)a;
}

void CColor::GetColor(int &r, int &g, int &b, int &a) const
{
	r = m_Color[0];
	g = m_Color[1];
	b = m_Color[2];
	a = m_Color[3];
}

void CColor::SetRawColor(int color32)
{
	*((int *)this) = color32;
}

COLORREF CColor::GetRawColor() const
{
	return RGB(r(), g(), b());
}

int CColor::r() const 
{ 
	return m_Color[0]; 
}

int CColor::g() const 
{ 
	return m_Color[1]; 
}

int CColor::b() const 
{ 
	return m_Color[2]; 
}

int CColor::a() const 
{ 
	return m_Color[3]; 
}

unsigned char& CColor::r() 
{ 
	return m_Color[0]; 
}

unsigned char& CColor::g() 
{ 
	return m_Color[1]; 
}

unsigned char& CColor::b() 
{ 
	return m_Color[2]; 
}

unsigned char& CColor::a() 
{ 
	return m_Color[3]; 
}

float CColor::rBase() const 
{ 
	return m_Color[0] / 255.0f; 
}

float CColor::gBase() const 
{ 
	return m_Color[1] / 255.0f; 
}

float CColor::bBase() const 
{ 
	return m_Color[2] / 255.0f; 
}

float CColor::aBase() const 
{ 
	return m_Color[3] / 255.0f; 
}

void CColor::SetAlpha(int a) 
{ 
	m_Color[3] = (unsigned char)a; 
}

int  CColor::GetAlpha() const 
{ 
	return m_Color[3]; 
}

unsigned char& CColor::operator[](int index)
{
	return m_Color[index];
}

unsigned char CColor::operator[](int index) const
{
	return m_Color[index];
}

bool CColor::operator==(const CColor &rhs) const
{
	return (*((int *)this) == *((int *)&rhs));
}

bool CColor::operator !=(const CColor &rhs) const
{
	return !(operator==(rhs));
}

CColor& CColor::operator=(const CColor &rhs)
{
	SetRawColor(rhs.GetRawColor());
	return *this;
}

float CColor::Hue() const
{
	if (m_Color[0] == m_Color[1] && m_Color[1] == m_Color[2])
	{
		return 0.0f;
	}

	float r = m_Color[0] / 255.0f;
	float g = m_Color[1] / 255.0f;
	float b = m_Color[2] / 255.0f;

	float max = r > g ? r : g > b ? g : b,
		min = r < g ? r : g < b ? g : b;
	float delta = max - min;
	float hue = 0.0f;

	if (r == max)
	{
		hue = (g - b) / delta;
	}
	else if (g == max)
	{
		hue = 2 + (b - r) / delta;
	}
	else if (b == max)
	{
		hue = 4 + (r - g) / delta;
	}
	hue *= 60;

	if (hue < 0.0f)
	{
		hue += 360.0f;
	}
	return hue;
}

float CColor::Saturation() const
{
	float r = m_Color[0] / 255.0f;
	float g = m_Color[1] / 255.0f;
	float b = m_Color[2] / 255.0f;

	float max = r > g ? r : g > b ? g : b,
		min = r < g ? r : g < b ? g : b;
	float l, s = 0;

	if (max != min)
	{
		l = (max + min) / 2;
		if (l <= 0.5f)
			s = (max - min) / (max + min);
		else
			s = (max - min) / (2 - max - min);
	}
	return s;
}

float CColor::Brightness() const
{
	float r = m_Color[0] / 255.0f;
	float g = m_Color[1] / 255.0f;
	float b = m_Color[2] / 255.0f;

	float max = r > g ? r : g > b ? g : b,
		min = r < g ? r : g < b ? g : b;
	return (max + min) / 2;
}

CColor CColor::FromHSB(float hue, float saturation, float brightness)
{
	float h = hue == 1.0f ? 0 : hue * 6.0f;
	float f = h - (int)h;
	float p = brightness * (1.0f - saturation);
	float q = brightness * (1.0f - saturation * f);
	float t = brightness * (1.0f - (saturation * (1.0f - f)));

	if (h < 1)
	{
		return CColor(
			(unsigned char)(brightness * 255),
			(unsigned char)(t * 255),
			(unsigned char)(p * 255)
		);
	}
	else if (h < 2)
	{
		return CColor(
			(unsigned char)(q * 255),
			(unsigned char)(brightness * 255),
			(unsigned char)(p * 255)
		);
	}
	else if (h < 3)
	{
		return CColor(
			(unsigned char)(p * 255),
			(unsigned char)(brightness * 255),
			(unsigned char)(t * 255)
		);
	}
	else if (h < 4)
	{
		return CColor(
			(unsigned char)(p * 255),
			(unsigned char)(q * 255),
			(unsigned char)(brightness * 255)
		);
	}
	else if (h < 5)
	{
		return CColor(
			(unsigned char)(t * 255),
			(unsigned char)(p * 255),
			(unsigned char)(brightness * 255)
		);
	}
	else
	{
		return CColor(
			(unsigned char)(brightness * 255),
			(unsigned char)(p * 255),
			(unsigned char)(q * 255)
		);
	}
}