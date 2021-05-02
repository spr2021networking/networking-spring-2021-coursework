﻿using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class MainMenu : MonoBehaviour
{
    public TMP_InputField ip;
    public Button button;

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
        if (ShieldClient.Instance.receivedLobbyInfo)
        {
            lobbyShown = true;
            ShowLobby();
        }
    }

    private void ShowLobby()
    {
        lobby.gameObject.SetActive(true);
        gameObject.SetActive(false);
    }

    public void Connect()
    {
        ShieldClient.Instance.Connect(ip.text);
    }

}