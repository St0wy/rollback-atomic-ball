#include "physics/smooth_position_solver.hpp"

#include <vector>

#include "physics/collision.hpp"
#include "physics/rigidbody.hpp"

namespace game
{
void SmoothPositionSolver::Solve(const std::vector<Collision>& collisions, float)
{
    for (const auto& [bodyA, bodyB, points] : collisions)
    {                                    
        const float aInvMass = bodyA ? bodyA->InvMass() : 0.0f;
        const float bInvMass = bodyB ? bodyB->InvMass() : 0.0f;

        core::Vec2f resolution = points.b - points.a;

        constexpr float slop = 0.05f;
        constexpr float percent = 0.8f;

        const core::Vec2f correction = points.normal * percent
            * std::max(resolution.GetMagnitude() - slop, 0.0f)
            / (aInvMass + bInvMass);

        if (bodyA ? bodyA->IsKinematic() : false)
        {
            const core::Vec2f deltaA = aInvMass * correction;
            bodyA->Trans()->position -= deltaA;
        }

        if (bodyB ? bodyB->IsKinematic() : false)
        {
            const core::Vec2f deltaB = bInvMass * correction;
            bodyB->Trans()->position += deltaB;
        }
    }
}
}
