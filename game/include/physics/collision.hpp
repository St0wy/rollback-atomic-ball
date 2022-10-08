#pragma once

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
    Rigidbody* bodyA{};

    /**
     * \brief Body B of the collision.
     */
    Rigidbody* bodyB{};

    /**
     * \brief Manifold of the collision.
     */
    Manifold manifold;
};
}
