/**
 * @file Collision.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Collision struct.
 * @version 1.0
 * @date 05.07.2022
 *
 * @copyright SAE (c) 2022
 *
 */
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
