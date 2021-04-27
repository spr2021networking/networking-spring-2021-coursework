using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PillarHealth : MonoBehaviour
{
    public int maxHealth = 10;
    private int _currentHealth = 10;
    public int CurrentHealth
    {
        get
        {
            return _currentHealth;
        }
        set
        {
            if (_currentHealth > value)
            {
                _rend.material = damageMaterial;
            }
            _currentHealth = value;
            _damageTime = 0.2f;
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
        if (_rend.material == damageMaterial)
        {
            _damageTime -= Time.deltaTime;
            if (_damageTime < 0)
            {
                _rend.material = defaultMaterial;
            }
        }
    }
}
