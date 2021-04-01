using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using TMPro;
using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.UI;

#pragma warning disable CS0618
public class ShieldClient : MonoBehaviour
{
    private const int MAX_CONNECTION = 100;

    private int port = 7777;

    private int hostID;
    private int webHostID;

    private int reliableChannel;
    private int unreliableChannel;

    private int connectionID;

    private float connectionTime;
    private bool isStarted = false;
    private bool isConnected = false;
    private byte error;

    public TextMeshProUGUI text;

    int playerIndex = -1;
    public RemoteInput remotePlayer;
    public PlayerInput localPlayer;

    private void Start()
    {
        DontDestroyOnLoad(gameObject);
        Vector3 vec = Vector3.up + 2 * Vector3.right;
        byte[] arr = MessageOps.GetBytes(vec);
        Vector3 recon = MessageOps.FromBytes<Vector3>(arr);

        Matrix4x4 mat = new Matrix4x4();
        mat.m32 = 100;
        arr = MessageOps.GetBytes(mat);
        Matrix4x4 mat2 = MessageOps.FromBytes<Matrix4x4>(arr);

        ConnectResponseMessage m = new ConnectResponseMessage();
        arr = MessageOps.GetBytes(m);
        ConnectResponseMessage m2 = MessageOps.FromBytes<ConnectResponseMessage>(arr);
    }

    public void Connect()
    {
        NetworkTransport.Init();
        ConnectionConfig cc = new ConnectionConfig();

        reliableChannel = cc.AddChannel(QosType.Reliable);
        unreliableChannel = cc.AddChannel(QosType.Unreliable);

        HostTopology topo = new HostTopology(cc, MAX_CONNECTION);

        hostID = NetworkTransport.AddHost(topo, 0);
        connectionID = NetworkTransport.Connect(hostID, "172.16.4.196", port, 0, out error);

        connectionTime = Time.time;
        isConnected = true;
    }

    public void Connect(string ip)
    {
        NetworkTransport.Init();
        ConnectionConfig cc = new ConnectionConfig();

        reliableChannel = cc.AddChannel(QosType.Reliable);
        unreliableChannel = cc.AddChannel(QosType.Unreliable);

        HostTopology topo = new HostTopology(cc, MAX_CONNECTION);

        hostID = NetworkTransport.AddHost(topo, 0);
        connectionID = NetworkTransport.Connect(hostID, ip, port, 0, out error);

        connectionTime = Time.time;
        isConnected = true;
    }

    private void Update()
    {
        if (!isConnected)
            return;

        int recHostID;
        int connectionID;
        int channelID;
        byte[] recBuffer = new byte[1024];
        int bufferSize = 1024;
        int dataSize;
        byte error;
        //byte[] buffer = BitConverter.GetBytes(localPlayer.transform.position.x);

        //SendPosition();
        NetworkEventType recData = NetworkTransport.Receive(out hostID, out connectionID, out channelID, recBuffer, bufferSize, out dataSize, out error);
        switch (recData)
        {
            case NetworkEventType.Nothing:
                break;
            case NetworkEventType.ConnectEvent:
                NetworkTransport.Send(hostID, connectionID, reliableChannel, new byte[] { (byte)MessageOps.MessageType.CONNECT_REQUEST }, 1, out error);
                break;
            case NetworkEventType.DataEvent:
                switch (MessageOps.ExtractMessageID(ref recBuffer, bufferSize, out byte[] subArr))
                {
                    case MessageOps.MessageType.CONNECT_RESPONSE:

                        ConnectResponseMessage mess = MessageOps.FromBytes<ConnectResponseMessage>(subArr);
                        playerIndex = mess.playerIndex;
                        Debug.Log("Received Player Index!");
                        break;
                }
                //remotePlayer.InterpretPosition(Encoding.UTF8.GetString(recBuffer, 0, dataSize));
                //string str = Encoding.Unicode.GetString(recBuffer, 0, dataSize);
                //text.text = str;
                break;
            case NetworkEventType.DisconnectEvent:
                break;
        }
    }

    private void SendPosition()
    {
        string pos = localPlayer.transform.position.ToString("0.00");
        byte[] buffer = Encoding.UTF8.GetBytes(pos);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, buffer, buffer.Length, out error);
    }
}

#pragma warning restore CS0618
