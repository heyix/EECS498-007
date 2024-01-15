using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EnemyController : MonoBehaviour
{
    Rigidbody2D rigid;
    float move_speed = 3;
    void Start()
    {
        rigid = GetComponent<Rigidbody2D>();
    }

    void Update()
    {
        Vector2 move_velocity = new Vector2(-1,0)*move_speed;
        rigid.velocity = move_velocity;
    }
}
