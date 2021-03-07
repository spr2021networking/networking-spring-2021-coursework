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

/// <summary>
/// Splits a cstring by a delimiter, with a maxmimum length. Returns the start point of the new string.
/// You can check if a string contains a character by comparing the return value with the in value. If they're equal,
/// no operation occurred.
/// WARNING: this function does null-terminate the input string, like strtok().
/// </summary>
/// <param name="in">The string to input</param>
/// <param name="length">How long the input string is (or how far to scan)</param>
/// <param name="delim">The character to scan for</param>
/// <returns></returns>
char* chopStr(char* in, int length, char delim);

enum GameMessages //extends Raknet's message IDs
{
	ID_USERNAME = ID_USER_PACKET_ENUM + 1,
	ID_RECEIVE_MESSAGE,
	ID_MESSAGE_STRUCT, //can carry chat messages or commands, including telling the server to let the player into a room
	ID_KICK,
	ID_GAMEMESSAGE,
	ID_GAMEMESSAGE_STRUCT, //carries actions
	ID_JOIN_ROOM //goes from server to client, passes in a room name and a player index (0 for spectator, 1 or 2 for player)
};

/// <summary>
/// sets the first several bytes of the bitstream so the timestamp is properly recognized.
/// </summary>
/// <param name="stream">The stream to prepare</param>
/// <param name="time">The timestamp</param>
/// <param name="mType">What the "true" message ID is.</param>
void prepBitStream(RakNet::BitStream* stream, RakNet::Time time, RakNet::MessageID mType = ID_MESSAGE_STRUCT);

#pragma pack(push, 1)
typedef struct Action Action;  //action struct stores all information about moves and states for checkers
struct Action
{
	char checkerRoomKey[17] = "lobby\0"; //the room this action pertains to
	char playerIndex; //what player we are
	char startX, startY; //start pos
	char endX, endY; //end pos

	bool hasCaptured = false; //determines if a piece is captured
	char capturedX, capturedY; //where the piece was captured

	bool endTurn = false; //ends the turn
	bool becomeKing = false; //set a piece as king

	int winner = 0; //set the winner

	bool readyToPlay = false; //only let the player make a move if ready

	/// <summary>
	/// Reads an Action from a RakNet::Packet
	/// </summary>
	/// <param name="packet"></param>
	/// <returns></returns>
	static Action parseAction(RakNet::Packet* packet);

	//Sets the name of the room this action should be sent to
	bool setName(std::string name);
	bool setName(const char* name, int length);

	//Resets most values in the Action, including the name if requested
	void reset(bool resetName = false);
};
#pragma pack(pop)

#endif	// !_GPRO_NET_H_