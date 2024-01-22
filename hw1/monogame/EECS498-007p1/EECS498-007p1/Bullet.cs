using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System.Collections.Generic;
using System.Diagnostics;
namespace EECS498_007p1
{
    internal class Bullet:Object
    {
        public Vector2 move_direction = new Vector2(1, 0);
        public int move_speed = 200;
        public static Texture2D bullet_texture;
        public float survive_time = 3f;
        public float current_time = 0;
        public override Vector2 sprite_ratio {
            get { return new Vector2(0.1f, 0.1f); }

        }
        public override void Update(GameTime gameTime)
        {
            base.Update(gameTime);
            this.position.X += this.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
        }
    }
}
