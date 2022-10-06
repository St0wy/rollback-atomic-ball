#include <engine/transform.h>

namespace core
{
void ScaleManager::AddComponent(const Entity entity)
{
	ComponentManager<Vec2f, 4>::AddComponent(entity);
	_components[entity] = Vec2f::One();
}

TransformManager::TransformManager(EntityManager& entityManager)
	: _positionManager(entityManager),
	  _scaleManager(entityManager),
	  _rotationManager(entityManager) {}

Vec2f TransformManager::GetPosition(const Entity entity) const
{
	return _positionManager.GetComponent(entity);
}

const std::vector<Vec2f>& TransformManager::GetAllPositions() const
{
	return _positionManager.GetAllComponents();
}

const std::vector<Vec2f>& TransformManager::GetAllScales() const
{
	return _scaleManager.GetAllComponents();
}

const std::vector<Degree>& TransformManager::GetAllRotations() const
{
	return _rotationManager.GetAllComponents();
}

void TransformManager::SetPosition(const Entity entity, const Vec2f position)
{
	_positionManager.SetComponent(entity, position);
}

Vec2f TransformManager::GetScale(const Entity entity) const
{
	return _scaleManager.GetComponent(entity);
}

void TransformManager::SetScale(const Entity entity, const Vec2f scale)
{
	_scaleManager.SetComponent(entity, scale);
}

Degree TransformManager::GetRotation(const Entity entity) const
{
	return _rotationManager.GetComponent(entity);
}

void TransformManager::SetRotation(const Entity entity, const Degree rotation)
{
	_rotationManager.SetComponent(entity, rotation);
}

void TransformManager::AddComponent(const Entity entity)
{
	_positionManager.AddComponent(entity);
	_scaleManager.AddComponent(entity);
	_rotationManager.AddComponent(entity);
}

void TransformManager::RemoveComponent(const Entity entity)
{
	_positionManager.AddComponent(entity);
	_scaleManager.AddComponent(entity);
	_rotationManager.AddComponent(entity);
}
}
