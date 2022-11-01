#pragma once
#include <cstdint>

/**
 * \brief Manages the score of the game.
 */
class ScoreManager
{
public:
	void AddScore(const std::uint32_t value) { _score += value; }

	[[nodiscard]] std::uint32_t GetScore() const { return _score; }
	void CopyAllComponents(const ScoreManager& other);

private:
	std::uint32_t _score{};
};
