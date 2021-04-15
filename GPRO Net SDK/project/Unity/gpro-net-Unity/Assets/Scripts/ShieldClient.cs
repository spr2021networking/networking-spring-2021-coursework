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

    private int _playerIndex = -1;
    public int PlayerIndex => _playerIndex;

    public bool OtherPlayerConnected { get; internal set; }

    public RemoteInput remotePlayer;
    public PlayerInput localPlayer;
    public GameObject bullet;

    public List<GameObject> remoteBullets;
    public List<GameObject> localBullets;

    private void Start()
    {
        DontDestroyOnLoad(gameObject);

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
        for (int i = 0; i < 20; i++)
        {
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
                            if (mess.self)
                            {
                                _playerIndex = mess.playerIndex;
                            }
                            else
                            {
                                OtherPlayerConnected = mess.connecting;
                                if (!OtherPlayerConnected)
                                {
                                    if (mess.playerIndex < _playerIndex)
                                    {
                                        _playerIndex--;
                                    }
                                }
                            }
                            Debug.Log("Received Player Index!");
                            break;
                        case MessageOps.MessageType.PLAYER_STATE:
                            PlayerStateMessage playerState = MessageOps.FromBytes<PlayerStateMessage>(subArr);
                            if (remotePlayer != null && playerState.playerIndex != PlayerIndex)
                            {
                                remotePlayer.ProcessInput(playerState);
                            }
                            break;
                        case MessageOps.MessageType.BULLET_CREATE:
                            BulletCreateMessage bulletCreation = MessageOps.FromBytes<BulletCreateMessage>(subArr);
                            if (remotePlayer != null && bulletCreation.playerIndex != PlayerIndex)
                            {
                                GameObject bulletToSpawn = Instantiate(bullet, bulletCreation.position, Quaternion.identity);
                                bulletToSpawn.GetComponent<Rigidbody>().velocity = bulletCreation.velocity;
                                bulletToSpawn.GetComponent<BulletScript>().bulletPlayerIndex = bulletCreation.playerIndex;
                                remoteBullets.Add(bulletToSpawn);
                                //remotePlayer.ProccessBullet(bulletState);
                            }
                            break;
                        case MessageOps.MessageType.BULLET_STATE:
                            BulletStateMessage bulletState = MessageOps.FromBytes<BulletStateMessage>(subArr);
                            foreach (GameObject remoteBullet in remoteBullets)
                            {
                                remoteBullet.transform.position = bulletState.position;
                                remoteBullet.GetComponent<Rigidbody>().velocity = bulletState.velocity;
                            }
                            break;
                        case MessageOps.MessageType.BULLET_DESTROY:
                            BulletDestroyMessage bulletDestroy = MessageOps.FromBytes<BulletDestroyMessage>(subArr);
                            //call playerInput to destroy the bullet if need be
                            //find the bullet with the correct ID and remove it from the list
                            localPlayer.DestroyRemoteBullet(remoteBullets[bulletDestroy.bulletIndex]);
                            //remoteBullets.Remove(remoteBullets[bulletDestroy.bulletIndex]);

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
        

        if (localPlayer && PlayerIndex >= 0)
        {
            SendPosition();
            UpdateLocalBullets();
        }
    }

    private void SendPosition()
    {
        PlayerStateMessage mess = new PlayerStateMessage
        {
            playerIndex = PlayerIndex,
            position = localPlayer.transform.position,
            velocity = localPlayer.rb.velocity,
            rotation = localPlayer.transform.rotation.eulerAngles.y,
            angVel = localPlayer.rb.angularVelocity.y,
            targetShieldRot = 0,
            ticks = DateTime.UtcNow.Ticks
        };
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.PLAYER_STATE);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
        Debug.Log("P" + error);
    }

    public void CreateBullet(Vector3 bPosition, Vector3 bVelocity)
    {
        BulletCreateMessage mess = new BulletCreateMessage
        {
            playerIndex = PlayerIndex,
            position = bPosition,
            velocity = bVelocity,
            ticks = DateTime.UtcNow.Ticks
        };
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.BULLET_CREATE);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
        Debug.Log("B" + error);
    }

    public void DestroyBulletEvent(int bulletIndex)
    {
        BulletDestroyMessage mess = new BulletDestroyMessage
        {
            bulletIndex = bulletIndex,
            ticks = DateTime.UtcNow.Ticks
        };
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.BULLET_DESTROY);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
        Debug.Log("D" + error);
    }

    public void UpdateLocalBullets()
    {
        foreach (GameObject localBullet in localBullets)
        {
            BulletStateMessage mess = new BulletStateMessage
            {
                bulletIndex = localBullet.GetComponent<BulletScript>().id,
                position = localBullet.transform.position,
                velocity = localBullet.GetComponent<Rigidbody>().velocity,
                ticks = DateTime.UtcNow.Ticks
            };
            byte[] sendBuffer = MessageOps.GetBytes(mess);
            sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.BULLET_STATE);
            NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
            Debug.Log("L" + error);
        }
    }
}

#pragma warning restore CS0618
