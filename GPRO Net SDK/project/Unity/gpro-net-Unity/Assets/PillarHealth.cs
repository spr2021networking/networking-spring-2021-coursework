using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Authors: Scott Dagen & Ben Cooper
/// Handles the pillar, which the AI are trying to attack
/// </summary>
public class PillarHealth : MonoBehaviour
{
    public int maxHealth = 10;
    private int _currentHealth = 10;
    public int CurrentHealth //get or set the current health of the pillar
    {
        get
        {
            return _currentHealth;
        }
        set
        {
            if (_currentHealth > value)
            {
                _rend.sharedMaterial = damageMaterial; //change the material when damaged
                _damageTime = 0.2f;
            }
            _currentHealth = value;
        }
    }

    public Material damageMaterial;
    public Material defaultMaterial;

    private MeshRenderer _rend;

    private float _damageTime;
    // Start is called before the first frame update
    void Start()
    {
        _rend = GetComponent<MeshRenderer>();
    }

    // Update is called once per frame
    void Update()
    {
        if (_rend.sharedMaterial == damageMaterial)
        {
            _damageTime -= Time.deltaTime;
            if (_damageTime < 0) //after a set time, change the material back
            {
                _rend.material = defaultMaterial;
            }
        }
    }
}
