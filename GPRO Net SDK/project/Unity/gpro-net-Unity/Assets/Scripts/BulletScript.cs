using UnityEngine;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Handles the bullet fired by players
/// </summary>
public class BulletScript : MonoBehaviour
{
    public PlayerInput owner; //only set in playerInput, if null then remote bullet
    public Rigidbody rb;
    [SerializeField]
    float timeUntilDeath = 5.0f;
    //bullet id, used to make sure there aren't too many bullets
    public int id;
    //player index
    public int bulletPlayerIndex;
    public bool hasHitShield;

    [SerializeField]
    float maxOffset = 2.0f;//how far the Bullet can be before we snap
    Vector3 tmpPos;

    // Update is called once per frame
    void FixedUpdate()
    {
        if (owner == null)//only run on remote bullets
        {
            Vector3 intendedPos = tmpPos;

            //dot prod of normalized direction, if less than 30 degrees
            if ((transform.position - tmpPos).magnitude > maxOffset) //snap position
            {
                transform.position = tmpPos;
            }
            else //we slerp and lerp to the new values
            {
                intendedPos += rb.velocity * Time.fixedDeltaTime;
                transform.position = Vector3.Lerp(transform.position, intendedPos, 0.5f);
                tmpPos = intendedPos;
            }

        }
        timeUntilDeath -= Time.fixedDeltaTime;
        if (timeUntilDeath <= 0.0f && owner != null)
        {
            owner.DestroyBullet(id);
        }

    }

    private void OnTriggerEnter(Collider other)
    {
        if (owner != null) //if it has an owner, it's a local bullet
        {
            if (other.CompareTag("Shield") && !hasHitShield) //check if we're hitting a shield
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
                float angle = (Vector2.SignedAngle(angleVec, Vector3.right) + 360) % 360; //find our angle
                Debug.Log("Self: " + angle);

                float angleDiff = Mathf.Abs(angle - other.transform.eulerAngles.y); //looking for 180 +- 67.5. Technically +- 45, with a 22.5 degree window of error

                float diffFrom180 = Mathf.Abs(angleDiff - 180);
                //if the difference is greater than 67.5, then the shield is closer to facing perpendicular to the bullet or away from it
                if (diffFrom180 > 67.5f)
                {
                    owner.DestroyBullet(id);
                }
                else
                {
                    hasHitShield = true;
                    if (diffFrom180 < 22.5) //the shield is close to facing the bullet
                    {
                        rb.velocity = -rb.velocity;
                    }
                    else
                    {
                        bool diffGreaterThan180 = angleDiff > 180;
                        bool selfGreaterThanShield = angle > other.transform.eulerAngles.y;

                        //this has been tested, it's a bit esoteric, but if both of these are true OR both of these are false,
                        //then our velocity should rotate by -90 degrees, otherwise it's 90 degrees
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
                owner.DestroyBullet(id); //if we hit something other than our own shield (even if that's ourselves), destroy the bullet
            }
        }
    }

    private void OnCollisionEnter(Collision collision)
    {
        if (owner != null)
        {
            owner.DestroyBullet(id); //destroy the bullet if this gets called. I'm not sure if this runs
        }
    }
    public void SetNewPositionAndVelocity(Vector3 pos, Vector3 vel)
    {
        //dead reckoning prep
        tmpPos = pos;
        rb.velocity = vel;
    }
}
