ConstantMovement = {
	x_vel = 0,
	y_vel = 0,

	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
		self.rb = self.actor:GetComponent("Rigidbody")
	end,

	OnUpdate = function(self)
		if self.rb then
			self.rb:Translate(self.x_vel,self.y_vel)
		else
			self.transform:Translate(self.x_vel,self.y_vel)
		end
	end
}

