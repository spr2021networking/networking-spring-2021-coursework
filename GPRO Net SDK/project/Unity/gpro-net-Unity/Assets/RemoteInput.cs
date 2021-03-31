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
        //(0.00, 0.00, 0.00)
        //0123456789ABCDEFGH
        if (pos.Length >= 18)
        {
            int firstNumStart = pos.IndexOf('(') + 1;
            int firstComma = pos.Substring(firstNumStart).IndexOf(',') + firstNumStart;
            int secondComma = pos.Substring(firstComma + 1).IndexOf(',') + firstComma + 1;
            int closeParen = pos.Substring(secondComma).IndexOf(')') + secondComma;
            vec.x = (float)Convert.ToDouble(pos.Substring(firstNumStart, firstComma - firstNumStart));
            vec.y = (float)Convert.ToDouble(pos.Substring(firstComma + 1, secondComma - (firstComma + 1)));
            vec.z = (float)Convert.ToDouble(pos.Substring(secondComma + 1, closeParen - (secondComma + 1)));
            Debug.Log(vec);
            transform.position = vec;
        }

        //transform.position = new Vector3(x, transform.position.y, transform.position.z);
    }
}
