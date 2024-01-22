/// @DnDAction : YoYo Games.Common.If_Expression
/// @DnDVersion : 1
/// @DnDHash : 12A3A955
/// @DnDArgument : "expr" "cool_down<=0"
if(cool_down<=0)
{
	/// @DnDAction : YoYo Games.Instances.Create_Instance
	/// @DnDVersion : 1
	/// @DnDHash : 29868CE9
	/// @DnDParent : 12A3A955
	/// @DnDArgument : "xpos" "150"
	/// @DnDArgument : "xpos_relative" "1"
	/// @DnDArgument : "ypos" "40"
	/// @DnDArgument : "ypos_relative" "1"
	/// @DnDArgument : "objectid" "Bullet"
	/// @DnDSaveInfo : "objectid" "Bullet"
	instance_create_layer(x + 150, y + 40, "Instances", Bullet);

	/// @DnDAction : YoYo Games.Common.Variable
	/// @DnDVersion : 1
	/// @DnDHash : 5AB9962C
	/// @DnDParent : 12A3A955
	/// @DnDArgument : "expr" "0.2"
	/// @DnDArgument : "var" "player.cool_down"
	player.cool_down = 0.2;
}