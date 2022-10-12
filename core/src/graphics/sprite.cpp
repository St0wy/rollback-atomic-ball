#include <engine/transform.hpp>

#include <graphics/sprite.hpp>

namespace core
{
void SpriteManager::SetOrigin(const Entity entity, sf::Vector2f origin)
{
	_components[entity].setOrigin(origin);
}

void SpriteManager::SetTexture(const Entity entity, const sf::Texture& texture)
{
	_components[entity].setTexture(texture);
}

void SpriteManager::Draw(sf::RenderTarget& window)
{
	for (Entity entity = 0; entity < _components.size(); entity++)
	{
		const bool hasSprite = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Sprite));
		if (!hasSprite) continue;

		const bool hasPosition = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Position));
		const bool hasScale = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Scale));
		const bool hasRotation = _entityManager.HasComponent(entity, static_cast<Component>(ComponentType::Rotation));

		if (hasPosition)
		{
			const auto position = _transformManager.GetPosition(entity);
			_components[entity].setPosition(
				position.x * PIXEL_PER_METER + _center.x,
				_windowSize.y - (position.y * PIXEL_PER_METER + _center.y));
		}

		if (hasScale)
		{
			const auto scale = _transformManager.GetScale(entity);
			_components[entity].setScale(scale);
		}

		if (hasRotation)
		{
			const auto rotation = _transformManager.GetRotation(entity);
			_components[entity].setRotation(rotation.Value());
		}

		window.draw(_components[entity]);
	}
}

void SpriteManager::SetColor(const Entity entity, const sf::Color color)
{
	_components[entity].setColor(color);
}
} // namespace core
