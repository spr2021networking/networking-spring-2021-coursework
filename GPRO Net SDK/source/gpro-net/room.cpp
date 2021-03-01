#include "gpro-net/room.h"
#include "gpro-net/gpro-net.h"
#include "RakNet/GetTime.h"

bool RoomJoinInfo::setName(std::string name)
{
	return setName(name.c_str(), (int)name.length());
}

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

void CheckerRoom::createAndJoinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName)
{
	RakNet::BitStream outStream;

	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom == roomStorage->end()) //not found, this is a new room
	{
		CheckerRoom room;
		room.name = roomName;
		room.player2.address = packet->systemAddress.ToString();
		room.player2.name = (*nameLookup)[room.player2.address];
		(*roomStorage)[roomName] = room;

		RoomJoinInfo joinInfo;
		joinInfo.setName(roomName);
		joinInfo.playerIndex = 2;

		prepBitStream(&outStream, RakNet::GetTime(), ID_JOIN_ROOM);
		outStream.Write(joinInfo);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	}
	else
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
	}
}

void CheckerRoom::joinRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName, int defaultPlayerIndex)
{
	RakNet::BitStream outStream;
	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom != roomStorage->end()) //room exists
	{
		CheckerRoom* room = &(foundRoom->second);

		std::string address = packet->systemAddress.ToString();
		std::string name = (*nameLookup)[room->player1.address];

		RoomJoinInfo joinInfo;
		joinInfo.setName(roomName);

		switch (defaultPlayerIndex)
		{
		case 1:
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
				spectateRoom(roomStorage, nameLookup, peer, packet, roomName);
				return;
			}
			break;
		case 2:
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
				spectateRoom(roomStorage, nameLookup, peer, packet, roomName);
				return;
			}
			break;
		case 0:
			spectateRoom(roomStorage, nameLookup, peer, packet, roomName);
			return;
		}
	
		//not spectating at this point
		prepBitStream(&outStream, RakNet::GetTime(), ID_JOIN_ROOM);
		outStream.Write(joinInfo);
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	}
	else
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
	}
}

void CheckerRoom::spectateRoom(std::map<std::string, CheckerRoom>* roomStorage, std::map<std::string, std::string>* nameLookup,
	RakNet::RakPeerInterface* peer, RakNet::Packet* packet, std::string roomName)
{
	RakNet::BitStream outStream;
	std::map<std::string, CheckerRoom>::iterator foundRoom = roomStorage->find(roomName);
	if (foundRoom != roomStorage->end()) //room exists
	{
		CheckerRoom* room = &(foundRoom->second);

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
	}
	else
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
	}
}