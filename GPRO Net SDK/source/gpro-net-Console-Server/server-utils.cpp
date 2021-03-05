#include "gpro-net-Console-Server/server-utils.h"

/// <summary>
/// Send a message to all users in the lobby
/// </summary>
/// <param name="IPToRoom">A map linking the IP to the room name</param>
/// <param name="packet">The packet that sent the original message</param>
/// <param name="peer">The interface that we use to send messages</param>
/// <param name="stream">The data to send</param>
/// <param name="ignoreSender">Whether to prevent the original sender from receiving the message</param>
void sendMessageToLobby(std::map<std::string, std::string>* IPToRoom, RakNet::Packet* packet, RakNet::RakPeerInterface* peer, RakNet::BitStream* stream, bool ignoreSender)
{
	std::map<std::string, std::string>::iterator it;
	std::string packetIP = packet->systemAddress.ToString();
	it = IPToRoom->find(packetIP);
	for (it = IPToRoom->begin(); it != IPToRoom->end(); it++) //check to see if we send the message to a user, as long as they aren't in a game, they get the message
	{
		std::string iteratorIP = it->first;
		if (strncmp(it->second.c_str(), "lobby", 5) == 0)
		{
			//if we're including the sender, this gets sent anyway.
			//If we _aren't_ ignoring the sender, we need to make sure the IP address of the packet doesn't match the iterator's IP
			if (!ignoreSender || strncmp(iteratorIP.c_str(), packetIP.c_str(), it->first.length()) != 0)
			{
				peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(iteratorIP.c_str()), false);
			}
		}
	}
}

void sendMessageToRoom(CheckerRoom* room, RakNet::Packet* packet, RakNet::RakPeerInterface* peer, RakNet::BitStream* stream, bool ignoreSender, bool forceSendAll)
{
	std::vector<Player>::iterator it;
	std::string packetIP = packet->systemAddress.ToString();
	bool isPlayer = false;

	for (it = room->spectators.begin(); it != room->spectators.end(); it++)
	{
		std::string iteratorIP = it->address;
		if (!ignoreSender || strncmp(iteratorIP.c_str(), packetIP.c_str(), iteratorIP.length()) != 0 || forceSendAll)
		{
			peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(it->address.c_str()), false);
		}
	}

	if (strncmp(packetIP.c_str(), room->player1.address.c_str(), packetIP.length()) == 0 || forceSendAll)
	{
		isPlayer = true;
		peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(room->player2.address.c_str()), false);
	}

	if (strncmp(packetIP.c_str(), room->player2.address.c_str(), packetIP.length()) == 0 || forceSendAll)
	{
		isPlayer = true;
		peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::SystemAddress(room->player1.address.c_str()), false);
	}

	if ((isPlayer && !ignoreSender) || forceSendAll)
	{
		peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	}
}
