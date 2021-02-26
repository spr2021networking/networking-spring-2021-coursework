#pragma once

#include <vector>
#include <string>
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"

struct Player
{
	std::string name;
	std::string address;
};

struct CheckerRoom
{
	std::string name;
	Player player1;
	Player player2;

	std::vector<Player> spectators;
};

struct RoomJoinInfo
{
	int playerIndex; //0 for spectator, 1 or 2 for player
	char roomID[17]; //16 char room name, plus null terminator

	bool setName(std::string name);
	bool setName(const char* name, int length);

	static RoomJoinInfo parseMessage(RakNet::Packet* packet);
};
