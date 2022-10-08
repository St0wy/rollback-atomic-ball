#include "physics/dynamics/rigidbody.hpp"

namespace game
{
Rigidbody::Rigidbody()
    : _invMass(1),
      _takesGravity(true)
{
    _isDynamic = true;
}

const core::Vec2f& Rigidbody::GravityForce() const
{
    return _gravityForce;
}

void Rigidbody::SetGravityForce(const core::Vec2f& gravityForce)
{
    _gravityForce = gravityForce;
}

const core::Vec2f& Rigidbody::Force() const
{
    return _force;
}

void Rigidbody::ApplyForce(const core::Vec2f& addedForce)
{
    this->_force += addedForce;
}

void Rigidbody::SetForce(const core::Vec2f& force)
{
    _force = force;
}

const core::Vec2f& Rigidbody::Velocity() const
{
    return _velocity;
}

void Rigidbody::SetVelocity(const core::Vec2f& velocity)
{
    _velocity = velocity;
}

float Rigidbody::Mass() const
{
    return 1.0f / _invMass;
}

float Rigidbody::InvMass() const
{
    return _invMass;
}

void Rigidbody::SetMass(const float mass)
{
    if (mass == 0)
    {
        _invMass = 0;
    }
    _invMass = 1.0f / mass;
}

bool Rigidbody::TakesGravity() const
{
    return _takesGravity;
}

void Rigidbody::SetTakesGravity(const bool takesGravity)
{
    _takesGravity = takesGravity;
    if (!_takesGravity)
    {
        SetGravityForce(core::Vec2f(0, 0));
    }
}

float Rigidbody::StaticFriction() const
{
    return _staticFriction;
}

void Rigidbody::SetStaticFriction(const float staticFriction)
{
    _staticFriction = staticFriction;
}

float Rigidbody::DynamicFriction() const
{
    return _dynamicFriction;
}

void Rigidbody::SetDynamicFriciton(const float dynamicFriciton)
{
    _dynamicFriction = dynamicFriciton;
}

float Rigidbody::Restitution() const
{
    return _restitution;
}

void Rigidbody::SetRestitution(const float restitution)
{
    _restitution = restitution;
}
}
