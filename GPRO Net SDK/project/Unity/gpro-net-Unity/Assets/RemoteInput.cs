using UnityEngine;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Remote Input: Handles all player behavior for the the remote player
/// </summary>
public class RemoteInput : MonoBehaviour
{
    Rigidbody _rb;

    //shield rotation data
    public GameObject shieldHolder;
    public float targetRot = 0;
    public float rotSpeed = 180.0f;
    public float movementSpeed = 10.0f;
    //2 frame buffer for rotation checking
    private float CloseEnough => 2 * rotSpeed / 60f;

    readonly float maxOffset = 2.0f;
    Vector3 tmpPos;
    Vector3 tmpVel;

    static readonly float COSTHIRTY = Mathf.Cos(30 * Mathf.Deg2Rad);
    // Start is called before the first frame update
    void Start()
    {
        _rb = GetComponent<Rigidbody>();
        shieldHolder = transform.GetChild(0).gameObject;
    }

    private void FixedUpdate()
    {
        //dead reckoning
        Vector3 intendedPos = tmpPos;
        float dotProd = Vector3.Dot(_rb.velocity, tmpVel);

        //dot prod of direction, if less than 30 degrees, snap position and assign velocity
        if (dotProd < COSTHIRTY * movementSpeed || (transform.position - tmpPos).magnitude > maxOffset)
        {
            _rb.velocity = tmpVel;
            transform.position = tmpPos;
        }
        else //we slerp and lerp to the new values
        {
            Vector3 intendedVel = Vector3.Slerp(_rb.velocity, tmpVel, 0.5f);
            intendedVel.y = 0;
            _rb.velocity = intendedVel;

            intendedPos += tmpVel * Time.fixedDeltaTime;
            transform.position = Vector3.Lerp(transform.position, intendedPos, 0.5f);
            tmpPos = intendedPos;
        }

        RemoteRotateShield(); //used between packets
    }
    private void RemoteRotateShield()
    {
        float y = shieldHolder.transform.eulerAngles.y;

        if (Mathf.Abs(y - targetRot) <= CloseEnough) //if we are within 2 frames of facing the correct direction (near zero check)
        {
            shieldHolder.transform.rotation = Quaternion.Euler(0, targetRot, 0);
            return;
        }

        if (y == (targetRot + 180 % 360)) //floats almost never equal exact integers, so y should NEVER hit this. It's a good safeguard though
        {
            y += 1;
        }
        float diff = Mathf.Abs(y - targetRot);
        int sign = y > targetRot == diff > 180.0f ? 1 : -1; //determine if it is negative or positive rotation
        y += Time.fixedDeltaTime * sign * rotSpeed;
        shieldHolder.transform.rotation = Quaternion.Euler(0, y, 0);
    }


    internal void ProcessInput(PlayerStateMessage playerState)
    {
        //dead reckoning prep
        tmpPos = playerState.position;
        tmpVel = playerState.velocity;

        //assign shield position and target rotation, it'll rotate on its own.
        shieldHolder.transform.rotation = Quaternion.Euler(0, playerState.currentShieldRot, 0);
        targetRot = playerState.targetShieldRot;
    }
}
