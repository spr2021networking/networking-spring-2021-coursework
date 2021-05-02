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
    [SerializeField]
    float maxOffset = 2.0f;
    Vector3 tmpPos;
    Vector3 tmpVel;

    static float COSTHIRTY = Mathf.Cos(30 * Mathf.Deg2Rad);
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
    private void FixedUpdate()
    {
        if (!isControlledLocally)
        {
            Vector3 intendedPos = tmpPos;
            Vector3 intendedVel = tmpVel;
            float dotProd = Vector3.Dot(rb.velocity, tmpVel);

            //dot prod of normalized direction, if less than 30 degrees
            if (dotProd < COSTHIRTY || (transform.position - tmpPos).magnitude > maxOffset)
            {
                rb.velocity = tmpVel;
                transform.position = tmpPos;
            }
            else
            {

                intendedVel = Vector3.Slerp(rb.velocity, tmpVel, 0.5f);
                intendedVel.y = 0;
                rb.velocity = intendedVel;

                intendedPos += tmpVel * Time.fixedDeltaTime;
                transform.position = Vector3.Lerp(transform.position, intendedPos, 0.5f);
                tmpPos = intendedPos;
            }
        }
        
    }
    private void OnTriggerEnter(Collider collision)
    {
        if (collision.gameObject.CompareTag("Bullet") && isControlledLocally)
        {
            if (collision.GetComponent<BulletScript>().hasHitShield)
            {
                client.DestroyLocalAI(id);
            }
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

    public void SetNewPositionAndVelocity(Vector3 pos, Vector3 vel)
    {
        tmpPos = pos;
        tmpVel = vel;
    }
}
