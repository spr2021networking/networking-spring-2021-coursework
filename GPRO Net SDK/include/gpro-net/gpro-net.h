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

#pragma pack(push, 1)
typedef struct ChatMessage ChatMessage;
struct ChatMessage
{
	RakNet::MessageID isTimestamp;
	RakNet::Time time;
	RakNet::MessageID id2;
	int messageType; //public = 0; private = 1; command = 2
	char recipient[17];
	char message[129];
};
#pragma pack(pop)

ChatMessage parseMessage(RakNet::Packet* packet)
{
	//RakNet::MessageID message2;
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	RakNet::MessageID tmp;
	bsIn.Read(tmp);
	RakNet::Time tm;
	bsIn.Read(tm);
	bsIn.Read(tmp);
	//char vals[sizeof(RakNet::Time)];
	//bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	//bsIn.Read(vals, sizeof(RakNet::Time));
	//RakNet::Time time = *(RakNet::Time*)&vals;
	//bsIn.IgnoreBytes(sizeof(RakNet::Time) + sizeof(RakNet::MessageID));
	//bsIn.Read(time);
	float betterTime = (float)tm;
	float seconds = betterTime / 1000.0f;
	float minutes = seconds / 60.0f;
	float hour = minutes / 60.0f;
	int hourVal = (int)hour % 12 + 1;
	int minutesInt = (int)((hour - (int)hour) * 60);
	printf("%d:%d\n", hourVal, minutesInt + 20);
	//bsIn.IgnoreBytes(sizeof(RakNet::Time) + sizeof(RakNet::MessageID));
	//bsIn.Read(message2);
	ChatMessage m;// = (ChatMessage*)bsIn.GetData();
	bsIn.Read(m);
	return m;
}


#endif	// !_GPRO_NET_H_