#pragma once

#include "engine/component.hpp"
#include "game/falling_wall_manager.hpp"

namespace game
{
struct Damager
{
	short damageAmount = 10;
};

/**
 * \brief This is the manager for a component that damages the player on collision.
 */
class DamageManager final :
	public core::ComponentManager<Damager, static_cast<core::EntityMask>(ComponentType::Damager)>,
	public OnCollisionInterface
{
public:
	explicit DamageManager(core::EntityManager& entityManager, PlayerCharacterManager& playerCharacterManager)
		: ComponentManager(entityManager), _playerCharacterManager(playerCharacterManager)
	{}

	void OnCollision(core::Entity entity1, core::Entity entity2) override;

private:
	void HandleCollision(core::Entity playerEntity) const;

	PlayerCharacterManager& _playerCharacterManager;
};
}
