DonnaAnimation = {
	OnStart = function(self)
		self.spriterenderer = self.actor:GetComponent("SpriteRenderer")
	end,

	OnUpdate = function(self)

		local anim_frame = Application.GetFrame() % 24
		if anim_frame < 6 then
			self.spriterenderer.sprite = "donna1"
		elseif anim_frame < 12 then
			self.spriterenderer.sprite = "donna2"
		elseif anim_frame < 18 then
			self.spriterenderer.sprite = "donna1"
		else
			self.spriterenderer.sprite = "donna3"
		end
	end
}

