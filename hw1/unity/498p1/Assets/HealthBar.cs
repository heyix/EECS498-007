using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HealthBar : MonoBehaviour
{
    public int position = 0;
    private void Update()
    {
        if (PlayerController.instance.health < position)
        {
            gameObject.SetActive(false);
        }
    }
}
