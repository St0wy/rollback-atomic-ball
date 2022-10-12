#include "engine/engine.hpp"

#include "network/simulation_app.hpp"

int main()
{

	core::Engine engine;
	game::SimulationApp app;
	engine.RegisterApp(&app);

	engine.Run();
	return 0;
}
