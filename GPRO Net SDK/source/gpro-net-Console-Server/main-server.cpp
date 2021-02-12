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
#include "RakNet/GetTime.h"


using namespace std;
#define MAX_CLIENTS 5
#define SERVER_PORT 7777


RakNet::RakPeerInterface* peer;
map<string, string> IPToUserName;
ofstream serverLog;

/// <summary>
/// Handle public, private, and command messages
/// </summary>
/// <param name="m"></param>
/// <param name="packet"></param>
void handleMessage(ChatMessage* m, RakNet::Packet* packet)
{
	float betterTime = (float)m->time;
	float seconds = betterTime / 1000.0f;
	float minutes = seconds / 60.0f;
	float hour = minutes / 60.0f;
	int hourVal = (int)hour % 12 + 11;
	hourVal %= 12;
	int minutesInt = (int)((hour - (int)hour) * 60);
	printf("%d:%d\n", hourVal, minutesInt + 20);
	string output = "[" + std::to_string(hourVal) + std::to_string(minutesInt + 20) + "] " + IPToUserName[packet->systemAddress.ToString()]; //timestamp + user who sent this

	RakNet::BitStream outStream;
	prepBitStream(&outStream, RakNet::GetTime());
	ChatMessage response;
	response.id2 = ID_MESSAGE_STRUCT;
	response.isTimestamp = ID_TIMESTAMP;
	response.messageType = 0;
	switch (m->messageType & 3) //trim off the admin flag for now
	{
	case PUBLIC: //public
	{
		output += " (publicly): ";
		output += m->message;
		strncpy(response.message, output.c_str(), output.length());
		response.message[output.length()] = 0;
		outStream.Write(response);
		map<string, string>::iterator it;
		for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
		{
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
		}
	}
	break;
	case PRIVATE: //private
	{
		output += " (privately): ";
		output += m->message;
		strncpy(response.message, output.c_str(), output.length());
		response.message[output.length()] = 0;
		outStream.Write(response);
		map<string, string>::iterator it;
		for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
		{
			if (strncmp(it->second.c_str(), m->recipient, it->second.size()) == 0)
			{
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
				break;
			}
		}
	}
	break;
	case COMMAND: //command
	{
		output += " (userlist): ";
		map<string, string>::iterator it;
		for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
		{
			output += it->second.c_str();
			output += "\n";
			
		}
		strncpy(response.message, output.c_str(), output.length());
		response.message[output.length()] = 0;
		outStream.Write(response);
		if (true)
		{
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			break;
		}
	}
		break;
	}
	serverLog << output << std::endl;
}

string adminName = "IAmTheAdmin";
int main(int const argc, char const* const argv[])
{
	peer = RakNet::RakPeerInterface::GetInstance();
	bool isServer;
	RakNet::Packet* packet;

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
				prepBitStream(&bsOut, RakNet::GetTime(), ID_USERNAME);
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

			case ID_USERNAME: //todo send this as a chat message, with the admin flag set too
			{
				ChatMessage m = parseMessage(packet);
				IPToUserName[packet->systemAddress.ToString()] = m.message;

				ChatMessage response;
				prepBitStream(&bsOut, RakNet::GetTime());
				if (strncmp(adminName.c_str(), m.message, adminName.length()) == 0)
				{
					response.messageType = ISADMIN;
				}
				else
				{
					response.messageType = 0;
				}
				string insert = (response.messageType == ISADMIN ? "Admin " : "");
				string output =  "Welcome, " + insert + IPToUserName[packet->systemAddress.ToString()] + "! Please enter a message.";
				strncpy(response.message, output.c_str(), output.length());
				response.message[output.length()] = 0;
				//bsOut.Write((RakNet::MessageID)ID_RECEIVE_MESSAGE);
				bsOut.Write(response);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
			break;
			/*case ID_RECEIVE_MESSAGE: //This section used to be relevant but it no longer is.
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
			}*/
			case ID_MESSAGE_STRUCT:
			{
				ChatMessage m = parseMessage(packet);

				handleMessage(&m, packet);
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
