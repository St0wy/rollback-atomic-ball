#include "graphics/shape_manager.hpp"

#include "engine/transform.hpp"

void core::RectangleShapeManager::SetOrigin(const Entity entity, const sf::Vector2f origin)
{
	_components[entity].setOrigin(origin * PIXEL_PER_METER);
}

void core::RectangleShapeManager::Draw(sf::RenderTarget& window)
{
	for (Entity entity = 0; entity < _components.size(); entity++)
	{
		const bool hasShape = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::RectangleShape));
		if (!hasShape) continue;

		const bool hasPosition = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Position));
		const bool hasScale = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Scale));
		const bool hasRotation = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Rotation));

		auto& rectangleShape = _components[entity];

		if (hasPosition)
		{
			const auto position = _transformManager.GetPosition(entity);
			rectangleShape.setPosition(
				position.x * PIXEL_PER_METER + _center.x,
				_windowSize.y - (position.y * PIXEL_PER_METER + _center.y));
		}

		if (hasScale)
		{
			const auto scale = _transformManager.GetScale(entity);
			rectangleShape.setScale(scale);
		}

		if (hasRotation)
		{
			const auto rotation = _transformManager.GetRotation(entity);
			rectangleShape.setRotation(rotation.Value());
		}

		window.draw(rectangleShape);
	}
}

void core::RectangleShapeManager::SetFillColor(const Entity entity, const sf::Color color)
{
	_components[entity].setFillColor(color);
}

void core::RectangleShapeManager::SetOutlineColor(const Entity entity, const sf::Color color)
{
	_components[entity].setOutlineColor(color);
}

void core::RectangleShapeManager::SetOutlineThickness(const Entity entity, const float thickness)
{
	_components[entity].setOutlineThickness(thickness);
}

void core::RectangleShapeManager::SetSize(const Entity entity, const Vec2f size)
{
	_components[entity].setSize(size * PIXEL_PER_METER);
}
