using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    Rigidbody2D rigid;
    float move_speed = 5;
    void Start()
    {
        rigid = GetComponent<Rigidbody2D>();
    }

    // Update is called once per frame
    void Update()
    {
        Vector2 move_velocity = Vector2.zero;
        move_velocity.x = Input.GetAxis("Horizontal") * move_speed;
        move_velocity.y= Input.GetAxis("Vertical") * move_speed;
        rigid.velocity = move_velocity;
    }
}
