KeyboardControl = {
	speed = 3,

	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self)

		-- Collect Inputs
		local vel_x = 0
		local vel_y = 0

		if Input.GetKey("right") then
			vel_x = vel_x + self.speed
		end

		if Input.GetKey("left") then
			vel_x = vel_x - self.speed
		end

		if Input.GetKey("up") then
			vel_y = vel_y - self.speed
		end

		if Input.GetKey("down") then
			vel_y = vel_y + self.speed
		end

		-- Apply to transform
		self.transform:Translate(vel_x,vel_y)

		-- Keep within gameplay area
		if self.transform.position.x > 600 then
			self.transform:SetWorldPositionXY(600,self.transform.position.y)
		end

		if self.transform.position.x < 0 then
			self.transform:SetWorldPositionXY(0,self.transform.position.y)
			self.transform.position.x = 0
		end

		if self.transform.position.y > 320 then
			self.transform:SetWorldPositionXY(self.transform.position.x,320)
		end

		if self.transform.position.y < 0 then
			self.transform:SetWorldPositionXY(self.transform.position.x,0)
		end
	end
}

