KeyboardControls = {
	speed = 0.5,
	jump_power = 350,
	jump_requested = false,

	OnStart = function(self)
		self.rb = self.actor:GetComponent("Rigidbody")
	end,

	OnUpdate = function(self)
		-- record input once per frame
		if Input.GetKeyDown("up") or Input.GetKeyDown("space") then
			self.jump_requested = true
		end
	end,

	OnFixedUpdate = function(self)
		local horizontal_input = 0
		if Input.GetKey("right") then horizontal_input = self.speed end
		if Input.GetKey("left") then horizontal_input = -self.speed end

		-- local vertical_input = 0
		-- local ground_object = Physics.Raycast(self.rb:GetPosition(), Vector2(0, 1), 1)

		-- -- use the buffered flag
		-- if self.jump_requested and ground_object ~= nil then
		-- 	vertical_input = -self.jump_power
		-- 	self.jump_requested = false -- consume it
		-- end
		local vertical_input = 0;
		if Input.GetKey("up") then vertical_input = -self.speed end
		if Input.GetKey("down") then vertical_input = self.speed end

		self.rb:Translate(horizontal_input, vertical_input)
	end
}

