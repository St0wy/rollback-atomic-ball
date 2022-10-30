#include "engine/engine.hpp"

#include "network/client_app.hpp"

int main()
{
	core::Engine engine;
	game::ClientApp app;
	engine.RegisterApp(&app);

	engine.Run();
	return 0;
}
