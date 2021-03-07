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

	/// <summary>
	/// Try to create a room with the specified name, and join the room after
	/// </summary>
	/// <param name="roomStorage">The dictionary storing all rooms</param>
	/// <param name="nameLookup">Lookup for player names, used for configuring the room</param>
	/// <param name="peer">The RakPeerInterface used to send the join packet</param>
	/// <param name="packet">The packet that contained the command calling this function</param>
	/// <param name="roomName">The name for the new room</param>
	/// <returns>Whether the room was created (whether it didn't already exist)</returns>
	static bool createAndJoinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName);

	/// <summary>
	/// Try to join a room with the specified name. Will spectate if all players are present
	/// </summary>
	/// <param name="roomStorage">The dictionary storing all rooms</param>
	/// <param name="nameLookup">Lookup for player names, used for configuring the room</param>
	/// <param name="peer">The RakPeerInterface used to send the join packet</param>
	/// <param name="packet">The packet that contained the command calling this function</param>
	/// <param name="roomName">The room to join</param>
	/// <returns>Whether the room was joined</returns>
	static bool joinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, int defaultPlayerIndex = 1);

	/// <summary>
	/// Try to spectate a room with the specified name
	/// </summary>
	/// <param name="roomStorage">The dictionary storing all rooms</param>
	/// <param name="nameLookup">Lookup for player names, used for configuring the room</param>
	/// <param name="peer">The RakPeerInterface used to send the join packet</param>
	/// <param name="packet">The packet that contained the command calling this function</param>
	/// <param name="roomName">The room to join</param>
	/// <returns>Whether the room was joined</returns>
	static bool spectateRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
		RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName);

	/// <summary>
	/// Leave room with the specified name
	/// </summary>
	/// <param name="roomStorage">The dictionary storing all rooms</param>
	/// <param name="nameLookup">Lookup for player names, used for configuring the room</param>
	/// <param name="peer">The RakPeerInterface used to send the leave packet</param>
	/// <param name="packet">The packet that contained the command calling this function</param>
	/// <param name="roomName">The room to leave</param>
	/// <param name="playerLeft">Whether the leaving user was a player</param>
	/// <param name="winner">If the player left, who won as a result of the forfeit</param>
	/// <returns>Whether the room was left</returns>
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

	//assign a value to roomID, with null termination
	bool setName(std::string name);
	bool setName(const char* name, int length);

	//retrieve a roomInfo from a correctly configured packet
	static RoomJoinInfo parseRoomInfo(RakNet::Packet* packet);
};
