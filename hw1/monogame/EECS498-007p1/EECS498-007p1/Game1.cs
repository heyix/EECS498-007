using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System.Collections.Generic;
using System.Diagnostics;

namespace EECS498_007p1
{
    public class Game1 : Game
    {
        private GraphicsDeviceManager _graphics;
        private SpriteBatch _spriteBatch;
        List<Enemy> enemies;
        Player player;
        Texture2D background;
        SoundEffect bgm;
        public Game1()
        {
            _graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
            IsMouseVisible = true;
        }

        protected override void Initialize()
        {
            // TODO: Add your initialization logic here
            float startX = _graphics.PreferredBackBufferWidth * 0.9f;
            enemies = new List<Enemy>()
            {
                new Enemy(){ position=new Vector2(startX,_graphics.PreferredBackBufferHeight*0.2f),texture_name="enemy1"},
                new Enemy(){ position=new Vector2(startX,_graphics.PreferredBackBufferHeight*0.5f),texture_name="enemy2"},
                new Enemy(){ position=new Vector2(startX,_graphics.PreferredBackBufferHeight*0.8f),texture_name="enemy3"},
            };
            player = new Player() { position = new Vector2(100, 100), texture_name = "player" };
            base.Initialize();
        }

        protected override void LoadContent()
        {
            _spriteBatch = new SpriteBatch(GraphicsDevice);

            // TODO: use this.Content to load your game content here
            foreach (Enemy enemy in enemies)
            {
                enemy.srpite_texture = Content.Load<Texture2D>(enemy.texture_name);
            }
            player.srpite_texture = Content.Load<Texture2D>(player.texture_name);
            background = Content.Load<Texture2D>("background");

            bgm = Content.Load<SoundEffect>("bgm");
            SoundEffectInstance bgm_instance = bgm.CreateInstance();
            bgm_instance.IsLooped = true;
            bgm_instance.Play();
        }

        protected override void Update(GameTime gameTime)
        {
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed || Keyboard.GetState().IsKeyDown(Keys.Escape))
                Exit();

            // TODO: Add your update logic here
            var kstate=Keyboard.GetState();
            if (kstate.IsKeyDown(Keys.Up))
            {
                player.position.Y -= player.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
            if (kstate.IsKeyDown(Keys.Down))
            {
                player.position.Y += player.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
            if (kstate.IsKeyDown(Keys.Left))
            {
                player.position.X -= player.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }
            if (kstate.IsKeyDown(Keys.Right))
            {
                player.position.X += player.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }

            foreach(Enemy enemy in enemies)
            {
                enemy.position.X -= enemy.move_speed * (float)gameTime.ElapsedGameTime.TotalSeconds;
            }

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);

            // TODO: Add your drawing code here
            _spriteBatch.Begin();
            _spriteBatch.Draw(background, new Vector2(0, 0), Color.White);
            Vector2 half_player_dimension = new Vector2(player.srpite_texture.Width, player.srpite_texture.Height) * 0.5f;
            _spriteBatch.Draw(player.srpite_texture, player.position- half_player_dimension, Color.White);
            Debug.WriteLine(player.position);
            foreach(Enemy enemy in enemies)
            {
                Vector2 half_enemy_dimension = new Vector2(enemy.srpite_texture.Width, enemy.srpite_texture.Height) * 0.5f;
                _spriteBatch.Draw(enemy.srpite_texture, enemy.position-half_enemy_dimension, Color.White);
            }






            _spriteBatch.End();
            base.Draw(gameTime);
        }
    }
}
