#pragma once

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>

#include "globals.hpp"
#include "engine/app.hpp"

namespace core
{
/**
 * \brief Engine is a class that manages the layer between the system and the application and runs the game loop.
 */
class Engine
{
public:
	Engine() = default;
	explicit Engine(sf::Vector2u size);

	/**
	 * \brief Run is a method that runs the Engine game loop.
	 */
	void Run();

	void RegisterApp(App* app);
	void RegisterSystem(SystemInterface*);
	void RegisterOnEvent(OnEventInterface*);
	void RegisterDraw(DrawInterface*);
	void RegisterDrawImGui(DrawImGuiInterface*);
protected:
	void Init();
	void Update(sf::Time dt) const;
	void Destroy();

	std::vector<SystemInterface*> _systems;
	std::vector<OnEventInterface*> _eventInterfaces;
	std::vector<DrawInterface*> _drawInterfaces;
	std::vector<DrawImGuiInterface*> _drawImGuiInterfaces;
	std::unique_ptr<sf::RenderWindow> _window;
	sf::Vector2u _windowSize = WINDOW_SIZE;
};
} // namespace core
