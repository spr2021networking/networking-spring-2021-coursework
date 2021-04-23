using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AIScript : MonoBehaviour
{
    public float speed;
    public ShieldClient client;
    public GameObject pillar;
    public bool isControlledLocally;
    // Start is called before the first frame update
    void Start()
    {
        pillar = GameObject.FindGameObjectWithTag("Pillar");
    }

    // Update is called once per frame
    void Update()
    {
        float step = speed * Time.deltaTime;
        transform.position = Vector3.MoveTowards(transform.position, pillar.transform.position, step);
    }

   
}
