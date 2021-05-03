using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Waiting Menu: staging area for game to start
/// </summary>
public class WaitingMenu : MonoBehaviour
{
    public TextMeshProUGUI player1Conn, player2Conn; //text renderers for the two players
    public Button confirmButton; //reference needed for interactability

    // Update is called once per frame
    void Update()
    {
        //if our player index is set, we're in a room. This tells us which field to label as connected
        if (ShieldClient.Instance.PlayerIndex >= 0)
        {
            (ShieldClient.Instance.PlayerIndex == 1 ? player2Conn : player1Conn).text = "Connected";
        }
        else //this shouldn't ever actually run, but it's a good safeguard
        {
            player2Conn.text = player1Conn.text = "Not Connected";
        }
        //display whether the other player is connected
        (ShieldClient.Instance.PlayerIndex == 1 ? player1Conn : player2Conn).text = (ShieldClient.Instance.OtherPlayerConnected ? "" : "Not ") + "Connected";

        //don't let the room start unless the other player is connected
        confirmButton.interactable = ShieldClient.Instance.OtherPlayerConnected;

        //we've received the load game signal
        if (ShieldClient.Instance.enteringGame)
        {
            ShieldClient.Instance.enteringGame = false;
            LoadGame();
        }
    }

    //button function, tells the client to ask the server to start
    public void RequestLoadGame()
    {
        ShieldClient.Instance.SendStartRequest();
    }

    //tell the ShieldClient that the game has started and load the scene.
    public void LoadGame()
    {
        ShieldClient.Instance.isGameStarted = true;
        SceneManager.LoadScene("Client");
    }

    //Exit the waiting room. Sends player back to main menu.
    public void Back()
    {
        ShieldClient.Instance.ResetClient();
    }
}
