#include "engine/engine.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "engine/globals.hpp"
#include "engine/system.hpp"

#include "graphics/graphics.hpp"

#include "utils/assert.hpp"
#include "utils/log.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace core
{
void Engine::Run()
{
	Init();
	sf::Clock clock;
	while (_window->isOpen())
	{
		try
		{
			const auto dt = clock.restart();
			Update(dt);
			#ifdef TRACY_ENABLE
			FrameMark;
			#endif
		}
		catch ([[maybe_unused]] const AssertException& e)
		{
			LogError("Exit with exception");
			_window->close();
		}
	}
	Destroy();
}

void Engine::RegisterApp(App* app)
{
	RegisterSystem(app);
	RegisterDraw(app);
	RegisterOnEvent(app);
	RegisterDrawImGui(app);
}

void Engine::RegisterSystem(SystemInterface* system)
{
	_systems.push_back(system);
}

void Engine::RegisterOnEvent(OnEventInterface* onEventInterface)
{
	_eventInterfaces.push_back(onEventInterface);
}

void Engine::RegisterDraw(DrawInterface* drawInterface)
{
	_drawInterfaces.push_back(drawInterface);
}

void Engine::RegisterDrawImGui(DrawImGuiInterface* drawImGuiInterface)
{
	_drawImGuiInterfaces.push_back(drawImGuiInterface);
}

void Engine::Init()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	_window = std::make_unique<sf::RenderWindow>(
		sf::VideoMode(WINDOW_SIZE.x, WINDOW_SIZE.y),
		"Rollback Game");
	const bool status = ImGui::SFML::Init(*_window);
	if (!status)
	{
		LogError("Could not init ImGui-SFML");
	}
	for (auto* system : _systems)
	{
		system->Begin();
	}
}

void Engine::Update(const sf::Time dt) const
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	sf::Event e{};
	while (_window->pollEvent(e))
	{
		ImGui::SFML::ProcessEvent(e);
		switch (e.type)
		{
		case sf::Event::Closed:
			_window->close();
			break;
		case sf::Event::Resized:
		{
			const auto width = static_cast<float>(e.size.width);
			const auto height = static_cast<float>(e.size.height);
			sf::FloatRect visibleArea(0, 0, width, height);
			_window->setView(sf::View(visibleArea));
			break;
		}
		default:
			break;
		}

		for (auto* eventInterface : _eventInterfaces)
		{
			eventInterface->OnEvent(e);
		}
	}

	for (auto* system : _systems)
	{
		system->Update(dt);
	}

	ImGui::SFML::Update(*_window, dt);
	_window->clear(sf::Color::Black);

	for (auto* drawInterface : _drawInterfaces)
	{
		drawInterface->Draw(*_window);
	}

	for (auto* drawImGuiInterface : _drawImGuiInterfaces)
	{
		drawImGuiInterface->DrawImGui();
	}

	ImGui::SFML::Render(*_window);

	_window->display();
}

void Engine::Destroy()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	for (auto* system : _systems)
	{
		system->End();
	}

	_window = nullptr;
}
} // namespace core
