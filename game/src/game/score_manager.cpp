#include "game/score_manager.hpp"

void ScoreManager::CopyAllComponents(const ScoreManager& other)
{
	_score = other.GetScore();
}
