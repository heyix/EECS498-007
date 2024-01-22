using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Xml.Linq;




namespace EECS498_007p1
{
    public class Game1 : Game
    {

        private GraphicsDeviceManager _graphics;
        private SpriteBatch _spriteBatch;
        HashSet<Enemy> enemies;
        List<Icon> icons;
        Player player;
        Texture2D background;
        Texture2D fail;
        Texture2D complete;
        Dictionary<string, SoundEffect> sound_effect_storage;
        List<string> sound_effect_name_list=new List<string>() { "bgm","chew","game_over","mission_complete", "mission_start", "ouch","snap"};
        public static Game1 instance;
        Dictionary<string, SoundEffectInstance> instance_map;
        public int target = 3;
        public float time=0;
        enum GameState
        {
            Normal,Victory,Fail
        }
        GameState game_state = GameState.Normal;
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
            enemies = new HashSet<Enemy>()
            {
                new Enemy(){ position=new Vector2(startX,_graphics.PreferredBackBufferHeight*0.2f),texture_name="enemy1"},
                new Enemy(){ position=new Vector2(startX,_graphics.PreferredBackBufferHeight*0.5f),texture_name="enemy2"},
                new Enemy(){ position=new Vector2(startX,_graphics.PreferredBackBufferHeight*0.8f),texture_name="enemy3"},
            };
            icons = new List<Icon>()
            {
                new Icon(1,new Vector2(_graphics.PreferredBackBufferWidth * 0.05f,_graphics.PreferredBackBufferHeight*0.05f)),
                new Icon(2,new Vector2(_graphics.PreferredBackBufferWidth * 0.15f,_graphics.PreferredBackBufferHeight*0.05f)), new Icon(3,new Vector2(_graphics.PreferredBackBufferWidth * 0.25f,_graphics.PreferredBackBufferHeight*0.05f))
            };
            player = new Player() { position = new Vector2(100, 100), texture_name = "player" };
            sound_effect_storage=new Dictionary<string, SoundEffect>();
            instance_map = new Dictionary<string, SoundEffectInstance>();
            instance = this;
            base.Initialize();
        }

        protected override void LoadContent()
        {
            _spriteBatch = new SpriteBatch(GraphicsDevice);

            // TODO: use this.Content to load your game content here
            foreach (Enemy enemy in enemies)
            {
                enemy.sprite_texture = Content.Load<Texture2D>(enemy.texture_name);
            }
            player.sprite_texture = Content.Load<Texture2D>(player.texture_name);
            Bullet.bullet_texture= Content.Load<Texture2D>("projectile");
            background = Content.Load<Texture2D>("background");
            complete = Content.Load<Texture2D>("donna_victory");
            fail = Content.Load<Texture2D>("donna_game_over");
            foreach (var icon in icons)
            {
                icon.sprite_texture = Content.Load<Texture2D>("donna_life");
            }

            foreach(var sound_effect_name in sound_effect_name_list)
            {
                sound_effect_storage[sound_effect_name]=load_sound_effect(sound_effect_name);
            }
            play_sound_effect("bgm", true,0.5f);
            play_sound_effect("mission_start");

        }

        protected override void Update(GameTime gameTime)
        {
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed || Keyboard.GetState().IsKeyDown(Keys.Escape))
                Exit();

            // TODO: Add your update logic here
            if (game_state == GameState.Normal)
            {
                player.Update(gameTime);
                foreach (Enemy enemy in enemies)
                {
                    enemy.Update(gameTime);
                    if (check_collision_between_object(player, enemy))
                    {
                        player.collide_with_enemy();
                        enemy.collide_with_player();
                        enemies.Remove(enemy);
                    }
                    if (player.check_bullet_collide_with_enemy(enemy))
                    {
                        play_sound_effect("chew");
                        enemies.Remove(enemy);
                    }
                }
            }

            if (player.health <= 0 && game_state==GameState.Normal)
            {
                game_state = GameState.Fail;
                play_sound_effect("game_over");
                instance_map["bgm"].Stop();
            }
            else if ((time >= 5 || target == 0) && game_state == GameState.Normal)
            {
                game_state = GameState.Victory;
                play_sound_effect("mission_complete");
                instance_map["bgm"].Stop();
            }
            time += (float)gameTime.ElapsedGameTime.TotalSeconds;
            
            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);

            _spriteBatch.Begin();


            if (game_state == GameState.Normal)
            {
                _spriteBatch.Draw(background, new Vector2(0, 0), Color.White);



                player.draw_object(_spriteBatch);
                foreach (Enemy enemy in enemies)
                {
                    enemy.draw_object(_spriteBatch);
                }

                foreach(var icon in icons)
                {
                    if (player.health >= icon.index)
                    {
                        icon.draw_object(_spriteBatch, true);
                    }
                }

            }
            if (game_state == GameState.Fail)
            {
                Vector2 ratio = new Vector2(0.45f, 0.45f);
                _spriteBatch.Draw(fail, new Vector2(), null, Color.White, 0f, Vector2.Zero, ratio, SpriteEffects.None, 0f);
            }

            if (game_state == GameState.Victory)
            {
                Vector2 ratio = new Vector2(0.45f, 0.45f);
                _spriteBatch.Draw(complete, new Vector2(), null, Color.White, 0f, Vector2.Zero, ratio, SpriteEffects.None, 0f);
            }









            _spriteBatch.End();
            base.Draw(gameTime);
        }

        SoundEffect load_sound_effect(string sound_effect_name)
        {
            SoundEffect sound_effect = Content.Load<SoundEffect>(sound_effect_name);
            return sound_effect;
        }

        public void play_sound_effect(string sound_effect_name,bool is_loop=false,float volume=1f)
        {
            SoundEffectInstance instance = sound_effect_storage[sound_effect_name].CreateInstance();
            instance.Volume = volume;
            instance.IsLooped = is_loop;
            instance.Play();
            instance_map[sound_effect_name] = instance;
        }

        public bool check_collision_between_object(Object object1, Object object2)
        {
            Rectangle sprite1Rectangle = new Rectangle(
                (int)object1.position.X,
                (int)object1.position.Y,
                (int)(object1.sprite_texture.Width * object1.sprite_ratio.X),
                (int)(object1.sprite_texture.Height * object1.sprite_ratio.Y)
            );

            Rectangle sprite2Rectangle = new Rectangle(
                (int)object2.position.X,
                (int)object2.position.Y,
                (int)(object2.sprite_texture.Width * object2.sprite_ratio.X),
                (int)(object2.sprite_texture.Height * object2.sprite_ratio.Y)
            );

            return sprite1Rectangle.Intersects(sprite2Rectangle);
        }
        public void draw_object_rect(Object target_object)
        {
            Texture2D pixelTexture = new Texture2D(GraphicsDevice, 1, 1);
            pixelTexture.SetData(new[] { Color.White });

            Rectangle sprite1Rectangle = new Rectangle(
                (int)target_object.position.X,
                (int)target_object.position.Y,
                (int)(target_object.sprite_texture.Width * target_object.sprite_ratio.X),
                (int)(target_object.sprite_texture.Height * target_object.sprite_ratio.Y)
            );
            Color rectColor = Color.Red; // Color of the rectangle

            // Draw the filled rectangle
            _spriteBatch.Draw(pixelTexture, sprite1Rectangle, rectColor);

        }
    }



}
