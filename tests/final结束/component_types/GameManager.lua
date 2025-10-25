GameManager = {
	num_enemies = 10,
	base_enemy_movement_speed = -1.5,

	OnStart = function(self)
		math.randomseed(494)

		-- Establish necessary references.
		self.donna = Actor.Find("donna"):GetComponent("Donna")

		-- Establish global collections.
		if all_enemies == nil then
			all_enemies = {}
		end

		if all_projectiles == nil then
			all_projectiles = {}
		end

		score = 0

		game_over = false
	end,

	OnUpdate = function(self)
		self.base_enemy_movement_speed = self.base_enemy_movement_speed - 0.001

		if Application.GetFrame() % 60 == 0 and not game_over then
			self:SpawnEnemy()
		end
	end,

	SpawnEnemy = function(self)
		local new_enemy = Actor.Instantiate("Enemy")
		local enemy_transform = new_enemy:GetComponent("Transform")

		-- Determine random sprite.
		local possible_sprites = { "enemy1", "enemy2", "enemy3" }
		local chosen_sprite = possible_sprites[ math.random(1, #possible_sprites) ]
		new_enemy:GetComponent("SpriteRenderer").sprite = chosen_sprite
		new_enemy:GetComponent("ConstantMovement").x_vel = self.base_enemy_movement_speed + math.random() * -1.0

		-- Determine random position.
		local enemyPos = enemy_transform:GetWorldPosition()
		enemyPos.x = 700 + math.random() * 200
		enemyPos.y = 50 + math.random() * 250
		enemy_transform.position = enemyPos
	end
}

