using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class WaitingMenu : MonoBehaviour
{
    public MainMenu main;

    public TextMeshProUGUI player1Conn, player2Conn;
    public Button confirmButton;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {

        if (ShieldClient.Instance.PlayerIndex >= 0)
        {
            (ShieldClient.Instance.PlayerIndex == 1 ? player2Conn : player1Conn).text = "Connected";
        }
        else
        {
            player2Conn.text = player1Conn.text = "Not Connected";
        }
        (ShieldClient.Instance.PlayerIndex == 1 ? player1Conn : player2Conn).text = (ShieldClient.Instance.OtherPlayerConnected ? "" : "Not ") + "Connected";

        confirmButton.interactable = ShieldClient.Instance.OtherPlayerConnected;

        if (ShieldClient.Instance.enteringGame)
        {
            ShieldClient.Instance.enteringGame = false;
            LoadGame();
        }

        //need a DC check for us
    }

    public void RequestLoadGame()
    {
        ShieldClient.Instance.SendStartRequest();
    }

    public void LoadGame()
    {
        ShieldClient.Instance.isStarted = true;
        SceneManager.LoadScene("Client");
    }
}
