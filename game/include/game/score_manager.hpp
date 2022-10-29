#pragma once
#include <cstdint>

class ScoreManager
{
public:
	void AddScore(const std::uint32_t value) { _score += value; }

	[[nodiscard]] std::uint32_t GetScore() const { return _score; }

private:
	std::uint32_t _score{};
};