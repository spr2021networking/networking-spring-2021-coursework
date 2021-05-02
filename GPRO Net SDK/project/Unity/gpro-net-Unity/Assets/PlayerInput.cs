using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInput : MonoBehaviour
{
    public Rigidbody rb;
    float movementSpeed = 10.0f;
    GameObject bullet;
    float fireDelay = 1.0f;
    float bulletSpeed = 20.0f;
    bool canShoot;
    public ShieldClient client;

    public GameObject shieldHolder;
    public float targetRot = 0;
    public float rotSpeed = 180.0f;
    private float CloseEnough => 2 * rotSpeed / 60f;

    private float bulletSpawnOffset = 7;

    private Vector3 fireDirection = Vector3.forward;

    
    // Start is called before the first frame update
    void Start()
    {
        rb = GetComponent<Rigidbody>();
        canShoot = true;
        shieldHolder = transform.GetChild(0).gameObject;
    }

    // Update is called once per frame
    void FixedUpdate()
    {

        float horizontalInput = Input.GetAxis("Horizontal");
        float verticalInput = Input.GetAxis("Vertical");
        Vector3 input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));

        bool hasXInput = Mathf.Abs(input.x) > 0.1;
        bool hasZInput = Mathf.Abs(input.z) > 0.1;
        int dirX = hasXInput ? Math.Sign(input.x) : 0;
        int dirZ = hasZInput ? Math.Sign(input.z) : 0;
        if (hasXInput || hasZInput)
        {
            fireDirection = new Vector3(dirX, 0, dirZ).normalized; //update our fire position if we have nonzero input
        }

        rb.velocity = input * movementSpeed;
        if (Input.GetKey(KeyCode.Space) && canShoot)
        {
            FireBullet();
            canShoot = false;
            StartCoroutine("FireLimit");
        }

        RotateShield();
    }

    /// <summary>
    /// Translate user input into shield rotation
    /// </summary>
    private void RotateShield()
    {
        int up = Input.GetKey(KeyCode.I) ? 1 : 0;
        int left = Input.GetKey(KeyCode.J) ? 1 : 0;
        int down = Input.GetKey(KeyCode.K) ? 1 : 0;
        int right = Input.GetKey(KeyCode.L) ? 1 : 0;

        float y = shieldHolder.transform.eulerAngles.y;

        //checking for conflicting inputs
        int xSum = left + right;
        int ySum = up + down;

        //if we have input, bind the new position
        if (ySum < 2 && xSum < 2 && xSum + ySum > 0)
        {
            Vector2 newAngleVec = new Vector2()
            {
                x = right - left,
                y = up - down
            };
            targetRot = (Vector2.SignedAngle(newAngleVec, Vector3.right) + 360) % 360;
        }

        //create a vector pointing in the desired direction and convert to angle.


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

    void FireBullet()
    {
        int availableBulletIndex = FirstAvailableBullet(client.localBullets);
        if (availableBulletIndex < 5)
        {
            GameObject spawnedBullet = Instantiate(client.bullet, transform.position + fireDirection * bulletSpawnOffset, Quaternion.identity);
            BulletScript bulletScript = spawnedBullet.GetComponent<BulletScript>();
            Vector3 vel = fireDirection * bulletSpeed;
            spawnedBullet.GetComponent<Rigidbody>().velocity = vel;
            bulletScript.id = availableBulletIndex;
            bulletScript.owner = this;
            bulletScript.bulletPlayerIndex = client.PlayerIndex;
            client.SendBulletCreate(bulletScript, spawnedBullet.GetComponent<Rigidbody>().velocity);
            client.localBullets[availableBulletIndex] = bulletScript;
        }


    }

    private int FirstAvailableBullet(BulletScript[] bullets)
    {
        for (int i = 0; i < bullets.Length; i++)
        {
            if (bullets[i] == null)
            {
                return i;
            }
        }
        return bullets.Length;
    }

    private IEnumerator FireLimit()
    {
        yield return new WaitForSeconds(fireDelay);
        canShoot = true;
    }

    public void DestroyBullet(int bulletID)
    {
        BulletScript bullet = client.localBullets[bulletID];
        if (bullet != null)
        {
            client.SendBulletDestroy(bulletID);
            Destroy(bullet.gameObject);
        }

    }

    public void DestroyRemoteBullet(int bulletIndex)
    {
        BulletScript bullet = client.remoteBullets[bulletIndex];
        if (bullet != null)
        {
            Destroy(bullet.gameObject);
            client.remoteBullets[bulletIndex] = null;
        }
    }
}
