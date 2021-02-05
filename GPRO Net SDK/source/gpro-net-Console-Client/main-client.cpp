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

#define MAX_CLIENTS 10
#define SERVER_PORT 7777

enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_GAME_MESSAGE_2
};

int main(int const argc, char const* const argv[])
{
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd;

	peer->Startup(1, &sd, 1);

	//Use this for inputs
	std::string stringBuffer;
	char ip[512];
	char name[17];
	char message[17];

	bool isServer = false;

	printf("Enter server IP or hit enter for 172.16.2.57\n");
	//std::cin >> inputBuffer;
	std::getline(std::cin, stringBuffer);
	if (stringBuffer.length() == 0)
	{
		stringBuffer = "172.16.2.57\0";
	}

	stringBuffer.copy(ip, stringBuffer.length() + 1);
	ip[stringBuffer.length()] = 0;

	//parse a name
	printf("Enter nickname (16 character max)\n");
	std::getline(std::cin, stringBuffer);
	strncpy(name, stringBuffer.c_str(), 16);
	name[16] = 0;

	printf("%i\n", (int)sizeof(time_t));

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
				bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
				bsOut.Write("Hello world");
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				if (!hasNameBeenSent)
				{
					RakNet::BitStream bsOut2;
					hasNameBeenSent = true;
					bsOut2.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
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

			case ID_GAME_MESSAGE_1:
			{
				//would output a text message if we received it.
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
				break;

			case ID_GAME_MESSAGE_2:
			{
				std::getline(std::cin, stringBuffer);
				strncpy(message, stringBuffer.c_str(), 16);
				message[16] = 0;

				RakNet::BitStream bsOut2;
				bsOut2.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
				bsOut2.Write(message);
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
