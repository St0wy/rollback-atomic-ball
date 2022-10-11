#pragma once

#include "engine/entity.hpp"

#include "physics/rigidbody.hpp"

namespace game
{
/**
* \brief Struct representing a collision.
*/
struct Collision
{
    /**
     * \brief Body A of the collision.
     */
    core::Entity bodyA{};

    /**
     * \brief Body B of the collision.
     */
    core::Entity bodyB{};

    /**
     * \brief Manifold of the collision.
     */
    Manifold manifold;
};
}
