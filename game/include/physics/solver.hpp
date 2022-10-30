#pragma once

#include <vector>

#include "physics/collision.hpp"
#include "physics/rigidbody.hpp"

namespace game
{
/**
* \brief Generic class for all solvers.
*/
class Solver
{
public:
	Solver(core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager);

	virtual ~Solver() = default;
	Solver(const Solver& other) = default;
	Solver(Solver&& other) = default;
	Solver& operator=(const Solver& other) = delete;
	Solver& operator=(Solver&& other) = delete;

	/**
	 * \brief Solves the provided collisions.
	 * \param collisions Collisions to solve.
	 * \param deltaTime Time elapsed since the last frame.
	 */
	virtual void Solve(const std::vector<Collision>& collisions, float deltaTime) = 0;

protected:
	core::EntityManager& _entityManager;
	RigidbodyManager& _rigidbodyManager;
};

/**
* \brief Solver with impulse and friction.
*/
class ImpulseSolver final : public Solver
{
public:
	ImpulseSolver(core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager)
		: Solver(entityManager, rigidbodyManager)
	{
	}

	void Solve(const std::vector<Collision>& collisions, float deltaTime) override;
};

/**
* \brief A solver to smooth out collision with collider that are in a tower placement.
*/
class SmoothPositionSolver final : public Solver
{
public:
	SmoothPositionSolver(core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager)
		: Solver(entityManager, rigidbodyManager)
	{
	}

	void Solve(const std::vector<Collision>& collisions, float deltaTime) override;
};
}
