#pragma once

#include <vector>
#include <string>
#include "RakNet/BitStream.h"
#include <map>
#include "chatmessage.h"
#include "RakNet/RakPeerInterface.h"

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

	static void createAndJoinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName);

	static void joinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, int defaultPlayerIndex = 1);

	static void spectateRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName);
};

struct RoomJoinInfo
{
	int playerIndex; //0 for spectator, 1 or 2 for player
	char roomID[17]; //16 char room name, plus null terminator

	bool setName(std::string name);
	bool setName(const char* name, int length);

	static RoomJoinInfo parseRoomInfo(RakNet::Packet* packet);
};
