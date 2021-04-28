using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class WaitingMenu : MonoBehaviour
{
    public MainMenu main;
    public ShieldClient client;

    public TextMeshProUGUI player1Conn, player2Conn;
    public Button confirmButton;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (client.PlayerIndex >= 0)
        {
            (client.PlayerIndex == 1 ? player2Conn : player1Conn).text = "Connected";
        }
        (client.PlayerIndex == 1 ? player1Conn : player2Conn).text = (client.OtherPlayerConnected ? "" : "Not ") + "Connected";

        confirmButton.interactable = client.OtherPlayerConnected;

        if (client.enteringGame)
        {
            client.enteringGame = false;
            LoadGame();
        }

        //need a DC check for us
    }

    public void RequestLoadGame()
    {
        client.SendStartRequest();
    }

    public void LoadGame()
    {
        SceneManager.LoadScene("Client");
    }
}
