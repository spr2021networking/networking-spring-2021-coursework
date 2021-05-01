#pragma warning disable CS0618 // Type or member is obsolete
using System;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Networking;

public static class MessageOps
{
    public enum MessageType
    {
        CONNECT_REQUEST, CONNECT_RESPONSE,
        PLAYER_ID, PLAYER_STATE,
        BULLET_CREATE, BULLET_STATE, BULLET_DESTROY,
        AI_CREATE, AI_STATE, AI_DESTROY,
        GAME_START, GAME_DC,
        PILLAR_DAMAGE, GAME_OVER,
        GAME_TIME
    }

    //inspired by https://stackoverflow.com/questions/3278827/how-to-convert-a-structure-to-a-byte-array-in-c
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

    public static byte[] PackMessageID(byte[] inArr, MessageType type)
    {
        byte[] ret = new byte[inArr.Length + 1];
        ret[0] = (byte)type;
        Array.Copy(inArr, 0, ret, 1, inArr.Length);
        return ret;
    }

    public static byte[] ToMessageArray<T>(T data) where T : IMessage
    {
        byte[] bytes = GetBytes(data);
        bytes = PackMessageID(bytes, data.MessageType());
        return bytes;
    }

    //inspired by https://stackoverflow.com/questions/3278827/how-to-convert-a-structure-to-a-byte-array-in-c
    public static T FromBytes<T>(byte[] arr)
    {
        T val;
        int size = Marshal.SizeOf(typeof(T));
        IntPtr ptr = Marshal.AllocHGlobal(size);

        Marshal.Copy(arr, 0, ptr, size);

        val = (T)Marshal.PtrToStructure(ptr, typeof(T));
        Marshal.FreeHGlobal(ptr);

        return val;
    }

    public static MessageType ExtractMessageID(ref byte[] inArr, int arrSize, out byte[] outArr)
    {
        outArr = new byte[arrSize - 1];
        Array.Copy(inArr, 1, outArr, 0, arrSize - 1);
        return (MessageType)inArr[0];
    }

    public static void SendMessageToServer<T>(T data, int hostID, int connectionID, int channelID, out byte error) where T : IMessage
    {
        byte[] sendBuffer = GetBytes(data);
        sendBuffer = PackMessageID(sendBuffer, data.MessageType());
        NetworkTransport.Send(hostID, connectionID, channelID, sendBuffer, sendBuffer.Length, out error);
    }
}

public interface IMessage
{
    public MessageOps.MessageType MessageType();
}


public struct ConnectResponseMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.CONNECT_RESPONSE;

    public int playerIndex;
    public bool self;
    public bool connecting;
}

public struct PlayerStateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.PLAYER_STATE;
    public int playerIndex;
    public Vector3 position;
    public Vector3 velocity;

    public float currentShieldRot;
    public float targetShieldRot;
}

public struct BulletCreateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.BULLET_CREATE;
    public int playerIndex;
    public Vector3 position;
    public Vector3 velocity;
    public int id; //id within the array
}

public struct BulletStateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.BULLET_STATE;
    public int bulletIndex;
    public Vector3 position;
    public Vector3 velocity;
}

public struct BulletDestroyMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.BULLET_DESTROY;
    public int bulletIndex;
}


public struct StartGameMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.GAME_START;
}

public struct AICreateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.AI_CREATE;
    public Vector3 position;
    public int id;
}

public struct AIStateMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.AI_STATE;
    public Vector3 position;
    public Vector3 velocity;
    public int id;
}

public struct AIDestroyMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.AI_DESTROY;
    public int id;
}

public struct PillarDamageMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.PILLAR_DAMAGE;
    public int newHealth;
}

public struct GameOverMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.GAME_OVER;
}

public struct GameTimeMessage : IMessage
{
    public MessageOps.MessageType MessageType() => MessageOps.MessageType.GAME_TIME;
    public int time;
}
#pragma warning restore CS0618 // Type or member is obsolete