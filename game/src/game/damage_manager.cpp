#include "game/damage_manager.hpp"

void game::DamageManager::OnCollision(const core::Entity entity1, const core::Entity entity2)
{
	const bool oneIsDamager = _entityManager.HasComponent(entity1,
	                                                      static_cast<core::EntityMask>(ComponentType::Damager));
	const bool twoIsPlayer = _entityManager.HasComponent(entity2,
	                                                     static_cast<core::EntityMask>(ComponentType::PlayerCharacter));

	const bool oneIsPlayer = _entityManager.HasComponent(entity1,
	                                                     static_cast<core::EntityMask>(ComponentType::PlayerCharacter));
	const bool twoIsDamager = _entityManager.HasComponent(entity2,
	                                                      static_cast<core::EntityMask>(ComponentType::Damager));

	if (oneIsDamager && twoIsPlayer)
	{
		HandleCollision(entity2);
	}
	else if (oneIsPlayer && twoIsDamager)
	{
		HandleCollision(entity1);
	}
}

void game::DamageManager::HandleCollision(const core::Entity playerEntity) const
{
	PlayerCharacter& playerCharacter = _playerCharacterManager.GetComponent(playerEntity);

	playerCharacter.isDead = true;
}
