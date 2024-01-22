using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EnemyController : MonoBehaviour
{
    Rigidbody2D rigid;
    float move_speed = 3;
    AudioSource audio_source;
    public float new_x = 10f;
    public float left_board = -10f;
    void Start()
    {
        audio_source = GetComponent<AudioSource>();
        rigid = GetComponent<Rigidbody2D>();
    }

    void Update()
    {
        Vector2 move_velocity = new Vector2(-1,0)*move_speed;
        rigid.velocity = move_velocity;
        if (transform.position.x < left_board)
        {
            Vector2 new_position = transform.position;
            new_position.x = new_x;
            transform.position = new_position;
        }
    }

    private void OnTriggerEnter2D(Collider2D collision)
    {
        if (collision.gameObject.layer == LayerMask.NameToLayer("bullet"))
        {
            Destroy(gameObject);
            audio_source.Play();
            GameController.instance.target -= 1;
        }
        if (collision.gameObject.layer == LayerMask.NameToLayer("Player"))
        {
            Destroy(gameObject);
            audio_source.Play();
            GameController.instance.target -= 1;

        }
    }
}
