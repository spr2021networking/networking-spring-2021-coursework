using TMPro;
using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Main Menu: handles initial connection to server
/// </summary>
public class MainMenu : MonoBehaviour
{
    public TMP_InputField ip; //set IP
    public Button button; //connect button

    public LobbyMenu lobby; //menu to go to

    // Use this for initialization
    void Start()
    {
        ip.text = "172.16.4.196"; //default IP (Scott's VM address)
    }

    // Update is called once per frame
    void Update()
    {
        //if the lobby exists, switch to it
        if (ShieldClient.Instance.receivedLobbyInfo)
        {
            ShowLobby();
        }
    }

    //hides the main menu and shows the lobby selection screen
    private void ShowLobby()
    {
        lobby.gameObject.SetActive(true);
        gameObject.SetActive(false);
    }

    //Tells the shield client to connect to the server
    public void Connect()
    {
        ShieldClient.Instance.Connect(ip.text);
    }

}