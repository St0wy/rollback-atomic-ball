/**
 * \file angle.h
 */
#pragma once

#include <cmath>
#include <numbers>

namespace core
{
inline constexpr static float PI = std::numbers::pi_v<float>;
class Degree;
/**
 * \brief Radian is an utility class that describes radian angles (0 to 2PI).
 * It can be easily converted to Degree with conversion constructor.
 * It can be used with trigonometric functions (Sin, Cos, Tan)
 */
class Radian
{
public:
	constexpr Radian() = default;
	// ReSharper disable once CppNonExplicitConvertingConstructor
	constexpr Radian(const float value)
		: _value(value) {}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	/**
	 * \brief Conversion constructor that implicitly converts Degree to Radian
	 * \param angle is the degree angle to be converted to Radian
	 */
	constexpr Radian(const Degree& angle);
	[[nodiscard]] constexpr float Value() const { return _value; }

	constexpr Radian operator+(const Radian angle) const { return {_value + angle.Value()}; }

	constexpr Radian& operator+=(const Radian angle)
	{
		_value += angle.Value();
		return *this;
	}

	constexpr Radian operator-(const Radian angle) const { return {_value - angle.Value()}; }

	constexpr Radian& operator-=(const Radian angle)
	{
		_value -= angle.Value();
		return *this;
	}

	constexpr Radian operator*(const float value) const { return {_value * value}; }
	constexpr Radian operator/(const float value) const { return {_value / value}; }
	constexpr Radian operator-() const { return {-_value}; }
private:
	float _value = 0.0f;
};

/**
 * \brief Degree is an utility class that describes degree angles (0 to 360).
 * It can be easily converted to Radian with conversion constructor.
 * It can be used with trigonometric functions (Sin, Cos, Tan)
 */
class Degree
{
public:
	constexpr Degree() = default;

	// ReSharper disable once CppNonExplicitConvertingConstructor
	constexpr Degree(const float value)
		: _value(value) {}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	/**
	 * \brief Conversion constructor that implicitly converts Radian to Degree
	 * \param angle is the radian angle to be converted to Degree
	 */
	constexpr Degree(const Radian& angle)
		: _value(angle.Value() / PI * 180.0f) {}

	[[nodiscard]] constexpr float Value() const { return _value; }
	constexpr Degree operator+(const Degree angle) const { return {_value + angle.Value()}; }

	constexpr Degree& operator+=(const Degree angle)
	{
		_value += angle.Value();
		return *this;
	}

	constexpr Degree operator-(const Degree angle) const { return {_value - angle.Value()}; }

	constexpr Degree& operator-=(const Degree angle)
	{
		_value -= angle.Value();
		return *this;
	}

	constexpr Degree operator*(const float value) const { return {_value * value}; }
	constexpr Degree operator/(const float value) const { return {_value / value}; }
	constexpr Degree operator-() const { return {-_value}; }
private:
	float _value = 0.0f;
};

constexpr Degree operator*(const float value, const Degree angle) { return angle.Value() * value; }


constexpr Radian::Radian(const Degree& angle)
{
	_value = angle.Value() / 180.0f * PI;
}

/**
 * \brief Sin is a function that calculates the sinus of a given angle.
 * \param angle is the given angle
 * \return the result of the sinus of angle
 */
inline float Sin(const Radian angle)
{
	return std::sin(angle.Value());
}

/**
 * \brief Cos is a function that calculates the cosinus of a given angle.
 * \param angle is the given angle
 * \return the result of the cosinus of angle
 */
inline float Cos(const Radian angle)
{
	return std::cos(angle.Value());
}

/**
 * \brief Tan is a function that calculates the tangent of a given angle.
 * \param angle is the given angle
 * \return the result of the tangent of angle
 */
inline float Tan(const Radian angle)
{
	return std::tan(angle.Value());
}

/**
 * \brief Asin is a function that calculates the angle of a given ratio.
 * \param ratio is the given ratio between the opponent and hypotenuse
 * \return the result of the a-sinus function
 */
inline Radian Asin(const float ratio)
{
	return {std::asin(ratio)};
}

/**
 * \brief Acos is a function that calculates the angle of a given ratio.
 * \param ratio is the given ratio between the adjacent and hypotenuse
 * \return the result of the a-cosinus function
 */
inline Radian Acos(const float ratio)
{
	return {std::acos(ratio)};
}

/**
 * \brief Atan is a function that calculates the angle of a given ratio.
 * \param ratio is the given ratio between the adjacent and hypotenuse
 * \return the result of the atan function
 */
inline Radian Atan(const float ratio)
{
	return {std::atan(ratio)};
}

/**
 * \brief Atan2 is a function that calculates the angle of a given ratio between two parameters.
 * \param y is the upper value of the ratio
 * \param x is the lower value of the ratio
 * \return the result of the atan function
 */
inline Radian Atan2(const float y, const float x)
{
	return {std::atan2(y, x)};
}
}
