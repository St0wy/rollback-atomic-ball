#pragma once

struct Walls
{
	core::Entity left;
	core::Entity right;
	core::Entity middle;
	core::Entity bottom;
	core::Entity top;

	constexpr std::array<core::Entity, 5> GetArray() const
	{
		return {
			left,
			right,
			middle,
			bottom,
			top
		};
	}
};