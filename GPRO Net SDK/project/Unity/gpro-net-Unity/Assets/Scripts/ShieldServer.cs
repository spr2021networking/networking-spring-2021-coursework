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
        NetworkEventType packetType = NetworkTransport.Receive(out int hostID, out int connectionID, out int channelID, buffer, 1024, out int receivedSize, out byte error);
        switch (packetType)
        {
            case NetworkEventType.Nothing: break;
            case NetworkEventType.ConnectEvent:
                connections.Add(connectionID);
                break;
            case NetworkEventType.DataEvent:
                switch (MessageOps.ExtractMessageID(ref buffer, receivedSize, out byte[] subArr))
                {
                    case MessageOps.MessageType.CONNECT_REQUEST:

                        ConnectResponseMessage mess = new ConnectResponseMessage();
                        mess.playerIndex = connections.IndexOf(connectionID);

                        byte[] messArr = MessageOps.GetBytes(mess);
                        messArr = MessageOps.PackMessageID(messArr, MessageOps.MessageType.CONNECT_RESPONSE);
                        NetworkTransport.Send(hostID, connectionID, channelID, messArr, receivedSize, out error);
                        break;
                }
                //for (int i = 0; i < connections.Count; i++)
                //{
                //    if (connections[i] != connectionID)
                //    {
                //        NetworkTransport.Send(hostID, connections[i], channelID, buffer, receivedSize, out error);
                //    }
                //}
                break;
            case NetworkEventType.DisconnectEvent: break;

            case NetworkEventType.BroadcastEvent: break;
        }
    }
}
#pragma warning restore CS0618 // Type or member is obsolete