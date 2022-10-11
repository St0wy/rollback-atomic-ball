#include <string>

#include <SFML/System/Clock.hpp>

#include "network/network_server.hpp"

int main(const int argc, char** argv)
{
	unsigned short port = 0;
	if (argc == 2)
	{
		const std::string portArg = argv[1];
		port = static_cast<unsigned short>(std::stoi(portArg));
	}

	game::NetworkServer server;
	if (port != 0)
	{
		server.SetTcpPort(port);
	}

	server.Begin();
	sf::Clock clock;
	while (server.IsOpen())
	{
		const auto dt = clock.restart();
		server.Update(dt);
	}

	return 0;
}
