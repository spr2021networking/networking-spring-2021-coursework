#include "gpro-net/chatmessage.h"

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

bool ChatMessage::setText(ChatMessageField field, const char* text, int length)
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
			strncpy(sender, text, length);
		}
		sender[length] = 0;
		break;
	}

	return true;
}

bool ChatMessage::setText(ChatMessageField field, std::string text)
{
	return setText(field, text.c_str(), (int)text.length());
}