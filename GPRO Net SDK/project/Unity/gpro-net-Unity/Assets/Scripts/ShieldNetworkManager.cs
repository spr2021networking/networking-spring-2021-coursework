#pragma warning disable CS0618 // Type or member is obsolete
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;

public class ShieldNetworkManager : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        //some init stuff that I don't yet understand
        NetworkTransport.Init();
        GlobalConfig gConfig = new GlobalConfig();
        gConfig.MaxPacketSize = 500;
        NetworkTransport.Init(gConfig);

        //define the type of connections. Channels are basically data streams, probably used for different types of info
        ConnectionConfig config = new ConnectionConfig();
        int myReliableChannelId = config.AddChannel(QosType.ReliableSequenced);


        HostTopology topology = new HostTopology(config, 10); //set to 1 for client, 10 for server. Use the ConnectionConfig from before

        //finish making the IP
        int hostId = NetworkTransport.AddHost(topology, 7777);
        Debug.Log(hostId);
        byte error;
        int connectionId = NetworkTransport.Connect(hostId, "192.168.1.42", 8888, 0, out error);
        //
        //byte[] buffer = new byte[16];
        //int bufferLength = 16;
        //NetworkTransport.Send(hostId, connectionId, myReliableChannelId, buffer, bufferLength, out error);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
#pragma warning restore CS0618 // Type or member is obsolete