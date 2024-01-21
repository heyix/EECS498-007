using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Bullet : MonoBehaviour
{
    public int fly_time = 10;
    public int destroy_time = 3;
    Rigidbody2D rigid;
    public int health = 3;
    private void Start()
    {
        Destroy(gameObject, destroy_time);
        rigid = GetComponent<Rigidbody2D>();
        rigid.velocity = new Vector2 (fly_time, 0 );
    }
    private void OnTriggerEnter2D(Collider2D collision)
    {
        if (collision.gameObject.layer == LayerMask.NameToLayer("Enemy"))
        {
            Destroy(gameObject);
        }
    }
}
