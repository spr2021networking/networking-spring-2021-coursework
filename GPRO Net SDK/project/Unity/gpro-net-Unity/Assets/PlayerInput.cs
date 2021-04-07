using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInput : MonoBehaviour
{
    Rigidbody m_Rigidbody;
    [SerializeField]
    float movementSpeed = 2.5f;
    [SerializeField]
    GameObject bullet;
    [SerializeField]
    float fireDelay = 1.0f;
    [SerializeField]
    float bulletSpeed = 7.5f;
    float newXPos;
    bool canShoot;
    // Start is called before the first frame update
    void Start()
    {
        m_Rigidbody = GetComponent<Rigidbody>();
        canShoot = true;
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        float horizontalInput = Input.GetAxis("Horizontal");
        float verticalInput = Input.GetAxis("Vertical");
        Vector3 m_Input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));
        m_Rigidbody.MovePosition(transform.position + m_Input * Time.deltaTime * movementSpeed);
        if (Input.GetKey(KeyCode.Space) && canShoot)
        {
            fireBullet();
            canShoot = false;
            StartCoroutine("FireLimit");
        }
    }

    void fireBullet()
    {
        if (Input.GetKey(KeyCode.A) && !Input.GetKey(KeyCode.S) && !Input.GetKey(KeyCode.W))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + (Vector3.left * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(-bulletSpeed, 0, 0);
        }
        else if (Input.GetKey(KeyCode.S) && !Input.GetKey(KeyCode.A) && !Input.GetKey(KeyCode.D))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + (Vector3.back * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(0, 0, -bulletSpeed);
        }
        else if(Input.GetKey(KeyCode.D) && !Input.GetKey(KeyCode.S) && !Input.GetKey(KeyCode.W))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + (Vector3.right * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(bulletSpeed, 0, 0);
        }
        else if(Input.GetKey(KeyCode.W) && !Input.GetKey(KeyCode.A) && !Input.GetKey(KeyCode.D))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + (Vector3.forward * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(0, 0, bulletSpeed);
        }
        else if(Input.GetKey(KeyCode.A) && Input.GetKey(KeyCode.S))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + ((Vector3.left + Vector3.back) * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(-bulletSpeed, 0, -bulletSpeed);
        }
        else if(Input.GetKey(KeyCode.A) && Input.GetKey(KeyCode.W))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + ((Vector3.left + Vector3.forward) * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(-bulletSpeed, 0, bulletSpeed);
        }
        else if(Input.GetKey(KeyCode.D) && Input.GetKey(KeyCode.S))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + ((Vector3.right + Vector3.back) * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(bulletSpeed, 0, -bulletSpeed);
        }
        else if(Input.GetKey(KeyCode.D) && Input.GetKey(KeyCode.W))
        {
            GameObject spawnedBullet = Instantiate(bullet, transform.position + ((Vector3.right + Vector3.forward) * 3), Quaternion.identity);
            spawnedBullet.GetComponent<Rigidbody>().velocity = new Vector3(bulletSpeed, 0, bulletSpeed);
        }

    }

    private IEnumerator FireLimit()
    {
        yield return new WaitForSeconds(fireDelay);
        canShoot = true;
    }
}
