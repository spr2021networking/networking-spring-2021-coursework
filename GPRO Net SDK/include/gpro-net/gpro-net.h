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


#endif	// !_GPRO_NET_H_