using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    Rigidbody2D rigid;
    float move_speed = 5;
    public Transform fire_position;
    public GameObject bullet_prefab;
    public float cool_down=0.1f;
    float current_cool_down = 0;
    public static PlayerController instance;
    public int health = 3;
    AudioSource audio_source;

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
    void Start()
    {
        rigid = GetComponent<Rigidbody2D>();
        audio_source = GetComponent<AudioSource>();
    }

    // Update is called once per frame
    void Update()
    {
        Vector2 move_velocity = Vector2.zero;
        move_velocity.x = Input.GetAxis("Horizontal") * move_speed;
        move_velocity.y= Input.GetAxis("Vertical") * move_speed;
        rigid.velocity = move_velocity;

        if (Input.GetKey(KeyCode.Space) && current_cool_down<=0)
        {
            GameObject bullet_instance = GameObject.Instantiate(bullet_prefab);
            bullet_instance.transform.position = fire_position.position;
            current_cool_down = cool_down;
        }
        if (current_cool_down > 0) current_cool_down -= Time.deltaTime;
    }
    private void OnTriggerEnter2D(Collider2D collision)
    {
        if (collision.gameObject.layer == LayerMask.NameToLayer("Enemy"))
        {
            health -= 1;
            audio_source.Play();
        }
    }
}
