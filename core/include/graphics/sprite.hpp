#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "graphics.hpp"

#include "engine/component.hpp"

namespace core
{
class TransformManager;

/**
 * \brief SpriteManager is a ComponentManager that manages sprites, order by greater entity index, background entity < foreground entity
 * Positions are centered at the center of the render target and use pixelPerMeter from globals.h
 */
class SpriteManager final :
	public ComponentManager<sf::Sprite, static_cast<Component>(ComponentType::Sprite)>,
	public DrawInterface
{
public:
	SpriteManager(EntityManager& entityManager, TransformManager& transformManager)
		: ComponentManager(entityManager),
		  _transformManager(transformManager)
	{
	}

	void SetOrigin(Entity entity, sf::Vector2f origin);
	void SetTexture(Entity entity, const sf::Texture& texture);
	void SetCenter(const sf::Vector2f center) { _center = center; }
	void SetWindowSize(const sf::Vector2f newWindowSize) { _windowSize = newWindowSize; }
	void Draw(sf::RenderTarget& window) override;
	void SetColor(Entity entity, sf::Color color);

protected:
	TransformManager& _transformManager;
	sf::Vector2f _center{};
	sf::Vector2f _windowSize{};
};
}
