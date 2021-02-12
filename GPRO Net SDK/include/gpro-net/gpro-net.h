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

#ifndef _GPRO_NET_H_
#define _GPRO_NET_H_

#include <stdlib.h>
#include "RakNet/RakString.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"
#include "RakNet/MessageIdentifiers.h"

char* chopStr(char* in, int length, char delim);

#pragma pack(push, 1)
typedef struct ChatMessage ChatMessage;
struct ChatMessage
{
	RakNet::MessageID isTimestamp; //currently unused, but indicates that this has ID_TIMESTAMP
	RakNet::Time time; //the time this was sent
	RakNet::MessageID id2; //the actual message type
	int messageType; //see MessageFlag
	char recipient[17]; //16 bytes of input + null terminator
	char sender[17]; //16 bytes of input + null terminator
	char message[129]; //128 bytes of input + null terminator
};
#pragma pack(pop)

/// <summary>
/// Extract a ChatMessage from packet data, assuming it was formatted properly (using prepBitStream)
/// </summary>
/// <param name="packet"></param>
/// <returns></returns>
ChatMessage parseMessage(RakNet::Packet* packet);

enum GameMessages
{
	ID_USERNAME = ID_USER_PACKET_ENUM + 1,
	ID_RECEIVE_MESSAGE,
	ID_MESSAGE_STRUCT,
	ID_KICK
};

enum MessageFlag
{
	PUBLIC = 0,
	PRIVATE,
	COMMAND,
	ISADMIN = 4 // set to 4 as a flag. 4 technically means "public admin", 5 = "private admin", 6 = "command admin"
};

/// <summary>
/// sets the first several bytes of the bitstream so the timestamp is properly recognized.
/// </summary>
/// <param name="stream"></param>
/// <param name="time"></param>
/// <param name="mType"></param>
void prepBitStream(RakNet::BitStream* stream, RakNet::Time time, RakNet::MessageID mType = ID_MESSAGE_STRUCT);

#endif	// !_GPRO_NET_H_