using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System.Collections.Generic;
using System.Diagnostics;

namespace EECS498_007p1
{
    internal class Icon:Object
    {
        public int index = 0;
        public Icon(int new_index,Vector2 new_position) { index = new_index; position = new_position; }
        public override Vector2 sprite_ratio
        {
            get { return new Vector2(0.05f, 0.05f); }

        }
    }
}
