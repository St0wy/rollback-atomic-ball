#include "physics/edge.hpp"

namespace game
{
float Edge::VecDot(const core::Vec2f& v) const
{
	return EdgeVector().Dot(v);
}

core::Vec2f Edge::EdgeVector() const
{
	return p2 - p1;
}
}
