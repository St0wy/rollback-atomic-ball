/**
 * @file Simplex.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Simplex struct.
 * @version 1.0
 * @date 05.07.2022
 *
 * @copyright SAE (c) 2022
 *
 */
#pragma once

#include <array>

#include "maths/vec2.h"

namespace game
{
/**
* \brief Struct representing the simplest form that can "select" an area in any dimension.
*/
struct Simplex
{
public:
    /**
     * \brief The max size of this simplex.
     */
    static constexpr unsigned long long MAX_SIZE = 3ull;

    Simplex()
        : _points({core::Vec2f(0, 0), {0, 0}, {0, 0}}), _size(0) {}

    Simplex& operator=(std::initializer_list<core::Vec2f> list);
    core::Vec2f operator[](std::size_t i) const;

    /**
     * \brief Adds a point in the simplex.
     * \param point Point to add in the simplex.
     */
    void PushFront(core::Vec2f point);
    /**
     * \brief Gets the size of the array.
     * \return The size of the array.
     */
    [[nodiscard]] std::size_t Size() const;
    [[nodiscard]] std::array<core::Vec2f, MAX_SIZE>::const_iterator Begin() const;
    [[nodiscard]] std::array<core::Vec2f, MAX_SIZE>::const_iterator End() const;
    /**
     * \brief Increments the size of the simplex with a max at MAX_SIZE.
     */
    void IncrementSize();

private:
    std::array<core::Vec2f, MAX_SIZE> _points;
    std::size_t _size;
};
}
