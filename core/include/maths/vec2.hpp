#pragma once

#include <maths/angle.hpp>

#include <SFML/System/Vector2.hpp>

namespace core
{
// ReSharper disable once CppInconsistentNaming
/**
 * \brief Vec2f is a utility class that represents a mathematical 2d vector.
 */
struct Vec2f
{
	float x = 0.0f;
	float y = 0.0f;

	constexpr Vec2f() = default;

	constexpr Vec2f(const float newX, const float newY)
		: x(newX), y(newY)
	{}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	Vec2f(sf::Vector2f v);

	static Vec2f FromAngle(Radian angle);
	static float Dot(Vec2f a, Vec2f b);
	static Vec2f Lerp(Vec2f a, Vec2f b, float t);
	static Vec2f Normalize(Vec2f v);

	[[nodiscard]] float GetMagnitude() const;
	void Normalize();
	[[nodiscard]] Vec2f GetNormalized() const;
	[[nodiscard]] float GetSqrMagnitude() const;
	[[nodiscard]] Vec2f Rotate(Degree rotation) const;

	/**
	 * \brief Computes the distance between this and other.
	 * \param other The other vector.
	 * \return The distance between this and other.
	 */
	[[nodiscard]] float Distance(const Vec2f& other) const;

	/**
	 * \brief Computes the angle between this and other.
	 * \param other The other vector.
	 * \return The angle between this and other.
	 */
	[[nodiscard]] Radian Angle(const Vec2f& other) const;

	[[nodiscard]] Radian GetAngle() const;

	/**
	 * \brief Gets the biggest component of this vector.
	 * \return The biggest component of this vector.
	 */
	[[nodiscard]] float Major() const;

	/**
	 * \brief Computes the perpendicular vector in +90 degrees.
	 * \return The perpendicular vector in the positive direction.
	 */
	[[nodiscard]] Vec2f PositivePerpendicular() const;

	/**
	 * \brief Computes the perpendicular vector in -90 degrees.
	 * \return The perpendicular vector in the negative direction.
	 */
	[[nodiscard]] Vec2f NegativePerpendicular() const;

	[[nodiscard]] float Dot(Vec2f other) const;

	/**
	 * \brief Sets the magnitude of this vector.
	 * \param newMagnitude The new magnitude.
	 */
	[[nodiscard]] Vec2f NewMagnitude(float newMagnitude) const;

	/**
	 * \brief Rotates this vector around the provided axis.
	 * \param center The axis to rotate around.
	 * \param angle The angle by which this vector should be rotated.
	 */
	void RotateAround(const Vec2f& center, float angle);

	[[nodiscard]] bool IsNaN() const;

	// ReSharper disable once CppNonExplicitConversionOperator
	[[nodiscard]] operator sf::Vector2f() const { return { x, y }; }

	Vec2f operator+(Vec2f v) const;
	Vec2f operator-(Vec2f v) const;
	Vec2f& operator+=(Vec2f v);
	Vec2f& operator-=(Vec2f v);
	Vec2f operator*(float f) const;
	Vec2f operator/(float f) const;
	Vec2f operator/=(float scalar);
	Vec2f operator*=(float scalar);
	Vec2f operator-() const;
	bool operator==(Vec2f other) const;

	static constexpr Vec2f Zero() { return {}; }
	static constexpr Vec2f One() { return { 1, 1 }; }
	static constexpr Vec2f Up() { return { 0, 1 }; }
	static constexpr Vec2f Down() { return { 0, -1 }; }
	static constexpr Vec2f Left() { return { -1, 0 }; }
	static constexpr Vec2f Right() { return { 1, 0 }; }
};

Vec2f operator*(float f, Vec2f v);
}
