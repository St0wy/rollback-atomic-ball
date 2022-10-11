#include "game/star_background.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "engine/globals.hpp"

#include "maths/basic.hpp"

namespace game
{
void StarBackground::Init()
{
    _vertexArray = sf::VertexArray(sf::Points, STAR_COUNT);

    for (std::size_t i = 0; i < STAR_COUNT; i++)
    {
        auto& vertex = _vertexArray[i];
        vertex.color = sf::Color::White;
        vertex.position = sf::Vector2f(
            core::RandomRange(-50.0f, 50.0f) * core::PIXEL_PER_METER,
            core::RandomRange(-50.0f, 50.0f) * core::PIXEL_PER_METER);
    }
}

void StarBackground::Draw(sf::RenderTarget& renderTarget)
{
    renderTarget.draw(_vertexArray);
}
}
