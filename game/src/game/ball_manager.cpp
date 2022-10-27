#include "game/ball_manager.hpp"

#include "game/game_manager.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
BallManager::BallManager(core::EntityManager& entityManager) :
    ComponentManager(entityManager)
{
}

void BallManager::FixedUpdate(const sf::Time)
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
            //Ball& ball = _components[entity];
        }
    }
}
}
