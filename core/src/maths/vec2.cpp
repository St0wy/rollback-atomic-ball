#include <cmath>

#include <maths/vec2.hpp>

namespace core
{
Vec2f::Vec2f(const sf::Vector2f v)
	: x(v.x), y(v.y)
{
}

Vec2f Vec2f::FromAngle(const Radian angle)
{
	return {Sin(angle), Cos(angle)};
}

float Vec2f::Dot(const Vec2f other) const
{
	return Dot(*this, other);
}

Vec2f Vec2f::NewMagnitude(const float newMagnitude) const
{
	return (*this * newMagnitude) / GetMagnitude();
}

void Vec2f::RotateAround(const Vec2f& center, const float angle)
{
	const Vec2f relative = (*this) - center;
	const float ca = std::cos(angle);
	const float sa = std::sin(angle);
	const auto rotated = Vec2f(ca * relative.x - sa * relative.y, sa * relative.x + ca * relative.y);
	(*this) = rotated + center;
}

bool Vec2f::IsNaN() const
{
	return std::isnan(x) || std::isnan(y);
}

Vec2f Vec2f::operator+(const Vec2f v) const
{
	return {x + v.x, y + v.y};
}

Vec2f& Vec2f::operator+=(const Vec2f v)
{
	x += v.x;
	y += v.y;
	return *this;
}

Vec2f Vec2f::operator-(const Vec2f v) const
{
	return {x - v.x, y - v.y};
}

Vec2f& Vec2f::operator-=(const Vec2f v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

Vec2f Vec2f::operator*(const float f) const
{
	return {x * f, y * f};
}

Vec2f Vec2f::operator/(const float f) const
{
	return {x / f, y / f};
}

Vec2f Vec2f::operator/=(const float scalar)
{
	this->x /= scalar;
	this->y /= scalar;
	return *this;
}

Vec2f Vec2f::operator*=(const float scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	return *this;
}

Vec2f Vec2f::operator-() const
{
	return {-x, -y};
}

bool Vec2f::operator==(const Vec2f other) const
{
	return x == other.x && y == other.y; // NOLINT(clang-diagnostic-float-equal)
}

Vec2f operator*(const float f, const Vec2f v)
{
	return v * f;
}

float Vec2f::GetMagnitude() const
{
	return std::sqrt(GetSqrMagnitude());
}

void Vec2f::Normalize()
{
	const auto magnitude = GetMagnitude();
	x /= magnitude;
	y /= magnitude;
}

Vec2f Vec2f::GetNormalized() const
{
	const auto magnitude = GetMagnitude();
	return (*this) / magnitude;
}

float Vec2f::GetSqrMagnitude() const
{
	return x * x + y * y;
}

Vec2f Vec2f::Rotate(const Degree rotation) const
{
	const auto cs = Cos(rotation);
	const auto sn = Sin(rotation);

	Vec2f v;
	v.x = x * cs - y * sn;
	v.y = x * sn + y * cs;
	return v;
}

float Vec2f::Distance(const Vec2f& other) const
{
	return std::sqrt((this->x - other.x) * (this->x - other.x) +
		(this->y - other.y) * (this->y - other.y));
}

Radian Vec2f::Angle(const Vec2f& other) const
{
	return std::acos(this->Dot(other) / GetMagnitude() * other.GetMagnitude());
}

Radian Vec2f::GetAngle() const
{
	const float angle = std::atan2(y, x);
	return angle;
}

float Vec2f::Major() const
{
	if (x >= y)
	{
		return x;
	}
	return y;
}

Vec2f Vec2f::PositivePerpendicular() const
{
	return {-y, x};
}

Vec2f Vec2f::NegativePerpendicular() const
{
	return {y, -x};
}

float Vec2f::Dot(const Vec2f a, const Vec2f b)
{
	return a.x * b.x + a.y * b.y;
}

Vec2f Vec2f::Lerp(const Vec2f a, const Vec2f b, const float t)
{
	return a + (b - a) * t;
}

Vec2f Vec2f::Normalize(const Vec2f v)
{
	return v.GetNormalized();
}
}
