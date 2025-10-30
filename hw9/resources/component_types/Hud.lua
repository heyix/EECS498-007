Hud = {
	transition_y = -360,
	transition_y_vel = 0,
	transition_frames = 0,
	played_ending_audio_clip = false,

	OnStart = function(self)
		self.donna = Actor.Find("donna"):GetComponent("Donna")
		Audio.Play(1, "mission_start_vocal", false)
	end,

	OnLateUpdate = function(self)
		-- Draw Score
		--Render.DrawUI(self.sprite, self.transform.x, self.transform.y, 0)
		Text.Draw("Score : " .. score, 500, 10, "NotoSans-Regular", 20, 255, 255, 255, 255)

		-- Draw lives
		Image.DrawUI("hud", 0, 0)
		local lives = self.donna.lives
	
		for i = 0, lives-1 do
			local x_pos = 15
			local y_pos = 20

			Image.DrawUI("life", x_pos + 50 * i, y_pos - 8 * i)
		end

		-- Draw Game Over
		if lives <= 0 then
			game_over = true
			-- Audio clips
			if self.transition_frames == 0 then
				Audio.Play(0, "ThisGameIsOver_by_mccartneytm_ccby", false)
			end

			self.transition_frames = self.transition_frames + 1
			if self.transition_frames > 150 and self.played_ending_audio_clip == false then
				Audio.Play(1, "game_over_vocal", false)
				self.played_ending_audio_clip = true
			end

			-- Accelerate downwards
			self.transition_y_vel = self.transition_y_vel + 0.2
			self.transition_y = self.transition_y + self.transition_y_vel

			-- bounce
			if self.transition_y > 0 then
				self.transition_y = 0
				self.transition_y_vel = -self.transition_y_vel * 0.3
			end

			Image.DrawUIEx("game_over_bad", 0, self.transition_y, 255, 255, 255, 255, 2)
		end
	end
}

