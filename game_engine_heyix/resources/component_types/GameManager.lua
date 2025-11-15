GameManager = {

	-- TILE CODES --
	-- 0 : nothing
	-- 1 : Static box
	-- 2 : player

	stage1 = {
		-- 40x40 all zeros, last row has one "2" as player
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},

	-- internal state for click vs hold
	_leftPressTime  = nil,
	_leftHoldMode   = false,
	_rightPressTime = nil,
	_rightHoldMode  = false,

	_holdThreshold = 0.5,   -- seconds

	SpawnAtMouse = function(self, prefabName)
		local mouse_pos = Input.GetMousePosition()
		local pixels_per_meter = 100.0
		local zoom = Camera.GetZoom()
		local cam_dim = Camera.GetCameraDimension()
		local x = Camera.GetPositionX()
		local y = Camera.GetPositionY()

		local world_x = (mouse_pos.x - cam_dim.x * 0.5 * (1.0 / zoom)) / pixels_per_meter + x
		local world_y = (mouse_pos.y - cam_dim.y * 0.5 * (1.0 / zoom)) / pixels_per_meter + y

		local new_body = Actor.Instantiate(prefabName)
		local new_body_rb = new_body:GetComponent("Rigidbody")
		new_body_rb.x = world_x
		new_body_rb.y = world_y

		Debug.Log("Spawned " .. prefabName .. " with ID: " .. tostring(new_body:GetID()))
	end,

	OnStart = function(self)
		-- Spawn stage
		for y = 1, #self.stage1 do
			for x = 1, #self.stage1[1] do
				local tile_code = self.stage1[y][x]
				local tile_pos = Vector2(x, y)

				if tile_code == 2 then
					local new_player = Actor.Instantiate("Player")
					local new_player_rb = new_player:GetComponent("Rigidbody")
					new_player_rb.x = tile_pos.x
					new_player_rb.y = tile_pos.y

				elseif tile_code == 1 then
					local new_box = Actor.Instantiate("KinematicBox")
					local new_box_rb = new_box:GetComponent("Rigidbody")
					new_box_rb.x = tile_pos.x
					new_box_rb.y = tile_pos.y
				end
			end
		end

		local new_body = Actor.Instantiate("Ground")
		Debug.Log("Spawned body with ID: " .. tostring(new_body:GetID()))

		local GROUND_WIDTH  = 20
		local GROUND_HEIGHT = 1

		local WALL_WIDTH    = 1 
		local WALL_HEIGHT   = 20

		local ground_tf = new_body:GetComponent("Transform")
		local gx0 = ground_tf:GetWorldPosition().x    
		local gy0 = ground_tf:GetWorldPosition().y  

		local half_ground_w = GROUND_WIDTH * 0.5
		local half_wall_w   = WALL_WIDTH   * 0.5
		local half_wall_h   = WALL_HEIGHT  * 0.5

		local cell_dx = GROUND_WIDTH + WALL_WIDTH  
		local cell_dy = WALL_HEIGHT             

		local function SpawnWallsAroundBox(gx, gy)
			local left_wall_x  = gx - half_ground_w - half_wall_w
			local right_wall_x = gx + half_ground_w + half_wall_w
			local wall_y       = gy - half_wall_h      
			local top_y        = gy - WALL_HEIGHT      

			local left_wall  = Actor.Instantiate("Wall")
			local right_wall = Actor.Instantiate("Wall")
			local up_wall    = Actor.Instantiate("Ground") 

			left_wall:GetComponent("Transform"):SetWorldPosition(Vector2(left_wall_x, wall_y))
			right_wall:GetComponent("Transform"):SetWorldPosition(Vector2(right_wall_x, wall_y))
			up_wall:GetComponent("Transform"):SetWorldPosition(Vector2(gx, top_y))

			Debug.Log("Left wall ID:  " .. tostring(left_wall:GetID()))
			Debug.Log("Right wall ID: " .. tostring(right_wall:GetID()))
		end

		SpawnWallsAroundBox(gx0, gy0)

		do
			local gx = gx0 + cell_dx
			local gy = gy0
			local ground = Actor.Instantiate("Ground")
			ground:GetComponent("Transform"):SetWorldPosition(Vector2(gx, gy))
			SpawnWallsAroundBox(gx, gy)
		end

		do
			local gx = gx0
			local gy = gy0 + cell_dy
			local ground = Actor.Instantiate("Ground")
			ground:GetComponent("Transform"):SetWorldPosition(Vector2(gx, gy))
			SpawnWallsAroundBox(gx, gy)
		end

		do
			local gx = gx0 + cell_dx
			local gy = gy0 + cell_dy
			local ground = Actor.Instantiate("Ground")
			ground:GetComponent("Transform"):SetWorldPosition(Vector2(gx, gy))
			SpawnWallsAroundBox(gx, gy)
		end
	end,

	OnUpdate = function(self)
		local now = Time.CurrentTime()

		if Input.GetMouseButtonDown(1) then
			self._leftPressTime = now
			self._leftHoldMode = false
		end

		if Input.GetMouseButton(1) and self._leftPressTime ~= nil and not self._leftHoldMode then
			if now - self._leftPressTime >= self._holdThreshold then
				self._leftHoldMode = true
			end
		end

		if Input.GetMouseButtonUp(1) and self._leftPressTime ~= nil then
			if not self._leftHoldMode then
				self:SpawnAtMouse("Body")
			end
			self._leftPressTime = nil
			self._leftHoldMode = false
		end

		if Input.GetMouseButtonDown(3) then
			self._rightPressTime = now
			self._rightHoldMode = false
		end

		if Input.GetMouseButton(3) and self._rightPressTime ~= nil and not self._rightHoldMode then
			if now - self._rightPressTime >= self._holdThreshold then
				self._rightHoldMode = true
			end
		end

		if Input.GetMouseButtonUp(3) and self._rightPressTime ~= nil then
			if not self._rightHoldMode then
				self:SpawnAtMouse("Body2")
			end
			self._rightPressTime = nil
			self._rightHoldMode = false
		end
	end,

	OnFixedUpdate = function(self)
		if self._leftHoldMode and Input.GetMouseButton(1) then
			self:SpawnAtMouse("Body")
		end

		if self._rightHoldMode and Input.GetMouseButton(3) then
			self:SpawnAtMouse("Body2")
		end
	end
}
