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

/*
* main-client.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: message handling (receiving from server and sending our own), joining and leaving server.
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
#include "Windows.h"
#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"

#include "gpro-net-Console-Client/checkers.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 7777

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

RakNet::RakPeerInterface* peer;
RakNet::SystemAddress serverAddress;
char name[17];
bool quitting = false;
CheckersInstance checkers;

/// <summary>
/// Closes our connection to the server after 300 ms, sending a notification that we're leaving first.
/// </summary>
void quit()
{
	quitting = true;
	RakNet::BitStream out;
	prepBitStream(&out, RakNet::GetTime(), ID_CONNECTION_LOST);
	out.Write(name);
	peer->Send(&out, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
	peer->Shutdown(300);
}

void tryCreateCommand(ChatMessage* messageToSend, std::string args, bool isAdmin)
{
	messageToSend->messageFlag = COMMAND;
	//checking what command type
	if (strncmp(args.c_str(), "userlist", 8) == 0) //userlist
	{
		messageToSend->setText(RECIPIENT, "userlist");
		messageToSend->setText(MESSAGE, " ");
	}
	else if (strncmp(args.c_str(), "kick", 4) == 0) //kick (admin only, nonfunctional)
	{
		if (true)
		{
			printf("[Error] Kicking currently does not function correctly!\n");
			messageToSend->setText(MESSAGE, "");
		}
		else if (!isAdmin)
		{
			printf("[Error] Only admins can kick!\n");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			char* startOfKickTarget = chopStr((char*)args.c_str(), (int)args.length(), ' ');
			if (startOfKickTarget == args.c_str()) //no kick target
			{
				printf("[Error] No kick target\n");
				messageToSend->setText(MESSAGE, "");
			}
			else
			{
				std::string messageBody = startOfKickTarget;
				messageToSend->setText(RECIPIENT, "kick");
				messageToSend->setText(MESSAGE, messageBody);
			}
		}
	}
	else if (strncmp(args.c_str(), "stop", 4) == 0) //stop server
	{
		if (!isAdmin)
		{
			printf("[Error] Only admins can close the server!\n");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			messageToSend->setText(RECIPIENT, "stop");
		}
	}
	else if (strncmp(args.c_str(), "createroom", 10) == 0) //create room
	{
		char* startOfRoomName = chopStr((char*)args.c_str(), (int)args.length(), ' ');
		if (startOfRoomName == args.c_str())
		{
			printf("[Error] No room stated\n");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			std::string messageBody = startOfRoomName;
			messageToSend->setText(RECIPIENT, "createroom");
			messageToSend->setText(MESSAGE, messageBody);
		}
	}
	else if (strncmp(args.c_str(), "joinroom", 8) == 0)
	{
		char* startOfRoomName = chopStr((char*)args.c_str(), (int)args.length(), ' ');
		if (startOfRoomName == args.c_str())
		{
			printf("[Error] No room stated\n");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			std::string messageBody = startOfRoomName;
			messageToSend->setText(RECIPIENT, "joinroom");
			messageToSend->setText(MESSAGE, messageBody);
		}
	}
	else if (strncmp(args.c_str(), "spectate", 8) == 0) //spectate room
	{
		char* startOfRoomName = chopStr((char*)args.c_str(), (int)args.length(), ' ');
		if (startOfRoomName == args.c_str())
		{
			printf("[Error] No room stated\n");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			std::string messageBody = startOfRoomName;
			messageToSend->setText(RECIPIENT, "spectate");
			messageToSend->setText(MESSAGE, messageBody);
		}
	}
	else
	{
		printf("[Error] Unknown command\n");
		messageToSend->setText(MESSAGE, "");
	}
}

int main(int const argc, char const* const argv[])
{
	checkers.reset();
	bool playingCheckers = true;
	//int out = 0;
	//while (playingCheckers)
	//{
	//	checkers.checkerLoop(&out);
	//}
	peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SocketDescriptor sd;

	peer->Startup(1, &sd, 1);
	peer->SetOccasionalPing(true);

	//Use this for inputs
	std::string stringBuffer;
	char ip[512];
	char message[129];

	bool isServer = false;

	//retrieve IP and name
	printf("Enter server IP or hit enter for 172.16.2.60\n");
	//std::cin >> inputBuffer;
	std::getline(std::cin, stringBuffer);
	if (stringBuffer.length() == 0)
	{
		stringBuffer = "172.16.2.60\0";
	}

	stringBuffer.copy(ip, stringBuffer.length() + 1);
	ip[stringBuffer.length()] = 0;
	stringBuffer = "";
	while (stringBuffer.length() == 0)
	{
		//parse a name
		printf("Enter nickname (16 character max)\n");
		std::getline(std::cin, stringBuffer);
		int prevLen = (int)stringBuffer.length();
		stringTrim(stringBuffer);
		int newLen = (int)stringBuffer.length();
		if (prevLen != newLen)
		{
			printf("Removing spaces ...\n");
		}
		if (newLen > 16)
		{
			printf("Removing any extra characters...\n");
			stringBuffer = stringBuffer.substr(0, min(stringBuffer.length(), 16));
		}

		if (stringBuffer.length() == 0)
		{
			printf("Username was empty, please try again. ");
		}
	}

	strcpy(name, stringBuffer.c_str());
	name[16] = 0;

	printf("Starting the client. WARNING: Timestamps may be extremely inaccurate. There is no obvious conversion between RakNet::GetTime() and the current system time\n");
	peer->Connect(ip, SERVER_PORT, 0, 0);

	bool hasNameBeenSent = false;

	bool isAdmin = false;

	while (!quitting)
	{
		//render the checkerboard before we do anything. Also shouldn't display if we are in a lobby, that'll be handled later. TODO
		if (checkers.action.checkerRoomKey[0] != 0 && checkers.dirty)
		{
			checkers.drawCheckers();
		}

		for (packet = peer->Receive(); packet && !quitting; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			//this function checks if there's a timestamp stored in the packet. If there is, we advance the switch past the timestamp
			int idIndex = 0;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			if (packet->data[0] == ID_TIMESTAMP)
			{
				RakNet::Time time;
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
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

				RakNet::BitStream bsOut;
				if (!hasNameBeenSent) //we send our username to the server here, and we receive an ID_USERNAME message back that sets our admin status
				{
					hasNameBeenSent = true;

					//create a ChatMessage and send our username
					prepBitStream(&bsOut, RakNet::GetTime(), ID_USERNAME);
					ChatMessage mess;
					strncpy(mess.sender, name, stringBuffer.length());
					mess.sender[stringBuffer.length()] = 0;
					bsOut.Write(mess);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress = packet->systemAddress, false);
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
				printf("We have been disconnected.\n");
				quitting = true;
				break;
			case ID_CONNECTION_LOST:
				printf("Connection lost.\n");
				quitting = true;
				break;

			case ID_USERNAME:
			{
				//if messageFlag is -1, the user already exists and we quit.
				ChatMessage m = ChatMessage::parseMessage(packet);
				printf("%s\n", m.message);
				if (m.messageFlag == -1)
				{
					peer->Shutdown(300); //quit() requires a username, so we do this instead
				}
				else
				{
					isAdmin = (m.messageFlag & ISADMIN) > 0;
				}
			}
			break;

			case ID_RECEIVE_MESSAGE: //legacy, currently unused.
			{
				ChatMessage m = ChatMessage::parseMessage(packet);
				printf("%s\n", m.message);
			}
			break;
			case ID_MESSAGE_STRUCT:
			{
				//parse message and output relevant data
				ChatMessage m = ChatMessage::parseMessage(packet);
				if (checkers.action.checkerRoomKey[0] != 0)
				{
					gpro_consoleSetCursor(0, 8);
				}
				printf("%s\n", m.message);
				break;
			}
			case ID_GAMEMESSAGE_STRUCT:
			{
				Action act = Action::parseAction(packet);
				checkers.processAction(&act);
				break;
			}
			case ID_JOIN_ROOM:
			{
				RoomJoinInfo r = RoomJoinInfo::parseMessage(packet);
				checkers.playerNum = r.playerIndex;
				checkers.action.setName(r.roomID, (int)strnlen(r.roomID, 16));
				break;
			}
			case ID_KICK:
				quit();
				printf("You have been kicked\n");
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[idIndex]);
				break;
			}
		}
		//checkerLoop(&chk);
		//check for user input! Iterate through keys and see if we're using any of them, if so let us type
		bool hasInput = false;
		for (int i = VK_SPACE; i <= 'Z'; i++)
		{
			if (i == VK_LEFT)
			{
				continue;
			}
			if (i == VK_RIGHT)
			{
				continue;
			}
			if (i == VK_UP)
			{
				continue;
			}
			if (i == VK_DOWN)
			{
				continue;
			}

			short ks = GetKeyState(i) >> 8;
			if (ks != 0)
			{
				hasInput = true;
				break;
			}
		}
		if (!hasInput && checkers.playerNum == checkers.currentPlayer)
		{
			checkers.checkInput();
			if (checkers.action.playerIndex != 0) //we need to store our local player somehow!
			{
				int winner = -1;
				if (checkers.checkWin(&winner))
				{
					//send notification that the game was won, then close after x seconds
				}
				else
				{
					RakNet::BitStream actionStream;
					prepBitStream(&actionStream, RakNet::GetTime(), (RakNet::MessageID)ID_GAMEMESSAGE_STRUCT);
					actionStream.Write(checkers.action);
					peer->Send(&actionStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
					checkers.action.playerIndex = 0;
				}
			}
		}
		if (hasInput)
		{
			if (checkers.action.checkerRoomKey[0] != 0)
			{
				gpro_consoleSetCursor(0, 8);
			}
			std::getline(std::cin, stringBuffer);
			strncpy(message, stringBuffer.c_str(), 128);
			message[128] = 0;
			RakNet::RakString messageBackup("%s", message);

			RakNet::Time timeStamp;
			RakNet::MessageID useTimeStamp;
			RakNet::MessageID messageID = ID_RECEIVE_MESSAGE;
			useTimeStamp = ID_TIMESTAMP;
			timeStamp = RakNet::GetTime();

			//prepare the bitstream and the ChatMessage
			RakNet::BitStream bsOut;
			prepBitStream(&bsOut, timeStamp);
			ChatMessage messageToSend;
			messageToSend.isTimestamp = ID_TIMESTAMP;
			messageToSend.time = timeStamp;
			messageToSend.id2 = ID_MESSAGE_STRUCT;

			char* startOfSecondWord = chopStr((char*)stringBuffer.c_str(), (int)stringBuffer.length(), ' ');

			if (strncmp(stringBuffer.c_str(), "quit", 4) == 0 && stringBuffer.length() == 4)
			{
				quit();
				break;
			}
			else if (startOfSecondWord == stringBuffer.c_str()) //there's no space in the message
			{
				if (strncmp(stringBuffer.c_str(), "command", 7) != 0) //make sure this isn't an empty command, meaning that it's a one-word public message
				{
					//set message to public and load the entire string buffer into the message.
					messageToSend.messageFlag = PUBLIC;
					messageToSend.setText(MESSAGE, stringBuffer);
				}
				else
				{
					printf("[Error] Cannot send empty command\n");
					messageToSend.setText(MESSAGE, "");
				}
			}
			else
			{
				std::string secondWord = startOfSecondWord;
				if (strncmp(stringBuffer.c_str(), "private", 7) == 0) //stringBuffer contains only the first word at this point
				{
					messageToSend.messageFlag = PRIVATE;
					char* startOfMessageBody = chopStr((char*)secondWord.c_str(), (int)strlen(startOfSecondWord), ' ');
					if (startOfMessageBody == secondWord.c_str()) //there's no actual message body
					{
						messageToSend.messageFlag = PUBLIC; //reset to public
						messageToSend.setText(MESSAGE, secondWord);
					}
					else
					{
						//copy message recipient and body into ChatMessage from input
						std::string messageBody = startOfMessageBody;
						messageToSend.setText(RECIPIENT, secondWord);
						messageToSend.setText(MESSAGE, messageBody);
					}
				}
				else if (strncmp(stringBuffer.c_str(), "command", 7) == 0) //we're using a command
				{
					tryCreateCommand(&messageToSend, secondWord, isAdmin);
				}
				else //defaulting to a public message because the first word wasn't recognized as anything
				{
					messageToSend.messageFlag = PUBLIC;
					messageToSend.setText(MESSAGE, messageBackup.C_String(), (int)messageBackup.GetLength());
				}
			}
			if (isAdmin)
			{
				messageToSend.messageFlag |= ISADMIN;
			}
			if (messageToSend.hasMessage()) //prevents empty messages or invalid messages.
			{
				messageToSend.setText(SENDER, name, (int)strnlen(name, 16));
				bsOut.Write(messageToSend);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
			}
		}

	}
	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}
