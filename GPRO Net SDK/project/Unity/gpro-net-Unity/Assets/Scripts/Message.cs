#pragma warning disable CS0618 // Type or member is obsolete
using System;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Networking;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// MessageOps: Many functions related to the creation and sending of messages
/// </summary>
public static class MessageOps
{
    public enum MessageType
    {
        SERVER_CONNECT_REQUEST, ROOM_CONNECT_REQUEST, ROOM_CONNECT_RESPONSE,
        PLAYER_ID, PLAYER_STATE,
        BULLET_CREATE, BULLET_STATE, BULLET_DESTROY,
        AI_CREATE, AI_STATE, AI_DESTROY,
        GAME_START, GAME_DC,
        PILLAR_DAMAGE, GAME_OVER,
        GAME_TIME,
        LOBBY_INFO, LOBBY_JOIN
    }

    //Generic-ified from https://stackoverflow.com/questions/3278827/how-to-convert-a-structure-to-a-byte-array-in-c
    //Converts a struct into a byte array using marshaling
    public static byte[] GetBytes<T>(T data)
    {
        int size = Marshal.SizeOf(data);
        byte[] arr = new byte[size];

        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(data, ptr, true);
        Marshal.Copy(ptr, arr, 0, size);
        Marshal.FreeHGlobal(ptr);
        return arr;
    }

    //Insert the message type at the front of a byte array, increasing the length of the array by one
    public static byte[] PackMessageID(byte[] inArr, MessageType type)
    {
        byte[] ret = new byte[inArr.Length + 1];
        ret[0] = (byte)type;
        Array.Copy(inArr, 0, ret, 1, inArr.Length);
        return ret;
    }

    //Wrapper for GetBytes and PackMessageID, using IMessage.MessageType() for the message type.
    public static byte[] ToMessageArray<T>(T data) where T : IMessage
    {
        byte[] bytes = GetBytes(data);
        bytes = PackMessageID(bytes, data.MessageType());
        return bytes;
    }

    //inspired by and modified from https://stackoverflow.com/questions/3278827/how-to-convert-a-structure-to-a-byte-array-in-c
    //Converts a byte array to T
    public static T FromBytes<T>(byte[] arr)
    {
        T val;
        int size = Marshal.SizeOf(typeof(T)); //this is where our code differs. We don't initialize a new T, we just take its type
        IntPtr ptr = Marshal.AllocHGlobal(size);

        Marshal.Copy(arr, 0, ptr, size);

        val = (T)Marshal.PtrToStructure(ptr, typeof(T)); //we then set val's value only once, saving a little memory and time
        Marshal.FreeHGlobal(ptr);

        return val;
    }

    //Take the first byte of the array and return it, with the extra sent as an out variable
    public static MessageType ExtractMessageID(ref byte[] inArr, int arrSize, out byte[] outArr)
    {
        outArr = new byte[arrSize - 1];
        Array.Copy(inArr, 1, outArr, 0, arrSize - 1);
        return (MessageType)inArr[0];
    }

    //Wrapper for ToMessageArray followed by NetworkTransport.Send.
    public static void SendMessageToServer<T>(T data, int hostID, int connectionID, int channelID, out byte error) where T : IMessage
    {
        byte[] sendBuffer = ToMessageArray(data);
        NetworkTransport.Send(hostID, connectionID, channelID, sendBuffer, sendBuffer.Length, out error);
    }

    /// <summary>
    /// Send a byte array to all members of a room, optionally ignoring the sender. Set connectionID to -1 if there is no sender.
    /// </summary>
    /// <param name="room"></param>
    /// <param name="sendBuffer"></param>
    /// <param name="ignoreSender"></param>
    /// <param name="hostID"></param>
    /// <param name="senderID"></param>
    /// <param name="channelID"></param>
    /// <param name="error"></param>
    public static void SendDataToRoom(ShieldServer.Room room, byte[] sendBuffer, bool ignoreSender, int hostID, int senderID, int channelID, out byte error)
    {
        bool sent = false;
        byte err = 0;
        for (int i = 0; i < room.connections.Count; i++)
        {
            if (!ignoreSender || room.connections[i] != senderID)
            {
                NetworkTransport.Send(hostID, room.connections[i], channelID, sendBuffer, sendBuffer.Length, out err);
                Debug.Log(err);
                sent = true;
            }
        }
        error = err;
        if (!sent)
        {
            error = (byte)NetworkError.BadMessage; //this is the closest enough error for "there was no one to send it to"
        }
    }
}

//public interface that lets us access MessageType from all structs
public interface IMessage
{
    public MessageOps.MessageType MessageType();
}

/// <summary>
/// All of our different message structs. Used for gameplay, connection, and more
/// </summary>

//Used for (1) connecting to a room, (2) notifying of other player connection/disconnection, and (3) kicking people out of an already-started room
public struct RoomConnectResponseMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.ROOM_CONNECT_RESPONSE;

    public int playerIndex;
    public bool self;
    public bool connecting;
    public int roomID;
}

//Player update message, updates the player of the corresponding index in the corresponding room
public struct PlayerStateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.PLAYER_STATE;
    public int playerIndex;
    public Vector3 position;
    public Vector3 velocity;

    public float currentShieldRot;
    public float targetShieldRot;
    public int roomID;
}

//Bullet create message, creates a bullet with the corresponding id in the corresponding room
public struct BulletCreateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.BULLET_CREATE;
    public int playerIndex;
    public Vector3 position;
    public Vector3 velocity;
    public int id; //id within the array
    public int roomID;
}

//Bullet update message, update a bullet with the corresponding id in the corresponding room
public struct BulletStateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.BULLET_STATE;
    public int bulletIndex;
    public Vector3 position;
    public Vector3 velocity;
    public bool hasHitShield;
    public int roomID;
}

public struct BulletDestroyMessage : IMessage //Bullet destroy message, destroy a bullet with the corresponding id in the corresponding room
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.BULLET_DESTROY;
    public int bulletIndex;
    public int roomID;
}


public struct GameStartMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.GAME_START;
    public int roomID;
}

//AI create message, creates an AI with the corresponding id in the corresponding room
public struct AICreateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.AI_CREATE;
    public Vector3 position;
    public int id; //id also determines who owns the AI (id % 2 == playerID % 2 grants control)
    public int roomID;
}

//AI update message, updates the AI with the corresponding id in the corresponding room
public struct AIStateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.AI_STATE;
    public Vector3 position;
    public Vector3 velocity;
    public int id;
    public int roomID;
}

//AI destroy message, destroys the AI with the corresponding id in the corresponding room
public struct AIDestroyMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.AI_DESTROY;
    public int id;
    public int roomID;
}

//pillar damage message, sends the pillar damage event to the correct room
public struct PillarDamageMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.PILLAR_DAMAGE;
    public int newHealth;
    public int roomID;
}

//send game over alert to client
public struct GameOverMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.GAME_OVER;
    public int roomID;
}

//send game time to client
public struct GameTimeMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.GAME_TIME;
    public int time;
    public int roomID;
}

// which lobbies can be accessed at that time
public struct LobbyInfoMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.LOBBY_INFO;
    public bool room0;
    public bool room1;
    public bool room2;
    public bool room3;
}

//ask server to join the specified room
public struct RoomJoinRequestMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.ROOM_CONNECT_REQUEST;
    public int roomID;
}

#pragma warning restore CS0618 // Type or member is obsolete
