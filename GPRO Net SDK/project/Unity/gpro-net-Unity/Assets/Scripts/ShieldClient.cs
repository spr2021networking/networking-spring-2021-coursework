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
    public GameObject AI;

    public bool enteringGame; //set to true by server, turned back off when waiting menu sees it

    internal BulletScript[] remoteBullets = new BulletScript[5];

    internal BulletScript[] localBullets = new BulletScript[5];

    public Dictionary<int, AIScript> AIDictionary = new Dictionary<int, AIScript>();
    public PillarHealth pillarHealth;

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
            NetworkEventType recData = NetworkTransport.Receive(out recHostID, out connectionID, out channelID, recBuffer, bufferSize, out dataSize, out error);
            switch (recData)
            {
                case NetworkEventType.Nothing:
                    break;
                case NetworkEventType.ConnectEvent:
                    NetworkTransport.Send(hostID, connectionID, reliableChannel, new byte[] { (byte)MessageOps.MessageType.CONNECT_REQUEST }, 1, out error);
                    break;
                case NetworkEventType.DataEvent:
                    MessageOps.MessageType type = MessageOps.ExtractMessageID(ref recBuffer, bufferSize, out byte[] subArr);
                    switch (type)
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
                            BulletCreateMessage bulletCreate = MessageOps.FromBytes<BulletCreateMessage>(subArr);
                            if (remotePlayer != null && bulletCreate.playerIndex != PlayerIndex)
                            {
                                GameObject spawnedBullet = Instantiate(this.bullet, bulletCreate.position, Quaternion.identity);
                                spawnedBullet.GetComponent<Rigidbody>().velocity = bulletCreate.velocity;
                                BulletScript bullet = spawnedBullet.GetComponent<BulletScript>();
                                bullet.bulletPlayerIndex = bulletCreate.playerIndex;
                                bullet.id = bulletCreate.id;
                                remoteBullets[bullet.id] = bullet;
                                //remotePlayer.ProccessBullet(bulletState);
                            }
                            break;
                        case MessageOps.MessageType.BULLET_STATE:

                            BulletStateMessage bulletState = MessageOps.FromBytes<BulletStateMessage>(subArr);
                            BulletScript remoteBullet = remoteBullets[bulletState.bulletIndex];
                            if (remoteBullet != null)
                            {
                                remoteBullet.transform.position = bulletState.position;
                                remoteBullet.GetComponent<Rigidbody>().velocity = bulletState.velocity;
                            }
                            else
                            {
                                Debug.LogWarning($"Bullet with id {bulletState.bulletIndex} from the other player is null");
                            }

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
                            Debug.Log("Received AI Message");
                            AICreateMessage aiCreate = MessageOps.FromBytes<AICreateMessage>(subArr);
                            GameObject spawnedAI = Instantiate(AI, aiCreate.position, Quaternion.identity);
                            AIScript ai = spawnedAI.GetComponent<AIScript>();
                            ai.id = aiCreate.id;
                            ai.pillar = pillarHealth;
                            ai.InitVelocity();
                            AIDictionary.Add(ai.id, ai);
                            if (ai.id % 2 == PlayerIndex % 2)
                            {
                                ai.client = this;
                                ai.isControlledLocally = true;
                                Debug.Log("Local");
                            }
                            else
                            {
                                ai.isControlledLocally = false;
                                Debug.Log("Remote");
                            }
                            break;
                        case MessageOps.MessageType.AI_STATE:
                            AIStateMessage aiState = MessageOps.FromBytes<AIStateMessage>(subArr);
                            if (aiState.id % 2 != PlayerIndex % 2)
                            {
                                AIScript AIToUpdate = AIDictionary[aiState.id];
                                if (AIToUpdate != null)
                                {
                                    AIToUpdate.transform.position = aiState.position;
                                    AIToUpdate.rb.velocity = aiState.velocity;
                                }
                            }
                            break;
                        case MessageOps.MessageType.AI_DESTROY:
                            AIDestroyMessage aiDestroy = MessageOps.FromBytes<AIDestroyMessage>(subArr);
                            AIScript AIToDestroy = AIDictionary[aiDestroy.id];
                            if (AIToDestroy != null)
                            {
                                AIDictionary.Remove(aiDestroy.id);
                                Destroy(AIToDestroy.gameObject);
                            }
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
                    break;
            }
        }


        if (localPlayer && PlayerIndex >= 0)
        {
            SendPlayerState();
            UpdateLocalBullets();
            UpdateLocalAI();
        }
    }

    /// <summary>
    /// Generate a PlayerStateMessage and send it.
    /// </summary>
    private void SendPlayerState()
    {
        if (localPlayer && PlayerIndex >= 0)
        {
            PlayerStateMessage mess = new PlayerStateMessage();
            mess.playerIndex = PlayerIndex;
            mess.position = localPlayer.transform.position;
            Rigidbody rb = localPlayer.rb;
            mess.velocity = rb ? rb.velocity : Vector3.zero;
            mess.rotation = localPlayer.transform.rotation.eulerAngles.y;
            mess.angVel = rb ? rb.angularVelocity.y : 0;
            mess.currentShieldRot = localPlayer.shieldHolder.transform.eulerAngles.y;
            mess.targetShieldRot = localPlayer.targetRot;
            mess.ticks = DateTime.UtcNow.Ticks;

            MessageOps.SendMessageToServer(mess, MessageOps.MessageType.PLAYER_STATE, hostID, connectionID, unreliableChannel, out error);
            Debug.Log("P" + error);
        }
        else
        {
            Debug.LogError("Either Local Player does not exist or PlayerIndex < 0");
        }
    }

    /// <summary>
    /// Generate a BulletCreateMessage and send it
    /// </summary>
    /// <param name="bullet"></param>
    /// <param name="bVelocity"></param>
    public void SendBulletCreate(BulletScript bullet, Vector3 bVelocity)
    {
        BulletCreateMessage mess = new BulletCreateMessage();
        mess.playerIndex = PlayerIndex;
        mess.position = bullet.transform.position;
        mess.velocity = bVelocity;
        mess.ticks = DateTime.UtcNow.Ticks;
        mess.id = bullet.id;

        MessageOps.SendMessageToServer(mess, MessageOps.MessageType.BULLET_CREATE, hostID, connectionID, reliableChannel, out error);
        Debug.Log("B" + error);
    }

    /// <summary>
    /// Genreate a BulletDestroyMessage and send it. Runs before the bullet object is locally destroyed
    /// </summary>
    /// <param name="bulletIndex"></param>
    public void SendBulletDestroy(int bulletIndex)
    {
        if (localBullets[bulletIndex] != null)
        {
            BulletDestroyMessage mess = new BulletDestroyMessage();
            mess.bulletIndex = bulletIndex;
            mess.ticks = DateTime.UtcNow.Ticks;

            MessageOps.SendMessageToServer(mess, MessageOps.MessageType.BULLET_DESTROY, hostID, connectionID, reliableChannel, out error);
            Debug.Log("D" + error);
        }
    }

    /// <summary>
    /// Loop through all local bullets and send messages to server.
    /// </summary>
    public void UpdateLocalBullets()
    {
        for (int i = 0; i < localBullets.Length; i++)
        {
            if (localBullets[i] != null)
            {
                BulletStateMessage mess = new BulletStateMessage();
                mess.bulletIndex = localBullets[i].GetComponent<BulletScript>().id;
                mess.position = localBullets[i].transform.position;
                mess.velocity = localBullets[i].GetComponent<Rigidbody>().velocity;
                mess.ticks = DateTime.UtcNow.Ticks;

                MessageOps.SendMessageToServer(mess, MessageOps.MessageType.BULLET_STATE, hostID, connectionID, unreliableChannel, out error);
                Debug.Log("L" + error);
            }
        }
    }

    /// <summary>
    /// Update all player-owned AI and send messages to server
    /// </summary>
    public void UpdateLocalAI()
    {
        List<AIScript> ais = AIDictionary.Values.ToList();
        for (int i = 0; i < ais.Count; i++)
        {
            if (ais[i] != null && ais[i].isControlledLocally)
            {
                AIStateMessage mess = new AIStateMessage();
                mess.position = ais[i].transform.position;
                mess.velocity = ais[i].GetComponent<Rigidbody>().velocity;
                mess.id = ais[i].id;

                MessageOps.SendMessageToServer(mess, MessageOps.MessageType.AI_STATE, hostID, connectionID, unreliableChannel, out error);
                Debug.Log("A" + error);
            }
        }
    }

    public void DestroyLocalAI(int id)
    {
        AIScript ai = AIDictionary[id];
        if (ai != null && ai.isControlledLocally)
        {
            AIDestroyMessage mess = new AIDestroyMessage();
            mess.id = id;

            MessageOps.SendMessageToServer(mess, MessageOps.MessageType.AI_DESTROY, hostID, connectionID, reliableChannel, out error);
            AIDictionary.Remove(id);
            Destroy(ai.gameObject);
        }
    }

    public void SendStartRequest()
    {
        StartGameMessage mess = new StartGameMessage();
        MessageOps.SendMessageToServer(mess, MessageOps.MessageType.GAME_START, hostID, connectionID, reliableChannel, out error);
    }

    public void SendPillarDamage()
    {
        PillarDamageMessage mess = new PillarDamageMessage();
        mess.newHealth = pillarHealth.CurrentHealth - 1;
        MessageOps.SendMessageToServer(mess, MessageOps.MessageType.PILLAR_DAMAGE, hostID, connectionID, reliableChannel, out error);
    }
}

#pragma warning restore CS0618
