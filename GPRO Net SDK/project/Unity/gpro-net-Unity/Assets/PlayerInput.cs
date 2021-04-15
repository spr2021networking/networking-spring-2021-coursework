using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInput : MonoBehaviour
{
    public Rigidbody rb;
    float movementSpeed = 5.0f;
    GameObject bullet;
    [SerializeField]
    float fireDelay = 1.0f;
    [SerializeField]
    float bulletSpeed = 7.5f;
    float newXPos;
    bool canShoot;
    public ShieldClient client;
    public int bulletsActive = 0;

    // Start is called before the first frame update
    void Start()
    {
        rb = GetComponent<Rigidbody>();
        canShoot = true;
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

        if (shouldSpawn && bulletsActive < 5)
        {
            bulletsActive++;
            
            GameObject spawnedBullet = Instantiate(client.bullet, transform.position + spawnOffset * 3, Quaternion.identity);
            BulletScript bulletScript = spawnedBullet.GetComponent<BulletScript>();
            Vector3 vel = spawnOffset * bulletSpeed;
            spawnedBullet.GetComponent<Rigidbody>().velocity = vel;
            client.CreateBullet(spawnedBullet.transform.position, spawnedBullet.GetComponent<Rigidbody>().velocity);
            bulletScript.id = bulletsActive;
            bulletScript.owner = this;
            bulletScript.bulletPlayerIndex = client.PlayerIndex;
            client.localBullets.Add(spawnedBullet);
        }


    }

    private IEnumerator FireLimit()
    {
        yield return new WaitForSeconds(fireDelay);
        canShoot = true;
    }

    public void DestroyBullet()
    {
        //here we send a message to the server to destroy the bullet
        client.DestroyBulletEvent(bulletsActive -1);
        bulletsActive--;
    }

    public void DestroyRemoteBullet(GameObject bulletToDestroy)
    {
        if (bulletToDestroy != null)
        {
            client.remoteBullets.Remove(bulletToDestroy);
            Destroy(bulletToDestroy);
        }
    }
}
