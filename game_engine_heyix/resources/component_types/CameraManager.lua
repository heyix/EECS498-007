CameraManager = {
	ease_factor = 1,
	tracking_player = false,

	OnUpdate = function(self)
		local player_actor = Actor.Find("player")
		if player_actor == nil then
			self.tracking_player = false
			return
		
		elseif self.tracking_player == false then
			self.tracking_player = true
			local transform = player_actor:GetComponent("Transform")
			local player_pos = transform:GetWorldPosition();
			Camera.SetPosition(player_pos.x, player_pos.y)
			return
		end

		local transform = player_actor:GetComponent("Transform")
		local desired_position = transform:GetWorldPosition();
		local current_position = Vector2(Camera.GetPositionX(), Camera.GetPositionY())

		local new_position = current_position + (desired_position - current_position) * self.ease_factor
		Camera.SetPosition(new_position.x, new_position.y)
	end
}