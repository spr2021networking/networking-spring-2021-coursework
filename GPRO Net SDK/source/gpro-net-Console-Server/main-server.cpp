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

	Modified by Scott Dagen and Benjamin Cooper
	Purpose: Demonstrate an understanding of how to make a chat client
	What we each did: Both worked on message handling and timestamping
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

#include "gpro-net-Console-Server/server-utils.h"


using namespace std;
#define MAX_CLIENTS 5
#define SERVER_PORT 7777


RakNet::RakPeerInterface* peer;
map<string, string> IPToUserName;
map<string, CheckerRoom> roomKeyToRoom;
map<string, string> IPToRoom;
ofstream serverLog;

string adminName = "IAmTheAdmin";
RakNet::SystemAddress adminAddress;
bool quitting = false;

void handleGameMessage(Action* gAction, RakNet::Packet* packet)
{
	RakNet::BitStream outStream;
	prepBitStream(&outStream, RakNet::GetTime(), ID_GAMEMESSAGE_STRUCT);
	outStream.Write(*gAction);
	if (gAction->playerIndex == 1) //we need to send the relavent information to player 2
	{
		map<string, CheckerRoom>::iterator it;
		it = roomKeyToRoom.find(gAction->checkerRoomKey);
		if (it != roomKeyToRoom.end())
		{
			CheckerRoom* ch = &(it->second);
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(ch->player2.address.c_str()), false);
			for (int i = 0; i < (int)ch->spectators.size(); i++)
			{
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(ch->spectators[i].address.c_str()), false);
			}
		}
		//update the piece position with endX & endY
		if (gAction->hasCaptured)
		{
			//send a message that tells player 2 where player 1's piece moved to and what piece(s) were captured
		}
		else if(!gAction->hasCaptured)
		{
			//send a message that tells player 2 where player 1's piece moved to
		}
		
	}
	else //we send the relavent information to player 1
	{
		map<string, CheckerRoom>::iterator it;
		it = roomKeyToRoom.find(gAction->checkerRoomKey);
		if (it != roomKeyToRoom.end())
		{
			CheckerRoom* ch = &(it->second);
			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(ch->player1.address.c_str()), false);
			for (int i = 0; i < (int)ch->spectators.size(); i++)
			{
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(ch->spectators[i].address.c_str()), false);
			}
		}
		//update the piece position with endX & endY
		if (gAction->hasCaptured)
		{
			//send a message that tells player 1 where player 2's piece moved to and what piece(s) were captured
		}
		else if(!gAction->hasCaptured)
		{
			//send a message that tells player 1 where player 2's piece moved to
		}
	}
}


/// <summary>
/// Handle public, private, and command messages
/// </summary>
/// <param name="m"></param>
/// <param name="packet"></param>
void handleMessage(ChatMessage* m, RakNet::Packet* packet)
{
	//time calculation, some logging
	float betterTime = (float)m->time;
	float seconds = betterTime / 1000.0f;
	float minutes = seconds / 60.0f;
	float hour = minutes / 60.0f;
	int hourVal = (int)hour % 12 + 11;
	hourVal %= 12;
	int minutesInt = (int)((hour - (int)hour) * 60);
	printf("%d:%d %s\n", hourVal, minutesInt + 20, m->sender);
	string output = "[" + std::to_string(hourVal) + ":" + std::to_string(minutesInt + 20) + "] " + m->sender; //timestamp + user who sent this

	RakNet::BitStream outStream;
	prepBitStream(&outStream, RakNet::GetTime());

	ChatMessage response;
	response.id2 = ID_MESSAGE_STRUCT;
	response.isTimestamp = ID_TIMESTAMP;

	switch (m->messageFlag & 3) //trim off the admin flag for now
	{
	case PUBLIC: //public
	{
		response.messageFlag = PUBLIC;
		output += " (publicly): ";
		output += m->message;

		response.setText(MESSAGE, output);

		outStream.Write(response);

		if (IPToRoom[packet->systemAddress.ToString()] == "lobby")
		{
			sendMessageToLobby(&IPToRoom, packet, peer, &outStream);
		}
		else
		{
			map<string, string>::iterator it;
			it = IPToRoom.find(packet->systemAddress.ToString());
			if (it != IPToRoom.end())
			{	
				sendMessageToRoom(&roomKeyToRoom[it->second], packet, peer, &outStream);
			}
		}
	}
	break;
	case PRIVATE: //private
	{
		output += " (privately): ";
		output += m->message;

		response.setText(MESSAGE, output);

		outStream.Write(response);
		map<string, string>::iterator it;
		bool sent = false;
		for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
		{
			if (strncmp(it->second.c_str(), m->recipient, it->second.length()) == 0)//check to see if the user exists
			{
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);

				string logOutput = "[" + std::to_string(hourVal) + ":" + std::to_string(minutesInt + 20) + "] " + IPToUserName[packet->systemAddress.ToString()];
				logOutput += " (privately to " + it->second + "): " + m->message;
				serverLog << logOutput << std::endl;

				outStream.Reset();

				prepBitStream(&outStream, RakNet::GetTime());
				response.setText(MESSAGE, logOutput);

				outStream.Write(response);
				peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, adminAddress, false);
				sent = true;
				break;
			}
		}
		if (!sent) //if no private message was sent
		{
			outStream.Reset();
			prepBitStream(&outStream, RakNet::GetTime());

			response.setText(MESSAGE, "[Error] user does not exist");

			outStream.Write(response);

			peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		}
	}
	break;
	case COMMAND: //command
	{
		if (strncmp(m->recipient, "createroom", 10) == 0) //create a checker room
		{
			map<string, string>::iterator it;
			it = IPToRoom.find(packet->systemAddress.ToString());
			if (strncmp(IPToRoom[packet->systemAddress.ToString()].c_str(), "lobby", 5) == 0)
			{
				IPToRoom[packet->systemAddress.ToString()] = roomKeyToRoom[m->message].name; //remove the player from the lobby so they don't get public messages
				CheckerRoom::createAndJoinRoom(&roomKeyToRoom, &IPToUserName, peer, packet, m->message);
				//alert other people
				output += " (publicly): ";
				output += " a new room has been created";
				strncpy(response.message, output.c_str(), output.length());
				response.message[output.length()] = 0;
				outStream.Write(response);
				if (IPToRoom[packet->systemAddress.ToString()] == "lobby")
				{
					map<string, string>::iterator it;
					it = IPToRoom.find(packet->systemAddress.ToString());
					for (it = IPToRoom.begin(); it != IPToRoom.end(); it++) //check to see if we send the message to a user, as long as they aren't in a game, they get the message
					{
						if (strncmp(IPToRoom[packet->systemAddress.ToString()].c_str(), "lobby", 5) == 0)
						{
							peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
						}
					}
				}
			}
			//send warning
		}
		if (strncmp(m->recipient, "joinroom", 8) == 0) //join a checker room
		{
			map<string, string>::iterator it;
			it = IPToRoom.find(packet->systemAddress.ToString());
			if (strncmp(IPToRoom[packet->systemAddress.ToString()].c_str(), "lobby", 5) == 0)
			{
				IPToRoom[packet->systemAddress.ToString()] = roomKeyToRoom[m->message].name; //remove the player from the lobby so they don't get public messages
				CheckerRoom::joinRoom(&roomKeyToRoom, &IPToUserName, peer, packet, m->message, 1);
				output += " a new user has joined the room";
				strncpy(response.message, output.c_str(), output.length());
				response.message[output.length()] = 0;
				outStream.Write(response);
				//alert other people
				if (it != IPToRoom.end())
				{
					CheckerRoom* messageRoom = &roomKeyToRoom[it->second];
					std::vector<Player>::iterator spcit;
					for (spcit = messageRoom->spectators.begin(); spcit != messageRoom->spectators.end(); spcit++)
					{
						peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(spcit->address.c_str()), false);
					}
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(messageRoom->player1.address.c_str()), false);
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(messageRoom->player2.address.c_str()), false);
				}
			}
			//send warning
		}
		if (strncmp(m->recipient, "spectate", 8) == 0)//spectate a checker room
		{
			map<string, string>::iterator it;
			it = IPToRoom.find(packet->systemAddress.ToString());
			if (strncmp(IPToRoom[packet->systemAddress.ToString()].c_str(), "lobby", 5) == 0)
			{
				IPToRoom[packet->systemAddress.ToString()] = roomKeyToRoom[m->message].name; //remove the player from the lobby so they don't get public messages
				CheckerRoom::spectateRoom(&roomKeyToRoom, &IPToUserName, peer, packet, m->message);
				output += " a new user has joined the room";
				strncpy(response.message, output.c_str(), output.length());
				response.message[output.length()] = 0;
				outStream.Write(response);
				if (it != IPToRoom.end())
				{
					CheckerRoom* messageRoom = &roomKeyToRoom[it->second];
					std::vector<Player>::iterator spcit;
					for (spcit = messageRoom->spectators.begin(); spcit != messageRoom->spectators.end(); spcit++)
					{
						peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(spcit->address.c_str()), false);
					}
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(messageRoom->player1.address.c_str()), false);
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(messageRoom->player2.address.c_str()), false);
				}
			}
			
		}
		if (strncmp(m->recipient, "userlist", 8) == 0) //get a list of users
		{
			output += " requested User List:";
			map<string, string>::iterator it;
			for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
			{
				output += "\n";
				string tmp;

				if ((m->messageFlag & ISADMIN) > 0) //if admin, add IP. This now makes it possible for a string to be too long to send
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
		if (strncmp(m->recipient, "kick", 4) == 0) //kick the user
		{
			outStream.Reset();
			prepBitStream(&outStream, RakNet::GetTime(), ID_KICK);
			map<string, string>::iterator it;
			for (it = IPToUserName.begin(); it != IPToUserName.end(); it++)
			{
				if (strncmp(it->second.c_str(), m->message, it->second.length()) == 0)
				{
					peer->Send(&outStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->first.c_str()), false);
					string logOutput = "[" + std::to_string(hourVal) + ":" + std::to_string(minutesInt + 20) + "] Kicked " + IPToUserName[packet->systemAddress.ToString()];
					serverLog << logOutput << std::endl;
					break;
				}
			}
		}
		if (strncmp(m->recipient, "stop", 4) == 0) //stop the server
		{
			string quitMessage = "[" + std::to_string(hourVal) + ":" + std::to_string(minutesInt + 20) + "] The server is now shutting down.";
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
	if ((m->messageFlag & 3) != PRIVATE && strncmp(m->recipient, "kick", 4) != 0 && strncmp(m->recipient, "stop", 4) != 0) //private has its own logging behavior
	{
		serverLog << output << std::endl;
	}
}


int main(int const argc, char const* const argv[])
{
	peer = RakNet::RakPeerInterface::GetInstance();
	bool isServer;
	RakNet::Packet* packet;

	//start the server
	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetOccasionalPing(true);
	isServer = true;

	serverLog.open("serverlog.txt"); //open the server log
	if (serverLog.is_open())
	{
		printf("file exists \n");
	}

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
			if (packet->data[0] == ID_TIMESTAMP) //timestamping
			{
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(time);
				idIndex += sizeof(RakNet::MessageID) + sizeof(RakNet::Time);
			}

			switch (packet->data[idIndex])//run through cases to see what to do
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
					if (userNameToRemove != IPToUserName.end())//remove the user who disconnected from the server and userlist
					{
						output += userNameToRemove->second;
						IPToUserName.erase(userNameToRemove);
					}
					RakNet::Time t = RakNet::GetTime();
					prepBitStream(&bsOut, t);

					ChatMessage response;
					float betterTime = (float)t;
					float seconds = betterTime / 1000.0f;
					float minutes = seconds / 60.0f;
					float hour = minutes / 60.0f;
					int hourVal = (int)hour % 12 + 11;
					hourVal %= 12;
					int minutesInt = (int)((hour - (int)hour) * 60);
					output = "[Received at " + std::to_string(hourVal) + ":" + std::to_string(minutesInt + 20) + "] " + output + " has disconnected."; //send a notification that someone disconnected
					strncpy(response.message, output.c_str(), output.length());
					response.message[output.length()] = 0;
					bsOut.Write(response);
					serverLog << output << '\n';

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(), true);

				}
				else {
					printf("Connection lost.\n");
				}
				break;

			case ID_USERNAME: //reuse and/or rebuild this to work with prompting a user to join the lobby and if they want to be a player or spectator
			{
				prepBitStream(&bsOut, RakNet::GetTime(), ID_USERNAME);
				ChatMessage m = ChatMessage::parseMessage(packet);
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
					response.messageFlag = -1;
					strncpy(response.message, "We're sorry, that username is already taken. Please restart the program and try again!", 87);//reprompt the user for a username
					response.message[87] = 0;
					bsOut.Write(response);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				}
				else
				{
					IPToUserName[packet->systemAddress.ToString()] = m.sender; //add the user to the username list
					IPToRoom[packet->systemAddress.ToString()] = true; //set the user to be in the lobby so they get public messages

					if (strncmp(adminName.c_str(), m.sender, adminName.length()) == 0)
					{
						response.messageFlag = ISADMIN;
						adminAddress = packet->systemAddress;
					}
					else
					{
						response.messageFlag = 0;
					}
					string insert = (response.messageFlag == ISADMIN ? "Admin " : "");
					string output = "Welcome, " + insert + m.sender + "! Please enter a message."; //initial welcome message
					strncpy(response.message, output.c_str(), output.length());
					response.message[output.length()] = 0;
					bsOut.Write(response);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					bsOut.Reset();
					RakNet::Time tmpTime = RakNet::GetTime();
					prepBitStream(&bsOut, tmpTime);

					//timestamping
					float betterTime = (float)tmpTime;
					float seconds = betterTime / 1000.0f;
					float minutes = seconds / 60.0f;
					float hour = minutes / 60.0f;
					int hourVal = (int)hour % 12 + 11;
					hourVal %= 12;
					int minutesInt = (int)((hour - (int)hour) * 60);
					string otherMess = "[" + std::to_string(hourVal) + ":" + std::to_string(minutesInt + 20) + "] " + m.sender + " has joined the chat.";
					strncpy(response.message, otherMess.c_str(), otherMess.length());
					response.message[otherMess.length()] = 0;
					bsOut.Write(response);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
					serverLog << otherMess << "\n";

				}	
			}
			break;
			
			case ID_GAMEMESSAGE:
			{
				prepBitStream(&bsOut, RakNet::GetTime(), ID_GAMEMESSAGE);
				break;
			}

			case ID_MESSAGE_STRUCT:
			{
				ChatMessage m = ChatMessage::parseMessage(packet);

				handleMessage(&m, packet); //message handling, done in the handleMessage function above main
				break;
			}
			case ID_GAMEMESSAGE_STRUCT:
			{
				Action gAction = Action::parseAction(packet);
				int i = 0;
				handleGameMessage(&gAction, packet);
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
