using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInput : MonoBehaviour
{
    public Rigidbody rb;
    float movementSpeed = 10.0f;
    GameObject bullet;
    [SerializeField]
    float fireDelay = 1.0f;
    [SerializeField]
    float bulletSpeed = 7.5f;
    bool canShoot;
    public ShieldClient client;

    public GameObject shieldHolder;
    public float targetRot = 0;
    public float rotSpeed = 180.0f;
    private float CloseEnough => 2 * rotSpeed / 60f;
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
        Vector3 m_Input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));
        rb.MovePosition(transform.position + m_Input * Time.deltaTime * movementSpeed);
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
        int left = Input.GetKey(KeyCode.J) ? 1 : 0; ;
        int down = Input.GetKey(KeyCode.K) ? 1 : 0; ;
        int right = Input.GetKey(KeyCode.L) ? 1 : 0; ;

        float y = shieldHolder.transform.eulerAngles.y;

        //checking for conflicting inputs
        int xSum = left + right;
        int ySum = up + down;
        //If ySum is 2, then both up and down are pressed. If xSum is 2, then both left and right are pressed. If xSum+ySum == 0, then no buttons are pressed
        if (ySum == 2 || xSum == 2 || xSum + ySum == 0)
        {
            return;
        }

        //create a vector pointing in the desired direction and convert to angle.
        Vector2 newAngleVec = new Vector2()
        {
            x = right - left,
            y = up - down
        };
        targetRot = (Vector2.SignedAngle(newAngleVec, Vector3.right) + 360) % 360;

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
        Vector3 spawnOffset = Vector3.zero;
        bool shouldSpawn = false;
        if (Input.GetKey(KeyCode.A) && !Input.GetKey(KeyCode.S) && !Input.GetKey(KeyCode.W))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.left;
        }
        else if (Input.GetKey(KeyCode.S) && !Input.GetKey(KeyCode.A) && !Input.GetKey(KeyCode.D))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.back;
        }
        else if (Input.GetKey(KeyCode.D) && !Input.GetKey(KeyCode.S) && !Input.GetKey(KeyCode.W))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.right;
        }
        else if (Input.GetKey(KeyCode.W) && !Input.GetKey(KeyCode.A) && !Input.GetKey(KeyCode.D))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.forward;
        }
        else if (Input.GetKey(KeyCode.A) && Input.GetKey(KeyCode.S))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.left + Vector3.back;
        }
        else if (Input.GetKey(KeyCode.A) && Input.GetKey(KeyCode.W))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.left + Vector3.forward;
        }
        else if (Input.GetKey(KeyCode.D) && Input.GetKey(KeyCode.S))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.right + Vector3.back;
        }
        else if (Input.GetKey(KeyCode.D) && Input.GetKey(KeyCode.W))
        {
            shouldSpawn = true;
            spawnOffset = Vector3.right + Vector3.forward;
        }
        spawnOffset = spawnOffset.normalized;

        int availableBulletIndex = FirstAvailableBullet(client.localBullets);
        if (shouldSpawn && availableBulletIndex < 5)
        {
            GameObject spawnedBullet = Instantiate(client.bullet, transform.position + spawnOffset * 3, Quaternion.identity);
            BulletScript bulletScript = spawnedBullet.GetComponent<BulletScript>();
            Vector3 vel = spawnOffset * bulletSpeed;
            spawnedBullet.GetComponent<Rigidbody>().velocity = vel;
            bulletScript.id = availableBulletIndex;
            bulletScript.owner = this;
            bulletScript.bulletPlayerIndex = client.PlayerIndex;
            client.localBullets[availableBulletIndex] = spawnedBullet;
            client.SendBulletCreate(bulletScript, spawnedBullet.GetComponent<Rigidbody>().velocity);
        }


    }

    private int FirstAvailableBullet(GameObject[] bullets)
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
        //here we send a message to the server to destroy the bullet
        client.localBullets[bulletID] = null;
        client.DestroyBulletEvent(bulletID);
    }

    public void DestroyRemoteBullet(int bulletIndex)
    {

        GameObject obj = client.remoteBullets[bulletIndex];
        Destroy(obj);
        client.remoteBullets[bulletIndex] = null;
    }
}
