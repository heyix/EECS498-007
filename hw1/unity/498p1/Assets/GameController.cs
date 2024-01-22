using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class GameController : MonoBehaviour
{
    public static GameController instance;
    public int target=3;
    public float time_to_survive = 5f;
    private void Awake()
    {
        if (instance == null)
        {
            instance = this;
        }
        else if (instance != this)
        {
            Destroy(gameObject);
        }
    }

    private void Update()
    {
        if (PlayerController.instance.health == 0)
        {
            SceneManager.LoadScene("Fail");
        }
        if (target == 0 && PlayerController.instance.health != 0)
        {
            SceneManager.LoadScene("Victory");
        }
        if (time_to_survive <= 0)
        {
            SceneManager.LoadScene("Victory");
        }
        time_to_survive -= Time.deltaTime;
    }
}
