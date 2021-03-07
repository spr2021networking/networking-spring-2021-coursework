/*
* server-utils.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: Messaging utilities
*/

#pragma once

#include <map>
#include <string>
#include "RakNet/RakNetTypes.h"
#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "gpro-net/gpro-net.h"

/// <summary>
/// Send a message to everyone in the lobby
/// </summary>
/// <param name="IPToRoom">The map telling us who's in what room</param>
/// <param name="packet">The packet that sent the original message</param>
/// <param name="peer">The interface for sending messages</param>
/// <param name="stream">The data to send</param>
/// <param name="ignoreSender">Whether we're ignoring the sender</param>
void sendMessageToLobby(std::map<std::string, std::string>* IPToRoom, RakNet::Packet* packet, RakNet::RakPeerInterface* peer, RakNet::BitStream* stream, bool ignoreSender = false);

/// <summary>
/// Send a message to everyone in a given room
/// </summary>
/// <param name="room">The room to send the message to</param>
/// <param name="packet">The packet that sent the original message</param>
/// <param name="peer">The interface for sending messages</param>
/// <param name="stream">The data to send</param>
/// <param name="ignoreSender">Whether we're ignoring the sender</param>
/// <param name="forceSendToAll">overrides the shortcuts that let us skip certain members of the server. Used mostly for leaving rooms</param>
void sendMessageToRoom(CheckerRoom* room, RakNet::Packet* packet, RakNet::RakPeerInterface* peer, RakNet::BitStream* stream, bool ignoreSender = false, bool forceSendToAll = false);