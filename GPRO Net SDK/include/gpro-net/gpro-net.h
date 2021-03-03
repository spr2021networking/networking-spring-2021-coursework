/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein

	gpro-net.h
	Main include for framework.
*/

/*
* gpro-net.h
* Contributors: Ben Cooper and Scott Dagen
* Contributions: shared functions that the server and client both use.
*/

#ifndef _GPRO_NET_H_
#define _GPRO_NET_H_

#include <stdlib.h>
#include "RakNet/RakString.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"
#include "RakNet/MessageIdentifiers.h"
#include "chatmessage.h"
#include "room.h"
#include "textbox.h"

char* chopStr(char* in, int length, char delim);

enum GameMessages
{
	ID_USERNAME = ID_USER_PACKET_ENUM + 1,
	ID_RECEIVE_MESSAGE,
	ID_MESSAGE_STRUCT, //can carry chat messages or commands, including telling the server to let the player into a room
	ID_KICK,
	ID_GAMEMESSAGE,
	ID_GAMEMESSAGE_STRUCT,
	ID_JOIN_ROOM //goes from server to client, passes in a room name and a player index (0 for spectator, 1 or 2 for player)
};

/// <summary>
/// sets the first several bytes of the bitstream so the timestamp is properly recognized.
/// </summary>
/// <param name="stream"></param>
/// <param name="time"></param>
/// <param name="mType"></param>
void prepBitStream(RakNet::BitStream* stream, RakNet::Time time, RakNet::MessageID mType = ID_MESSAGE_STRUCT);

#pragma pack(push, 1)
typedef struct Action Action;
struct Action
{
	char checkerRoomKey[17] = "lobby\0";
	char playerIndex;
	char startX, startY;
	char endX, endY;

	bool hasCaptured = false;
	char capturedX, capturedY;

	bool endTurn = false;
	bool becomeKing = false;

	static Action parseAction(RakNet::Packet* packet);
	bool setName(std::string name);
	bool setName(const char* name, int length);
	void reset(bool resetName = false);
};
#pragma pack(pop)

#endif	// !_GPRO_NET_H_