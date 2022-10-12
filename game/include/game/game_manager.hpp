#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include "game_globals.hpp"
#include "rollback_manager.hpp"

#include "engine/entity.hpp"
#include "engine/system.hpp"
#include "engine/transform.hpp"

#include "graphics/graphics.hpp"
#include "graphics/sprite.hpp"

#include "network/packet_type.hpp"

namespace game
{
class PacketSenderInterface;

/**
 * \brief GameManager is a class which manages the state of the game. It is shared between the client and the server.
 */
class GameManager
{
public:
	GameManager();
	virtual ~GameManager() = default;
	GameManager(const GameManager& other) = delete;
	GameManager(GameManager&& other) = delete;
	GameManager& operator=(const GameManager& other) = delete;
	GameManager& operator=(GameManager&& other) = delete;

	virtual void SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position, core::Degree rotation);
	virtual core::Entity SpawnBullet(PlayerNumber, core::Vec2f position, core::Vec2f velocity);
	virtual void DestroyBullet(core::Entity entity);
	[[nodiscard]] core::Entity GetEntityFromPlayerNumber(PlayerNumber playerNumber) const;
	[[nodiscard]] Frame GetCurrentFrame() const { return _currentFrame; }
	[[nodiscard]] Frame GetLastValidateFrame() const { return _rollbackManager.GetLastValidateFrame(); }
	[[nodiscard]] const core::TransformManager& GetTransformManager() const { return _transformManager; }
	[[nodiscard]] const RollbackManager& GetRollbackManager() const { return _rollbackManager; }
	virtual void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame);

	/**
	 * \brief Validate is a method called by the server to validate a frame.
	 */
	void Validate(Frame newValidateFrame);
	[[nodiscard]] PlayerNumber CheckWinner() const;
	virtual void WinGame(PlayerNumber winner);

protected:
	core::EntityManager _entityManager;
	core::TransformManager _transformManager;
	RollbackManager _rollbackManager;
	std::array<core::Entity, MAX_PLAYER_NMB> _playerEntityMap{};
	Frame _currentFrame = 0;
	PlayerNumber _winner = INVALID_PLAYER;
};

/**
 * \brief ClientGameManager is a class that inherits from GameManager by adding the visual part and specific implementations needed by the clients.
 */
class ClientGameManager final : public GameManager,
								public core::DrawInterface, public core::DrawImGuiInterface,
								public core::SystemInterface
{
public:
	enum State : std::uint32_t
	{
		Started = 1u << 0u,
		Finished = 1u << 1u,
	};

	explicit ClientGameManager(PacketSenderInterface& packetSenderInterface);
	void StartGame(unsigned long long int startingTime);
	void Begin() override;
	void Update(sf::Time dt) override;
	void End() override;
	void SetWindowSize(sf::Vector2u windowsSize);
	[[nodiscard]] sf::Vector2u GetWindowSize() const { return _windowSize; }
	void Draw(sf::RenderTarget& target) override;
	void SetClientPlayer(PlayerNumber clientPlayer);

	/**
	 * \brief SpawnPlayer is method that is called when receiving a SpawnPlayerPacket from the server.
	 * \param playerNumber is the player number to be spawned
	 * \param position is where the player character will be spawned
	 * \param rotation is the spawning angle of the player character
	 */
	void SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position, core::Degree rotation) override;
	core::Entity SpawnBullet(PlayerNumber playerNumber, core::Vec2f position, core::Vec2f velocity) override;
	void FixedUpdate();
	void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame) override;
	void DrawImGui() override;
	void ConfirmValidateFrame(Frame newValidateFrame, const std::array<PhysicsState, MAX_PLAYER_NMB>& physicsStates);
	[[nodiscard]] PlayerNumber GetPlayerNumber() const { return _clientPlayer; }
	void WinGame(PlayerNumber winner) override;
	[[nodiscard]] std::uint32_t GetState() const { return _state; }
protected:
	void UpdateCameraView();

	PacketSenderInterface& _packetSenderInterface;
	sf::Vector2u _windowSize;
	sf::View _originalView;
	sf::View _cameraView;
	PlayerNumber _clientPlayer = INVALID_PLAYER;
	core::SpriteManager _spriteManager;
	float _fixedTimer = 0.0f;
	unsigned long long _startingTime = 0;
	std::uint32_t _state = 0;

	sf::Texture _playerNoBallTexture;
	sf::Texture _ballTexture;
	sf::Font _font;

	sf::Text _textRenderer;
	bool _drawPhysics = true;
};
}
