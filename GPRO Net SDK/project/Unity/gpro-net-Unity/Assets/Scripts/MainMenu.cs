using System.Collections;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Assets.Scripts
{
    public class MainMenu : MonoBehaviour
    {
        public TMP_InputField ip;
        public Button button;

        public ShieldClient client;
        // Use this for initialization
        void Start()
        {
            ip.text = "172.16.4.196";
        }

        // Update is called once per frame
        void Update()
        {

        }

        public void Connect()
        {
            client.Connect(ip.text);
        }

    }
}