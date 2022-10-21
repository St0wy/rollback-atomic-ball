#include "engine/engine.hpp"

#include "network/simulation_app.hpp"

int main()
{

	core::Engine engine(game::DEBUG_WINDOW_SIZE);
	game::SimulationApp app;
	engine.RegisterApp(&app);

	engine.Run();
	return 0;
}
