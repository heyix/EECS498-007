Hud = {
	seconds_elapsed = 0,
	finish = false,

	OnStart = function(self)
		Event.Subscribe("event_victory", self, self.OnEventVictory)
	end,

	OnUpdate = function(self)

		if not self.finish then
			self.seconds_elapsed = self.seconds_elapsed + Time.DeltaTime()
		end

		local seconds_int = math.floor(self.seconds_elapsed)
		local text = "Time : " .. seconds_int
		local x = 10
		local y = 10

		if self.finish then
			text = "FINISH! Final Time : " .. seconds_int  .. " seconds!"
		end

		Text.Draw(text, x, y, "NotoSans-Regular", 24, 0, 0, 0, 255)
	end,

	OnEventVictory = function(self)
		self.finish = true
	end
}

