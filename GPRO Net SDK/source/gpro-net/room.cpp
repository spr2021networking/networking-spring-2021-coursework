/*
* room.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: Information about how a room is structured, mostly used by server
*/


#include "gpro-net/room.h"
#include "gpro-net/gpro-net.h"
#include "RakNet/GetTime.h"

//set name and null terminate
bool RoomJoinInfo::setName(std::string name)
{
	return setName(name.c_str(), (int)name.length());
}

//set name and null terminate
bool RoomJoinInfo::setName(const char* name, int length)
{
	int maxLen = 16;
	if (length > maxLen)
	{
		return false;
	}

	if (length > 0)
	{
		strncpy(roomID, name, length);
	}
	roomID[length] = 0;
	return true;
}

//similar to parseMessage, reads in a RoomJoinInfo from a packet
RoomJoinInfo RoomJoinInfo::parseRoomInfo(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	RakNet::MessageID mID;
	RakNet::Time time;
	RoomJoinInfo r;

	//skip the bytes that need to be sent before this m is sent
	bsIn.Read(mID);
	bsIn.Read(time);
	bsIn.Read(mID);

	bsIn.Read(r);
	return r;
}

/// <summary>
/// Tries to create a room and then join it
/// </summary>
/// <param name="roomStorage"></param>
/// <param name="nameLookup"></param>
/// <param name="peer"></param>
/// <param name="packet"></param>
/// <param name="roomName"></param>
/// <returns></returns>
bool CheckerRoom::createAndJoinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName)
{
	RakNet::BitStream outStream;

	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom == roomStorage->end()) //not found, this is a new room
	{
		CheckerRoom room;
		room.name = roomName;
		(*roomStorage)[roomName] = room;

		return joinRoom(roomStorage, nameLookup, peer, packet, roomName, 2);
	}
	else //failed to create/join room, room already exists so we send an error.
	{
		ChatMessage response;
		response.isTimestamp = ID_TIMESTAMP;
		response.id2 = ID_MESSAGE_STRUCT;
		response.messageFlag = PUBLIC;
		response.setText(SENDER, "[System]");
		response.setText(MESSAGE, "Room " + roomName + " already exists.");
		response.setText(RECIPIENT, (*nameLookup)[packet->systemAddress.ToString()]);

		prepBitStream(&outStream, RakNet::GetTime());
		outStream.Write(response);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		return false;
	}
}

// Try to join an existing room either as a player or spectator. See room.h for more information
bool CheckerRoom::joinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, int defaultPlayerIndex)
{
	RakNet::BitStream outStream;
	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom != roomStorage->end()) //room exists
	{
		CheckerRoom* room = &(foundRoom->second);

		std::string address = packet->systemAddress.ToString();
		std::string name = (*nameLookup)[address];

		RoomJoinInfo joinInfo;
		joinInfo.setName(roomName);

		switch (defaultPlayerIndex) //check default index and try to add player to room in the right index
		{
		case 1: //if we're requesting player 1, we first try to enter player 1, if not we try for player 2. If both fail, we spectate
			if (room->player1.name == "")
			{
				room->player1.address = address;
				room->player1.name = name;
				joinInfo.playerIndex = 1;
			}
			else if (room->player2.name == "")
			{
				room->player2.address = address;
				room->player2.name = name;
				joinInfo.playerIndex = 2;
			}
			else
			{
				return spectateRoom(roomStorage, nameLookup, peer, packet, roomName);
			}
			break;
		case 2: //if we're requesting player 2, we first try to enter player 2, if not we try for player 1. If both fail, we spectate
			if (room->player2.name == "")
			{
				room->player2.address = address;
				room->player2.name = name;
				joinInfo.playerIndex = 2;
			}
			else if (room->player1.name == "")
			{
				room->player1.address = address;
				room->player1.name = name;
				joinInfo.playerIndex = 1;
			}
			else
			{
				return spectateRoom(roomStorage, nameLookup, peer, packet, roomName);
			}
			break;
		case 0: //auto spectate
			return spectateRoom(roomStorage, nameLookup, peer, packet, roomName);
		}
	
		//not spectating at this point
		prepBitStream(&outStream, RakNet::GetTime(), ID_JOIN_ROOM);
		outStream.Write(joinInfo);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		return true;
	}
	else //error, room doesn't exist
	{
		ChatMessage response;
		response.isTimestamp = ID_TIMESTAMP;
		response.id2 = ID_MESSAGE_STRUCT;
		response.messageFlag = PUBLIC;
		response.setText(SENDER, "[System]");
		response.setText(MESSAGE, "Room " + roomName + " does not exist.");
		response.setText(RECIPIENT, (*nameLookup)[packet->systemAddress.ToString()]);

		prepBitStream(&outStream, RakNet::GetTime());
		outStream.Write(response);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		return false;
	}
}

/// <summary>
/// Code specifically for spectating
/// </summary>
/// <param name="roomStorage"></param>
/// <param name="nameLookup"></param>
/// <param name="peer"></param>
/// <param name="packet"></param>
/// <param name="roomName"></param>
/// <returns></returns>
bool CheckerRoom::spectateRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName)
{
	RakNet::BitStream outStream;
	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom != roomStorage->end()) //room exists
	{
		CheckerRoom* room = &(foundRoom->second);
		if (!room->closed) //if game hasn't started yet, add to spectator list
		{
			std::string address = packet->systemAddress.ToString();
			std::string name = (*nameLookup)[room->player1.address];

			Player player;
			player.address = address;
			player.name = name;
			room->spectators.push_back(player);

			RoomJoinInfo joinInfo;
			joinInfo.setName(roomName);
			joinInfo.playerIndex = 0;

			//not spectating at this point
			prepBitStream(&outStream, RakNet::GetTime(), ID_JOIN_ROOM);
			outStream.Write(joinInfo);
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			return true;
		}
		else //error, game is in progress already
		{
			ChatMessage response;
			response.isTimestamp = ID_TIMESTAMP;
			response.id2 = ID_MESSAGE_STRUCT;
			response.messageFlag = PUBLIC;
			response.setText(SENDER, "[System]");
			response.setText(MESSAGE, "Room " + roomName + " is closed.");
			response.setText(RECIPIENT, (*nameLookup)[packet->systemAddress.ToString()]);

			prepBitStream(&outStream, RakNet::GetTime());
			outStream.Write(response);
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			return false;
		}

	}
	else //error, room doesn't exist
	{
		ChatMessage response;
		response.isTimestamp = ID_TIMESTAMP;
		response.id2 = ID_MESSAGE_STRUCT;
		response.messageFlag = PUBLIC;
		response.setText(SENDER, "[System]");
		response.setText(MESSAGE, "Room " + roomName + " does not exist.");
		response.setText(RECIPIENT, (*nameLookup)[packet->systemAddress.ToString()]);

		prepBitStream(&outStream, RakNet::GetTime());
		outStream.Write(response);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		return false;
	}
}

/// <summary>
/// Handle leaving/conceding a match. Outputs two variables playerLeft and winner
/// </summary>
/// <param name="roomStorage"></param>
/// <param name="nameLookup"></param>
/// <param name="peer"></param>
/// <param name="packet"></param>
/// <param name="roomName"></param>
/// <param name="playerLeft"></param>
/// <param name="winner"></param>
/// <returns></returns>
bool CheckerRoom::leaveRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, bool* playerLeft, int* winner)
{
	RakNet::BitStream outStream;
	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom != roomStorage->end()) //room exists
	{
		CheckerRoom* room = &(foundRoom->second);

		std::string address = packet->systemAddress.ToString();
		std::string name = (*nameLookup)[address];

		RoomJoinInfo joinInfo;
		joinInfo.setName("lobby");
		joinInfo.playerIndex = 0;

		if (strncmp(name.c_str(), room->player1.name.c_str(), name.length()) == 0)
		{
			if (room->readyToPlay()) //the room had two players, then leaving can cause a forfeit
			{
				*playerLeft = true;
				*winner = 2;
			}
			room->player1.address = "";
			room->player1.name = "";

		}
		else if (strncmp(name.c_str(), room->player2.name.c_str(), name.length()) == 0)
		{
			if (room->readyToPlay()) //the room had two players
			{
				*playerLeft = true;
				*winner = 1;
			}
			room->player2.address = "";
			room->player2.name = "";

		}
		else //remove from spectators
		{
			for (int i = 0; i < (int)room->spectators.size(); i++)
			{
				if (strncmp(name.c_str(), room->spectators[i].name.c_str(), room->spectators[i].name.length()) == 0)
				{
					room->spectators.erase(room->spectators.begin() + i);
				}
			}
		}

		prepBitStream(&outStream, RakNet::GetTime(), ID_JOIN_ROOM);
		outStream.Write(joinInfo);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

		return true;
	}
	return false;
}

//checks if the players are both present (names aren't empty)
bool CheckerRoom::readyToPlay()
{
	return player2.name.length() > 0 && player1.name.length() > 0;
}
