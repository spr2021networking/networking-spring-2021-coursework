using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class MainMenu : MonoBehaviour
{
    public TMP_InputField ip;
    public Button button;

    public WaitingMenu waiting;
    public LobbyMenu lobby;

    public bool lobbyShown;
    // Use this for initialization
    void Start()
    {
        ip.text = "172.16.4.196";
    }

    // Update is called once per frame
    void Update()
    {
        if (ShieldClient.Instance.PlayerIndex >= 0 && !lobbyShown)
        {
            lobbyShown = true;
            ShowLobby();
        }
    }

    private void ShowLobby()
    {
        waiting.gameObject.SetActive(true);
        gameObject.SetActive(false);
    }

    public void Connect()
    {
        ShieldClient.Instance.Connect(ip.text);
    }

}