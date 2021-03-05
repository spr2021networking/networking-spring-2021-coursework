#include "gpro-net/chatmessage.h"
#include "gpro-net/gpro-net.h"


/*
* main-client.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: chat messaging handling
*/

/// <summary>
/// Extract a ChatMessage from packet data, assuming it was formatted properly (using prepBitStream)
/// </summary>
/// <param name="packet"></param>
/// <returns></returns>
ChatMessage ChatMessage::parseMessage(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	RakNet::MessageID mID;
	RakNet::Time time;
	ChatMessage m;

	//skip the bytes that need to be sent before this message is sent
	bsIn.Read(mID);
	bsIn.Read(time);
	bsIn.Read(mID);

	bsIn.Read(m);
	return m;
}

bool ChatMessage::hasMessage()
{
	return message[0] != 0;
}

bool ChatMessage::setText(ChatMessageField field, const char* text, int length) //set either the message, sender, and recipient
{
	if (field < 0 || field > 2 || length < 0)
	{
		return false;
	}

	int maxLen = field == MESSAGE ? 128 : 16;
	if (length > maxLen)
	{
		return false;
	}

	switch (field)
	{
	case MESSAGE:
		if (length > 0)
		{
			strncpy(message, text, length);
		}
		message[length] = 0;
		break;

	case SENDER:
		if (length > 0)
		{
			strncpy(sender, text, length);
		}
		sender[length] = 0;
		break;

	case RECIPIENT:
		if (length > 0)
		{
			strncpy(recipient, text, length);
		}
		recipient[length] = 0;
		break;
	}

	return true;
}

bool ChatMessage::setText(ChatMessageField field, std::string text)
{
	return setText(field, text.c_str(), (int)text.length());
}

void ChatMessage::tryCreateCommand(ChatMessage* messageToSend, std::string args, TextBox* box, bool isAdmin) //create a command if valid
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
			box->addMessage("[Error] Kicking currently does not function correctly!");
			messageToSend->setText(MESSAGE, "");
			return;
		}
		/*
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
		}*/
	}
	else if (strncmp(args.c_str(), "stop", 4) == 0) //stop server
	{
		if (!isAdmin)
		{
			box->addMessage("[Error] Only admins can close the server!");
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
		if (startOfRoomName == args.c_str()) //failsafe check to require a room name
		{
			box->addMessage("[Error] No room stated");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			std::string messageBody = startOfRoomName;
			messageToSend->setText(RECIPIENT, "createroom");
			messageToSend->setText(MESSAGE, messageBody);
		}
	}
	else if (strncmp(args.c_str(), "joinroom", 8) == 0) //join room
	{
		char* startOfRoomName = chopStr((char*)args.c_str(), (int)args.length(), ' ');
		if (startOfRoomName == args.c_str()) //failsafe check to require a room name
		{
			box->addMessage("[Error] No room stated");
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
		if (startOfRoomName == args.c_str()) //failsafe check to require a room name
		{
			box->addMessage("[Error] No room stated");
			messageToSend->setText(MESSAGE, "");
		}
		else
		{
			std::string messageBody = startOfRoomName;
			messageToSend->setText(RECIPIENT, "spectate");
			messageToSend->setText(MESSAGE, messageBody);
		}
	}
	else if (strncmp(args.c_str(), "roomlist", 8) == 0) // list all rooms
	{
		messageToSend->setText(RECIPIENT, "roomlist");
		messageToSend->setText(MESSAGE, " ");
	}
	else if (strncmp(args.c_str(), "leaveroom", 9) == 0) //leave room
	{
		messageToSend->setText(RECIPIENT, "leaveroom");
		messageToSend->setText(MESSAGE, " ");
	}
	else
	{
		box->addMessage("[Error] Unknown command");
		messageToSend->setText(MESSAGE, "");
	}
}