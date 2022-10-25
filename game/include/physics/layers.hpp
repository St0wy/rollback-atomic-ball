#pragma once
#include <optional>

namespace game
{
enum class Layer : std::uint8_t
{
	None = 0,
	Wall,
	Door,
	MiddleWall,
	Player,
	Ball,
};

struct LayerMask
{
	std::uint8_t player : 1 = 1;
	std::uint8_t wall : 1 = 1;
	std::uint8_t door : 1 = 1;
	std::uint8_t middleWall : 1 = 1;
	std::uint8_t ball : 1 = 1;

	[[nodiscard]] bool HasCollision(Layer layer) const;

	void SetCollision(Layer layer, bool value);
};

struct LayerCollisionMatrix
{
	LayerMask player{};
	LayerMask wall{};
	LayerMask door{};
	LayerMask middleWall{};
	LayerMask ball{};

	std::optional<LayerMask> GetMask(Layer layer);
	LayerMask& GetMaskRef(Layer layer);

	bool HasCollision(Layer layerOne, Layer layerTwo);

	void SetCollision(Layer layerOne, Layer layerTwo, bool value);
};
}
