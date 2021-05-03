using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;
using System.Linq;
using UnityEngine.SceneManagement;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Shield Client: handles message processing for clients, including sending data to and receiving data from the server
/// </summary>
#pragma warning disable CS0618
public class ShieldClient : MonoBehaviour
{
    //network connectivity settings
    private const int MAX_CONNECTION = 100;
    private readonly int port = 7777;
    private int hostID = -1;
    private int reliableChannel;
    private int unreliableChannel;
    private int connectionID;

    public bool isGameStarted = false;
    //cut off the update loop if not connected
    private bool isConnected = false;
    private byte error;

    //whether we're player 0 or 1
    private int _playerIndex = -1;
    public int PlayerIndex => _playerIndex;
    //triggers moving to the lobby menu
    public bool receivedLobbyInfo;
    //which lobbies are open
    public bool[] lobbyStates = new bool[4];

    //what lobby we're in
    public int roomID = -1;
    public bool OtherPlayerConnected { get; internal set; }

    //prefabs and player references
    public RemoteInput remotePlayer;
    public PlayerInput localPlayer;
    public GameObject bullet;
    public GameObject AI;

    //reference to the object that needs to be protected
    public PillarHealth pillarHealth;

    public bool enteringGame; //set to true by server, turned back off when waiting menu sees it

    //trackers for bullets (id is limited to [0,4]
    internal BulletScript[] remoteBullets = new BulletScript[5];
    internal BulletScript[] localBullets = new BulletScript[5];

    //trackers for AI (id isn't limited)
    public Dictionary<int, AIScript> AIDictionary = new Dictionary<int, AIScript>();

    //how many seconds the game has gone on for
    public int gameTimer;

    //whether the game has ended.
    public bool gameOver;

    //singleton instantiation
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

    //create connection data. Source: https://docs.unity3d.com/2017.4/Documentation/Manual/UNetUsingTransport.html
    public void Connect(string ip)
    {
        NetworkTransport.Init();
        ConnectionConfig cc = new ConnectionConfig();

        reliableChannel = cc.AddChannel(QosType.Reliable);
        unreliableChannel = cc.AddChannel(QosType.Unreliable);

        HostTopology topo = new HostTopology(cc, MAX_CONNECTION);

        hostID = NetworkTransport.AddHost(topo, 0);
        connectionID = NetworkTransport.Connect(hostID, ip, port, 0, out error);

        isConnected = true;
    }

    //derived from https://docs.unity3d.com/2017.4/Documentation/Manual/UNetUsingTransport.html
    private void Update()
    {
        if (!isConnected)
            return;

        byte[] recBuffer = new byte[1024];
        int bufferSize = recBuffer.Length;
        
        //we handle 20 packets a frame, we won't reasonably ever need to handle more.
        //1) Player State
        //2-6) Up to 5 bullet create/state/destroy
        //7) Pillar damage or game over
        //8) Game timer
        //9+) AI States (the odds of there being more than 11 AIs in existence at once for more than a second or two are pretty much 0 as the pillar would be long dead)
        for (int i = 0; i < 20; i++)
        {
            NetworkEventType recData = NetworkTransport.Receive(out int recHostID, out int connectionID, out int channelID, recBuffer, bufferSize, out int dataSize, out byte error);
            switch (recData)
            {
                case NetworkEventType.Nothing:
                    break;
                case NetworkEventType.ConnectEvent: //when we first connect, request lobby info
                    NetworkTransport.Send(hostID, connectionID, reliableChannel, new byte[] { (byte)MessageOps.MessageType.SERVER_CONNECT_REQUEST }, 1, out error);
                    break;
                case NetworkEventType.DataEvent:
                    MessageOps.MessageType type = MessageOps.ExtractMessageID(ref recBuffer, bufferSize, out byte[] subArr);
                    switch (type)
                    {
                        case MessageOps.MessageType.LOBBY_INFO: //upon receiving lobby info, tell the menu to show the lobby
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
                            if (isGameStarted && !connectResponse.self)
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
                        case MessageOps.MessageType.PLAYER_STATE: //receiving the other player's state
                            PlayerStateMessage playerState = MessageOps.FromBytes<PlayerStateMessage>(subArr);
                            if (remotePlayer != null && playerState.playerIndex != PlayerIndex)
                            {
                                //minor dead reckoning
                                remotePlayer.ProcessInput(playerState);
                            }
                            break;
                        case MessageOps.MessageType.BULLET_CREATE: //receiving a bullet create message from the other player
                            BulletCreateMessage bulletCreate = MessageOps.FromBytes<BulletCreateMessage>(subArr);
                            if (remotePlayer != null && bulletCreate.playerIndex != PlayerIndex && remoteBullets[bulletCreate.id] == null)
                            {
                                GameObject spawnedBullet = Instantiate(this.bullet, bulletCreate.position, Quaternion.identity);
                                spawnedBullet.GetComponent<Rigidbody>().velocity = bulletCreate.velocity;
                                BulletScript bullet = spawnedBullet.GetComponent<BulletScript>();
                                bullet.bulletPlayerIndex = bulletCreate.playerIndex;
                                bullet.id = bulletCreate.id;
                                remoteBullets[bullet.id] = bullet;
                            }
                            break;
                        case MessageOps.MessageType.BULLET_STATE: //receiving a bullet state message from the other player
                            BulletStateMessage bulletState = MessageOps.FromBytes<BulletStateMessage>(subArr);
                            BulletScript remoteBullet = remoteBullets[bulletState.bulletIndex];
                            if (remoteBullet != null)
                            {
                                //minor dead reckoning
                                remoteBullet.SetNewPositionAndVelocity(bulletState.position, bulletState.velocity);
                                remoteBullet.hasHitShield = bulletState.hasHitShield;
                            }
                            else
                            {
                                Debug.LogWarning($"Bullet with id {bulletState.bulletIndex} from the other player is null");
                            }

                            break;
                        case MessageOps.MessageType.BULLET_DESTROY: //receiving a bullet destroy message from the other player
                            BulletDestroyMessage bulletDestroyMessage = MessageOps.FromBytes<BulletDestroyMessage>(subArr);
                            BulletScript bulletToDestroy = remoteBullets[bulletDestroyMessage.bulletIndex];
                            if (bulletToDestroy != null)
                            {
                                Destroy(bulletToDestroy.gameObject);
                                remoteBullets[bulletDestroyMessage.bulletIndex] = null;
                            }
                            break;
                        case MessageOps.MessageType.GAME_START: //set the start game flag
                            enteringGame = true;
                            break;

                        case MessageOps.MessageType.AI_CREATE: //received AI create message from server
                            Debug.Log("Received AI Create Message");
                            AICreateMessage aiCreate = MessageOps.FromBytes<AICreateMessage>(subArr);
                            GameObject spawnedAI = Instantiate(AI, aiCreate.position, Quaternion.identity);
                            AIScript ai = spawnedAI.GetComponent<AIScript>();

                            //basic initialization
                            ai.id = aiCreate.id;
                            ai.pillar = pillarHealth;
                            ai.SetVelocity();
                            AIDictionary.Add(ai.id, ai);

                            //assign ownership
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
                        case MessageOps.MessageType.AI_STATE: //received AI state message from other player
                            AIStateMessage aiState = MessageOps.FromBytes<AIStateMessage>(subArr);
                            if (aiState.id % 2 != PlayerIndex % 2)
                            {
                                //if AI exists, set position/velocity
                                if (AIDictionary.TryGetValue(aiState.id, out AIScript AIToUpdate))
                                {
                                    if (AIToUpdate != null)
                                    {
                                        //sets variables for dead reckoning
                                        AIToUpdate.SetNewPositionAndVelocity(aiState.position, aiState.velocity);
                                    }
                                }
                            }
                            break;
                        case MessageOps.MessageType.AI_DESTROY: //received AI destroy message from other player
                            AIDestroyMessage aiDestroy = MessageOps.FromBytes<AIDestroyMessage>(subArr);
                            AIScript AIToDestroy = AIDictionary[aiDestroy.id];
                            if (AIToDestroy != null)
                            {
                                AIDictionary.Remove(aiDestroy.id);
                                Destroy(AIToDestroy.gameObject);
                            }
                            break;
                        case MessageOps.MessageType.PILLAR_DAMAGE: //damage pillar
                            PillarDamageMessage pillarDamage = MessageOps.FromBytes<PillarDamageMessage>(subArr);
                            pillarHealth.CurrentHealth = pillarDamage.newHealth;
                            break;
                        case MessageOps.MessageType.GAME_OVER: //end game
                            Destroy(pillarHealth.gameObject);
                            gameOver = true;
                            //set UI
                            FindObjectOfType<PlayerReference>().gameOver.text = $"Game Over! Time Survived: {gameTimer} Seconds";
                            //send back to main menu after 5 seconds
                            Invoke(nameof(ResetClient), 5.0f);
                            break;
                        case MessageOps.MessageType.GAME_TIME:
                            //set value for UI to pull from
                            GameTimeMessage time = MessageOps.FromBytes<GameTimeMessage>(subArr);
                            gameTimer = time.time;
                            break;
                    }
                    break;
                case NetworkEventType.DisconnectEvent:
                    break;
            }
        }

        //local player only exists if the level has been loaded
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
            //set basic state information and send to server
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
            localBullets[bulletIndex] = null;
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
            if (localBullets[i] != null) //if this index in the bullet array isn't null, package the data and send it
            {
                BulletStateMessage mess = new BulletStateMessage();
                mess.bulletIndex = localBullets[i].GetComponent<BulletScript>().id;
                mess.position = localBullets[i].transform.position;
                mess.velocity = localBullets[i].GetComponent<Rigidbody>().velocity;
                mess.roomID = roomID;
                mess.hasHitShield = localBullets[i].hasHitShield;

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
            if (ais[i] != null && ais[i].isControlledLocally) //loop through AIs. If locally controlled, send data
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

    public void DestroyLocalAI(int id) //kill AI by id and send the mssage to the server
    {
        if (AIDictionary.TryGetValue(id, out AIScript ai)) //key check
        {
            if (ai != null && ai.isControlledLocally) //null and local check
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

    public void SendStartRequest() //tell server to try to start the game
    {
        GameStartMessage mess = new GameStartMessage();
        mess.roomID = roomID;
        MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
    }

    public void SendPillarDamage() //tell server to decrement the pillar's health. If 0, game over.
    {
        PillarDamageMessage mess = new PillarDamageMessage();
        mess.newHealth = pillarHealth.CurrentHealth - 1;
        mess.roomID = roomID;
        MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
    }


    //try to join the given room. Server can refuse.
    internal void SendRoomJoinRequest(int index)
    {
        if (lobbyStates[index])
        {
            RoomJoinRequestMessage mess = new RoomJoinRequestMessage();
            mess.roomID = index;
            MessageOps.SendMessageToServer(mess, hostID, connectionID, reliableChannel, out error);
        }
    }

    //reset all variables
    public void ResetClient()
    {
        //disconnect before resetting
        NetworkTransport.Disconnect(hostID, connectionID, out error);

        //connectivity settings
        hostID = -1;
        reliableChannel = 0;
        unreliableChannel = 0;
        connectionID = -1;
        isConnected = false;

        error = 0;

        //lobby info
        receivedLobbyInfo = false;
        lobbyStates = new bool[4];

        //game loading states
        _playerIndex = -1;
        OtherPlayerConnected = false;
        roomID = -1;
        enteringGame = false;
        isGameStarted = false;

        //game data
        gameTimer = 0;
        remotePlayer = null;
        localPlayer = null;
        pillarHealth = null;
        Array.Clear(localBullets, 0, localBullets.Length);
        Array.Clear(remoteBullets, 0, remoteBullets.Length);
        AIDictionary.Clear();
        gameOver = false;

        SceneManager.LoadScene("ClientMenu");
    }

}

#pragma warning restore CS0618
