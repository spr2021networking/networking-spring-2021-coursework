#pragma once

#include "RakNet/RakString.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/BitStream.h"
#include "RakNet/MessageIdentifiers.h"
#include <string>
#include "textbox.h"

/*
* main-client.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: chat messaging handling
*/

enum ChatMessageField
{
	RECIPIENT,
	SENDER,
	MESSAGE
};

enum MessageFlag
{
	PUBLIC = 0,
	PRIVATE,
	COMMAND,
	ISADMIN = 4 // set to 4 as a flag. 4 technically means "public admin", 5 = "private admin", 6 = "command admin"
};

#pragma pack(push, 1)
typedef struct ChatMessage ChatMessage;
struct ChatMessage
{
	RakNet::MessageID isTimestamp; //currently unused, but indicates that this has ID_TIMESTAMP
	RakNet::Time time; //the time this was sent
	RakNet::MessageID id2; //the actual message type
	int messageFlag; //see MessageFlag
	char recipient[17]; //16 bytes of input + null terminator
	char sender[17]; //16 bytes of input + null terminator
	char message[129]; //128 bytes of input + null terminator

	bool setText(ChatMessageField field, const char* text, int length);
	bool setText(ChatMessageField field, std::string text);
	static ChatMessage parseMessage(RakNet::Packet* packet);
	static void tryCreateCommand(ChatMessage* messageToSend, std::string args, TextBox* box, bool isAdmin = false);

	bool hasMessage();

};
#pragma pack(pop)
