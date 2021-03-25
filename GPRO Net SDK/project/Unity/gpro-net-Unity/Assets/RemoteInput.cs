using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RemoteInput : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void InterpretPosition(float x)
    {
        transform.position = new Vector3(x, transform.position.y, transform.position.z);
    }
}
