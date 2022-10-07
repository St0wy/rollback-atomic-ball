#include "physics/collision/Simplex.hpp"

namespace game
{
Simplex& Simplex::operator=(std::initializer_list<core::Vec2f> list)
{
    for (auto v = list.begin(); v != list.end(); v++)
    {
        _points[std::distance(list.begin(), v)] = *v;
    }

    _size = list.size();
    return *this;
}

core::Vec2f Simplex::operator[](const std::size_t i) const
{
    return _points[i];
}

void Simplex::PushFront(const core::Vec2f point)
{
    _points = {point, _points[0], _points[1]};
    IncrementSize();
}

std::size_t Simplex::Size() const
{
    return _size;
}

void Simplex::IncrementSize()
{
    _size = std::min(_size + 1, MAX_SIZE);
}

std::array<core::Vec2f, Simplex::MAX_SIZE>::const_iterator Simplex::End() const
{
    return _points.end();
}

std::array<core::Vec2f, Simplex::MAX_SIZE>::const_iterator Simplex::Begin() const
{
    return _points.begin();
}
}