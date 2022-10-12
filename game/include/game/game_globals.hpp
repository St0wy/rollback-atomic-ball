#pragma once
#include <array>

#include "engine/component.hpp"
#include "engine/entity.hpp"

#include "graphics/color.hpp"

#include "maths/angle.hpp"
#include "maths/vec2.hpp"


namespace game
{
/**
 * \brief PlayerNumber is a type used to define the number of the player.
 * Starting from 0 to maxPlayerNmb
 */
using PlayerNumber = std::uint8_t;

/**
 * \brief INVALID_PLAYER is an integer constant that defines an invalid player number.
 */
constexpr auto INVALID_PLAYER = std::numeric_limits<PlayerNumber>::max();

/**
 * \brief ClientId is a type used to define the client identification.
 * It is given by the server to clients.
 */
enum class ClientId : std::uint16_t {};

constexpr auto INVALID_CLIENT_ID = ClientId{0};
using Frame = std::uint32_t;

/**
 * \brief maxPlayerNmb is a integer constant that defines the maximum number of player per game
 */
constexpr std::uint32_t MAX_PLAYER_NMB = 2;
constexpr short PLAYER_HEALTH = 5;
constexpr float PLAYER_SPEED = 15.0f;
constexpr core::Degree PLAYER_ANGULAR_SPEED = core::Degree(3.0f);
constexpr float PLAYER_SHOOTING_PERIOD = 0.3f;
constexpr float BULLET_SPEED = 2.0f;
constexpr float BULLET_SCALE = 0.1f;
constexpr float BULLET_PERIOD = 3.0f;
constexpr float PLAYER_INVINCIBILITY_PERIOD = 1.5f;
constexpr float INVINCIBILITY_FLASH_PERIOD = 0.5f;

/**
 * \brief windowBufferSize is the size of input stored by a client. 5 seconds of frame at 50 fps
 */
constexpr std::size_t WINDOW_BUFFER_SIZE = 5ull * 50ull;

/**
 * \brief startDelay is the delay to wait before starting a game in milliseconds
 */
constexpr long long START_DELAY = 3000;

/**
 * \brief maxInputNmb is the number of inputs stored into an PlayerInputPacket
 */
constexpr std::size_t MAX_INPUT_NMB = 50;

/**
 * \brief fixedPeriod is the period used in seconds to start a new FixedUpdate method in the game::GameManager
 */
constexpr float FIXED_PERIOD = 0.02f; //50fps


constexpr std::array PLAYER_COLORS
{
	core::Color::Red(),
	core::Color::Blue(),
	core::Color::Yellow(),
	core::Color::Cyan()
};

constexpr std::array SPAWN_POSITIONS
{
	core::Vec2f(0, 1),
	core::Vec2f(0, -1),
	core::Vec2f(1, 0),
	core::Vec2f(-1, 0),
};

constexpr std::array SPAWN_ROTATIONS
{
	core::Degree(0.0f),
	core::Degree(180.0f),
	core::Degree(-90.0f),
	core::Degree(90.0f)
};

enum class ComponentType : core::EntityMask
{
	PlayerCharacter = static_cast<core::EntityMask>(core::ComponentType::OtherType),
	Bullet = static_cast<core::EntityMask>(core::ComponentType::OtherType) << 1u,
	Asteroid = static_cast<core::EntityMask>(core::ComponentType::OtherType) << 2u,
	PlayerInput = static_cast<core::EntityMask>(core::ComponentType::OtherType) << 3u,
	Destroyed = static_cast<core::EntityMask>(core::ComponentType::OtherType) << 4u,
};

/**
 * \brief PlayerInput is a type defining the input data from a player.
 */
using PlayerInput = std::uint8_t;

namespace player_input_enum
{
enum PlayerInput : std::uint8_t
{
	None = 0u,
	Up = 1u << 0u,
	Down = 1u << 1u,
	Left = 1u << 2u,
	Right = 1u << 3u,
	Shoot = 1u << 4u,
};
}
}
