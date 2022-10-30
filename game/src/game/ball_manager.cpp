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
}
