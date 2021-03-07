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

	/// <summary>
	/// Set text for either the recipient, sender, or message. Has two forms, a string or a char* + int
	/// </summary>
	/// <param name="field">An enum specifying what field to load data into</param>
	/// <param name="text">The data to load, including the number of characters for a cstring</param>
	/// <returns></returns>
	bool setText(ChatMessageField field, const char* text, int length);
	bool setText(ChatMessageField field, std::string text);

	/// <summary>
	/// Extract a ChatMessage from packet data, assuming it was formatted properly (using prepBitStream)
	/// </summary>
	/// <param name="packet"></param>
	/// <returns></returns>
	static ChatMessage parseMessage(RakNet::Packet* packet);

	/// <summary>
	/// Given an input string, try to generate a command that will be sent later
	/// </summary>
	/// <param name="messageToSend">A pointer to a ChatMessage that will be sent</param>
	/// <param name="args">The command parameters, including the key word</param>
	/// <param name="box">The textbox that errors will be written to</param>
	/// <param name="isAdmin">Whether an admin sent the command</param>
	static void tryCreateCommand(ChatMessage* messageToSend, std::string args, TextBox* box, bool isAdmin = false);

	/// <summary>
	/// Quick check that the ChatMessage is not empty. Commands with no additional arguments have a single space written to the message field.
	/// </summary>
	/// <returns></returns>
	bool hasMessage();

};
#pragma pack(pop)
