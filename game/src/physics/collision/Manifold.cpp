#include "physics/collision/manifold.hpp"

#include <ostream>

namespace game
{
Manifold::Manifold(const core::Vec2f& a, const core::Vec2f& b, const core::Vec2f& normal, const float depth)
    : a(a), b(b), normal(normal), depth(depth), hasCollision(true) {}

Manifold::Manifold(const core::Vec2f& normal, const float depth)
    : Manifold(core::Vec2f(), core::Vec2f(), normal, depth) {}

Manifold::Manifold()
    : Manifold(core::Vec2f(), 0.0f)
{
    hasCollision = false;
}

Manifold Manifold::Swaped() const
{
    Manifold copy = *this;
    copy.a = b;
    copy.b = a;
    copy.normal = -normal;
    return copy;
}
}
