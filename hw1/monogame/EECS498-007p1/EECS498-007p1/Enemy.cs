using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Xna.Framework;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace EECS498_007p1
{
    class Enemy:Object
    {
        public Vector2 move_direction=new Vector2(-1,0);
        public int move_speed = 200;
        public override void Update(GameTime gameTime)
        {
            base.Update(gameTime);
            this.position.X -= this.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
        }
        public void collide_with_player()
        {
            Game1.instance.play_sound_effect("chew");
            Game1.instance.target -= 1;
        }
    }
}
