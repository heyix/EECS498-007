Projectile = {
	destroyed = false,

	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
		-- Register this enemy in the global table.
		table.insert(all_projectiles, self)
	end,

	OnLateUpdate = function(self)
		
		if self.transform.position.x > 640 then
			self.destroyed = true
		end
		
		if self.destroyed then
			self:OnDestroy()
			Actor.Destroy(self.actor)
		end
	end,

	-- This OnDestroy function is not a lifecycle function, and is not automatically called from the engine.
	-- It is called manually by other lua components.
	-- In homework 8, you'll make this an automatically-called lifecycle function.
	OnDestroy = function(self)
		-- Remove this enemy from the global table.
		for i, projectile in ipairs(all_projectiles) do
			if projectile == self then
				table.remove(all_projectiles, i)
				break
			end
		end
	end
}

