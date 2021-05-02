using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class LobbyMenu : MonoBehaviour
{
    public Button[] buttons;
    public WaitingMenu menu;
    // Start is called before the first frame update
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {
        for (int i = 0; i < buttons.Length; i++)
        {
            buttons[i].interactable = ShieldClient.Instance.lobbyStates[i];
        }
        if (ShieldClient.Instance.roomID >= 0 && ShieldClient.Instance.PlayerIndex >= 0)
        {
            if (!menu.gameObject.activeSelf)
            {
                menu.gameObject.SetActive(true);
                gameObject.SetActive(false);
            }
        }
    }

    public void JoinRoom(int index)
    {
        ShieldClient.Instance.SendRoomJoinRequest(index);
    }
}
