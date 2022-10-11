#include <engine/engine.hpp>

#include "network/network_client_debug_app.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::Engine engine;
    game::NetworkClientDebugApp app;
    engine.RegisterApp(&app);

    engine.Run();

    return EXIT_SUCCESS;
}
