#pragma once

#include <maths/angle.h>

#include <SFML/System/Vector2.hpp>

namespace core
{
// ReSharper disable once CppInconsistentNaming
/**
 * \brief Vec2f is a utility class that represents a mathematical 2d vector.
 */
struct Vec2f
{
	float x = 0.0f, y = 0.0f;

	constexpr Vec2f() = default;

	constexpr Vec2f(const float newX, const float newY)
		: x(newX), y(newY) {}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	Vec2f(sf::Vector2f v);


	[[nodiscard]] float GetMagnitude() const;
	void Normalize();
	[[nodiscard]] Vec2f GetNormalized() const;
	[[nodiscard]] float GetSqrMagnitude() const;
	[[nodiscard]] Vec2f Rotate(Degree rotation) const;
	static float Dot(Vec2f a, Vec2f b);
	static Vec2f Lerp(Vec2f a, Vec2f b, float t);

	// ReSharper disable once CppNonExplicitConversionOperator
	[[nodiscard]] operator sf::Vector2f() const { return {x, y}; }

	Vec2f operator+(Vec2f v) const;
	Vec2f& operator+=(Vec2f v);
	Vec2f operator-(Vec2f v) const;
	Vec2f& operator-=(Vec2f v);
	Vec2f operator*(float f) const;
	Vec2f operator/(float f) const;

	static constexpr Vec2f Zero() { return {}; }
	static constexpr Vec2f One() { return {1, 1}; }
	static constexpr Vec2f Up() { return {0, 1}; }
	static constexpr Vec2f Down() { return {0, -1}; }
	static constexpr Vec2f Left() { return {-1, 0}; }
	static constexpr Vec2f Right() { return {1, 0}; }
};

Vec2f operator*(float f, Vec2f v);
}
