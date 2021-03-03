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
TextBox textBox;

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
	textBox.addMessage("Enter server IP or hit enter for 172.16.2.64");
	textBox.draw(0, 0);
	//std::cin >> inputBuffer;
	std::getline(std::cin, stringBuffer);
	if (stringBuffer.length() == 0)
	{
		stringBuffer = "172.16.2.64\0";
	}
	textBox.addMessage(stringBuffer);
	stringBuffer.copy(ip, stringBuffer.length() + 1);
	ip[stringBuffer.length()] = 0;
	stringBuffer = "";
	while (stringBuffer.length() == 0)
	{
		//parse a name
		textBox.addMessage("Enter nickname (16 character max)");
		textBox.draw(0, 0);
		std::getline(std::cin, stringBuffer);
		int prevLen = (int)stringBuffer.length();
		stringTrim(stringBuffer);
		int newLen = (int)stringBuffer.length();
		if (prevLen != newLen)
		{
			textBox.addMessage("Removing spaces ...");
		}
		if (newLen > 16)
		{
			textBox.addMessage("Removing any extra characters...");
			stringBuffer = stringBuffer.substr(0, min(stringBuffer.length(), 16));
		}
		textBox.draw(0, 0);
		if (stringBuffer.length() == 0)
		{
			textBox.addMessage("Username was empty, please try again. ");
		}
	}

	strcpy(name, stringBuffer.c_str());
	name[16] = 0;
	textBox.addMessage(stringBuffer);
	textBox.draw(0, 0);
	peer->Connect(ip, SERVER_PORT, 0, 0);

	bool hasNameBeenSent = false;

	bool isAdmin = false;


	while (!quitting)
	{
		int textY = 0;

		if (strncmp(checkers.action.checkerRoomKey, "lobby", 5) != 0)
		{
			textY = 8;
			textBox.setLineCount(5);
			if (checkers.dirty)
			{
				checkers.drawCheckers(); //this clears the board, so we want to draw afterwards
				textBox.dirty = true; //force a redraw
			}
		}
		else
		{
			textBox.setLineCount(20);
		}
		textBox.draw(0, textY);


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
				textBox.addMessage("Another client has disconnected.");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				textBox.addMessage("Another client has lost the connection.");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				textBox.addMessage("Another client has lost the connection.");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				textBox.addMessage("Our connection request has been accepted.");

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
				textBox.addMessage("A connection is incoming.");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				textBox.addMessage("The server is full.");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				textBox.addMessage("We have been disconnected.");
				quitting = true;
				break;
			case ID_CONNECTION_LOST:
				textBox.addMessage("Connection lost.");
				quitting = true;
				break;

			case ID_USERNAME:
			{
				//if messageFlag is -1, the user already exists and we quit.
				ChatMessage m = ChatMessage::parseMessage(packet);
				textBox.addMessage(m.message);
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
				textBox.addMessage(m.message);
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
				textBox.addMessage(m.message);
				break;
			}
			case ID_GAMEMESSAGE_STRUCT:
			{
				Action act = Action::parseAction(packet);
				checkers.processAction(&act);
				//check win state, send message to all spectators and players if there's a winner
				break;
			}
			case ID_JOIN_ROOM:
			{
				RoomJoinInfo r = RoomJoinInfo::parseRoomInfo(packet);
				checkers.playerNum = r.playerIndex;
				checkers.action.setName(r.roomID, (int)strnlen(r.roomID, 16));
				textBox.clear();
				break;
			}
			case ID_KICK:
				quit();
				textBox.addMessage("You have been kicked");
				break;
			default:
				textBox.addMessage("Message with identifier " + std::to_string(packet->data[idIndex]) + " has arrived.");
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
				gpro_consoleSetCursor(0, textBox.getInputY(textY));
			}
			std::getline(std::cin, stringBuffer);
			checkers.dirty = true;
			strncpy(message, stringBuffer.c_str(), 128);
			message[128] = 0;
			RakNet::RakString messageBackup("%s", message);

			RakNet::Time timeStamp = RakNet::GetTime();

			//create the message to send
			ChatMessage messageToSend;
			messageToSend.isTimestamp = ID_TIMESTAMP;
			messageToSend.time = timeStamp;
			messageToSend.id2 = ID_MESSAGE_STRUCT;

			//prepare the bitstream and the ChatMessage
			RakNet::BitStream bsOut;
			prepBitStream(&bsOut, timeStamp);


			char* startOfSecondWord = chopStr((char*)stringBuffer.c_str(), (int)stringBuffer.length(), ' ');

			std::string secondWord = startOfSecondWord;
			if (strncmp(stringBuffer.c_str(), "private", 7) == 0) //stringBuffer contains only the first word at this point
			{

				char* startOfMessageBody = chopStr((char*)secondWord.c_str(), (int)strlen(startOfSecondWord), ' ');
				if (startOfMessageBody == secondWord.c_str()) //there's no actual message body, we're just sending the words "private" and another word
				{
					messageToSend.messageFlag = PUBLIC; //reset to public
					messageToSend.setText(MESSAGE, secondWord);
				}
				else
				{
					std::string messageBody = startOfMessageBody;

					//copy message recipient and body into ChatMessage from input
					messageToSend.messageFlag = PRIVATE;
					messageToSend.setText(RECIPIENT, secondWord);
					messageToSend.setText(MESSAGE, messageBody);
				}
			}
			else if (strncmp(stringBuffer.c_str(), "command", 7) == 0) //we're using a command
			{
				if (strncmp(secondWord.c_str(), "quit", 4) == 0) //special handling for quit
				{
					quit();
					break;
				}
				else
				{
					ChatMessage::tryCreateCommand(&messageToSend, secondWord, isAdmin);
				}
			}
			else //defaulting to a public message because the first word wasn't recognized as anything
			{
				messageToSend.messageFlag = PUBLIC;
				messageToSend.setText(MESSAGE, messageBackup.C_String(), (int)messageBackup.GetLength());
			}

			if (isAdmin) //add admin flag if needed
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