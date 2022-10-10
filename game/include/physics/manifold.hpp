#pragma once

#include "maths/vec2.h"

namespace game
{
struct Manifold
{
public:
    Manifold(const core::Vec2f& a, const core::Vec2f& b, const core::Vec2f& normal, float depth);
    Manifold(const core::Vec2f& normal, float depth);
    Manifold();

    /**
     * \brief Point a of the manifold.
     */
    core::Vec2f a;

    /**
     * \brief Point b of the manifold.
     */
    core::Vec2f b;

    /**
     * \brief The normal of the manifold.
     * Represents the direction in which the collision should be solved.
     */
	core::Vec2f normal;

    /**
     * \brief The depth of the collision. Can be seen as the magnitude of the normal.
     */
    float depth{};

    /**
     * \brief Boolean indicating whether a collision happened.
     */
    bool hasCollision{};

    /**
     * \brief Gets an empty manifold with no collisions.
     * \return An empty manifold.
     */
    static Manifold Empty()
    {
        return {};
    }

    [[nodiscard]] Manifold Swaped() const;
};
}
