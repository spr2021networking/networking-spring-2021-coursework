/*
* room.h
* Contributors: Ben Cooper and Scott Dagen
* Contributions: Information about how a room is structured, mostly used by server
*/

#pragma once

#include <vector>
#include <string>
#include "RakNet/BitStream.h"
#include <map>
#include "chatmessage.h"
#include "RakNet/RakPeerInterface.h"

//wrapper for name + address
struct Player
{
	std::string name;
	std::string address;
};

//contains the players, its own name, and a list of spectators
struct CheckerRoom
{
	std::string name;
	Player player1;
	Player player2;

	bool closed = false; //prevent new spectators as the game is in progress

	std::vector<Player> spectators;

	//utility functions for how to create, join, spectate, or leave a room. Returns whether the room exists
	static bool createAndJoinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName);

	static bool joinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, int defaultPlayerIndex = 1);

	static bool spectateRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName);

	static bool leaveRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, bool* playerLeft, int* winner);

	//checks if the players are both present
	bool readyToPlay();
};

/// <summary>
/// A message that can be sent to players telling them what room they've been added to. Also used to join the lobby.
/// </summary>
struct RoomJoinInfo
{
	int playerIndex; //0 for spectator, 1 or 2 for player
	char roomID[17]; //16 char room name, plus null terminator

	bool setName(std::string name);
	bool setName(const char* name, int length);

	static RoomJoinInfo parseRoomInfo(RakNet::Packet* packet);
};
