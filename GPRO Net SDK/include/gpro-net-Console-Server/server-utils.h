#pragma once

#include <map>
#include <string>
#include "RakNet/RakNetTypes.h"
#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "gpro-net/gpro-net.h"

void sendMessageToLobby(std::map<std::string, std::string>* IPToRoom, RakNet::Packet* packet, RakNet::RakPeerInterface* peer, RakNet::BitStream* stream, bool ignoreSender = false);
void sendMessageToRoom(CheckerRoom* room, RakNet::Packet* packet, RakNet::RakPeerInterface* peer, RakNet::BitStream* stream, bool ignoreSender = false, bool forceSendToAll = false);