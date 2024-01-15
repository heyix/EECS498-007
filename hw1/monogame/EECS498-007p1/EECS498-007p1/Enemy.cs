using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Xna.Framework;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework.Graphics;

namespace EECS498_007p1
{
    class Enemy
    {
        public Vector2 position;
        public string texture_name;
        public Vector2 move_direction=new Vector2(-1,0);
        public int move_speed = 200;
        public Texture2D srpite_texture;
    }
}
