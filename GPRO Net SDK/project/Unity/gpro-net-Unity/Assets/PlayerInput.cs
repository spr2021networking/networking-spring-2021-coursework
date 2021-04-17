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
    private float _targetRot = 0;
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

    private void RotateShield()
    {
        bool right = Input.GetKey(KeyCode.L); //works
        bool left = Input.GetKey(KeyCode.J); //works
        bool up = Input.GetKey(KeyCode.I); //broken
        bool down = Input.GetKey(KeyCode.K); //points up

        float y = shieldHolder.transform.eulerAngles.y;

        if (right && !left && !up && !down)
        {
            _targetRot = 0;
        }
        else if (right && !left && !up && down)
        {
            _targetRot = 45;
        }
        else if (!right && !left && !up && down)
        {
            _targetRot = 90;
        }
        else if (!right && left && !up && down)
        {
            _targetRot = 135;
        }
        else if (!right && left && !up && !down)
        {
            _targetRot = 180;
        }
        else if (!right && left && up && !down)
        {
            _targetRot = 225;
        }
        else if (!right && !left && up && !down)
        {
            _targetRot = 270;
        }
        else if (right && !left && up && !down)
        {
            _targetRot = 315;
        }

        if (Mathf.Abs(y - _targetRot) <= CloseEnough)
        {
            shieldHolder.transform.rotation = Quaternion.Euler(0, _targetRot, 0);
            return;
        }

        if (y == (_targetRot + 180 % 360)) //floats almost never equal exact integers, so y should NEVER hit this. It's a good safeguard though
        {
            y += 1;
        }
        float diff = Mathf.Abs(y - _targetRot);
        int sign = y > _targetRot == diff > 180.0f ? 1 : -1;
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

        if (shouldSpawn)
        {
            GameObject spawnedBullet = Instantiate(client.bullet, transform.position + spawnOffset * 3, Quaternion.identity);
            Vector3 vel = spawnOffset * bulletSpeed;
            spawnedBullet.GetComponent<Rigidbody>().velocity = vel;
            client.CreateBullet(spawnedBullet.transform.position, spawnedBullet.GetComponent<Rigidbody>().velocity);
            client.bulletTracker.Add(ShieldClient.bulletIDTracker, spawnedBullet.GetComponent<BulletScript>());
            spawnedBullet.GetComponent<BulletScript>().id = ShieldClient.bulletIDTracker;
            spawnedBullet.GetComponent<BulletScript>().bulletPlayerIndex = client.PlayerIndex;
            ShieldClient.bulletIDTracker++;
        }


    }

    private IEnumerator FireLimit()
    {
        yield return new WaitForSeconds(fireDelay);
        canShoot = true;
    }
}
