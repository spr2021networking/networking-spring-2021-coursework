using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RemoteInput : MonoBehaviour
{
    Rigidbody _rb;
    public ShieldClient client;

    public GameObject shieldHolder;
    public float targetRot = 0;
    public float rotSpeed = 180.0f;

    private float CloseEnough => 2 * rotSpeed / 60f;
    // Start is called before the first frame update
    void Start()
    {
        _rb = GetComponent<Rigidbody>();
        shieldHolder = transform.GetChild(0).gameObject;
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    private void FixedUpdate()
    {
        RemoteRotateShield(); //used between packets
    }
    private void RemoteRotateShield()
    {
        float y = shieldHolder.transform.eulerAngles.y;

        if (Mathf.Abs(y - targetRot) <= CloseEnough)
        {
            shieldHolder.transform.rotation = Quaternion.Euler(0, targetRot, 0);
            return;
        }

        if (y == (targetRot + 180 % 360)) //floats almost never equal exact integers, so y should NEVER hit this. It's a good safeguard though
        {
            y += 1;
        }
        float diff = Mathf.Abs(y - targetRot);
        int sign = y > targetRot == diff > 180.0f ? 1 : -1;
        y += Time.fixedDeltaTime * sign * rotSpeed;
        shieldHolder.transform.rotation = Quaternion.Euler(0, y, 0);
    }


    internal void ProcessInput(PlayerStateMessage playerState)
    {
        transform.position = playerState.position;
        _rb.velocity = playerState.velocity;
        transform.rotation = Quaternion.Euler(transform.rotation.eulerAngles.x, playerState.rotation, transform.rotation.eulerAngles.z);
        _rb.angularVelocity = new Vector3(_rb.angularVelocity.x, playerState.angVel, _rb.angularVelocity.z);

        shieldHolder.transform.rotation = Quaternion.Euler(0, playerState.currentShieldRot, 0);
        targetRot = playerState.targetShieldRot;
        //need shield rotation
        //time shenanigans?
    }

    internal void ProccessBullet(BulletStateMessage bulletState)
    {
        GameObject bulletToSpawn = Instantiate(client.bullet, bulletState.position, Quaternion.identity);
        bulletToSpawn.GetComponent<Rigidbody>().velocity = bulletState.velocity;
    }
}
