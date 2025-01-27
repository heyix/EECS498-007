/// @DnDAction : YoYo Games.Common.Variable
/// @DnDVersion : 1
/// @DnDHash : 00BE0912
/// @DnDArgument : "expr" "-1"
/// @DnDArgument : "expr_relative" "1"
/// @DnDArgument : "var" "player_health"
player_health += -1;

/// @DnDAction : YoYo Games.Audio.Play_Audio
/// @DnDVersion : 1.1
/// @DnDHash : 4DC05CC6
/// @DnDArgument : "soundid" "takedamage"
/// @DnDSaveInfo : "soundid" "takedamage"
audio_play_sound(takedamage, 0, 0, 1.0, undefined, 1.0);