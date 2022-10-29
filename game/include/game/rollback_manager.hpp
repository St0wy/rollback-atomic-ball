#pragma once
#include "ball_manager.hpp"
#include "falling_wall_manager.hpp"
#include "game_globals.hpp"
#include "player_character.hpp"

#include "engine/entity.hpp"
#include "engine/transform.hpp"

#include "network/packet_type.hpp"

#include "physics/physics_manager.hpp"
#include "damage_manager.hpp"
#include "score_manager.hpp"

namespace game
{
class GameManager;

/**
 * \brief CreatedEntity is a struct that contains information on the newly created entities.
 * It is used by the RollbackManager to destroy newly created entities when going back in time.
 */
struct CreatedEntity
{
	core::Entity entity = core::INVALID_ENTITY;
	Frame createdFrame = 0;
};

/**
 * \brief RollbackManager is a class that manages all the rollback mechanisms of the game.
 * It contains two copies of the world (PhysicsManager, TransformManager, etc...), the current one and the validated one.
 * When receiving new information, it can re-update the current copy of the world.
 */
class RollbackManager final : public OnTriggerInterface, OnCollisionInterface
{
public:
	RollbackManager(GameManager& gameManager, core::EntityManager& entityManager);

	/**
	 * \brief SimulateToCurrentFrame is a method that simulates all players with new inputs, method call only by the clients to update the current state of the visuals
	 */
	void SimulateToCurrentFrame();

	/**
	 * \brief SetPlayerInput is a method that set the input of a certain player on a certain game frame.
	 * It can change an input between the last validated frame and the current frame.
	 * It is called by the GameManager when receiving new inputs from packets.
	 * \param playerNumber is the player number whose input will change
	 * \param playerInput is the new input
	 * \param inputFrame is the game frame of the new input
	 */
	void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, Frame inputFrame);
	void StartNewFrame(Frame newFrame);

	/**
	 * \brief ValidateFrame is a method that validates all the frames from lastValidateFrame_ to newValidateFrame.
	 * It changes lastValidateFrame_ to be newValidateFrame.
	 * \param newValidateFrame is the new value of lastValidateFrame_
	 */
	void ValidateFrame(Frame newValidateFrame);

	/**
	 * \brief ConfirmFrame is a method that confirms the new validate frame by checking the Physics State checksum
	 * It is called by the clients when receiving Confirm Frame packet
	 * \param newValidatedFrame is the new frame that is validated
	 * \param serverPhysicsState is the physics state given by the server through a packet
	 */
	void ConfirmFrame(Frame newValidatedFrame, const std::array<PhysicsState, MAX_PLAYER_NMB>& serverPhysicsState);

	[[nodiscard]] PhysicsState GetValidatePhysicsState(PlayerNumber playerNumber) const;
	[[nodiscard]] Frame GetLastValidateFrame() const { return _lastValidateFrame; }

	[[nodiscard]] Frame GetLastReceivedFrame(const PlayerNumber playerNumber) const
	{
		return _lastReceivedFrame[playerNumber];
	}

	[[nodiscard]] Frame GetCurrentFrame() const { return _currentFrame; }
	[[nodiscard]] const core::TransformManager& GetTransformManager() const { return _currentTransformManager; }
	[[nodiscard]] const PlayerCharacterManager& GetPlayerCharacterManager() const { return _currentPlayerManager; }
	[[nodiscard]] const ScoreManager& GetScoreManager() const { return _currentScoreManager; }
	void SetupLevel(core::Entity wallLeftEntity, core::Entity wallRightEntity, core::Entity wallMiddleEntity,
		core::Entity wallBottomEntity, core
		::Entity wallTopEntity);
	void SpawnFallingWall(core::Entity backgroundWall, core::Entity door, float doorPosition, bool requiresBall);
	void CreateWall(core::Entity entity, core::Vec2f position, core::Vec2f size, Layer layer = Layer::Wall);
	void SpawnPlayer(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Degree rotation);
	void SpawnBall(core::Entity entity, core::Vec2f position, core::Vec2f velocity);

	/**
	 * \brief DestroyEntity is a method that does not destroy the entity definitely, but puts the DESTROY flag on.
	 * An entity is truly destroyed when the destroy frame is validated.
	 * \param entity is the entity to be "destroyed"
	 */
	void DestroyEntity(core::Entity entity);

	void OnTrigger(core::Entity entity1, core::Entity entity2) override;
	void OnCollision(core::Entity entity1, core::Entity entity2) override;

	[[nodiscard]] const std::array<PlayerInput, WINDOW_BUFFER_SIZE>& GetInputs(const PlayerNumber playerNumber) const
	{
		return _inputs[playerNumber];
	}

	PhysicsManager& GetCurrentPhysicsManager() { return _currentPhysicsManager; }

	void SetNextFallingWallSpawnInstructions(const FallingWallSpawnInstructions fallingWallSpawnInstructions)
	{
		_currentFallingWallSpawnManager.SetNextFallingWallSpawnInstructions(fallingWallSpawnInstructions);
		_lastValidateFallingWallSpawnManager.SetNextFallingWallSpawnInstructions(fallingWallSpawnInstructions);
	}

	[[nodiscard]] FallingWallSpawnInstructions GetNextFallingWallSpawnInstructions() const { return _currentFallingWallSpawnManager.GetNextFallingWallSpawnInstructions(); }

	void SetTextTEMP(const std::string_view name) { _currentFallingWallSpawnManager.name = name; _lastValidateFallingWallSpawnManager.name = name; }

private:
	[[nodiscard]] PlayerInput GetInputAtFrame(PlayerNumber playerNumber, Frame frame) const;

	GameManager& _gameManager;
	core::EntityManager& _entityManager;

	/**
	 * \brief Used for rendering
	 */
	core::TransformManager _currentTransformManager;
	PhysicsManager _currentPhysicsManager;
	PlayerCharacterManager _currentPlayerManager;
	BallManager _currentBulletManager;
	FallingObjectManager _currentFallingObjectManager;
	FallingDoorManager _currentFallingDoorManager;
	DamageManager _currentDamageManager;
	FallingWallSpawnManager _currentFallingWallSpawnManager;
	ScoreManager _currentScoreManager{};

	/**
	 * Last Validate (confirm frame) Component Managers used for rollback
	 */
	PhysicsManager _lastValidatePhysicsManager;
	PlayerCharacterManager _lastValidatePlayerManager;
	BallManager _lastValidateBulletManager;
	FallingObjectManager _lastValidateFallingObjectManager;
	FallingDoorManager _lastValidateFallingDoorManager;
	DamageManager _lastValidateDamageManager;
	FallingWallSpawnManager _lastValidateFallingWallSpawnManager;
	ScoreManager _lastValidateScoreManager{};

	/**
	 * \brief lastValidateFrame_ is the last validated frame from the server side.
	 */
	Frame _lastValidateFrame = 0;

	/**
	 * \brief currentFrame_ is the current frame on the client side.
	 */
	Frame _currentFrame = 0;

	/**
	 * \brief testedFrame_ is the current simulated frame used mainly for entity creation and collision.
	 */
	Frame _testedFrame = 0;

	std::array<std::uint32_t, MAX_PLAYER_NMB> _lastReceivedFrame{};
	std::array<std::array<PlayerInput, WINDOW_BUFFER_SIZE>, MAX_PLAYER_NMB> _inputs{};

	/**
	 * \brief Array containing all the created entities in the window between the confirm frame and the current frame
	 * to destroy them when doing a rollback.
	 */
	std::vector<CreatedEntity> _createdEntities;
};
}
