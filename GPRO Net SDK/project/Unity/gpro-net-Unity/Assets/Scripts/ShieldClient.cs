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

    public RemoteInput remotePlayer;
    public PlayerInput localPlayer;

    private void Start()
    {
        Vector3 vec = Vector3.up + 2 * Vector3.right;
        byte[] arr = GetBytes(vec);
        Vector3 recon = FromBytes<Vector3>(arr);

        Matrix4x4 mat = new Matrix4x4();
        mat.m32 = 100;
        arr = GetBytes(mat);
        Matrix4x4 mat2 = FromBytes<Matrix4x4>(arr);
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

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.G))
        {
            Connect();
        }
        if (!isConnected)
            return;

        int recHostID;
        int connectionID;
        int channelID;
        byte[] recBuffer = new byte[1024];
        int bufferSize = 1024;
        int dataSize;
        byte error;
        byte[] buffer = BitConverter.GetBytes(localPlayer.transform.position.x);

        SendPosition();
        NetworkEventType recData = NetworkTransport.Receive(out hostID, out connectionID, out channelID, recBuffer, bufferSize, out dataSize, out error);
        switch (recData)
        {
            case NetworkEventType.Nothing:
                break;
            case NetworkEventType.ConnectEvent:
                break;
            case NetworkEventType.DataEvent:
                remotePlayer.InterpretPosition(Encoding.UTF8.GetString(recBuffer, 0, dataSize));
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


    public byte[] GetBytes<T>(T data)
    {
        int size = Marshal.SizeOf(data);
        byte[] arr = new byte[size];

        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(data, ptr, true);
        Marshal.Copy(ptr, arr, 0, size);
        Marshal.FreeHGlobal(ptr);
        return arr;
    }

    public T FromBytes<T>(byte[] arr)
    {
        T val;
        int size = Marshal.SizeOf(typeof(T));
        IntPtr ptr = Marshal.AllocHGlobal(size);

        Marshal.Copy(arr, 0, ptr, size);

        val = (T)Marshal.PtrToStructure(ptr, typeof(T));
        Marshal.FreeHGlobal(ptr);

        return val;
    }
}

#pragma warning restore CS0618
