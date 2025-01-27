/// @DnDAction : YoYo Games.Movement.Wrap_Room
/// @DnDVersion : 1
/// @DnDHash : 1031ED9B
/// @DnDArgument : "margin" "100"
move_wrap(1, 1, 100);

/// @DnDAction : YoYo Games.Common.If_Expression
/// @DnDVersion : 1
/// @DnDHash : 6321ED4E
/// @DnDArgument : "expr" "cool_down>0"
if(cool_down>0)
{
	/// @DnDAction : YoYo Games.Common.Variable
	/// @DnDVersion : 1
	/// @DnDHash : 1BDDABB9
	/// @DnDParent : 6321ED4E
	/// @DnDArgument : "expr" "-delta_time/ 1000000"
	/// @DnDArgument : "expr_relative" "1"
	/// @DnDArgument : "var" "cool_down"
	cool_down += -delta_time/ 1000000;
}