/// @DnDAction : YoYo Games.Common.If_Expression
/// @DnDVersion : 1
/// @DnDHash : 05704854
/// @DnDArgument : "expr" "player.player_health<index"
if(player.player_health<index)
{
	/// @DnDAction : YoYo Games.Instances.Destroy_Instance
	/// @DnDVersion : 1
	/// @DnDHash : 45174E70
	/// @DnDParent : 05704854
	instance_destroy();
}