using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System.Collections.Generic;
using System.Diagnostics;

namespace EECS498_007p1
{
    public class Object
{
        public Texture2D sprite_texture;
        public Vector2 position;
        public string texture_name;
        public virtual Vector2 sprite_ratio
        {
            get { return new Vector2(1, 1); }
            set { sprite_ratio = value; }
        }
        public virtual void draw_object(SpriteBatch _spriteBatch,bool change_sprite_scale=false)
        {
            if (!change_sprite_scale)
            {
                Vector2 half_dimension = new Vector2(this.sprite_texture.Width, this.sprite_texture.Height) * 0.5f;
                _spriteBatch.Draw(this.sprite_texture, this.position - half_dimension, Color.White);
            }
            else
            {
                Vector2 half_dimension = new Vector2(this.sprite_texture.Width* sprite_ratio.X, this.sprite_texture.Height* sprite_ratio.Y) * 0.5f;
                _spriteBatch.Draw(this.sprite_texture, this.position - half_dimension, null, Color.White, 0f, Vector2.Zero, sprite_ratio, SpriteEffects.None, 0f);

            }

        }
        public virtual void Update(GameTime gameTime)
        {

        }


    }
}
