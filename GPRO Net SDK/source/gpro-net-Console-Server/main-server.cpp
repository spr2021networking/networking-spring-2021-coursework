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

	main-server.c/.cpp
	Main source for console server application.
*/

#include "gpro-net/gpro-net.h"

#include <cstdio>
#include <stdlib.h>
#include <string.h> //these two are actually different! This one's for C, the one below is for C++
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"  // MessageID


using namespace std;
#define MAX_CLIENTS 10
#define SERVER_PORT 7777

enum GameMessages
{
	ID_USERNAME = ID_USER_PACKET_ENUM + 1,
	ID_RECEIVE_MESSAGE,
	ID_PROMPT_MESSAGE,
};

void handleMessage(ChatMessage* m, RakNet::Packet* packet)
{
	switch (m->messageType)
	{
	case 0: //public
		break;
	case 1: //private
		break;
	case 2: //command
		break;
	}
}

int main(int const argc, char const* const argv[])
{
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	bool isServer;
	RakNet::Packet* packet;
	ofstream serverLog;

	map<string, string> IPToUserName;

	//printf("(C) or (S)erver?\n");
	//scanf("%s", str);
	//if ((str[0] == 'S') || (str[0] == 'S'))
	//{
	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetOccasionalPing(true);
	isServer = true;
	//}

	serverLog.open("serverlog.txt");
	if (serverLog.is_open())
	{
		printf("file exists \n");
	}

	// TODO - Add code body here
	if (isServer)
	{
		printf("Starting the server.\n");
		// We need to let the server accept incoming connections from the clients
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	}

	while (1)
	{
		serverLog.open("serverlog.txt", std::ios_base::app);
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			RakNet::BitStream bsOut;
			RakNet::Time time;
			int idIndex = 0;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			if (packet->data[0] == ID_TIMESTAMP)
			{
				//char vals[sizeof(RakNet::Time)];
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				//bsIn.Read(vals, sizeof(RakNet::Time));
				//RakNet::Time time = *(RakNet::Time*)&vals;
				//bsIn.IgnoreBytes(sizeof(RakNet::Time) + sizeof(RakNet::MessageID));
				bsIn.Read(time);
				idIndex += sizeof(RakNet::MessageID) + sizeof(RakNet::Time);
			}

			switch (packet->data[idIndex])
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
				bsOut.Write((RakNet::MessageID)ID_USERNAME);
				bsOut.Write("Hello world");
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
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
					RakNet::RakString rs;
					//RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					string temp = rs.C_String();
					map <string, string>::iterator userNameToRemove;
					userNameToRemove = IPToUserName.find(temp);
					if (userNameToRemove != IPToUserName.end())
					{
						IPToUserName.erase(userNameToRemove);
					}
					//get username w/ systemaddress
					//search map and then remove
				}
				else {
					printf("Connection lost.\n");
				}
				break;

			case ID_USERNAME:
			{
				RakNet::RakString rs;
				//RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
				string temp = rs.C_String();
				printf("%s\n", packet->systemAddress.ToString());
				IPToUserName[packet->systemAddress.ToString()] = temp;
				serverLog << temp;
				serverLog << "\n";
				temp = "Welcome, " + IPToUserName[packet->systemAddress.ToString()] + "! Please enter a message.";
				bsOut.Write((RakNet::MessageID)ID_RECEIVE_MESSAGE);
				bsOut.Write(temp.c_str());
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
			break;
			case ID_RECEIVE_MESSAGE:
			{
				RakNet::RakString rs;
				RakNet::MessageID message2;
				//RakNet::BitStream bsIn(packet->data, packet->length, false);
				//char vals[sizeof(RakNet::Time)];
				//bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				//bsIn.Read(vals, sizeof(RakNet::Time));
				//RakNet::Time time = *(RakNet::Time*)&vals;
				//bsIn.IgnoreBytes(sizeof(RakNet::Time) + sizeof(RakNet::MessageID));
				//bsIn.Read(time);
				float betterTime = (float)time;
				float seconds = betterTime / 1000.0f;
				float minutes = seconds / 60.0f;
				float hour = minutes / 60.0f;
				int hourVal = (int)hour % 12 + 11;
				hourVal %= 12;
				int minutesInt = (int)((hour - (int)hour) * 60);
				printf("%d:%d\n", hourVal, minutesInt + 20);
				//bsIn.IgnoreBytes(sizeof(RakNet::Time) + sizeof(RakNet::MessageID));
				bsIn.Read(message2);
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
				string temp = rs.C_String();
				string newString = chopStr((char*)temp.c_str(), (int)temp.length(), ' ');
				serverLog << temp;
				serverLog << "\n";
				bsOut.Write((RakNet::MessageID)ID_RECEIVE_MESSAGE);
				string output = "[";
				output = output + std::to_string(time);
				output = output + "] " + IPToUserName[packet->systemAddress.ToString()]; //need to make sure we don't get an invalid user!!
				bool dm = temp != newString && strcmp(temp.c_str(), "all") != 0 && strcmp(temp.c_str(), "server") != 0;
				if (dm)
				{
					output += " (privately)";
				}
				else
				{
					output += " (publicly)";
				}
				output += ": " + newString;
				bsOut.Write(output.c_str());
				if (!dm)
				{
					map<string, string>::iterator it;
					for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
					{
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
					}
				}
				else
				{
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				}
				break;
			}
			case ID_PROMPT_MESSAGE:
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

				handleMessage(&m, &packet);
				break;
			}
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
		serverLog.close();
	}
	
	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}
