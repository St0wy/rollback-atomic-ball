#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "graphics/graphics.h"


namespace game
{
/**
 * \brief StarBackground is a drawable object that draws a star field on a screen.
 */
class StarBackground final : public core::DrawInterface
{
public:
    void Init();
    void Draw(sf::RenderTarget& renderTarget) override;
private:
    static constexpr std::size_t STAR_COUNT = 1024;
    sf::VertexArray _vertexArray;
};

}
