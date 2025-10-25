SpriteRenderer = {
	sprite = "???",
	r = 255,
	g = 255,
	b = 255,
	a = 255,
	sorting_order = 0,

	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self)
		self.position = self.transform:GetWorldPosition()
		Image.DrawUIEx(self.sprite, self.position.x,self.position.y, self.r, self.g, self.b, self.a, self.sorting_order)
	end
}

