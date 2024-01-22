using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace EECS498_007p1
{
    class Player:Object
    {
        public Vector2 move_direction = new Vector2(-1, 0);
        public int move_speed = 300;
        public Vector2 fire_position;
        public HashSet<Bullet> bullets_pool=new HashSet<Bullet>();
        public float cool_down = 0.2f;
        float current_cool_down = 0;
        public int health = 3;

        public override void Update(GameTime gameTime)
        {
            base.Update(gameTime);
            move(gameTime);
            fire_position=position+new Vector2(90,-40);
            check_fire();
            check_bullet_valid(gameTime);
            foreach(var bullet in bullets_pool)
            {
                bullet.Update(gameTime);
            }
            if (current_cool_down > 0) current_cool_down -= (float)gameTime.ElapsedGameTime.TotalSeconds;
        }

        void move(GameTime gameTime)
        {
            var kstate = Keyboard.GetState();
            if (kstate.IsKeyDown(Keys.Up))
            {
                this.position.Y -= this.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
            if (kstate.IsKeyDown(Keys.Down))
            {
                this.position.Y += this.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
            if (kstate.IsKeyDown(Keys.Left))
            {
                this.position.X -= this.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
            if (kstate.IsKeyDown(Keys.Right))
            {
                this.position.X += this.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
        }
        void check_fire()
        {
            var kstate = Keyboard.GetState();
            if (kstate.IsKeyDown(Keys.Space) && current_cool_down<=0)
            {
                Bullet bullet=new Bullet();
                bullet.position = fire_position;
                bullet.sprite_texture = Bullet.bullet_texture;
                bullets_pool.Add(bullet);
                current_cool_down = cool_down;
                Game1.instance.play_sound_effect("snap");
            }

        }
        void check_bullet_valid(GameTime gameTime) { 
            foreach(var bullet in bullets_pool)
            {
                bullet.current_time += (float)gameTime.ElapsedGameTime.TotalSeconds;
                if (bullet.current_time > bullet.survive_time)
                {
                    bullets_pool.Remove(bullet);
                }
            }
        }
        public override void draw_object(SpriteBatch _spriteBatch, bool change_sprite_scale = false)
        {
            base.draw_object(_spriteBatch);
            foreach(var bullet in bullets_pool)
            {
                bullet.draw_object(_spriteBatch,true);
            }
        }
        public bool check_bullet_collide_with_enemy(Enemy enemy)
        {

            foreach(var bullet in bullets_pool)
            {
                if (Game1.instance.check_collision_between_object(bullet, enemy))
                {
                    bullets_pool.Remove(bullet);
                    Game1.instance.target -= 1;
                    return true;
                }
            }
            return false;
        }

        public void collide_with_enemy()
        {
            Game1.instance.play_sound_effect("ouch");
            health--;
        }

    }
}
