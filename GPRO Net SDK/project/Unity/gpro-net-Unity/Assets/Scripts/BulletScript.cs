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

    private void OnTriggerEnter(Collider other)
    {
        if (owner != null)
        {
            if (other.CompareTag("Shield") && !hasHitShield)
            {
                if (other.transform.parent.gameObject == owner.gameObject) //don't collide with our own shield
                {
                    return;
                }
                Debug.Log("Other: " + other.transform.eulerAngles.y);
                Vector2 angleVec = new Vector2()
                {
                    x = rb.velocity.x,
                    y = rb.velocity.z
                };
                float angle = (Vector2.SignedAngle(angleVec, Vector3.right) + 360) % 360;
                Debug.Log("Self: " + angle);

                float angleDiff = Mathf.Abs(angle - other.transform.eulerAngles.y); //looking for 180 +- 67.5

                float diffFrom180 = Mathf.Abs(angleDiff - 180);
                if (diffFrom180 > 67.5f)
                {
                    owner.DestroyBullet(id);
                    Destroy(gameObject);
                }
                else
                {
                    hasHitShield = true;
                    if (diffFrom180 < 22.5) //it's 'opposite'
                    {
                        rb.velocity = -rb.velocity;
                    }
                    else
                    {
                        bool diffGreaterThan180 = angleDiff > 180;
                        bool selfGreaterThanShield = angle > other.transform.eulerAngles.y;

                        float rot = (diffGreaterThan180 == selfGreaterThanShield ? -90 : 90) * Mathf.Deg2Rad;

                        float cosRot = Mathf.Cos(rot), sinRot = Mathf.Sin(rot);

                        //slightly faster than building a matrix out
                        Vector2 newDir = new Vector2(
                            angleVec.x * cosRot - angleVec.y * sinRot,
                            angleVec.x * sinRot + angleVec.y * cosRot);
                        rb.velocity = new Vector3(newDir.x, 0, newDir.y);
                    }
                }
            }
            else
            {
                owner.DestroyBullet(id);
                Destroy(gameObject);
            }
        }
    }

    private void OnCollisionEnter(Collision collision)
    {
        if (owner != null)
        {
            owner.DestroyBullet(id);
            Destroy(gameObject);
        }
    }
}
