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
        
    }

   
}
