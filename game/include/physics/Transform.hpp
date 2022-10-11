#pragma once

#include "maths/angle.hpp"
#include "maths/vec2.hpp"

namespace game
{
/**
* \brief Transform of a physical object.
* Warning : Don't mix it up with the core::Transform.
* Contains position, scale and rotation.
*/
struct Transform
{
public:
	/**
	 * \brief The position of this object.
	 */
	core::Vec2f position{};

	/**
	 * \brief The scale of this object.
	 */
	core::Vec2f scale{ 1, 1 };

	/**
	 * \brief The rotation of this object.
	 */
	core::Radian rotation{};
};
}
