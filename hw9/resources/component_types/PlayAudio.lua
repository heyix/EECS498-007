PlayAudio = {
	clip = "",
	loop = true,

	OnStart = function(self)
		Audio.Play(0, self.clip, true)
	end
}

