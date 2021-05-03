#pragma warning disable CS0618 // Type or member is obsolete
using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;


/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Shield Client: Network Server for game. The start function is adapted from
/// https://docs.unity3d.com/Manual/UNetUsingTransport.html (UNet docs)
/// https://www.youtube.com/watch?v=qGkkaNkq8co (N3K EN video)
/// </summary>
public class ShieldServer : MonoBehaviour
{
    /// <summary>
    /// Stores all the data for a single room, allowing for up to 4 sessions at once
    /// </summary>
    public class Room
    {
        public int roomID;
        //how many seconds have elapsed in-game
        public float totalTime;
        public float timer = 5.0f;
        public float timerStorage = 10.0f;
        public int AItoSpawn = 1;
        public int AICounter = 0;

        public bool started;
        public bool gameOver;

        public List<int> connections = new List<int>();

        public void Reset()
        {
            totalTime = 0;
            timer = 5.0f;
            timerStorage = 10.0f;
            AItoSpawn = 1;
            AICounter = 0;
            started = false;
            gameOver = false;
            connections.Clear();
        }

        //whether people can join the room
        public bool Open => connections.Count < 2 && !started;

    }

    //connectivity settings
    private int reliableChannelID;
    private int unreliableChannelID;
    private int hostID;
    private byte error;
    private readonly int port = 7777;

    public Room[] rooms = new Room[4];
    public List<int> allConnections = new List<int>();


    // Start is called before the first frame update
    void Start()
    {
        for (int i = 0; i < 4; i++)
        {
            rooms[i] = new Room();
            rooms[i].roomID = i;
        }
        //start of adapted code: https://docs.unity3d.com/Manual/UNetUsingTransport.html (UNet docs)
        //https://www.youtube.com/watch?v=qGkkaNkq8co (N3K EN video)
        NetworkTransport.Init();

        //define the type of connections. Channels are basically data streams, probably used for different types of info
        ConnectionConfig config = new ConnectionConfig();
        reliableChannelID = config.AddChannel(QosType.Reliable);
        unreliableChannelID = config.AddChannel(QosType.Unreliable);

        HostTopology topology = new HostTopology(config, 10); //set to 1 for client, 10 for server. Use the ConnectionConfig from before

        //finish making the IP
        hostID = NetworkTransport.AddHost(topology, port);
        //end of adapted code
    }


    void Update()
    {
        byte[] recBuffer = new byte[1024];
        byte[] sendBuffer;
        for (int j = 0; j < 20; j++)
        {
            NetworkEventType packetType = NetworkTransport.Receive(out int hostID, out int connectionID, out int channelID, recBuffer, 1024, out int receivedSize, out byte error);
            switch (packetType)
            {
                case NetworkEventType.Nothing: break;
                case NetworkEventType.ConnectEvent: //register connection ID in master list
                    if (!allConnections.Contains(connectionID))
                    {
                        allConnections.Add(connectionID);
                    }
                    break;
                case NetworkEventType.DataEvent:
                    switch (MessageOps.ExtractMessageID(ref recBuffer, receivedSize, out byte[] messageBuffer))
                    {
                        case MessageOps.MessageType.SERVER_CONNECT_REQUEST:
                            //send the connecting client data on lobbies
                            SendLobbyInfo(hostID, connectionID, out error);

                            break;
                        case MessageOps.MessageType.ROOM_CONNECT_REQUEST:
                            //read request to join the room and find the requested room reference
                            RoomJoinRequestMessage roomJoin = MessageOps.FromBytes<RoomJoinRequestMessage>(messageBuffer);
                            Room roomToJoin = rooms[roomJoin.roomID];

                            //if the room we want to connect is available, add us to the room
                            if (roomToJoin.Open)
                            {
                                roomToJoin.connections.Add(connectionID);
                                RoomConnectResponseMessage connMess = new RoomConnectResponseMessage
                                {
                                    playerIndex = roomToJoin.connections.Count - 1,
                                    self = true,
                                    connecting = true,
                                    roomID = roomToJoin.roomID
                                };

                                //notify player
                                sendBuffer = MessageOps.ToMessageArray(connMess);
                                NetworkTransport.Send(hostID, connectionID, reliableChannelID, sendBuffer, sendBuffer.Length, out error);

                                //notify other players in that room
                                connMess.self = false;
                                sendBuffer = MessageOps.ToMessageArray(connMess);
                                MessageOps.SendDataToRoom(roomToJoin, sendBuffer, true, hostID, connectionID, reliableChannelID, out error);

                                //update lobby info for everyone else
                                for (int i = 0; i < allConnections.Count; i++)
                                {
                                    SendLobbyInfo(hostID, allConnections[i], out error);
                                }

                                //if we're not the first to join, tell us who else is in the room
                                if (connMess.playerIndex > 0)
                                {
                                    for (int i = 0; i < roomToJoin.connections.Count; i++)
                                    {
                                        if (roomToJoin.connections[i] != connectionID) //if this isn't our index
                                        {
                                            connMess.playerIndex = i; //set the player index to i and send to the new player
                                            sendBuffer = MessageOps.ToMessageArray(connMess);
                                            NetworkTransport.Send(hostID, connectionID, reliableChannelID, sendBuffer, sendBuffer.Length, out error); //can't simply send to room because we make changes to the message
                                        }
                                    }
                                }
                            }
                            else //room isn't open, refresh our info
                            {
                                SendLobbyInfo(hostID, connectionID, out error);
                            }
                            break;

                            //the next several cases are identical: read message, get room ID, relay message with appropriate channel.
                        case MessageOps.MessageType.PLAYER_STATE:
                            PlayerStateMessage playerState = MessageOps.FromBytes<PlayerStateMessage>(messageBuffer);
                            MessageOps.SendDataToRoom(rooms[playerState.roomID], recBuffer, true, hostID, connectionID, unreliableChannelID, out error);
                            break;
                        case MessageOps.MessageType.BULLET_CREATE:
                            BulletCreateMessage bulletCreate = MessageOps.FromBytes<BulletCreateMessage>(messageBuffer);
                            MessageOps.SendDataToRoom(rooms[bulletCreate.roomID], recBuffer, true, hostID, connectionID, reliableChannelID, out error);
                            break;
                        case MessageOps.MessageType.BULLET_STATE:
                            BulletStateMessage bulletState = MessageOps.FromBytes<BulletStateMessage>(messageBuffer);
                            MessageOps.SendDataToRoom(rooms[bulletState.roomID], recBuffer, true, hostID, connectionID, unreliableChannelID, out error);
                            break;
                        case MessageOps.MessageType.BULLET_DESTROY:
                            BulletDestroyMessage bulletDestroy = MessageOps.FromBytes<BulletDestroyMessage>(messageBuffer);
                            MessageOps.SendDataToRoom(rooms[bulletDestroy.roomID], recBuffer, true, hostID, connectionID, reliableChannelID, out error);
                            break;
                        case MessageOps.MessageType.AI_STATE:
                            AIStateMessage aiState = MessageOps.FromBytes<AIStateMessage>(messageBuffer);
                            MessageOps.SendDataToRoom(rooms[aiState.roomID], recBuffer, true, hostID, connectionID, unreliableChannelID, out error);
                            break;
                        case MessageOps.MessageType.AI_DESTROY:
                            AIDestroyMessage aiDestroy = MessageOps.FromBytes<AIDestroyMessage>(messageBuffer);
                            MessageOps.SendDataToRoom(rooms[aiDestroy.roomID], recBuffer, true, hostID, connectionID, reliableChannelID, out error);
                            break;
                        case MessageOps.MessageType.GAME_START:
                            //check if room _can_ start before starting it.
                            GameStartMessage gameStart = MessageOps.FromBytes<GameStartMessage>(messageBuffer);
                            if (rooms[gameStart.roomID].connections.Count == 2)
                            {
                                rooms[gameStart.roomID].started = true;
                                MessageOps.SendDataToRoom(rooms[gameStart.roomID], recBuffer, false, hostID, connectionID, reliableChannelID, out error);
                            }
                            break;
                        case MessageOps.MessageType.PILLAR_DAMAGE:
                            PillarDamageMessage pillarDamage = MessageOps.FromBytes<PillarDamageMessage>(messageBuffer);
                            //check pillar health before sending message.
                            if (pillarDamage.newHealth > 0)
                            {
                                MessageOps.SendDataToRoom(rooms[pillarDamage.roomID], recBuffer, false, hostID, connectionID, reliableChannelID, out error);
                            }
                            else
                            {
                                //send game over message. It'll reset when people leave
                                GameOverMessage gameOverMess = new GameOverMessage();
                                gameOverMess.roomID = pillarDamage.roomID;
                                sendBuffer = MessageOps.ToMessageArray(gameOverMess);
                                MessageOps.SendDataToRoom(rooms[gameOverMess.roomID], sendBuffer, false, hostID, connectionID, reliableChannelID, out error);
                                rooms[gameOverMess.roomID].gameOver = true;
                            }

                            break;
                    }
                    break;
                case NetworkEventType.DisconnectEvent:
                    //find the room that contains the disconnected player
                    Room room = Array.Find(rooms, room => room.connections.Contains(connectionID));
                    if (room != null) //they were in a room, quit from room
                    {
                        RoomConnectResponseMessage dcMess = new RoomConnectResponseMessage();
                        dcMess.playerIndex = allConnections.IndexOf(connectionID);
                        dcMess.self = false;
                        dcMess.connecting = false;
                        dcMess.roomID = room.roomID;
                        room.connections.Remove(connectionID);

                        //send to room telling them about the DC
                        sendBuffer = MessageOps.ToMessageArray(dcMess);
                        MessageOps.SendDataToRoom(room, sendBuffer, true, hostID, connectionID, reliableChannelID, out error);

                        //reset room if no one's in it or if the game had already started
                        if (room.connections.Count == 0 || room.started)
                        {
                            room.Reset();
                        }
                    }

                    //remove connection
                    allConnections.Remove(connectionID);

                    //tell everyone about the new lobby data
                    for (int i = 0; i < allConnections.Count; i++)
                    {
                        SendLobbyInfo(hostID, allConnections[i], out error);
                    }
                    break;

                case NetworkEventType.BroadcastEvent: break;
            }
        }

        //tick the room
        for (int i = 0; i < rooms.Length; i++)
        {
            Room room = rooms[i];
            //if the game is currently running, decrement timer
            if (room.started && !room.gameOver)
            {
                room.timer -= Time.deltaTime;
                //if we're ready to spawn enemies
                if (room.timer <= 0.0f)
                {
                    //spawn enemies
                    for (int j = 0; j < room.AItoSpawn; j++)
                    {
                        //calculate AI position
                        float random = UnityEngine.Random.Range(0.0f, 360.0f) * Mathf.Deg2Rad;
                        Vector3 position = new Vector3(Mathf.Cos(random), 0, Mathf.Sin(random));
                        position *= 45;
                        position.y = 1.5f;

                        //send message
                        AICreateMessage aiCreateMessage = new AICreateMessage
                        {
                            position = position,
                            id = room.AICounter,
                            roomID = room.roomID
                        };
                        sendBuffer = MessageOps.ToMessageArray(aiCreateMessage);
                        MessageOps.SendDataToRoom(room, sendBuffer, false, hostID, -1, reliableChannelID, out error);
                        Debug.Log("A: " + (int)error);
                        room.AICounter++;
                    }
                    room.timer = room.timerStorage;
                }

                //increment total time, and when a full second has passed, send the GameTimeMessage.
                //could be used for future timing events
                float tmp = room.totalTime + Time.deltaTime;
                if (Mathf.FloorToInt(room.totalTime) == Mathf.FloorToInt(tmp) - 1)
                {
                    GameTimeMessage gameTimeMess = new GameTimeMessage();
                    gameTimeMess.time = Mathf.FloorToInt(tmp);
                    sendBuffer = MessageOps.ToMessageArray(gameTimeMess);
                    MessageOps.SendDataToRoom(room, sendBuffer, false, hostID, -1, reliableChannelID, out error);
                }
                room.totalTime = tmp;
            }
        }
    }

    //send lobby data to the client
    private void SendLobbyInfo(int hostID, int connectionID, out byte error)
    {
        LobbyInfoMessage lobbyMess = new LobbyInfoMessage();
        lobbyMess.room0 = rooms[0].Open;
        lobbyMess.room1 = rooms[1].Open;
        lobbyMess.room2 = rooms[2].Open;
        lobbyMess.room3 = rooms[3].Open;
        byte[] sendBuffer = MessageOps.ToMessageArray(lobbyMess);
        NetworkTransport.Send(hostID, connectionID, reliableChannelID, sendBuffer, sendBuffer.Length, out error);
    }
}
#pragma warning restore CS0618 // Type or member is obsolete
