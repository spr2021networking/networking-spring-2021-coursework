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

string adminName = "IAmTheAdmin";
RakNet::SystemAddress adminAddress;
bool quitting = false;

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
		peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(), true);
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
		bool sent = false;
		for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
		{
			if (strncmp(it->second.c_str(), m->recipient, it->second.length()) == 0)
			{
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
				string logOutput = "[" + std::to_string(hourVal) + std::to_string(minutesInt + 20) + "] " + IPToUserName[packet->systemAddress.ToString()];
				logOutput += " (privately to " + it->second + "): " + m->message;
				serverLog << logOutput << std::endl;
				outStream.Reset();
				prepBitStream(&outStream, RakNet::GetTime());
				strncpy(response.message, logOutput.c_str(), (int)logOutput.length());
				response.message[logOutput.length()] = 0;
				outStream.Write(response);
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, adminAddress, false);
				sent = true;
				break;
			}
		}
		if (!sent)
		{
			outStream.Reset();
			prepBitStream(&outStream, RakNet::GetTime());
			strncpy(response.message, "[Error] user does not exist", 28);
			response.message[28] = 0;
			outStream.Write(response);
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		}
	}
	break;
	case COMMAND: //command
	{
		if (strncmp(m->recipient, "userlist", 8) == 0)
		{
			output += " requested User List:";
			map<string, string>::iterator it;
			for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
			{
				output += "\n";
				string tmp;

				if ((m->messageType & ISADMIN) > 0) //if admin, add IP. This now makes it possible for a string to be too long to send
				{
					tmp += it->first + " ";
				}
				tmp += it->second.c_str();

				if (output.length() + tmp.length() >= 128) //if string length is too long for us to add tmp, send what we have and reset the bitstream
				{
					strncpy(response.message, output.c_str(), output.length());
					response.message[output.length()] = 0;
					outStream.Write(response);
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false); //send part of message
					outStream.Reset(); //clear bitstream
					prepBitStream(&outStream, RakNet::GetTime()); //reset bitstream to receive new response
					output = tmp; //store tmp in output to prepare for next loop.
				}
				else
				{
					output += tmp;
				}
			}
			strncpy(response.message, output.c_str(), output.length()); //output final loop iteration.
			response.message[output.length()] = 0;
			outStream.Write(response);
			if (true)
			{
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
		}
		if (strncmp(m->recipient, "kick", 4) == 0)
		{
			outStream.Reset();
			prepBitStream(&outStream, RakNet::GetTime(), ID_KICK);
			map<string, string>::iterator it;
			for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
			{
				if (strncmp(it->second.c_str(), m->message, it->second.length()) == 0)
				{
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
					string logOutput = "[" + std::to_string(hourVal) + std::to_string(minutesInt + 20) + "] Kicked " + IPToUserName[packet->systemAddress.ToString()];
					serverLog << logOutput << std::endl;
					break;
				}
			}
		}
		if (strncmp(m->recipient, "stop", 4) == 0)
		{
			string quitMessage = "[" + std::to_string(hourVal) + std::to_string(minutesInt + 20) + "] The server is now shutting down.";
			strncpy(response.message, quitMessage.c_str(), quitMessage.length());
			response.message[quitMessage.length()] = 0;
			outStream.Write(response);
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(), true);
			serverLog << quitMessage << "\n";
			quitting = true;
		}
	}
	break;
	}
	if ((m->messageType & 3) != PRIVATE && strncmp(m->recipient, "kick", 4) != 0) //private has its own logging behavior
	{
		serverLog << output << std::endl;
	}
}


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

	while (!quitting)
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
					map <string, string>::iterator userNameToRemove;
					userNameToRemove = IPToUserName.find(packet->systemAddress.ToString());
					string output = "";
					if (userNameToRemove != IPToUserName.end())
					{
						output += userNameToRemove->second;
						IPToUserName.erase(userNameToRemove);
					}
					RakNet::Time t = RakNet::GetTime();
					prepBitStream(&bsOut, t);

					float betterTime = (float)t;
					float seconds = betterTime / 1000.0f;
					float minutes = seconds / 60.0f;
					float hour = minutes / 60.0f;
					int hourVal = (int)hour % 12 + 11;
					hourVal %= 12;
					int minutesInt = (int)((hour - (int)hour) * 60);
					output = "[Received at " + std::to_string(hourVal) + std::to_string(minutesInt + 20) + "] " + output + " has disconnected.";
					bsOut.Write(output);
					serverLog << output << '\n';

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(), true);

				}
				else {
					printf("Connection lost.\n");
				}
				break;

			case ID_USERNAME:
			{
				prepBitStream(&bsOut, RakNet::GetTime(), ID_USERNAME);
				ChatMessage m = parseMessage(packet);
				map<string, string>::iterator it;
				bool alreadyExists = false;
				for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
				{
					if (strncmp(m.message, it->second.c_str(), it->second.length()) == 0) //username already exists
					{
						alreadyExists = true;
					}
				}

				ChatMessage response;

				if (alreadyExists)
				{
					response.messageType = -1;
					strncpy(response.message, "We're sorry, that username is already taken. Please restart the program and try again!", 87);
					response.message[87] = 0;
					bsOut.Write(response);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				}
				else
				{
					IPToUserName[packet->systemAddress.ToString()] = m.message;

					if (strncmp(adminName.c_str(), m.message, adminName.length()) == 0)
					{
						response.messageType = ISADMIN;
						adminAddress = packet->systemAddress;
					}
					else
					{
						response.messageType = 0;
					}
					string insert = (response.messageType == ISADMIN ? "Admin " : "");
					string output = "Welcome, " + insert + IPToUserName[packet->systemAddress.ToString()] + "! Please enter a message.";
					strncpy(response.message, output.c_str(), output.length());
					response.message[output.length()] = 0;
					bsOut.Write(response);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					bsOut.Reset();
					prepBitStream(&bsOut, RakNet::GetTime());

					float betterTime = (float)m.time;
					float seconds = betterTime / 1000.0f;
					float minutes = seconds / 60.0f;
					float hour = minutes / 60.0f;
					int hourVal = (int)hour % 12 + 11;
					hourVal %= 12;
					int minutesInt = (int)((hour - (int)hour) * 60);
					string otherMess = "[" + std::to_string(hourVal) + std::to_string(minutesInt + 20) + "] " + IPToUserName[packet->systemAddress.ToString()] + " has joined the chat.";
					strncpy(response.message, otherMess.c_str(), otherMess.length());
					response.message[otherMess.length()];
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
					serverLog << otherMess << "\n";

				}
				


				
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
	peer->Shutdown(300);
	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}
