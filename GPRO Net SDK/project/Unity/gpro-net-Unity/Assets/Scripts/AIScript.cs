using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AIScript : MonoBehaviour
{
    public float speed;
    public ShieldClient client;
    public PillarHealth pillar;
    public bool isControlledLocally;
    public Rigidbody rb;
    public int id;

    private float timeBetweenHits = 1.5f;
    private float _timer = 0;

    public bool firstFrame = true;
    // Start is called before the first frame update
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {
        if (isControlledLocally)
        {
            _timer -= Time.deltaTime;
            InitVelocity();
        }
    }

    private void OnTriggerEnter(Collider collision)
    {
        if (collision.gameObject.CompareTag("Bullet") && isControlledLocally)
        {
            client.DestroyLocalAI(id);
        }
    }

    private void OnCollisionStay(Collision collision)
    {
        if (collision.gameObject.CompareTag("Pillar") && isControlledLocally && _timer <= 0)
        {
            client.SendPillarDamage();
            _timer = timeBetweenHits;
        }
    }

    internal void InitVelocity()
    {
        if (!rb)
        {
            rb = GetComponent<Rigidbody>();
        }
        if (pillar)
        {
            Vector3 direction = pillar.transform.position - transform.position;
            direction.y = 0;
            direction.Normalize();
            direction *= speed;
            rb.velocity = direction;
        }
    }
}
