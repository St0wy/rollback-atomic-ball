#pragma once
#include "maths/vec2.h"

namespace game
{
struct Edge
{
	core::Vec2f max;
    core::Vec2f p1;
    core::Vec2f p2;

    [[nodiscard]] float VecDot(const core::Vec2f& v) const;
    [[nodiscard]] core::Vec2f EdgeVector() const;
};
}
