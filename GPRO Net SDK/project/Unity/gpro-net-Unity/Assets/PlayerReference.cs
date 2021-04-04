using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerReference : MonoBehaviour
{
    public GameObject cube;
    public GameObject sphere;
    // Start is called before the first frame update
    void Start()
    {
        ShieldClient s = FindObjectOfType<ShieldClient>();
        bool isPlayerZero = s.PlayerIndex == 0;

        s.localPlayer = (isPlayerZero ? cube : sphere).AddComponent<PlayerInput>();
        s.remotePlayer = (!isPlayerZero ? cube : sphere).AddComponent<RemoteInput>();
    }

    // Update is called once per frame
    void Update()
    {

    }
}
