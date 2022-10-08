#include "physics/physical_world.hpp"

#include <ranges>

namespace game
{
PhysicalWorld::PhysicalWorld() :PhysicalWorld({}, {}) {}

PhysicalWorld::PhysicalWorld(
	std::unordered_map<std::uint64_t, Rigidbody*> bodies,
	std::vector<Solver*> solvers
)
	: _bodies(std::move(bodies)),
	_solvers(std::move(solvers)),
	_grid(-100, 100, -100, 100, 10)
{}

void PhysicalWorld::AddRigidbody(Rigidbody* body)
{
	if (!body) return;

	if (body->TakesGravity())
	{
		body->SetGravityForce(_gravity);
	}

	_bodies.insert({ body->id, body });
}

void PhysicalWorld::RemoveRigidbody(const Rigidbody* body)
{
	if (!body) return;

	_bodies.erase(body->id);
}

void PhysicalWorld::AddSolver(Solver* solver)
{
	_solvers.push_back(solver);
}

void PhysicalWorld::RemoveSolver(Solver* solver)
{
	if (!solver) return;

	const auto itr = std::ranges::find(_solvers, solver);

	if (itr == _solvers.end()) return;

	_solvers.erase(itr);
}

void PhysicalWorld::SolveCollisions(const std::vector<Collision>& collisions, const float deltaTime) const
{
	for (Solver* solver : _solvers)
	{
		solver->Solve(collisions, deltaTime);
	}
}

void PhysicalWorld::ResolveCollisions(const float deltaTime)
{
	// Vector for the collisions that have been detected
	std::vector<Collision> collisions;

	// Vector for the collisions that have been caused by trigger colliders
	std::vector<Collision> triggers;

	// Update the grid and find the object that can collide together
	_grid.Update(_bodies);
	const auto collisionPairs = _grid.GetCollisionPairs();

	for (auto& [firstId, secondId] : collisionPairs)
	{
		Rigidbody* a = _bodies[firstId];
		Rigidbody* b = _bodies[secondId];

		if (!a->Col() || !b->Col()) continue;

		const Manifold manifold = a->Col()->TestCollision(
			a->Trans(),
			b->Col(),
			b->Trans());

		if (!manifold.hasCollision) continue;

		if (a->IsTrigger() || b->IsTrigger())
		{
			triggers.emplace_back(a, b, manifold);
		}
		else
		{
			collisions.emplace_back(a, b, manifold);
		}
	}

	SolveCollisions(collisions, deltaTime);

	SendCollisionCallbacks(collisions, deltaTime);
	SendCollisionCallbacks(triggers, deltaTime);
}

void PhysicalWorld::ApplyGravity() const
{
	for (const auto body : _bodies | std::views::values)
	{
		// ReSharper disable once CppCStyleCast
		const auto rigidbody = (Rigidbody*)body;
		const core::Vec2f force = rigidbody->GravityForce() * rigidbody->Mass();
		rigidbody->ApplyForce(force);
	}
}

void PhysicalWorld::MoveBodies(const float deltaTime) const
{
	for (const auto body : _bodies | std::views::values)
	{
		// ReSharper disable once CppCStyleCast
		const auto rigidbody = (Rigidbody*)body;

		const core::Vec2f vel = rigidbody->Velocity() + rigidbody->Force() * rigidbody->InvMass() * deltaTime;
		rigidbody->SetVelocity(vel);

		//rigidbody->UpdateLastTransform();

		core::Vec2f pos = rigidbody->Position() + rigidbody->Velocity() * deltaTime;
		rigidbody->SetPosition(pos);

		rigidbody->SetForce({ 0, 0 });
	}
}

void PhysicalWorld::Step(const float deltaTime)
{
	ApplyGravity();
	ResolveCollisions(deltaTime);
	MoveBodies(deltaTime);
}

void PhysicalWorld::SetWorldGravity(const core::Vec2f gravity)
{
	_gravity = gravity;
}
}
