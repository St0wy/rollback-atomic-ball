#include "physics/solver.hpp"

#include "engine/component.hpp"

#include "physics/collision.hpp"
#include "physics/rigidbody.hpp"

namespace game
{
Solver::Solver(core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager)
	: _entityManager(entityManager), _rigidbodyManager(rigidbodyManager)
{
}

void ImpulseSolver::Solve(const std::vector<Collision>& collisions, float)
{
	for (const auto& [entityA, entityB, manifold] : collisions)
	{
		const bool isRigidbodyA = _entityManager.HasComponent(entityA,
		                                                      static_cast<core::EntityMask>(
			                                                      core::ComponentType::Rigidbody));
		const bool isRigidbodyB = _entityManager.HasComponent(entityB,
		                                                      static_cast<core::EntityMask>(
			                                                      core::ComponentType::Rigidbody));

		if (!isRigidbodyA || !isRigidbodyB) continue;

		Rigidbody& bodyA = _rigidbodyManager.GetComponent(entityA);
		Rigidbody& bodyB = _rigidbodyManager.GetComponent(entityB);

		// ReSharper disable CppCStyleCast
		Rigidbody* aBody = bodyA.HasCollisions() ? &bodyA : nullptr;
		Rigidbody* bBody = bodyB.HasCollisions() ? &bodyB : nullptr;
		// ReSharper restore CppCStyleCast

		core::Vec2f aVel = aBody ? aBody->Velocity() : core::Vec2f::Zero();
		core::Vec2f bVel = bBody ? bBody->Velocity() : core::Vec2f::Zero();
		core::Vec2f relativeVelocity = bVel - aVel;

		// Calculate relative velocity in terms of the normal direction
		float velocityAlongNormal = relativeVelocity.Dot(manifold.normal);

		// Do not resolve if velocities are separating
		if (velocityAlongNormal >= 0) continue;

		const float aInvMass = aBody ? aBody->InvMass() : 1.0f;
		const float bInvMass = bBody ? bBody->InvMass() : 1.0f;

		// Impulse

		const float e = std::min(aBody ? aBody->Restitution() : 1.0f, bBody ? bBody->Restitution() : 1.0f);
		const float j = -(1.0f + e) * velocityAlongNormal / (aInvMass + bInvMass);

		const core::Vec2f impulse = j * manifold.normal;

		if (aBody ? !aBody->IsKinematic() : false)
		{
			aVel -= impulse * aInvMass;
		}

		if (bBody ? !bBody->IsKinematic() : false)
		{
			bVel += impulse * bInvMass;
		}

		// Friction
		relativeVelocity = bVel - aVel;
		velocityAlongNormal = relativeVelocity.Dot(manifold.normal);

		const core::Vec2f tangent = core::Vec2f::Normalize(relativeVelocity - velocityAlongNormal * manifold.normal);

		const float fVel = relativeVelocity.Dot(tangent);

		const float aSf = aBody ? aBody->StaticFriction() : 0.0f;
		const float bSf = bBody ? bBody->StaticFriction() : 0.0f;
		const float aDf = aBody ? aBody->DynamicFriction() : 0.0f;
		const float bDf = bBody ? bBody->DynamicFriction() : 0.0f;
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

		if (aBody ? !aBody->IsKinematic() : false)
		{
			if (friction.IsNaN())
			{
				aBody->SetVelocity(aVel);
			}
			else
			{
				aBody->SetVelocity(aVel - friction * aInvMass);
			}
		}

		if (bBody ? !bBody->IsKinematic() : false)
		{
			if (friction.IsNaN())
			{
				bBody->SetVelocity(bVel);
			}
			else
			{
				bBody->SetVelocity(bVel + friction * bInvMass);
			}
		}
	}
}

void SmoothPositionSolver::Solve(const std::vector<Collision>& collisions, float)
{
	for (const auto& [entityA, entityB, points] : collisions)
	{
		const bool isRigidbodyA = _entityManager.HasComponent(entityA,
		                                                      static_cast<core::EntityMask>(
			                                                      core::ComponentType::Rigidbody));
		const bool isRigidbodyB = _entityManager.HasComponent(entityB,
		                                                      static_cast<core::EntityMask>(
			                                                      core::ComponentType::Rigidbody));

		if (!isRigidbodyA || !isRigidbodyB) continue;

		Rigidbody& bodyA = _rigidbodyManager.GetComponent(entityA);
		Rigidbody& bodyB = _rigidbodyManager.GetComponent(entityB);

		// ReSharper disable CppCStyleCast
		Rigidbody* aBody = bodyA.HasCollisions() ? &bodyA : nullptr;
		Rigidbody* bBody = bodyB.HasCollisions() ? &bodyB : nullptr;
		// ReSharper restore CppCStyleCast

		const float aInvMass = aBody ? aBody->InvMass() : 0.0f;
		const float bInvMass = bBody ? bBody->InvMass() : 0.0f;

		core::Vec2f resolution = points.b - points.a;

		constexpr float slop = 0.05f;
		constexpr float percent = 0.8f;

		const core::Vec2f correction = points.normal * percent
			* std::max(resolution.GetMagnitude() - slop, 0.0f)
			/ (aInvMass + bInvMass);

		if (aBody ? !aBody->IsKinematic() : false)
		{
			const core::Vec2f deltaA = aInvMass * correction;
			aBody->Trans().position -= deltaA;
		}

		if (bBody ? !bBody->IsKinematic() : false)
		{
			const core::Vec2f deltaB = bInvMass * correction;
			bBody->Trans().position += deltaB;
		}
	}
}
}
