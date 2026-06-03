/*
Complex Extension Field

Copyright 2026 Ahmet Inan <xdsopl@gmail.com>
*/

#pragma once

namespace CODE {

template <typename T>
class ComplexField
{
	T re, im;
public:
	typedef T value_type;
	constexpr ComplexField() : re(0), im(0) {}
	constexpr ComplexField(T r) : re(r), im(0) {}
	constexpr ComplexField(T r, T i) : re(r), im(i) {}
	constexpr T real() const { return re; }
	constexpr T imag() const { return im; }
	inline void real(T r) { re = r; }
	inline void imag(T i) { im = i; }
	inline ComplexField<T> operator = (T a)
	{
		re = a;
		im = 0;
		return *this;
	}
	inline ComplexField<T> operator += (ComplexField<T> a)
	{
		return *this = a + *this;
	}
	inline ComplexField<T> operator -= (ComplexField<T> a)
	{
		return *this = *this - a;
	}
	inline ComplexField<T> operator *= (ComplexField<T> a)
	{
		return *this = a * *this;
	}
	inline ComplexField<T> operator *= (T a)
	{
		return *this = a * *this;
	}
	inline ComplexField<T> operator /= (T a)
	{
		return *this = *this / a;
	}
	inline ComplexField<T> operator /= (ComplexField<T> a)
	{
		return *this = *this / a;
	}
};

template <typename T>
static constexpr bool operator == (ComplexField<T> a, ComplexField<T> b)
{
	return a.real() == b.real() && a.imag() == b.imag();
}

template <typename T>
static constexpr bool operator != (ComplexField<T> a, ComplexField<T> b)
{
	return a.real() != b.real() || a.imag() != b.imag();
}

template <typename T>
static constexpr ComplexField<T> operator + (ComplexField<T> a, ComplexField<T> b)
{
	return ComplexField<T>(a.real() + b.real(), a.imag() + b.imag());
}

template <typename T>
static constexpr ComplexField<T> operator + (ComplexField<T> a)
{
	return a;
}

template <typename T>
static constexpr ComplexField<T> operator - (ComplexField<T> a, ComplexField<T> b)
{
	return ComplexField<T>(a.real() - b.real(), a.imag() - b.imag());
}

template <typename T>
static constexpr ComplexField<T> operator - (ComplexField<T> a)
{
	return ComplexField<T>(-a.real(), -a.imag());
}

template <typename T>
static constexpr ComplexField<T> operator * (T a, ComplexField<T> b)
{
	return ComplexField<T>(a * b.real(), a * b.imag());
}

template <typename T>
static constexpr ComplexField<T> operator / (ComplexField<T> a, T b)
{
	return ComplexField<T>(a.real() / b, a.imag() / b);
}

template <typename T>
static constexpr ComplexField<T> operator * (ComplexField<T> a, ComplexField<T> b)
{
	return ComplexField<T>(a.real() * b.real() - a.imag() * b.imag(), a.real() * b.imag() + a.imag() * b.real());
}

template <typename T>
static constexpr ComplexField<T> operator / (T a, ComplexField<T> b)
{
	return ComplexField<T>((a * b.real()) / (b.real() * b.real() + b.imag() * b.imag()),
			- (a * b.imag()) / (b.real() * b.real() + b.imag() * b.imag()));
}

template <typename T>
static constexpr ComplexField<T> operator / (ComplexField<T> a, ComplexField<T> b)
{
	return ComplexField<T>((a.real() * b.real() + a.imag() * b.imag()) / (b.real() * b.real() + b.imag() * b.imag()),
			(a.imag() * b.real() - a.real() * b.imag()) / (b.real() * b.real() + b.imag() * b.imag()));
}

template <typename T>
static constexpr ComplexField<T> conj(ComplexField<T> a)
{
	return ComplexField<T>(a.real(), -a.imag());
}

template <typename T>
static constexpr T norm(ComplexField<T> a)
{
	return a.real() * a.real() + a.imag() * a.imag();
}

template <typename T>
static constexpr ComplexField<T> pow(ComplexField<T> a, int m)
{
	ComplexField<T> t(T(1));
	for (;m; m >>= 1, a *= a)
		if (m & 1)
			t *= a;
	return t;
}

}

