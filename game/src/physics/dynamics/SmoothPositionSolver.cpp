#include "physics/dynamics/SmoothPositionSolver.hpp"

#include <vector>

#include "physics/collision/Collision.hpp"
#include "physics/dynamics/Rigidbody.hpp"

namespace game
{
void SmoothPositionSolver::Solve(const std::vector<Collision>& collisions, float)
{
    for (const auto& [bodyA, bodyB, points] : collisions)
    {
        // ReSharper disable CppCStyleCast
        Rigidbody* aBody = bodyA->IsDynamic() ? (Rigidbody*)bodyA : nullptr;
        Rigidbody* bBody = bodyB->IsDynamic() ? (Rigidbody*)bodyB : nullptr;
        // ReSharper restore CppCStyleCast

        const float aInvMass = aBody ? aBody->InvMass() : 0.0f;
        const float bInvMass = bBody ? bBody->InvMass() : 0.0f;

        core::Vec2f resolution = points.b - points.a;

        constexpr float slop = 0.05f;
        constexpr float percent = 0.8f;

        const core::Vec2f correction = points.normal * percent
            * std::max(resolution.GetMagnitude() - slop, 0.0f)
            / (aInvMass + bInvMass);

        if (aBody ? aBody->IsKinematic() : false)
        {
            const core::Vec2f deltaA = aInvMass * correction;
            aBody->Trans()->position -= deltaA;
        }

        if (bBody ? bBody->IsKinematic() : false)
        {
            const core::Vec2f deltaB = bInvMass * correction;
            bBody->Trans()->position += deltaB;
        }
    }
}
}
