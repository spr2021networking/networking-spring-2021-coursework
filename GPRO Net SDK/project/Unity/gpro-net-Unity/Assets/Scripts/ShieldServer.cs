#pragma warning disable CS0618 // Type or member is obsolete
using System.Collections;
using System.Collections.Generic;
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

    // Update is called once per frame
    void Update()
    {
        
    }
}
#pragma warning restore CS0618 // Type or member is obsolete