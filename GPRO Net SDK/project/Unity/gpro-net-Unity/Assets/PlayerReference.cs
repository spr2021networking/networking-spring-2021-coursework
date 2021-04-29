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

    ShieldClient s;
    // Start is called before the first frame update
    void Start()
    {
        s = FindObjectOfType<ShieldClient>();
        bool isPlayerZero = s.PlayerIndex == 0;

        s.localPlayer = (isPlayerZero ? cube : sphere).AddComponent<PlayerInput>();
        s.remotePlayer = (!isPlayerZero ? cube : sphere).AddComponent<RemoteInput>();
        s.remotePlayer.client = s;
        s.localPlayer.client = s;
        s.pillarHealth = pillarHealth;
        viewCamera.transform.parent = s.localPlayer.transform;
        viewCamera.transform.localPosition = new Vector3(0, viewCamera.transform.localPosition.y, viewCamera.transform.localPosition.z);
    }

    // Update is called once per frame
    void Update()
    {
        timer.text = "Game Timer: " + s.timer;
        health.text = "" + pillarHealth.CurrentHealth;
        gameOver.enabled = s.gameOver;
    }
}
