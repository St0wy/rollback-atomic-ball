#include "physics/impulse_solver.hpp"

#include "physics/collision.hpp"
#include "physics/rigidbody.hpp"

namespace game
{
void ImpulseSolver::Solve(const std::vector<Collision>& collisions, float)
{
	for (const auto& [bodyA, bodyB, manifold] : collisions)
	{

		core::Vec2f aVel = bodyA ? bodyA->Velocity() : core::Vec2f::Zero();
		core::Vec2f bVel = bodyB ? bodyB->Velocity() : core::Vec2f::Zero();
		core::Vec2f relativeVelocity = bVel - aVel;

		// Calculate relative velocity in terms of the normal direction
		float velocityAlongNormal = relativeVelocity.Dot(manifold.normal);

		// Do not resolve if velocities are separating
		if (velocityAlongNormal >= 0) continue;

		const float aInvMass = bodyA ? bodyA->InvMass() : 1.0f;
		const float bInvMass = bodyB ? bodyB->InvMass() : 1.0f;

		// Impulse

		const float e = std::min(bodyA ? bodyA->Restitution() : 1.0f, bodyB ? bodyB->Restitution() : 1.0f);
		const float j = -(1.0f + e) * velocityAlongNormal / (aInvMass + bInvMass);

		const core::Vec2f impulse = j * manifold.normal;

		if (bodyA ? bodyA->IsKinematic() : false)
		{
			aVel -= impulse * aInvMass;
		}

		if (bodyB ? bodyB->IsKinematic() : false)
		{
			bVel += impulse * bInvMass;
		}

		// Friction
		relativeVelocity = bVel - aVel;
		velocityAlongNormal = relativeVelocity.Dot(manifold.normal);

		const core::Vec2f tangent = core::Vec2f::Normalize(relativeVelocity - velocityAlongNormal * manifold.normal);

		const float fVel = relativeVelocity.Dot(tangent);

		const float aSf = bodyA ? bodyA->StaticFriction() : 0.0f;
		const float bSf = bodyB ? bodyB->StaticFriction() : 0.0f;
		const float aDf = bodyA ? bodyA->DynamicFriction() : 0.0f;
		const float bDf = bodyB ? bodyB->DynamicFriction() : 0.0f;
		float mu = core::Vec2f(aSf, bSf).GetMagnitude();
		const float f = -fVel / (aInvMass + bInvMass);

		core::Vec2f friction;
		if (std::abs(f) < j * mu)
		{
			friction = f * tangent;
		}
		else
		{
			mu = core::Vec2f(aDf, bDf).GetMagnitude();
			friction = -j * tangent * mu;
		}

		if (bodyA ? bodyA->IsKinematic() : false)
		{
			bodyA->SetVelocity(aVel - friction * aInvMass);
		}

		if (bodyB ? bodyB->IsKinematic() : false)
		{
			bodyB->SetVelocity(bVel + friction * bInvMass);
		}
	}
}
}
