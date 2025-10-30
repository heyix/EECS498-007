MovingBackground = {
	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self)
		self.transform:Translate(-1,0)

		if self.transform.position.x <= -667 then
			local new_position = self.transform.position
			new_position.x = 667
			self.transform:SetWorldPosition(new_position)
		end
	end
}

