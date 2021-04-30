using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

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

        ShieldClient.Instance.localPlayer = (isPlayerZero ? cube : sphere).AddComponent<PlayerInput>();
        ShieldClient.Instance.remotePlayer = (!isPlayerZero ? cube : sphere).AddComponent<RemoteInput>();
        ShieldClient.Instance.remotePlayer.client = ShieldClient.Instance;
        ShieldClient.Instance.localPlayer.client = ShieldClient.Instance;
        ShieldClient.Instance.pillarHealth = pillarHealth;
        viewCamera.transform.parent = ShieldClient.Instance.localPlayer.transform;
        viewCamera.transform.localPosition = new Vector3(0, viewCamera.transform.localPosition.y, viewCamera.transform.localPosition.z);
    }

    // Update is called once per frame
    void Update()
    {
        timer.text = "Game Timer: " + ShieldClient.Instance.timer;
        health.text = "" + pillarHealth.CurrentHealth;
        gameOver.enabled = ShieldClient.Instance.gameOver;
    }
}
