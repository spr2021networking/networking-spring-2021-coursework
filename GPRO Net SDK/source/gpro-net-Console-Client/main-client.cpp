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
#include "Windows.h"

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

void quit()
{
	RakNet::BitStream out;
	prepBitStream(&out, RakNet::GetTime(), ID_CONNECTION_LOST);
	out.Write(name);
	peer->Send(&out, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
	peer->Shutdown(300);
}

int main(int const argc, char const* const argv[])
{
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

	printf("Enter server IP or hit enter for 172.16.2.51\n");
	//std::cin >> inputBuffer;
	std::getline(std::cin, stringBuffer);
	if (stringBuffer.length() == 0)
	{
		stringBuffer = "172.16.2.51\0";
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

	printf("Starting the client.\n");
	peer->Connect(ip, SERVER_PORT, 0, 0);

	bool hasNameBeenSent = false;

	bool isAdmin = false;
	while (!quitting)
	{
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
			case ID_REMOTE_NEW_INCOMING_CONNECTION: //todo make this announce who joins
				printf("Another client has connected.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted.\n");

				RakNet::BitStream bsOut;
				if (!hasNameBeenSent) //we send our username to the server here, and we receive an ID_USERNAME message back that sets our admin status
				{
					hasNameBeenSent = true;

					RakNet::BitStream bsOut2;
					prepBitStream(&bsOut2, RakNet::GetTime(), ID_USERNAME);
					ChatMessage mess;
					strncpy(mess.message, name, stringBuffer.length());
					mess.message[stringBuffer.length()] = 0;
					bsOut2.Write(mess);
					peer->Send(&bsOut2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress = packet->systemAddress, false);
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
				ChatMessage m = parseMessage(packet);
				printf("%s\n", m.message);
				if (m.messageType == -1)
				{
					quit();
				}
				else
				{
					isAdmin = (m.messageType & ISADMIN) > 0;
				}
			}
			break;

			case ID_RECEIVE_MESSAGE:
			{
				ChatMessage m = parseMessage(packet);
				printf("%s\n", m.message);
			}
			break;
			case ID_MESSAGE_STRUCT:
			{
				ChatMessage m = parseMessage(packet);
				printf("%s\n", m.message);
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
		bool hasInput = false;
		for (int i = VK_SPACE; i <= 'Z'; i++)
		{
			short ks = GetKeyState(i) >> 8;
			if (ks != 0)
			{
				hasInput = true;
				break;
			}
		}
		if (hasInput)
		{
			std::getline(std::cin, stringBuffer);
			strncpy(message, stringBuffer.c_str(), 128);
			message[128] = 0;
			RakNet::RakString messageBackup("%s", message);

			RakNet::Time timeStamp;
			RakNet::MessageID useTimeStamp;
			RakNet::MessageID messageID = ID_RECEIVE_MESSAGE;
			useTimeStamp = ID_TIMESTAMP;
			timeStamp = RakNet::GetTime();

			if (false)
			{
				RakNet::BitStream bsOut2;
				bsOut2.Write(useTimeStamp);
				bsOut2.Write(timeStamp);
				bsOut2.Write(messageID);
				bsOut2.Write(messageBackup);
				peer->Send(&bsOut2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
			}
			else
			{
				RakNet::BitStream bsOut3;
				prepBitStream(&bsOut3, timeStamp);
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
					if (strncmp(stringBuffer.c_str(), "command", 7) != 0) //make sure this isn't an empty command
					{
						//set message to public and load the entire string buffer into the message.
						messageToSend.messageType = 0;
						strncpy(messageToSend.message, stringBuffer.c_str(), (int)stringBuffer.length());
						messageToSend.message[stringBuffer.length()] = 0;
					}
					else
					{
						printf("[Error] Cannot send empty command\n");
						messageToSend.message[0] = 0;
					}
				}
				else
				{
					std::string secondWord = startOfSecondWord;
					if (strncmp(stringBuffer.c_str(), "private", 7) == 0) //stringBuffer contains only the first word at this point
					{
						messageToSend.messageType = 1;
						char* startOfMessageBody = chopStr((char*)secondWord.c_str(), (int)strlen(startOfSecondWord), ' ');
						if (startOfMessageBody == secondWord.c_str()) //there's no actual message body
						{
							messageToSend.messageType = 0; //reset to public

							strncpy(messageToSend.message, secondWord.c_str(), (int)secondWord.length());
							messageToSend.message[secondWord.length()] = 0; //null terminate
						}
						else
						{
							std::string messageBody = startOfMessageBody;
							strncpy(messageToSend.recipient, secondWord.c_str(), (int)secondWord.length());
							strncpy(messageToSend.message, messageBody.c_str(), (int)messageBody.length());
							messageToSend.message[messageBody.length()] = 0; //null terminate
						}
					}
					else if (strncmp(stringBuffer.c_str(), "command", 7) == 0)
					{
						messageToSend.messageType = 2;

						if (strncmp(secondWord.c_str(), "userlist", 8) == 0)
						{
							strncpy(messageToSend.recipient, "userlist", 8);
							messageToSend.recipient[8] = 0;
							messageToSend.message[0] = ' ';
						}
						if (strncmp(secondWord.c_str(), "kick", 4) == 0)
						{
							if (!isAdmin)
							{
								printf("[Error] Only admins can kick!\n");
								messageToSend.message[0] = 0;
							}
							else
							{
								char* startOfKickTarget = chopStr((char*)secondWord.c_str(), (int)strlen(startOfSecondWord), ' ');
								if (startOfKickTarget == secondWord.c_str()) //no kick target
								{
									printf("[Error] No kick target\n");
									messageToSend.message[0] = 0;
								}
								else
								{
									std::string messageBody = startOfKickTarget;
									strncpy(messageToSend.recipient, secondWord.c_str(), (int)secondWord.length());
									strncpy(messageToSend.message, messageBody.c_str(), (int)messageBody.length());
									messageToSend.message[messageBody.length()] = 0; //null terminate
								}
							}
						}
						if (strncmp(secondWord.c_str(), "stop", 4) == 0)
						{
							if (!isAdmin)
							{
								printf("[Error] Only admins can close the server!\n");
								messageToSend.message[0] = 0;
							}
							else
							{
								strncpy(messageToSend.recipient, "stop", 4);
								messageToSend.message[4] = 0; //null terminate
							}
						}
						else
						{
							printf("[Error] Unknown command\n");
							messageToSend.message[0] = 0;
						}
					}
					else
					{
						messageToSend.messageType = 0;
						strncpy(messageToSend.message, messageBackup.C_String(), (int)messageBackup.GetLength());
						messageToSend.message[messageBackup.GetLength()] = 0;
					}
				}
				if (isAdmin)
				{
					messageToSend.messageType |= ISADMIN;
				}
				bsOut3.Write(messageToSend);
				if (messageToSend.message[0] != 0) //prevents empty messages
				{
					peer->Send(&bsOut3, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
				}
			}
		}

	}
	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}
