// ReSharper disable CppClangTidyClangDiagnosticCoveredSwitchDefault
#include "physics/layers.hpp"
#include "utils/assert.hpp"

bool game::LayerMask::HasCollision(const Layer layer) const
{
	switch (layer)
	{
	case Layer::Player:
		return player;
	case Layer::Wall:
		return wall;
	case Layer::MiddleWall:
		return middleWall;
	case Layer::Ball:
		return ball;
	case Layer::None:
	default:
		return true;
	}
}

void game::LayerMask::SetCollision(const Layer layer, const bool value)
{
	switch (layer)
	{
	case Layer::Player:
		player = value;
		break;
	case Layer::Wall:
		wall = value;
		break;
	case Layer::MiddleWall:
		middleWall = value;
		break;
	case Layer::Ball:
		ball = value;
		break;
	case Layer::None:
	default:
		break;
	}
}

std::optional<game::LayerMask> game::LayerCollisionMatrix::GetMask(const Layer layer)
{
	switch (layer)
	{
	case Layer::Player:
		return player;
	case Layer::Wall:
		return wall;
	case Layer::MiddleWall:
		return middleWall;
	case Layer::Ball:
		return ball;
	case Layer::None:
	default:
		return {};
	}
}

game::LayerMask& game::LayerCollisionMatrix::GetMaskRef(const Layer layer)
{
	switch (layer)
	{
	case Layer::Player:
		return player;
	case Layer::Wall:
		return wall;
	case Layer::MiddleWall:
		return middleWall;
	case Layer::Ball:
		return ball;
	case Layer::None:
	default:
		throw core::AssertException("Bad Layer in GetMaskRef");
	}
}

bool game::LayerCollisionMatrix::HasCollision(const Layer layerOne, const Layer layerTwo)
{
	const auto firstLayerMask = GetMask(layerOne);

	if (!firstLayerMask) return true;

	return firstLayerMask->HasCollision(layerTwo);
}

void game::LayerCollisionMatrix::SetCollision(const Layer layerOne, const Layer layerTwo, const bool value)
{
	if (layerOne == Layer::None || layerTwo == Layer::None) return;

	LayerMask& maskOne = GetMaskRef(layerOne);
	LayerMask& maskTwo = GetMaskRef(layerTwo);

	maskOne.SetCollision(layerTwo, value);
	maskTwo.SetCollision(layerOne, value);
}
