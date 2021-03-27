using System;
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

    public void InterpretPosition(string pos)
    {
        Vector3 vec = Vector3.zero;
        //(0.00,0.00,0.00)
        //0123456789ABCDEF
        vec.x = (float) Convert.ToDouble(pos.Substring(1, 4));
        vec.y = (float) Convert.ToDouble(pos.Substring(6, 4));
        vec.z = (float) Convert.ToDouble(pos.Substring(11, 4));
        transform.position = vec;
        //transform.position = new Vector3(x, transform.position.y, transform.position.z);
    }
}
