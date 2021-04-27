#pragma warning disable CS0618 // Type or member is obsolete
using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using UnityEngine.Networking;

/// <summary>
/// Network Server for game. The start function is adapted from
/// https://docs.unity3d.com/Manual/UNetUsingTransport.html (UNet docs)
/// https://www.youtube.com/watch?v=qGkkaNkq8co (N3K EN video)
/// 
/// Authors: Scott Dagen, Ben Cooper
/// </summary>
public class ShieldServer : MonoBehaviour
{

    private int reliableChannelID;
    private int unreliableChannelID;

    private int hostID;

    private byte error;

    private int port = 7777;

    private bool running = false;

    private float timer = 5.0f;
    private float timerStorage = 5.0f;
    private int AItoSpawn = 3;
    private int AICounter = 0;
    // Start is called before the first frame update
    void Start()
    {
        //start of adapted code
        NetworkTransport.Init();

        //define the type of connections. Channels are basically data streams, probably used for different types of info
        ConnectionConfig config = new ConnectionConfig();
        reliableChannelID = config.AddChannel(QosType.Reliable);
        unreliableChannelID = config.AddChannel(QosType.Unreliable);

        HostTopology topology = new HostTopology(config, 10); //set to 1 for client, 10 for server. Use the ConnectionConfig from before

        //finish making the IP
        hostID = NetworkTransport.AddHost(topology, port);

        running = true;
        //end of adapted code
    }


    List<int> connections = new List<int>();

    private int positionLength = Encoding.UTF8.GetBytes(new Vector3(0, 0, 0).ToString("0.00")).Length;
    // Update is called once per frame
    void Update()
    {
        Debug.Log("Update");
        byte[] buffer = new byte[1024];
        byte[] sendBuffer;
        for (int j = 0; j < 20; j++)
        {
            NetworkEventType packetType = NetworkTransport.Receive(out int hostID, out int connectionID, out int channelID, buffer, 1024, out int receivedSize, out byte error);
            switch (packetType)
            {
                case NetworkEventType.Nothing: break;
                case NetworkEventType.ConnectEvent:
                    if (!connections.Contains(connectionID))
                    {
                        connections.Add(connectionID);
                    }
                    break;
                case NetworkEventType.DataEvent:
                    switch (MessageOps.ExtractMessageID(ref buffer, receivedSize, out byte[] subArr))
                    {
                        case MessageOps.MessageType.CONNECT_REQUEST:

                            //create a ConnectResponseMessage saying that we have connected.
                            ConnectResponseMessage connMess = new ConnectResponseMessage
                            {
                                playerIndex = connections.IndexOf(connectionID),
                                self = true,
                                connecting = true
                            };
                            Debug.Log(connMess.self + " " + connMess.playerIndex + " " + connMess.connecting);
                            //pack as message
                            sendBuffer = MessageOps.GetBytes(connMess);
                            sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.CONNECT_RESPONSE);
                            NetworkTransport.Send(hostID, connectionID, channelID, sendBuffer, sendBuffer.Length, out error);

                            //reformat the message so it can be sent to other players
                            connMess.self = false;
                            sendBuffer = MessageOps.GetBytes(connMess);
                            sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.CONNECT_RESPONSE);

                            //send message to other players.
                            for (int i = 0; i < connections.Count; i++)
                            {
                                if (connections[i] != connectionID)
                                {
                                    NetworkTransport.Send(hostID, connections[i], channelID, sendBuffer, sendBuffer.Length, out error);
                                }
                            }

                            if (connMess.playerIndex > 0) //this is a new joining player, they don't know who else is here
                            {
                                for (int i = 0; i < connections.Count; i++)
                                {
                                    if (connections[i] != connectionID) //if this isn't our index
                                    {
                                        connMess.playerIndex = i; //set the player index to i and send to the new player
                                        sendBuffer = MessageOps.GetBytes(connMess);
                                        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.CONNECT_RESPONSE);
                                        NetworkTransport.Send(hostID, connectionID, channelID, sendBuffer, sendBuffer.Length, out error);
                                    }
                                }
                            }
                            break;

                        case MessageOps.MessageType.PLAYER_STATE:
                        case MessageOps.MessageType.BULLET_CREATE:
                        case MessageOps.MessageType.BULLET_STATE:
                        case MessageOps.MessageType.BULLET_DESTROY:
                        case MessageOps.MessageType.AI_STATE:
                        case MessageOps.MessageType.AI_DESTROY:
                            for (int i = 0; i < connections.Count; i++)
                            {
                                if (connections[i] != connectionID)
                                {
                                    NetworkTransport.Send(hostID, connections[i], channelID, buffer, receivedSize, out error);
                                }
                            }
                            break;
                        case MessageOps.MessageType.GAME_START:
                        case MessageOps.MessageType.PILLAR_DAMAGE:
                            for (int i = 0; i < connections.Count; i++)
                            {
                                NetworkTransport.Send(hostID, connections[i], channelID, buffer, receivedSize, out error);
                            }
                            break;
                    }
                    break;
                case NetworkEventType.DisconnectEvent:

                    ConnectResponseMessage dcMess = new ConnectResponseMessage();
                    dcMess.playerIndex = connections.IndexOf(connectionID);
                    dcMess.self = false;
                    dcMess.connecting = false;

                    connections.Remove(connectionID);

                    sendBuffer = MessageOps.GetBytes(dcMess);
                    sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.CONNECT_RESPONSE);

                    for (int i = 0; i < connections.Count; i++)
                    {
                        NetworkTransport.Send(hostID, connections[i], channelID, sendBuffer, sendBuffer.Length, out error);
                    }
                    break;

                case NetworkEventType.BroadcastEvent: break;
            }
        }
        //run a timer, this is how frequent enemies spawn, if zero, gen valid coord, send create message
        //gen random float 0 360
        //multiply by deg to rad
        //make a vector of
        //cos of x (degtorad value)
        //sin of z (degtorad value)
        //multiply the vector by radius 45
        timer -= Time.deltaTime;
        if (timer <= 0.0f)
        {
            for (int i = 0; i < AItoSpawn; i++)
            {
                float random = Random.Range(0.0f, 360.0f) * Mathf.Deg2Rad;
                Vector3 position = new Vector3(Mathf.Cos(random), 1.5f, Mathf.Sin(random));
                AICreateMessage message = new AICreateMessage
                {
                    position = position,
                    id = AICounter,
                };
                sendBuffer = MessageOps.GetBytes(message);
                sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.AI_CREATE);
                for (int j = 0; j < connections.Count; j++)
                {
                    NetworkTransport.Send(hostID, connections[j], reliableChannelID, buffer, sendBuffer.Length, out error);
                }
                AICounter++;
            }
            timer = timerStorage;
        }
    }
}
#pragma warning restore CS0618 // Type or member is obsolete
