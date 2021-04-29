using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using TMPro;
using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.UI;
using System.Linq;
using UnityEngine.SceneManagement;
#pragma warning disable CS0618
public class ShieldClient : MonoBehaviour
{
    private static ShieldClient _instance;

    private const int MAX_CONNECTION = 100;

    private int port = 7777;

    private int hostID;
    private int webHostID;

    private int reliableChannel;
    private int unreliableChannel;

    private int connectionID;

    private float connectionTime;
    public bool isStarted = false;
    private bool isConnected = false;
    private byte error;

    public TextMeshProUGUI text;

    private int _playerIndex = -1;
    public int PlayerIndex => _playerIndex;

    public bool OtherPlayerConnected { get; internal set; }

    public RemoteInput remotePlayer;
    public PlayerInput localPlayer;
    public GameObject bullet;
    public GameObject AI;

    public bool enteringGame; //set to true by server, turned back off when waiting menu sees it

    public GameObject[] remoteBullets;
    public GameObject[] localBullets;
    Dictionary<int, GameObject> AIDictionary;
    public PillarHealth pillarHealth;
    public static ShieldClient Instance { get{ return _instance; } }
    private void Awake()
    {
        if (_instance != null && _instance != this)
        {
            Destroy(this.gameObject);
        }
        else
        {
            _instance = this;
        }
    }
    private void Start()
    {
        DontDestroyOnLoad(gameObject);
        remoteBullets = new GameObject[5];
        localBullets = new GameObject[5];
        AIDictionary = new Dictionary<int, GameObject>();
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
            NetworkEventType recData = NetworkTransport.Receive(out recHostID, out connectionID, out channelID, recBuffer, bufferSize, out dataSize, out error);
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
                            if (isStarted && !mess.self)
                            {
                                NetworkTransport.Disconnect(hostID, connectionID, out error);
                                resetClient();
                                SceneManager.LoadScene("ClientMenu");
                            }
                            else if (mess.self)
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
                                BulletScript bulletInstance = bulletToSpawn.GetComponent<BulletScript>();
                                bulletInstance.bulletPlayerIndex = bulletCreation.playerIndex;
                                bulletInstance.id = bulletCreation.id;
                                remoteBullets[bulletInstance.id] = bulletToSpawn;
                                //remotePlayer.ProccessBullet(bulletState);
                            }
                            break;
                        case MessageOps.MessageType.BULLET_STATE:

                            BulletStateMessage bulletState = MessageOps.FromBytes<BulletStateMessage>(subArr);
                            GameObject remoteBullet = remoteBullets[bulletState.bulletIndex];
                            remoteBullet.transform.position = bulletState.position;
                            remoteBullet.GetComponent<Rigidbody>().velocity = bulletState.velocity;

                            break;
                        case MessageOps.MessageType.BULLET_DESTROY:
                            BulletDestroyMessage bulletDestroy = MessageOps.FromBytes<BulletDestroyMessage>(subArr);
                            //call playerInput to destroy the bullet if need be
                            //find the bullet with the correct ID and remove it from the list
                            localPlayer.DestroyRemoteBullet(bulletDestroy.bulletIndex);
                            //remoteBullets.Remove(remoteBullets[bulletDestroy.bulletIndex]);

                            break;
                        case MessageOps.MessageType.GAME_START:
                            enteringGame = true;
                            break;

                        case MessageOps.MessageType.AI_CREATE:
                            AICreateMessage aICreate = MessageOps.FromBytes<AICreateMessage>(subArr);
                            GameObject AIToSpawn = Instantiate(AI, aICreate.position, Quaternion.identity);
                            AIDictionary.Add(aICreate.id, AIToSpawn);
                            AIScript ai = AIToSpawn.GetComponent<AIScript>();
                            ai.id = aICreate.id;
                            if (aICreate.id % 2 == PlayerIndex % 2)
                            {
                                ai.client = this;
                                ai.isControlledLocally = true;
                            }
                            else
                            {
                                ai.isControlledLocally = false;
                            }
                            break;
                        case MessageOps.MessageType.AI_STATE:
                            AIStateMessage aIState = MessageOps.FromBytes<AIStateMessage>(subArr);
                            if (aIState.id % 2 != PlayerIndex % 2)
                            {
                                GameObject AIToUpdate = AIDictionary[aIState.id];
                                AIToUpdate.transform.position = aIState.position;
                                AIToUpdate.GetComponent<Rigidbody>().velocity = aIState.velocity;
                            }
                            break;
                        case MessageOps.MessageType.AI_DESTROY:
                            AIDestroyMessage aIDestroy = MessageOps.FromBytes<AIDestroyMessage>(subArr);
                            Destroy(AIDictionary[aIDestroy.id]);
                            AIDictionary.Remove(aIDestroy.id);
                            break;
                        case MessageOps.MessageType.PILLAR_DAMAGE:
                            PillarDamageMessage pillarDamage = MessageOps.FromBytes<PillarDamageMessage>(subArr);
                            pillarHealth.CurrentHealth = pillarDamage.newHealth;
                            break;
                        //case MessageOps.MessageType.PILLAR_DESTROY:
                        //    PillarDestroyMessage pillarDestroy = MessageOps.FromBytes<PillarDestroyMessage>(subArr);
                        //    Destroy(pillarHealth.gameObject);
                        //    //display text: Game Over! Time Survived: X Seconds
                    }
                    //remotePlayer.InterpretPosition(Encoding.UTF8.GetString(recBuffer, 0, dataSize));
                    //string str = Encoding.Unicode.GetString(recBuffer, 0, dataSize);
                    //text.text = str;
                    break;
                case NetworkEventType.DisconnectEvent:
                        //make disconnect case
                        //call reset
                        //then load the menu scene

                    break;
            }
        }


        if (localPlayer && PlayerIndex >= 0)
        {
            SendPosition();
            UpdateLocalBullets();
            UpdateLocalAI();
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
            currentShieldRot = localPlayer.shieldHolder.transform.eulerAngles.y,
            targetShieldRot = localPlayer.targetRot,
            ticks = DateTime.UtcNow.Ticks
        };
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.PLAYER_STATE);
        NetworkTransport.Send(hostID, connectionID, unreliableChannel, sendBuffer, sendBuffer.Length, out error);
        Debug.Log("P" + error);
    }

    public void SendBulletCreate(BulletScript bullet, Vector3 bVelocity)
    {
        BulletCreateMessage mess = new BulletCreateMessage
        {
            playerIndex = PlayerIndex,
            position = bullet.transform.position,
            velocity = bVelocity,
            ticks = DateTime.UtcNow.Ticks,
            id = bullet.id
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
        for (int i = 0; i < localBullets.Length; i++)
        {
            if (localBullets[i] != null)
            {
                BulletStateMessage mess = new BulletStateMessage
                {
                    bulletIndex = localBullets[i].GetComponent<BulletScript>().id,
                    position = localBullets[i].transform.position,
                    velocity = localBullets[i].GetComponent<Rigidbody>().velocity,
                    ticks = DateTime.UtcNow.Ticks
                };
                byte[] sendBuffer = MessageOps.GetBytes(mess);
                sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.BULLET_STATE);
                NetworkTransport.Send(hostID, connectionID, unreliableChannel, sendBuffer, sendBuffer.Length, out error);
                Debug.Log("L" + error);
            }
        }
    }

    public void UpdateLocalAI()
    {
        List<GameObject> coll = AIDictionary.Values.ToList();
        for (int i = 0; i < coll.Count; i++)
        {
            if (coll[i] != null && coll[i].GetComponent<AIScript>().isControlledLocally)
            {
                AIStateMessage mess = new AIStateMessage
                {
                    position = AIDictionary[i].transform.position,
                    velocity = AIDictionary[i].GetComponent<Rigidbody>().velocity,
                    id = i
                };
                byte[] sendBuffer = MessageOps.GetBytes(mess);
                sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.AI_STATE);
                NetworkTransport.Send(hostID, connectionID, unreliableChannel, sendBuffer, sendBuffer.Length, out error);
            }
        }
    }

    public void DestroyLocalAI(AIScript ai)
    {

        AIDestroyMessage mess = new AIDestroyMessage
        {
            id = ai.id
        };
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.AI_DESTROY);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
        AIDictionary.Remove(ai.id);
    }
        
    public void RequestStart()
    {
        StartGameMessage mess = new StartGameMessage();
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.GAME_START);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
    }

    public void SendPillarDamage()
    {
        PillarDamageMessage mess = new PillarDamageMessage
        {
            newHealth = pillarHealth.CurrentHealth - 1
        };
        byte[] sendBuffer = MessageOps.GetBytes(mess);
        sendBuffer = MessageOps.PackMessageID(sendBuffer, MessageOps.MessageType.GAME_START);
        NetworkTransport.Send(hostID, connectionID, reliableChannel, sendBuffer, sendBuffer.Length, out error);
    }

    void resetClient()
    {
        hostID = 0;
        webHostID = 0;

        reliableChannel = 0;
        unreliableChannel = 0;

        connectionID = 0;

        connectionTime = 0.0f;
        isStarted = false;
        isConnected = false;
        _playerIndex = -1;
        enteringGame = false;
        remotePlayer = null;
        Array.Clear(localBullets, 0, localBullets.Length);
        Array.Clear(remoteBullets, 0, remoteBullets.Length);
        AIDictionary.Clear();
        pillarHealth = null;
        error = 0;
    }
}

#pragma warning restore CS0618
