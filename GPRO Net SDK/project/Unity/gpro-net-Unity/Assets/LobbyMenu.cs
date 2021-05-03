using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Waiting Menu: lets player select a room to join
/// </summary>
public class LobbyMenu : MonoBehaviour
{
    public Button[] buttons; //buttons corresponding to the lobbies
    public WaitingMenu menu; //the next menu to load

    // Update is called once per frame
    void Update()
    {
        //set each bullet's interactability based on the last time we got lobby info (which is only sent when a lobby's state may have changed)
        for (int i = 0; i < buttons.Length; i++)
        {
            buttons[i].interactable = ShieldClient.Instance.lobbyStates[i];
        }
        //room has been selected, so we switch to the new room.
        if (ShieldClient.Instance.roomID >= 0 && ShieldClient.Instance.PlayerIndex >= 0)
        {
            if (!menu.gameObject.activeSelf)
            {
                menu.gameObject.SetActive(true);
                gameObject.SetActive(false);
            }
        }
    }

    //button function, sends the server a request to join a room. May be rejected with no unexpected behavior if room fills at the same frame 
    public void JoinRoom(int index)
    {
        ShieldClient.Instance.SendRoomJoinRequest(index);
    }
}
