#pragma once

namespace game
{
class OnTriggerInterface
{
public:
	OnTriggerInterface() = default;
	virtual ~OnTriggerInterface() = default;
	OnTriggerInterface(const OnTriggerInterface& other) = default;
	OnTriggerInterface(OnTriggerInterface&& other) = default;
	OnTriggerInterface& operator=(const OnTriggerInterface& other) = default;
	OnTriggerInterface& operator=(OnTriggerInterface&& other) = default;

	virtual void OnTrigger(core::Entity entity1, core::Entity entity2) = 0;
};

class OnCollisionInterface
{
public:
	OnCollisionInterface() = default;
	virtual ~OnCollisionInterface() = default;
	OnCollisionInterface(const OnCollisionInterface& other) = default;
	OnCollisionInterface(OnCollisionInterface&& other) = default;
	OnCollisionInterface& operator=(const OnCollisionInterface& other) = default;
	OnCollisionInterface& operator=(OnCollisionInterface&& other) = default;

	virtual void OnCollision(core::Entity entity1, core::Entity entity2) = 0;
};
}
