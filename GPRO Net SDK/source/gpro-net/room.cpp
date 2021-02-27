#include "gpro-net\room.h"

bool RoomJoinInfo::setName(std::string name)
{
	return setName(name.c_str(), (int)name.length());
}

bool RoomJoinInfo::setName(const char* name, int length)
{
	int maxLen = 16;
	if (length > maxLen)
	{
		return false;
	}

	if (length > 0)
	{
		strncpy(roomID, name, length);
	}
	roomID[length] = 0;
	return true;
}

RoomJoinInfo RoomJoinInfo::parseMessage(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	RakNet::MessageID mID;
	RakNet::Time time;
	RoomJoinInfo r;

	//skip the bytes that need to be sent before this message is sent
	bsIn.Read(mID);
	bsIn.Read(time);
	bsIn.Read(mID);

	bsIn.Read(r);
	return r;
}