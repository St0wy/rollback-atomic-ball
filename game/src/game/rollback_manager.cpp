#include <fmt/format.h>

#include <game/game_manager.hpp>
#include <game/rollback_manager.hpp>

#include <utils/log.hpp>

#include "utils/assert.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
RollbackManager::RollbackManager(GameManager& gameManager, core::EntityManager& entityManager)
	: OnTriggerInterface(), _gameManager(gameManager), _entityManager(entityManager),
	_currentTransformManager(entityManager),
	_currentPhysicsManager(entityManager), _currentPlayerManager(entityManager, _currentPhysicsManager, _gameManager),
	_currentBulletManager(entityManager, gameManager),
	_lastValidatePhysicsManager(entityManager),
	_lastValidatePlayerManager(entityManager, _lastValidatePhysicsManager, _gameManager),
	_lastValidateBulletManager(entityManager, gameManager)
{
	for (auto& input : _inputs)
	{
		std::ranges::fill(input, '\0');
	}
	_currentPhysicsManager.RegisterTriggerListener(*this);
}

void RollbackManager::SimulateToCurrentFrame()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	const auto currentFrame = _gameManager.GetCurrentFrame();
	const auto lastValidateFrame = _gameManager.GetLastValidateFrame();
	//Destroying all created Entities after the last validated frame
	for (const auto& [entity, createdFrame] : _createdEntities)
	{
		if (createdFrame > lastValidateFrame)
		{
			_entityManager.DestroyEntity(entity);
		}
	}
	_createdEntities.clear();
	//Remove DESTROY flags
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
		{
			_entityManager.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));
		}
	}

	//Revert the current game state to the last validated game state
	_currentBulletManager.CopyAllComponents(_lastValidateBulletManager.GetAllComponents());
	_currentPhysicsManager.CopyAllComponents(_lastValidatePhysicsManager);
	_currentPlayerManager.CopyAllComponents(_lastValidatePlayerManager.GetAllComponents());

	for (Frame frame = lastValidateFrame + 1; frame <= currentFrame; frame++)
	{
		_testedFrame = frame;
		//Copy player inputs to player manager
		for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
		{
			const auto playerInput = GetInputAtFrame(playerNumber, frame);
			const auto playerEntity = _gameManager.GetEntityFromPlayerNumber(playerNumber);
			if (playerEntity == core::INVALID_ENTITY)
			{
				core::LogWarning(fmt::format("Invalid Entity in {}:line {}", __FILE__, __LINE__));
				continue;
			}
			auto& playerCharacter = _currentPlayerManager.GetComponent(playerEntity);
			playerCharacter.input = playerInput;
			_currentPlayerManager.SetComponent(playerEntity, playerCharacter);
		}
		//Simulate one frame of the game
		_currentBulletManager.FixedUpdate(sf::seconds(FIXED_PERIOD));
		_currentPlayerManager.FixedUpdate(sf::seconds(FIXED_PERIOD));
		_currentPhysicsManager.FixedUpdate(sf::seconds(FIXED_PERIOD));
	}
	//Copy the physics states to the transforms
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (!_entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody) |
			static_cast<core::EntityMask>(core::ComponentType::Transform)))
			continue;
		const auto& body = _currentPhysicsManager.GetRigidbody(entity);
		_currentTransformManager.SetPosition(entity, body.Position());
		_currentTransformManager.SetRotation(entity, body.Rotation());
	}
}

void RollbackManager::SetPlayerInput(const PlayerNumber playerNumber, const PlayerInput playerInput, const Frame inputFrame)
{
	//Should only be called on the server
	if (_currentFrame < inputFrame)
	{
		StartNewFrame(inputFrame);
	}
	_inputs[playerNumber][_currentFrame - inputFrame] = playerInput;
	if (_lastReceivedFrame[playerNumber] < inputFrame)
	{
		_lastReceivedFrame[playerNumber] = inputFrame;
		//Repeat the same inputs until currentFrame
		for (size_t i = 0; i < _currentFrame - inputFrame; i++)
		{
			_inputs[playerNumber][i] = playerInput;
		}
	}
}

void RollbackManager::StartNewFrame(const Frame newFrame)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	if (_currentFrame > newFrame)
		return;
	const auto delta = newFrame - _currentFrame;
	if (delta == 0)
	{
		return;
	}
	for (auto& inputs : _inputs)
	{
		for (auto i = inputs.size() - 1; i >= delta; i--)
		{
			inputs[i] = inputs[i - delta];
		}

		for (Frame i = 0; i < delta; i++)
		{
			inputs[i] = inputs[delta];
		}
	}
	_currentFrame = newFrame;
}

void RollbackManager::ValidateFrame(const Frame newValidateFrame)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	const auto lastValidateFrame = _gameManager.GetLastValidateFrame();
	//We check that we got all the inputs
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		if (GetLastReceivedFrame(playerNumber) < newValidateFrame)
		{
			gpr_assert(false, "We should not validate a frame if we did not receive all inputs!!!");
			return;
		}
	}
	//Destroying all created Entities after the last validated frame
	for (const auto& createdEntity : _createdEntities)
	{
		if (createdEntity.createdFrame > lastValidateFrame)
		{
			_entityManager.DestroyEntity(createdEntity.entity);
		}
	}
	_createdEntities.clear();
	//Remove DESTROYED flag
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
		{
			_entityManager.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));
		}
	}
	_createdEntities.clear();

	//We use the current game state as the temporary new validate game state
	_currentBulletManager.CopyAllComponents(_lastValidateBulletManager.GetAllComponents());
	_currentPhysicsManager.CopyAllComponents(_lastValidatePhysicsManager);
	_currentPlayerManager.CopyAllComponents(_lastValidatePlayerManager.GetAllComponents());

	//We simulate the frames until the new validated frame
	for (Frame frame = _lastValidateFrame + 1; frame <= newValidateFrame; frame++)
	{
		_testedFrame = frame;
		//Copy the players inputs into the player manager
		for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
		{
			const auto playerInput = GetInputAtFrame(playerNumber, frame);
			const auto playerEntity = _gameManager.GetEntityFromPlayerNumber(playerNumber);
			auto& playerCharacter = _currentPlayerManager.GetComponent(playerEntity);
			playerCharacter.input = playerInput;
			_currentPlayerManager.SetComponent(playerEntity, playerCharacter);
		}
		//We simulate one frame
		_currentBulletManager.FixedUpdate(sf::seconds(FIXED_PERIOD));
		_currentPlayerManager.FixedUpdate(sf::seconds(FIXED_PERIOD));
		_currentPhysicsManager.FixedUpdate(sf::seconds(FIXED_PERIOD));
	}
	//Definitely remove DESTROY entities
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
		{
			_entityManager.DestroyEntity(entity);
		}
	}
	//Copy back the new validate game state to the last validated game state
	_lastValidateBulletManager.CopyAllComponents(_currentBulletManager.GetAllComponents());
	_lastValidatePlayerManager.CopyAllComponents(_currentPlayerManager.GetAllComponents());
	_lastValidatePhysicsManager.CopyAllComponents(_currentPhysicsManager);
	_lastValidateFrame = newValidateFrame;
	_createdEntities.clear();
}

void RollbackManager::ConfirmFrame(Frame newValidatedFrame,
	const std::array<PhysicsState, MAX_PLAYER_NMB>& serverPhysicsState)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	ValidateFrame(newValidatedFrame);
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		const PhysicsState lastPhysicsState = GetValidatePhysicsState(playerNumber);
		if (serverPhysicsState[playerNumber] != lastPhysicsState)
		{
			gpr_assert(false,
				fmt::format(
				"Physics State are not equal for player {} (server frame: {}, client frame: {}, server: {}, client: {})",
				playerNumber + 1,
				newValidatedFrame,
				_lastValidateFrame,
				serverPhysicsState[playerNumber],
				lastPhysicsState));
		}
	}
}

PhysicsState RollbackManager::GetValidatePhysicsState(const PlayerNumber playerNumber) const
{
	PhysicsState state = 0;
	const core::Entity playerEntity = _gameManager.GetEntityFromPlayerNumber(playerNumber);
	const Rigidbody& rigidbody = _lastValidatePhysicsManager.GetRigidbody(playerEntity);

	const auto& pos = rigidbody.Position();
	const auto* posPtr = reinterpret_cast<const PhysicsState*>(&pos);

	//Adding position
	for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
	{
		state += posPtr[i];
	}

	//Adding velocity
	const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&rigidbody.Velocity());
	for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
	{
		state += velocityPtr[i];
	}

	//Adding rotation
	const auto angle = rigidbody.Trans().rotation.Value();
	const auto* anglePtr = reinterpret_cast<const PhysicsState*>(&angle);
	for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
	{
		state += anglePtr[i];
	}

	return state;
}

void RollbackManager::SpawnPlayer(const PlayerNumber playerNumber, const core::Entity entity,
	const core::Vec2f position, const core::Degree rotation)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	Rigidbody playerBody;
	playerBody.SetPosition(position);
	playerBody.SetRotation(rotation);
	playerBody.SetTakesGravity(false);
	playerBody.SetDragFactor(0.9f);
	playerBody.SetBodyType(BodyType::Dynamic);

	CircleCollider playerCircle;
	playerCircle.radius = 0.25f;

	PlayerCharacter playerCharacter;
	playerCharacter.playerNumber = playerNumber;

	_currentPlayerManager.AddComponent(entity);
	_currentPlayerManager.SetComponent(entity, playerCharacter);

	_currentPhysicsManager.AddRigidbody(entity);
	_currentPhysicsManager.SetRigidbody(entity, playerBody);
	_currentPhysicsManager.AddCircleCollider(entity);
	_currentPhysicsManager.SetCircleCollider(entity, playerCircle);

	_lastValidatePlayerManager.AddComponent(entity);
	_lastValidatePlayerManager.SetComponent(entity, playerCharacter);

	_lastValidatePhysicsManager.AddRigidbody(entity);
	_lastValidatePhysicsManager.SetRigidbody(entity, playerBody);
	_lastValidatePhysicsManager.AddCircleCollider(entity);
	_lastValidatePhysicsManager.SetCircleCollider(entity, playerCircle);

	_currentTransformManager.AddComponent(entity);
	_currentTransformManager.SetPosition(entity, position);
	_currentTransformManager.SetRotation(entity, rotation);
}

PlayerInput RollbackManager::GetInputAtFrame(const PlayerNumber playerNumber, const Frame frame) const
{
	gpr_assert(_currentFrame - frame < _inputs[playerNumber].size(),
		"Trying to get input too far in the past");
	return _inputs[playerNumber][_currentFrame - frame];
}

void RollbackManager::OnTrigger(const core::Entity entity1, const core::Entity entity2)
{
	const std::function<void(const PlayerCharacter&, core::Entity, const Bullet&, core::Entity)> manageCollision =
		[this](const auto& player, auto playerEntity, const auto& bullet, auto bulletEntity)
	{
		if (player.playerNumber != bullet.playerNumber)
		{
			_gameManager.DestroyBullet(bulletEntity);
			//lower health point
			auto& playerCharacter = _currentPlayerManager.GetComponent(playerEntity);
			if (playerCharacter.invincibilityTime <= 0.0f)
			{
				core::LogDebug(fmt::format("Player {} is hit by bullet", playerCharacter.playerNumber));
				--playerCharacter.health;
				playerCharacter.invincibilityTime = PLAYER_INVINCIBILITY_PERIOD;
			}
			_currentPlayerManager.SetComponent(playerEntity, playerCharacter);
		}
	};
	if (_entityManager.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PlayerCharacter)) &&
		_entityManager.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::Bullet)))
	{
		const auto& player = _currentPlayerManager.GetComponent(entity1);
		const auto& bullet = _currentBulletManager.GetComponent(entity2);
		manageCollision(player, entity1, bullet, entity2);
	}
	if (_entityManager.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PlayerCharacter)) &&
		_entityManager.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::Bullet)))
	{
		const auto& player = _currentPlayerManager.GetComponent(entity2);
		const auto& bullet = _currentBulletManager.GetComponent(entity1);
		manageCollision(player, entity2, bullet, entity1);
	}
}

void RollbackManager::SpawnBullet(const PlayerNumber playerNumber, const core::Entity entity,
	const core::Vec2f position, const core::Vec2f velocity)
{
	_createdEntities.push_back({ entity, _testedFrame });

	Rigidbody bulletBody;
	bulletBody.SetPosition(position);
	bulletBody.SetVelocity(velocity);
	const auto scale = core::Vec2f::One() * BULLET_SCALE;
	bulletBody.Trans().scale = scale;
	bulletBody.SetTakesGravity(false);
	bulletBody.SetIsTrigger(true);
	bulletBody.SetBodyType(BodyType::Dynamic);

	CircleCollider bulletCircle;
	bulletCircle.radius = 0.25f;

	_currentBulletManager.AddComponent(entity);
	_currentBulletManager.SetComponent(entity, { BULLET_PERIOD, playerNumber });

	_currentPhysicsManager.AddRigidbody(entity);
	_currentPhysicsManager.SetRigidbody(entity, bulletBody);
	_currentPhysicsManager.AddCircleCollider(entity);
	_currentPhysicsManager.SetCircleCollider(entity, bulletCircle);

	_currentTransformManager.AddComponent(entity);
	_currentTransformManager.SetPosition(entity, position);
	//_currentTransformManager.SetScale(entity, scale);
	_currentTransformManager.SetRotation(entity, core::Degree(0.0f));
}

void RollbackManager::DestroyEntity(core::Entity entity)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	//we don't need to save a bullet that has been created in the time window
	if (std::ranges::find_if(_createdEntities,
		[entity](auto newEntity)
	{
		return newEntity.entity == entity;
	}) != _createdEntities.end())
	{
		_entityManager.DestroyEntity(entity);
		return;
	}
	_entityManager.AddComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));
}
}
