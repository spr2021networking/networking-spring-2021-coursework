using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class MainMenu : MonoBehaviour
{
    public TMP_InputField ip;
    public Button button;

    public ShieldClient client;
    public WaitingMenu waiting;

    private bool _lobbyShown;
    // Use this for initialization
    void Start()
    {
        ip.text = "172.16.4.196";
    }

    // Update is called once per frame
    void Update()
    {
        if (client.PlayerIndex >= 0 && !_lobbyShown)
        {
            _lobbyShown = true;
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
        client.Connect(ip.text);
    }

}