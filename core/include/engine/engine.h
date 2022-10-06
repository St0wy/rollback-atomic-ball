#pragma once

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>

#include "engine/app.h"

namespace core
{
/**
 * \brief Engine is a class that manages the layer between the system and the application and runs the game loop.
 */
class Engine
{
public:
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
};
} // namespace core
