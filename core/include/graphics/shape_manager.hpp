#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "graphics.hpp"

#include "engine/component.hpp"
#include "maths/vec2.hpp"

namespace core
{
class TransformManager;

class RectangleShapeManager final :
	public ComponentManager<sf::RectangleShape, static_cast<Component>(ComponentType::RectangleShape)>,
	public DrawInterface
{
public:
	RectangleShapeManager(EntityManager& entityManager, TransformManager& transformManager)
		: ComponentManager(entityManager),
		  _transformManager(transformManager)
	{
	}

	void SetOrigin(Entity entity, sf::Vector2f origin);
	void SetCenter(const sf::Vector2f center) { _center = center; }
	void SetWindowSize(const sf::Vector2f newWindowSize) { _windowSize = newWindowSize; }
	void Draw(sf::RenderTarget& window) override;
	void SetFillColor(Entity entity, sf::Color color);
	void SetOutlineColor(Entity entity, sf::Color color);
	void SetOutlineThickness(Entity entity, float thickness);
	void SetSize(Entity entity, Vec2f size);

protected:
	TransformManager& _transformManager;
	sf::Vector2f _center{};
	sf::Vector2f _windowSize{};
};
}
