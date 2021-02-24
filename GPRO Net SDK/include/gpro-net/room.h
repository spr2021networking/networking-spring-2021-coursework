#pragma once

#include <vector>
#include <string>

struct Player
{
	std::string name;
	std::string address;
};

struct CheckerRoom
{
	Player player1;
	Player player2;

	std::vector<Player> spectators;
};
