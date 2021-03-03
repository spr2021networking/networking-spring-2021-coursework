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

	gpro-net.c
	Main source for framework.
*/

/*
* gpro-net.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: shared functions that the server and client both use.
*/

#include "gpro-net/gpro-net.h"

char* chopStr(char* in, int length, char delim)
{
	for (int i = 0; i < length; i++)
	{
		if (in[i] == delim)
		{
			in[i] = 0;
			return in + i + 1;
		}
	}
	return in;
}

void prepBitStream(RakNet::BitStream* stream, RakNet::Time time, RakNet::MessageID mType)
{
	stream->Write((RakNet::MessageID)ID_TIMESTAMP);
	stream->Write(time);
	stream->Write(mType);
}

Action Action::parseAction(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	RakNet::MessageID mID;
	RakNet::Time time;
	Action gAction;

	bsIn.Read(mID);
	bsIn.Read(time);
	bsIn.Read(mID);

	bsIn.Read(gAction);
	return gAction;
}

bool Action::setName(std::string name)
{
	return setName(name.c_str(), (int)name.length());
}

bool Action::setName(const char* name, int length)
{
	if (length < 0 || length > 16)
	{
		return false;
	}
	strncpy(checkerRoomKey, name, length);
	checkerRoomKey[length] = 0;
	return true;
}

void Action::reset(bool resetName)
{
	if (resetName)
	{
		setName("lobby");
	}
	playerIndex = winner = 0;
	startX = startY = endX = endY = capturedX = capturedY = -1;
	hasCaptured = endTurn = becomeKing = false;
}
