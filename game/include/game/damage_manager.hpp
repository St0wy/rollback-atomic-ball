#pragma once

#include "engine/component.hpp"
#include "game/falling_wall_manager.hpp"

namespace game
{
struct Damager
{
	short damageAmount = 1;
};

class DamageManager final :
	public core::ComponentManager<Damager, static_cast<core::EntityMask>(ComponentType::Damager)>,
	public OnCollisionInterface
{
public:
	explicit DamageManager(core::EntityManager& entityManager) : ComponentManager(entityManager) {}
	void OnCollision(core::Entity entity1, core::Entity entity2) override;
};
}
