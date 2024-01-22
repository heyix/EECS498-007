/// @DnDAction : YoYo Games.Common.If_Expression
/// @DnDVersion : 1
/// @DnDHash : 39D3C0E4
/// @DnDArgument : "expr" "target==0"
if(target==0)
{
	/// @DnDAction : YoYo Games.Rooms.Go_To_Room
	/// @DnDVersion : 1
	/// @DnDHash : 7B56FE30
	/// @DnDParent : 39D3C0E4
	/// @DnDArgument : "room" "Victory"
	/// @DnDSaveInfo : "room" "Victory"
	room_goto(Victory);
}

/// @DnDAction : YoYo Games.Common.If_Expression
/// @DnDVersion : 1
/// @DnDHash : 7E2119BB
/// @DnDArgument : "expr" "player.player_health<=0"
if(player.player_health<=0)
{
	/// @DnDAction : YoYo Games.Rooms.Go_To_Room
	/// @DnDVersion : 1
	/// @DnDHash : 74149AE1
	/// @DnDParent : 7E2119BB
	/// @DnDArgument : "room" "Fail"
	/// @DnDSaveInfo : "room" "Fail"
	room_goto(Fail);
}

/// @DnDAction : YoYo Games.Common.If_Expression
/// @DnDVersion : 1
/// @DnDHash : 14B0825B
/// @DnDArgument : "expr" "current_time-start_time>10000"
if(current_time-start_time>10000)
{
	/// @DnDAction : YoYo Games.Rooms.Go_To_Room
	/// @DnDVersion : 1
	/// @DnDHash : 04E17122
	/// @DnDParent : 14B0825B
	/// @DnDArgument : "room" "Victory"
	/// @DnDSaveInfo : "room" "Victory"
	room_goto(Victory);
}