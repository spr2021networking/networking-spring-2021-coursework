using TMPro;
using UnityEngine;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Player Reference: Contains references to the players, the pillar, and some UI elements
/// </summary>
public class PlayerReference : MonoBehaviour
{
    public GameObject cube;
    public GameObject sphere;
    public PillarHealth pillarHealth;
    public Camera viewCamera;

    public TextMeshProUGUI timer, health, gameOver;

    // Start is called before the first frame update
    void Start()
    {
        bool isPlayerZero = ShieldClient.Instance.PlayerIndex == 0;

        //initialization calls upon game start. Binds the local and remote player, as well as the pillar
        ShieldClient.Instance.localPlayer = (isPlayerZero ? cube : sphere).AddComponent<PlayerInput>();
        ShieldClient.Instance.remotePlayer = (!isPlayerZero ? cube : sphere).AddComponent<RemoteInput>();
        ShieldClient.Instance.localPlayer.client = ShieldClient.Instance;
        ShieldClient.Instance.pillarHealth = pillarHealth;
        viewCamera.transform.parent = ShieldClient.Instance.localPlayer.transform;
        viewCamera.transform.localPosition = new Vector3(0, viewCamera.transform.localPosition.y, viewCamera.transform.localPosition.z);
    }

    // Update is called once per frame
    void Update()
    {
        //updates the UI to show the timer and health, and toggles the game over screen.
        timer.text = "Game Timer: " + ShieldClient.Instance.gameTimer;
        health.text = "" + pillarHealth.CurrentHealth;
        gameOver.enabled = ShieldClient.Instance.gameOver;
    }
}
