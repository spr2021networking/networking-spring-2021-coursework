using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletScript : MonoBehaviour
{
    public PlayerInput owner; //only set in playerInput, if null then remote bullet
    public Rigidbody rb;
    [SerializeField]
    float timeUntilDeath = 5.0f;
    public int id;
    public int bulletPlayerIndex;
    public bool hasHitShield;
    // Start is called before the first frame update
    void Start()
    {
        rb = GetComponent<Rigidbody>();
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        timeUntilDeath -= Time.deltaTime;
        if (timeUntilDeath <= 0.0f && owner != null)
        {
            owner.DestroyBullet(id);
            Destroy(gameObject);
        }
    }
}
