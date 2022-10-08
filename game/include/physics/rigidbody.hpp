#pragma once

#include "physics/manifold_factory.hpp"

namespace game
{
/**
* \brief A Rigidbody that has dynamics.
*/
struct Rigidbody
{
public:
    Rigidbody();

    std::uint64_t id;

	/**
	 * \brief Gets the transform of the body.
	 * \return The transform of the body.
	 */
	Transform* Trans();

	/**
	 * \brief Sets the transform of the body.
	 * \param transform Transform to set to the body.
	 */
	void SetTransform(const Transform& transform);

	/**
	 * \brief Gets the collider of the body.
	 * \return The collider of the body.
	 */
	[[nodiscard]] Collider* Col() const;

	/**
	 * \brief Sets the collider of the body.
	 * \param collider The collider to set on the body.
	 */
	void SetCollider(Collider* collider);

	/**
	 * \brief A boolean saying if the collider is a trigger collider.
	 * This means that it will not have a collision response and will only have a callback.
	 * \return If the collider is a trigger collider.
	 */
	[[nodiscard]] bool IsTrigger() const;

	/**
	 * \brief Sets is trigger on the body.
	 * True means that it will not have a collision response and will only have a callback.
	 * \param isTrigger The boolean saying if the collider is trigger.
	 */
	void SetIsTrigger(bool isTrigger);

	/**
	 * \brief Gets the position of the body in the world.
	 * \return The position of the body in the world.
	 */
	[[nodiscard]] const core::Vec2f& Position() const;

	/**
	 * \brief Sets the position of the body in the world.
	 * \param position The new position of the body.
	 */
	void SetPosition(const core::Vec2f& position);

	/**
	 * \brief Gets a boolean that indicates if this body will receive a collision response.
	 * \return A boolean that indicates if this body is kinematic.
	 */
	[[nodiscard]] bool IsKinematic() const;

	/**
	 * \brief Sets a boolean that indicates if this body will receive a collision response.
	 * \param isKinematic The new isKinematic status.
	 */
	void SetIsKinematic(bool isKinematic);

    /**
     * \brief Gets the force of the gravity on this body.
     * \return The force of the gravity.
     */
    [[nodiscard]] const core::Vec2f& GravityForce() const;
    /**
     * \brief Sets the gravity force.
     * \param gravityForce New gravity force.
     */
    void SetGravityForce(const core::Vec2f& gravityForce);

    /**
     * \brief Gets the force on this body.
     * \return The force on this body.
     */
    [[nodiscard]] const core::Vec2f& Force() const;
    /**
     * \brief Adds force to this body.
     * \param addedForce The force to add to this body.
     */
    void ApplyForce(const core::Vec2f& addedForce);
    /**
     * \brief Sets the force of this body.
     * \param force The new force.
     */
    void SetForce(const core::Vec2f& force);

    /**
     * \brief Gets the velocity of this body.
     * \return The velocity of this body.
     */
    [[nodiscard]] const core::Vec2f& Velocity() const;
    /**
     * \brief Sets the velocity of this body.
     * \param velocity The new velocity.
     */
    void SetVelocity(const core::Vec2f& velocity);

    /**
     * \brief Computes the mass of this body. Only the inverted mass is stored,
     * so it's computed as 1 / InvMass();
     * \see InvMass()
     * \return The mass of this body.
     */
    [[nodiscard]] float Mass() const;
    /**
     * \brief Returns 1 / Mass of this body.
     * \return The inverted mass of this body.
     */
    [[nodiscard]] float InvMass() const;
    /**
     * \brief Sets the mass of this body.
     * \param mass The new mass.
     */
    void SetMass(float mass);

    /**
     * \brief Gets a boolean indicating whether this body takes gravity.
     * \return True if this body takes gravity.
     */
    [[nodiscard]] bool TakesGravity() const;
    /**
     * \brief Sets a boolean indicating whether this body takes gravity.
     * \param takesGravity The new takesGravity.
     */
    void SetTakesGravity(bool takesGravity);

    /**
     * \brief Gets the static friction of this body.
     * \return The static friction.
     */
    [[nodiscard]] float StaticFriction() const;
    /**
     * \brief Sets the static friction of this body.
     * \param staticFriction The new static friction.
     */
    void SetStaticFriction(float staticFriction);

    /**
     * \brief Gets the dynamic friction of this body.
     * \return The dynamic friction.
     */
    [[nodiscard]] float DynamicFriction() const;
    /**
     * \brief Sets the dynamic friction of this body.
     * \param dynamicFriction The new dynamic friction.
     */
    void SetDynamicFriction(float dynamicFriction);

    /**
     * \brief Gets the restitution of this body. Can be seen as the "Bounciness".
     * \return The restitution.
     */
    [[nodiscard]] float Restitution() const;
    /**
     * \brief Sets the restitution of this body. Can be seen as the "Bounciness".
     * \param restitution The new restitution.
     */
    void SetRestitution(float restitution);

    [[nodiscard]] bool IsDynamic() const;
    void SetIsDynamic(bool isDynamic);

private:
    core::Vec2f _gravityForce;
    core::Vec2f _force;
    core::Vec2f _velocity;

    float _invMass{};
    bool _takesGravity{};

    float _staticFriction{};
    float _dynamicFriction{};
    float _restitution{};

    Transform _transform{};
	Collider* _collider{};

    bool _isDynamic = false;
	bool _isTrigger = false;
	bool _isKinematic = false;
};
}
