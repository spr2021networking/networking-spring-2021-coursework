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

    public bool receivedLobbyInfo;
    public bool[] lobbyStates = new bool[4];

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
    public int timer;

    public bool gameOver;

    public int roomID = 0;

    public static ShieldClient Instance { get; private set; }
    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
        }
        else
        {
            Instance = this;
        }
    }
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
        FindObjectOfType<MainMenu>().lobbyShown = false;
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
                    NetworkTransport.Send(hostID, connectionID, reliableChannel, new byte[] { (byte)MessageOps.MessageType.SERVER_CONNECT_REQUEST }, 1, out error);
                    break;
                case NetworkEventType.DataEvent:
                    MessageOps.MessageType type = MessageOps.ExtractMessageID(ref recBuffer, bufferSize, out byte[] subArr);
                    switch (type)
                    {
                        case MessageOps.MessageType.LOBBY_INFO:
                            LobbyInfoMessage mess = MessageOps.FromBytes<LobbyInfoMessage>(subArr);
                            receivedLobbyInfo = true;
                            lobbyStates[0] = mess.room0;
                            lobbyStates[1] = mess.room1;
                            lobbyStates[2] = mess.room2;
                            lobbyStates[3] = mess.room3;
                            break;
                        case MessageOps.MessageType.ROOM_CONNECT_RESPONSE:
                            RoomConnectResponseMessage connectResponse = MessageOps.FromBytes<RoomConnectResponseMessage>(subArr);

                            //someone disconnected from the room, kick to main menu
                            if (isStarted && !connectResponse.self)
                            {
                                ResetClient();
                            }
                            
                            //we're joining a room, we literally can't receive this if the room has already started
                            //similarly, we can't receive self not connecting because we've already DCed
                            else if (connectResponse.self)
                            {
                                _playerIndex = connectResponse.playerIndex;
                                roomID = connectResponse.roomID;
                            }

                            //room hasn't started, someone either joined or disconnected and it wasn't you so we update IDs.
                            else
                            {
                                OtherPlayerConnected = connectResponse.connecting;
                                if (!OtherPlayerConnected)
                                {
                                    if (connectResponse.playerIndex < _playerIndex)
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
                            }
                            break;
                        case MessageOps.MessageType.BULLET_STATE:

                            BulletStateMessage bulletState = MessageOps.FromBytes<BulletStateMessage>(subArr);
                            BulletScript remoteBullet = remoteBullets[bulletState.bulletIndex];
                            if (remoteBullet != null)
                            {
                                
                                remoteBullet.SetNewPositionAndVelocity(bulletState.position, bulletState.velocity);
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
                                if (AIDictionary.TryGetValue(aiState.id, out AIScript AIToUpdate))
                                {
                                    if (AIToUpdate != null)
                                    {
                                        AIToUpdate.SetNewPositionAndVelocity(aiState.position, aiState.velocity);
                                    }
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
                        case MessageOps.MessageType.GAME_OVER:
                            Destroy(pillarHealth.gameObject);
                            gameOver = true;
                            FindObjectOfType<PlayerReference>().gameOver.text = $"Game Over! Time Survived: {timer} Seconds";
                            Invoke(nameof(ResetClient), 5.0f);
                            break;
                        case MessageOps.MessageType.GAME_TIME:
                            GameTimeMessage time = MessageOps.FromBytes<GameTimeMessage>(subArr);
                            timer = time.time;
                            break;
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
            mess.currentShieldRot = localPlayer.shieldHolder.transform.eulerAngles.y;
            mess.targetShieldRot = localPlayer.targetRot;
            mess.roomID = roomID;

            MessageOps.SendMessageToServer(mess, hostID, connectionID, unreliableChannel, out error);
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
        mess.id = bullet.id;
        mess.roomID = roomID;

        MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
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
            mess.roomID = roomID;

            MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
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
                mess.roomID = roomID;

                MessageOps.SendMessageToServer(mess, hostID, connectionID, unreliableChannel, out error);
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
                mess.roomID = roomID;

                MessageOps.SendMessageToServer(mess, hostID, connectionID, unreliableChannel, out error);
                Debug.Log("A" + error);
            }
        }
    }

    public void DestroyLocalAI(int id)
    {
        if (AIDictionary.TryGetValue(id, out AIScript ai))
        {
            if (ai != null && ai.isControlledLocally)
            {
                AIDestroyMessage mess = new AIDestroyMessage();
                mess.id = id;
                mess.roomID = roomID;

                MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
                AIDictionary.Remove(id);
                Destroy(ai.gameObject);
            }
        }
    }

    public void SendStartRequest()
    {
        GameStartMessage mess = new GameStartMessage();
        mess.roomID = roomID;
        MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
    }

    public void SendPillarDamage()
    {
        PillarDamageMessage mess = new PillarDamageMessage();
        mess.newHealth = pillarHealth.CurrentHealth - 1;
        mess.roomID = roomID;
        MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
    }

    public void ResetClient()
    {
        NetworkTransport.Disconnect(hostID, connectionID, out error);
        hostID = 0;
        webHostID = 0;

        reliableChannel = 0;
        unreliableChannel = 0;

        connectionID = 0;
        OtherPlayerConnected = false;

        connectionTime = 0.0f;
        isStarted = false;
        isConnected = false;
        _playerIndex = -1;
        enteringGame = false;
        gameOver = false;
        timer = 0;
        remotePlayer = null;
        Array.Clear(localBullets, 0, localBullets.Length);
        Array.Clear(remoteBullets, 0, remoteBullets.Length);
        AIDictionary.Clear();
        pillarHealth = null;
        error = 0;
        roomID = -1;
        receivedLobbyInfo = false;
        lobbyStates = new bool[4];
        SceneManager.LoadScene("ClientMenu");
    }

    internal void SendRoomJoinRequest(int index)
    {
        if (lobbyStates[index])
        {
            RoomJoinRequestMessage mess = new RoomJoinRequestMessage();
            mess.roomID = index;
            MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
        }
    }

}

#pragma warning restore CS0618
