/*
Edwards Curve

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#pragma once

template <typename T, int D>
struct EdwardsCurve
{
	T x, y;
	EdwardsCurve(T x, T y) : x(x), y(y) {}
	EdwardsCurve(uint32_t compressed)
	{
		uint32_t sign = compressed >> 31;
		x = T(compressed & 0x7FFFFFFF);
		y = findY(x);
		if ((y() & 1) != sign)
			y = -y;
	}
	static T findY(T x)
	{
		T xx = x * x;
		T yy = (T(1) - xx) / (T(1) - T(D) * xx);
		return pow(yy, 1U << 29);
	}
	EdwardsCurve<T, D> operator += (EdwardsCurve<T, D> a)
	{
		return *this = a + *this;
	}
	uint32_t compress() const
	{
		uint32_t sign = y() & 1;
		return x() | (sign << 31);
	}
};

template <typename T, int D>
static bool operator == (EdwardsCurve<T, D> a, EdwardsCurve<T, D> b)
{
	return a.x == b.x && a.y == b.y;
}

template <typename T, int D>
static bool operator != (EdwardsCurve<T, D> a, EdwardsCurve<T, D> b)
{
	return a.x != b.x || a.y != b.y;
}

template <typename T, int D>
static EdwardsCurve<T, D> operator + (EdwardsCurve<T, D> a, EdwardsCurve<T, D> b)
{
	T xx = a.x * b.x;
	T yy = a.y * b.y;
	T xy = a.x * b.y;
	T yx = a.y * b.x;
	T dxy = T(D) * xx * yy;
	T x = (xy + yx) / (T(1) + dxy);
	T y = (yy - xx) / (T(1) - dxy);
	return EdwardsCurve<T, D>(x, y);
}

template <typename T, int D>
static EdwardsCurve<T, D> operator - (EdwardsCurve<T, D> a)
{
	return EdwardsCurve<T, D>(-a.x, a.y);
}

template <typename T, int D>
static EdwardsCurve<T, D> operator - (EdwardsCurve<T, D> a, EdwardsCurve<T, D> b)
{
	return a + -b;
}

template <typename T, int D>
static EdwardsCurve<T, D> operator * (uint32_t a, EdwardsCurve<T, D> b)
{
	EdwardsCurve<T, D> t(T(0), T(1));
	for (;a; a >>= 1, b += b)
		if (a & 1)
			t += b;
	return t;
}

