#pragma once

namespace game
{
/**
* \brief A transform of an object.
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
	core::Vec2f scale{1, 1};

    /**
     * \brief The rotation of this object.
     */
    float rotation{};
};
}
