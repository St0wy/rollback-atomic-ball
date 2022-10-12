#include "game/bullet_manager.hpp"

#include "game/game_manager.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
BulletManager::BulletManager(core::EntityManager& entityManager, GameManager& gameManager) :
    ComponentManager(entityManager), _gameManager(gameManager)
{
}

void BulletManager::FixedUpdate(const sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
    {
        if(_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
            continue;

        if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Bullet)))
        {
            auto& [remainingTime, playerNumber] = _components[entity];
            remainingTime -= dt.asSeconds();
            if (remainingTime < 0.0f)
            {
                _gameManager.DestroyBullet(entity);
            }
        }
    }
}
}
