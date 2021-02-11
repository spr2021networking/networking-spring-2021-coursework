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

	main-client.c/.cpp
	Main source for console client application.
*/

#include "gpro-net/gpro-net.h"

#include <cstdio>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <stdio.h>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"  // MessageID
#include "RakNet/GetTime.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 7777

enum GameMessages
{
	ID_USERNAME = ID_USER_PACKET_ENUM + 1,
	ID_RECEIVE_MESSAGE,
	ID_PROMPT_MESSAGE
};

/// <summary>
/// Takes in a string reference (pointers don't allow for array access as easily) and removes all spaces.
/// </summary>
/// <param name="strToTrim"></param>
void stringTrim(std::string& strToTrim)
{
	for (int i = 0; i < strToTrim.length(); i++)
	{
		if (std::isspace(strToTrim[i]))
		{
			strToTrim.erase(i, 1);
		}
	}
}

int main(int const argc, char const* const argv[])
{
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd;

	peer->Startup(1, &sd, 1);
	peer->SetOccasionalPing(true);

	//Use this for inputs
	std::string stringBuffer;
	char ip[512];
	char name[17];
	char message[129];

	bool isServer = false;

	printf("Enter server IP or hit enter for 172.16.2.61\n");
	//std::cin >> inputBuffer;
	std::getline(std::cin, stringBuffer);
	if (stringBuffer.length() == 0)
	{
		stringBuffer = "172.16.2.61\0";
	}

	stringBuffer.copy(ip, stringBuffer.length() + 1);
	ip[stringBuffer.length()] = 0;
	stringBuffer = "";
	while (stringBuffer.length() == 0)
	{
		//parse a name
		printf("Enter nickname (16 character max)\n");
		std::getline(std::cin, stringBuffer);
		printf("Removing any spaces that may exist ...\n");
		stringTrim(stringBuffer);
		printf("Removing any extra characters...\n");
		stringBuffer = stringBuffer.substr(0, min(stringBuffer.length(), 16));
		if (stringBuffer.length() == 0)
		{
			printf("Username was empty, please try again. ");
		}
	}
	
	strcpy(name, stringBuffer.c_str());
	name[16] = 0;

	printf("%i\n", (int)sizeof(RakNet::Time));
	printf("%i\n", (int)sizeof(char));

	printf("Starting the client.\n");
	peer->Connect(ip, SERVER_PORT, 0, 0);

	bool hasNameBeenSent = false;
	// TODO - Add code body here
	while (1)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			switch (packet->data[0])
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("Another client has disconnected.\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				printf("Another client has lost the connection.\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("Another client has connected.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted.\n");

				// Use a BitStream to write a custom user message
				// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
				RakNet::BitStream bsOut;
				//bsOut.Write((RakNet::MessageID)ID_USERNAME);
				//bsOut.Write("Hello world");
				//peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				if (!hasNameBeenSent)
				{
					RakNet::BitStream bsOut2;
					hasNameBeenSent = true;
					bsOut2.Write((RakNet::MessageID)ID_USERNAME);
					bsOut2.Write(name);
					peer->Send(&bsOut2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				}
			}
			break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				if (isServer) {
					printf("A client has disconnected.\n");
				}
				else {
					printf("We have been disconnected.\n");
				}
				break;
			case ID_CONNECTION_LOST:
				if (isServer) {
					printf("A client lost the connection.\n");
				}
				else {
					printf("Connection lost.\n");
				}
				break;

			case ID_USERNAME:
			{
				//would output a text message if we received it.
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
				break;

			case ID_PROMPT_MESSAGE:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());

				std::getline(std::cin, stringBuffer);
				strncpy(message, stringBuffer.c_str(), 128);
				message[128] = 0;
				RakNet::RakString messageRk("%s", message);

				RakNet::Time timeStamp;
				RakNet::MessageID useTimeStamp;
				RakNet::MessageID messageID = ID_RECEIVE_MESSAGE;
				useTimeStamp = ID_TIMESTAMP;
				timeStamp = RakNet::GetTime();
				RakNet::BitStream bsOut2;
				bsOut2.Write(useTimeStamp);
				bsOut2.Write(timeStamp);
				bsOut2.Write(messageID);
				bsOut2.Write(messageRk);
				peer->Send(&bsOut2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
				break;

			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
	}
	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}
