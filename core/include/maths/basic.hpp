#pragma once

#include <random>

namespace core
{
/**
 * \brief Abs is a constexpr function that returns the absolute value.
 * \param v is the value that will be transform to absolute value
 * \return the absolute value of v
 */
constexpr float Abs(const float v)
{
	return v < 0.0f ? -v : v;
}

constexpr bool Equal(const float a, const float b, const float epsilon = 0.0000001f)
{
	return Abs(a - b) < epsilon;
}

/**
 * \brief Lerp is a function calculating the linear interpolation between two points.
 * \param start is the lower value of the interpolation.
 * \param end is the upper value of the interpolation.
 * \param t is the ratio of the interpolation.
 * t = 0, return start
 * t = 1, return end
 * t > 1 or t < 0, return an extrapolation of the linear function
 * \return the linear interpolation result
 */
constexpr float Lerp(const float start, const float end, const float t)
{
	return start + (end - start) * t;
}

/**
 * \brief Clamp is a math function that clamps the input value between an upper and lower value.
 * \tparam T is the inputs and outputs type
 * \param value is the input value that will be clamped
 * \param lower is the lower bound of the clamp
 * \param upper is the upper bound of the clamp
 * \return the clamped value between lower and upper
 */
template <typename T>
constexpr float Clamp(T value, T lower, T upper)
{
	return value < lower ? lower : (value > upper ? upper : value);
}

template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> RandomRange(T start, T end)
{
	// Will be used to obtain a seed for the random number engine
	static std::random_device rd;
	// Standard mersenne_twister_engine seeded with rd()
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<T> dis(start, end);
	return dis(gen);
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> RandomRange(T start, T end)
{
	// Will be used to obtain a seed for the random number engine
	static std::random_device rd;
	// Standard mersenne_twister_engine seeded with rd()
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<T> dis(start, end);
	return dis(gen);
}

inline bool RandomBool()
{
	return RandomRange<float>(0.0f, 1.0f) > 0.5f;
}

template <typename T>
T constexpr SqrtNewtonRaphson(T x, T current, T previous)
{
	return current == previous  // NOLINT(clang-diagnostic-float-equal)
		? current
		: SqrtNewtonRaphson<T>(x, (current + x / current) * T(0.5), current);
}

/**
* \brief Sqrt is a constexpr version of the square root
* \tparam T is the input and output value type
* \param x is the input value
* \return
*   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
*   - Otherwise, returns NaN
*/
template <typename T>
T constexpr Sqrt(T x)
{
	return x >= 0 && x < std::numeric_limits<T>::infinity()
		? SqrtNewtonRaphson<T>(x, x, 0)
		: std::numeric_limits<T>::quiet_NaN();
}

template <typename T>
T constexpr Sign(T val)
{
	return static_cast<T>(T{ 0 } < val) - (val < T{ 0 });
}
}
