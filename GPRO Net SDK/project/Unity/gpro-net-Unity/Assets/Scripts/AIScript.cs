using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AIScript : MonoBehaviour
{
    public float speed;
    public ShieldClient client;
    public GameObject pillar;
    public bool isControlledLocally;
    public Rigidbody rb;
    public int id;

    private float timeBetweenHits = 1.5f;
    private float _timer = 0;
    // Start is called before the first frame update
    void Start()
    {
        rb = GetComponent<Rigidbody>();
        pillar = GameObject.FindGameObjectWithTag("Pillar");
        Vector3 direction = (pillar.transform.position - transform.position).normalized * speed;
        rb.velocity = direction;
    }

    // Update is called once per frame
    void Update()
    {
        if (isControlledLocally)
        {
            _timer -= Time.deltaTime;
        }
    }

    private void OnCollisionEnter(Collision collision)
    {
        if (collision.gameObject.CompareTag("Bullet") && isControlledLocally)
        {
            client.DestroyLocalAI(this);
            Destroy(gameObject);
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
}
