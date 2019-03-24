#pragma once
#include <Windows.h>

class CColor
{
public:
	explicit CColor(COLORREF raw);

	CColor();
	CColor(int r, int g, int b);
	CColor(int r, int g, int b, int a);

	void SetColor(int r, int g, int b, int a = 255);
	void GetColor(int &r, int &g, int &b, int &a) const;

	void SetRawColor(int color32);
	COLORREF GetRawColor() const;

	int r() const;
	int g() const;
	int b() const;
	int a() const;

	unsigned char& r();
	unsigned char& g();
	unsigned char& b();
	unsigned char& a();

	float rBase() const;
	float gBase() const;
	float bBase() const;
	float aBase() const;

	void SetAlpha(int a);
	int GetAlpha() const;

	unsigned char &operator[](int index);
	unsigned char operator[](int index) const;

	bool operator == (const CColor &rhs) const;
	bool operator != (const CColor &rhs) const;

	CColor &operator=(const CColor &rhs);

	float Hue() const;
	float Saturation() const;
	float Brightness() const;

	static CColor FromHSB(float hue, float saturation, float brightness);

	static CColor Red;
	static HBRUSH RedBrush;

	static CColor Orange;
	static HBRUSH OrangeBrush;

	static CColor Yellow;
	static HBRUSH YellowBrush;

	static CColor Green;
	static HBRUSH GreenBrush;

	static CColor Blue;
	static HBRUSH BlueBrush;

	static CColor Indigo;
	static HBRUSH IndigoBrush;

	static CColor Violet;
	static HBRUSH VioletBrush;

	static CColor White;
	static HBRUSH WhiteBrush;

	static CColor Silver;
	static HBRUSH SilverBrush;

	static CColor Black;
	static HBRUSH BlackBrush;

private:
	unsigned char m_Color[4];
};