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
	: OnTriggerInterface(), OnCollisionInterface(), _gameManager(gameManager), _entityManager(entityManager),
	_currentTransformManager(entityManager),
	_currentPhysicsManager(entityManager), _currentPlayerManager(entityManager, _currentPhysicsManager, _gameManager),
	_currentBulletManager(entityManager),
	_currentFallingObjectManager(entityManager, _currentPhysicsManager),
	_currentFallingDoorManager(entityManager, _currentPlayerManager, _gameManager),
	_currentDamageManager(entityManager, _currentPlayerManager),
	_lastValidatePhysicsManager(entityManager),
	_lastValidatePlayerManager(entityManager, _lastValidatePhysicsManager, _gameManager),
	_lastValidateBulletManager(entityManager),
	_lastValidateFallingObjectManager(entityManager, _lastValidatePhysicsManager),
	_lastValidateFallingDoorManager(entityManager, _lastValidatePlayerManager, _gameManager),
	_lastValidateDamageManager(entityManager, _lastValidatePlayerManager)
{
	for (auto& input : _inputs)
	{
		std::ranges::fill(input, '\0');
	}

	_currentPhysicsManager.RegisterTriggerListener(*this);
	_currentPhysicsManager.RegisterCollisionListener(*this);
	_currentPhysicsManager.RegisterCollisionListener(_currentFallingDoorManager);
	_currentPhysicsManager.RegisterCollisionListener(_currentDamageManager);
}

void RollbackManager::SimulateToCurrentFrame()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	const auto currentFrame = _gameManager.GetCurrentFrame();
	const auto lastValidateFrame = _gameManager.GetLastValidateFrame();

	// Destroying all created Entities after the last validated frame
	for (const auto& [entity, createdFrame] : _createdEntities)
	{
		if (createdFrame > lastValidateFrame)
		{
			_entityManager.DestroyEntity(entity);
		}
	}

	_createdEntities.clear();

	// Remove DESTROY flags
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
		{
			_entityManager.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));
		}
	}

	// Revert the current game state to the last validated game state
	_currentBulletManager.CopyAllComponents(_lastValidateBulletManager.GetAllComponents());
	_currentPhysicsManager.CopyAllComponents(_lastValidatePhysicsManager);
	_currentPlayerManager.CopyAllComponents(_lastValidatePlayerManager.GetAllComponents());
	_currentFallingObjectManager.CopyAllComponents(_lastValidateFallingObjectManager.GetAllComponents());
	_currentFallingDoorManager.CopyAllComponents(_lastValidateFallingDoorManager.GetAllComponents());


	for (Frame frame = lastValidateFrame + 1; frame <= currentFrame; frame++)
	{
		_testedFrame = frame;

		// Copy player inputs to player manager
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
		}

		// Simulate one frame of the game
		const sf::Time period = sf::seconds(FIXED_PERIOD);
		_currentBulletManager.FixedUpdate(period);
		_currentPlayerManager.FixedUpdate(period);
		_currentFallingObjectManager.FixedUpdate(period);
		_currentPhysicsManager.FixedUpdate(period);
	}

	// Copy the physics states to the transforms
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (!_entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody) |
			static_cast<core::EntityMask>(core::ComponentType::Transform)))
			continue;
		const Rigidbody& body = _currentPhysicsManager.GetRigidbody(entity);
		_currentTransformManager.SetPosition(entity, body.Position());
		_currentTransformManager.SetRotation(entity, body.Rotation());
	}
}

void RollbackManager::SetPlayerInput(const PlayerNumber playerNumber, const PlayerInput playerInput,
	const Frame inputFrame)
{
	// Should only be called on the server
	if (_currentFrame < inputFrame)
	{
		StartNewFrame(inputFrame);
	}

	const std::size_t frameDifference = static_cast<std::size_t>(_currentFrame) - inputFrame;

	_inputs[playerNumber][frameDifference] = playerInput;
	if (_lastReceivedFrame[playerNumber] < inputFrame)
	{
		_lastReceivedFrame[playerNumber] = inputFrame;

		// Repeat the same inputs until currentFrame
		for (size_t i = 0; i < frameDifference; i++)
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

	if (_currentFrame > newFrame) return;

	const auto delta = newFrame - _currentFrame;
	if (delta == 0) return;

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

	// We check that we got all the inputs
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		if (GetLastReceivedFrame(playerNumber) < newValidateFrame)
		{
			gpr_assert(false, "We should not validate a frame if we did not receive all inputs!!!");
			return;
		}
	}

	// Destroying all created Entities after the last validated frame
	for (const auto& createdEntity : _createdEntities)
	{
		if (createdEntity.createdFrame > lastValidateFrame)
		{
			_entityManager.DestroyEntity(createdEntity.entity);
		}
	}

	_createdEntities.clear();
	// Remove DESTROYED flag
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
		{
			_entityManager.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));
		}
	}

	_createdEntities.clear();

	// We use the current game state as the temporary new validate game state
	_currentBulletManager.CopyAllComponents(_lastValidateBulletManager.GetAllComponents());
	_currentPhysicsManager.CopyAllComponents(_lastValidatePhysicsManager);
	_currentPlayerManager.CopyAllComponents(_lastValidatePlayerManager.GetAllComponents());
	_currentFallingObjectManager.CopyAllComponents(_lastValidateFallingObjectManager.GetAllComponents());
	_currentFallingDoorManager.CopyAllComponents(_lastValidateFallingDoorManager.GetAllComponents());
	_currentDamageManager.CopyAllComponents(_lastValidateDamageManager.GetAllComponents());

	// We simulate the frames until the new validated frame
	for (Frame frame = _lastValidateFrame + 1; frame <= newValidateFrame; frame++)
	{
		_testedFrame = frame;
		// Copy the players inputs into the player manager
		for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
		{
			const auto playerInput = GetInputAtFrame(playerNumber, frame);
			const auto playerEntity = _gameManager.GetEntityFromPlayerNumber(playerNumber);
			auto& playerCharacter = _currentPlayerManager.GetComponent(playerEntity);
			playerCharacter.input = playerInput;
		}

		// We simulate one frame
		const sf::Time period = sf::seconds(FIXED_PERIOD);
		_currentBulletManager.FixedUpdate(period);
		_currentPlayerManager.FixedUpdate(period);
		_currentFallingObjectManager.FixedUpdate(period);
		_currentPhysicsManager.FixedUpdate(period);
	}

	// Definitely remove DESTROY entities
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (_entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
		{
			_entityManager.DestroyEntity(entity);
		}
	}

	// Copy back the new validate game state to the last validated game state
	_lastValidateBulletManager.CopyAllComponents(_currentBulletManager.GetAllComponents());
	_lastValidatePlayerManager.CopyAllComponents(_currentPlayerManager.GetAllComponents());
	_lastValidatePhysicsManager.CopyAllComponents(_currentPhysicsManager);
	_lastValidateFallingObjectManager.CopyAllComponents(_currentFallingObjectManager.GetAllComponents());
	_lastValidateFallingDoorManager.CopyAllComponents(_currentFallingDoorManager.GetAllComponents());
	_lastValidateDamageManager.CopyAllComponents(_currentDamageManager.GetAllComponents());
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

	// Adding position
	for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
	{
		state += posPtr[i];
	}

	// Adding velocity
	const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&rigidbody.Velocity());
	for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
	{
		state += velocityPtr[i];
	}

	// Adding rotation
	const auto angle = rigidbody.Trans().rotation.Value();
	const auto* anglePtr = reinterpret_cast<const PhysicsState*>(&angle);
	for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
	{
		state += anglePtr[i];
	}

	return state;
}

void RollbackManager::SetupLevel(const core::Entity wallLeftEntity, const core::Entity wallRightEntity,
	const core::Entity wallMiddleEntity, const core::Entity wallBottomEntity,
	const core::Entity wallTopEntity)
{
	CreateWall(wallLeftEntity, WALL_LEFT_POS, VERTICAL_WALLS_SIZE);
	CreateWall(wallRightEntity, WALL_RIGHT_POS, VERTICAL_WALLS_SIZE);
	CreateWall(wallMiddleEntity, WALL_MIDDLE_POS, MIDDLE_WALL_SIZE, Layer::MiddleWall);
	CreateWall(wallBottomEntity, WALL_BOTTOM_POS, HORIZONTAL_WALLS_SIZE);
	CreateWall(wallTopEntity, WALL_TOP_POS, HORIZONTAL_WALLS_SIZE);

	_currentDamageManager.AddComponent(wallBottomEntity);
	_lastValidateDamageManager.AddComponent(wallBottomEntity);
}

void RollbackManager::SpawnFallingWall(const core::Entity backgroundWall, const core::Entity door)
{
	_createdEntities.push_back({ backgroundWall, _testedFrame });
	_createdEntities.push_back({ door, _testedFrame });

	float doorPosition = 1.0f;

	Rigidbody wallBody;
	wallBody.SetPosition({ 0,3.0f });
	wallBody.SetDragFactor(0);
	wallBody.SetBodyType(BodyType::Kinematic);
	wallBody.SetRestitution(1.0f);
	wallBody.SetMass(100);
	wallBody.SetLayer(Layer::Wall);

	AabbCollider wallCollider;
	wallCollider.halfWidth = FALLING_WALL_SIZE.x / 2.0f;
	wallCollider.halfHeight = FALLING_WALL_SIZE.y / 2.0f;

	_currentPhysicsManager.AddRigidbody(backgroundWall);
	_currentPhysicsManager.SetRigidbody(backgroundWall, wallBody);
	_currentPhysicsManager.AddAabbCollider(backgroundWall);
	_currentPhysicsManager.SetAabbCollider(backgroundWall, wallCollider);
	_lastValidatePhysicsManager.AddRigidbody(backgroundWall);
	_lastValidatePhysicsManager.SetRigidbody(backgroundWall, wallBody);
	_lastValidatePhysicsManager.AddAabbCollider(backgroundWall);
	_lastValidatePhysicsManager.SetAabbCollider(backgroundWall, wallCollider);

	_currentFallingObjectManager.AddComponent(backgroundWall);
	_lastValidateFallingObjectManager.AddComponent(backgroundWall);

	Rigidbody doorBody;
	doorBody.SetPosition({ doorPosition,3.0f });
	doorBody.SetDragFactor(0);
	doorBody.SetBodyType(BodyType::Kinematic);
	doorBody.SetRestitution(1.0f);
	doorBody.SetMass(10);
	doorBody.SetLayer(Layer::Door);

	AabbCollider doorCollider;
	doorCollider.halfWidth = FALLING_WALL_DOOR_SIZE.x / 2.0f;
	doorCollider.halfHeight = FALLING_WALL_DOOR_SIZE.y / 2.0f;

	_currentPhysicsManager.AddRigidbody(door);
	_currentPhysicsManager.SetRigidbody(door, doorBody);
	_currentPhysicsManager.AddAabbCollider(door);
	_currentPhysicsManager.SetAabbCollider(door, doorCollider);
	_lastValidatePhysicsManager.AddRigidbody(door);
	_lastValidatePhysicsManager.SetRigidbody(door, doorBody);
	_lastValidatePhysicsManager.AddAabbCollider(door);
	_lastValidatePhysicsManager.SetAabbCollider(door, doorCollider);

	_currentFallingObjectManager.AddComponent(door);
	_lastValidateFallingObjectManager.AddComponent(door);

	_currentFallingDoorManager.AddComponent(door);
	_currentFallingDoorManager.SetFallingDoor(door, { backgroundWall, false });
	_lastValidateFallingDoorManager.AddComponent(door);
	_lastValidateFallingDoorManager.SetFallingDoor(door, { backgroundWall, false });
}

void RollbackManager::CreateWall(const core::Entity entity, const core::Vec2f position, const core::Vec2f size,
	const Layer layer)
{
	Rigidbody wallBody;
	wallBody.SetPosition(position);
	wallBody.SetTakesGravity(false);
	wallBody.SetBodyType(BodyType::Static);
	wallBody.SetMass(std::numeric_limits<float>::max());
	wallBody.SetRestitution(1.0f);
	wallBody.SetLayer(layer);

	AabbCollider wallCollider;
	wallCollider.halfHeight = size.y;
	wallCollider.halfWidth = size.x;

	_currentPhysicsManager.AddRigidbody(entity);
	_currentPhysicsManager.SetRigidbody(entity, wallBody);
	_currentPhysicsManager.AddAabbCollider(entity);
	_currentPhysicsManager.SetAabbCollider(entity, wallCollider);

	_lastValidatePhysicsManager.AddRigidbody(entity);
	_lastValidatePhysicsManager.SetRigidbody(entity, wallBody);
	_lastValidatePhysicsManager.AddAabbCollider(entity);
	_lastValidatePhysicsManager.SetAabbCollider(entity, wallCollider);

	_currentTransformManager.AddComponent(entity);
	_currentTransformManager.SetPosition(entity, position);
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
	playerBody.SetRestitution(1.0f);
	playerBody.SetMass(10);
	playerBody.SetLayer(Layer::Player);

	CircleCollider playerCircle;
	playerCircle.radius = 0.25f;

	PlayerCharacter playerCharacter;
	playerCharacter.playerNumber = playerNumber;
	playerCharacter.hasBall = playerNumber == 0;

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

	_currentFallingObjectManager.AddComponent(entity);
	_lastValidateFallingObjectManager.AddComponent(entity);
}

PlayerInput RollbackManager::GetInputAtFrame(const PlayerNumber playerNumber, const Frame frame) const
{
	const std::size_t frameDifference = static_cast<std::size_t>(_currentFrame) - frame;

	gpr_assert(frameDifference < _inputs[playerNumber].size(),
		"Trying to get input too far in the past");
	return _inputs[playerNumber][frameDifference];
}

void RollbackManager::OnTrigger(const core::Entity, const core::Entity)
{}

void RollbackManager::OnCollision(const core::Entity entity1, const core::Entity entity2)
{
	const auto manageCollision =
		[this](const core::Entity playerEntity, const core::Entity ballEntity)
	{
		PlayerCharacter& playerCharacter = _currentPlayerManager.GetComponent(playerEntity);
		if (!playerCharacter.hasBall)
		{
			_gameManager.DestroyEntity(ballEntity);
			playerCharacter.CatchBall();
		}
	};

	const auto arePlayerAndBullet = [this](const core::Entity firstEntity, const core::Entity secondEntity)
	{
		return _entityManager.HasComponent(firstEntity, static_cast<core::EntityMask>(ComponentType::PlayerCharacter))
			&&
			_entityManager.HasComponent(secondEntity, static_cast<core::EntityMask>(ComponentType::Bullet));
	};

	if (arePlayerAndBullet(entity1, entity2))
	{
		manageCollision(entity1, entity2);
	}

	if (arePlayerAndBullet(entity2, entity1))
	{
		manageCollision(entity2, entity1);
	}
}

void RollbackManager::SpawnBall(const core::Entity entity, const core::Vec2f position, const core::Vec2f velocity)
{
	_createdEntities.push_back({ entity, _testedFrame });

	Rigidbody ballBody;
	ballBody.SetPosition(position);
	ballBody.SetVelocity(velocity);
	const auto scale = core::Vec2f::One() * BALL_SCALE;
	ballBody.Trans().scale = scale;
	ballBody.SetTakesGravity(false);
	ballBody.SetIsTrigger(false);
	ballBody.SetBodyType(BodyType::Dynamic);
	ballBody.SetLayer(Layer::Ball);

	CircleCollider ballCircle;
	ballCircle.radius = 0.25f;

	_currentBulletManager.AddComponent(entity);
	_currentBulletManager.SetComponent(entity, {});

	_currentPhysicsManager.AddRigidbody(entity);
	_currentPhysicsManager.SetRigidbody(entity, ballBody);
	_currentPhysicsManager.AddCircleCollider(entity);
	_currentPhysicsManager.SetCircleCollider(entity, ballCircle);

	_currentTransformManager.AddComponent(entity);
	_currentTransformManager.SetPosition(entity, position);
	_currentTransformManager.SetRotation(entity, core::Degree(0.0f));
}

void RollbackManager::DestroyEntity(core::Entity entity)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	// we don't need to save a bullet that has been created in the time window
	const auto predicate = [entity](auto newEntity)
	{
		return newEntity.entity == entity;
	};

	if (std::ranges::find_if(_createdEntities, predicate) != _createdEntities.end())
	{
		_entityManager.DestroyEntity(entity);
		return;
	}

	_entityManager.AddComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));
}
}
