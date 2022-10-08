#pragma once

#include <vector>

#include "physics/collision.hpp"

namespace game
{
/**
* \brief Generic class for all solvers.
*/
class Solver
{
public:
	/**
	 * \brief Solves the provided collisions.
	 * \param collisions Collisions to solve.
	 * \param deltaTime Time elapsed since the last frame.
	 */
	virtual void Solve(const std::vector<Collision>& collisions, float deltaTime) = 0;
};

/**
* \brief Solver with impulse and friction.
*/
class ImpulseSolver final : public Solver
{
public:
	void Solve(const std::vector<Collision>& collisions, float deltaTime) override;
};

/**
* \brief A solver to smooth out collision with collider that are in a tower placement.
*/
class SmoothPositionSolver : public Solver
{
public:
    void Solve(const std::vector<Collision>& collisions, float deltaTime) override;
};
}
