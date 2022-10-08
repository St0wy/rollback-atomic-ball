#pragma once

#include <unordered_map>

#include "rigidbody.hpp"
#include "solver.hpp"

#include "physics/broad_phase_grid.hpp"

namespace game
{
/**
* \brief A world with dynamics in it.
*/
class PhysicalWorld
{
public:
    PhysicalWorld();
    PhysicalWorld(std::unordered_map<std::uint64_t, Rigidbody*> bodies, std::vector<Solver*> solvers);

    /**
     * \brief Adds a collision body to the world.
     * \param body Body to add.
     */
    void AddRigidbody(Rigidbody* body);

    /**
     * \brief Removes a collision body to the world.
     * \param body Body to remove.
     */
    void RemoveRigidbody(const Rigidbody* body);

    /**
     * \brief Adds a solver to the world.
     * \param solver Solver to add.
     */
    void AddSolver(Solver* solver);

    /**
     * \brief Removes the solver from the world.
     * \param solver Solver to remove.
     */
    void RemoveSolver(Solver* solver);

    /**
     * \brief Solves the collisions with the provided solvers.
     * \param collisions Collisions to solve.
     * \param deltaTime Time elapsed since the last frame.
     */
    void SolveCollisions(const std::vector<Collision>& collisions, float deltaTime) const;

    /**
     * \brief Resolves all the collisions that happened in this world.
     * \param deltaTime Time elapsed since the last frame.
     */
    void ResolveCollisions(float deltaTime);

    /**
     * \brief Applies the gravity to all the rigidbody.
     */
    void ApplyGravity() const;

    /**
     * \brief Moves all the rigidbodies.
     * \param deltaTime Time elapsed since the last frame.
     */
    void MoveBodies(float deltaTime) const;

    /**
     * \brief Steps the world.
     * \param deltaTime Time elapsed since the last frame.
     */
    void Step(float deltaTime);

    /**
     * \brief Sets the gravity in the world.
     * \param gravity Gravity to set.
     */
    void SetWorldGravity(core::Vec2f gravity);

private:
    std::unordered_map<std::uint64_t, Rigidbody*> _bodies;
    std::vector<Solver*> _solvers;
    BroadPhaseGrid _grid;

    core::Vec2f _gravity = {0, -9.81f};
};
}
