#pragma once

#include <cstdint>

#include <SFML/Graphics/Color.hpp>

namespace core
{
/**
 * \brief Color is a struct defining an RGBA color with 4 bytes
 */
struct Color
{
	std::uint8_t r = 0u;
	std::uint8_t g = 0u;
	std::uint8_t b = 0u;
	std::uint8_t a = 0u;
	constexpr Color() = default;

	constexpr Color(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue,
					const std::uint8_t alpha = 255u)
		: r(red), g(green), b(blue), a(alpha) {}

	// ReSharper disable once CppNonExplicitConversionOperator
	operator sf::Color() const { return {r, g, b, a}; }

	static constexpr Color Red() { return {255u, 0u, 0u, 255u}; }
	static constexpr Color Green() { return {0u, 255u, 0u, 255u}; }
	static constexpr Color Blue() { return {0u, 0u, 255u, 255u}; }
	static constexpr Color Yellow() { return {255u, 255u, 0u, 255u}; }
	static constexpr Color Black() { return {0u, 0u, 0u, 255u}; }
	static constexpr Color White() { return {255u, 255u, 255u, 255u}; }
	static constexpr Color Magenta() { return {255u, 0u, 255u, 255u}; }
	static constexpr Color Cyan() { return {0u, 255u, 255u, 255u}; }
	static constexpr Color Transparent() { return {0u, 0u, 0u, 0u}; }
};
} // namespace core
