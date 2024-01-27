#include "Game1.h"

void Game1::start() {
	std::cout << game_start_message << std::endl;
	for (int i = 0; i < hardcoded_actors.size(); i++) {
		Actor& actor = hardcoded_actors[i];
		actor_layer[actor.position.y][actor.position.x].insert(i);
	}
}

void Game1::render()
{
	update_map();
	int left_index = player.position.x - 6;
	int right_index = player.position.x + 6;
	int up_index = player.position.y - 4;
	int down_index =player.position.y + 4;
	for (int i = up_index; i <= down_index; i++) {
		for (int j = left_index; j <= right_index; j++) {
			if (check_out_of_bound(i, j))frame_output << ' ';
			else frame_output << render_layer[i][j];
		}
		frame_output << std::endl;
	}
}

void Game1::update()
{
	//render first frame->check dialogue of rendered frame->start to make update for next frame
	check_dialogue();

	if (game_status == GameStatus_running) {
		input();
		update_actor();
	}
	check_game_status();
	cout_frame_output();
}



void Game1::input()
{
	frame_output << "health : " << player_health << ", score : " << score << std::endl;
	frame_output << "Please make a decision..." << std::endl;
	frame_output << "Your options are \"n\", \"e\", \"s\", \"w\", \"quit\"" << std::endl;
	cout_frame_output();
	std::cin >> user_input;
	if (user_input == "quit") {
		game_status = GameStatus_quit;
	}
}

void Game1::update_map()
{
	for (int i = 0; i < HARDCODED_MAP_HEIGHT; i++) {
		for (int j = 0; j < HARDCODED_MAP_WIDTH + 1; j++) {
			render_layer[i][j] = hardcoded_map[i][j];
		}
	}
	for (int i = 0; i < hardcoded_actors.size();i++) {
		Actor& actor = hardcoded_actors[i];
		render_layer[actor.position.y][actor.position.x] = actor.view;
	}

	render_layer[player.position.y][player.position.x] = player.view;
}

void Game1::update_actor()
{
	for (int i = 0; i < hardcoded_actors.size(); i++) {
		Actor& actor = hardcoded_actors[i];
		if (actor.velocity.x == 0 && actor.velocity.y == 0)continue;
		int target_x = actor.position.x + actor.velocity.x;
		int target_y = actor.position.y + actor.velocity.y;
		if (check_grid_accessible(target_y, target_x)) {
			move_actor(i, target_y, target_x);
		}
		else {
			actor.velocity.x *= -1;
			actor.velocity.y *= -1;
		}
	}


	if (user_input == "w" && check_grid_accessible(player.position.y, player.position.x - 1))player.position.x -= 1;
	if (user_input == "e" && check_grid_accessible(player.position.y, player.position.x + 1))player.position.x += 1;
	if (user_input == "n" && check_grid_accessible(player.position.y - 1, player.position.x))player.position.y -= 1;
	if (user_input == "s" && check_grid_accessible(player.position.y + 1, player.position.x))player.position.y += 1;
}

void Game1::check_dialogue()
{
	for (auto& actor:hardcoded_actors) {
		int diff_x = actor.position.x - player.position.x;
		int diff_y = actor.position.y - player.position.y;
		if (diff_x == 0 && diff_y == 0 && actor.contact_dialogue != "")	trigger_contact_dialogue(actor);
		else if (std::abs(diff_x) <= 1 && std::abs(diff_y) <= 1 && actor.nearby_dialogue!="") {
			trigger_nearby_dialogue(actor);
		}
	}
}

bool Game1::check_grid_accessible(int index_y, int index_x)
{
	if (check_out_of_bound(index_y, index_x));
	if (render_layer[index_y][index_x] == 'b')return false;
	if (actor_layer[index_y][index_x].size() != 0) {
		for (int index : actor_layer[index_y][index_x]) {
			Actor& actor = hardcoded_actors[index];
			if (actor.blocking == true)return false;
		}
	}
	return true;
}

void Game1::trigger_contact_dialogue(Actor& actor)
{
	frame_output << actor.contact_dialogue << std::endl;
	if (check_substring_exist(actor.contact_dialogue,special_dialogue[0])) {
		change_player_health(-1);
	}
	else if (check_substring_exist(actor.contact_dialogue, special_dialogue[1]) && !actor.triggered_score_up) {
		score += 1;
		actor.triggered_score_up = true;
	}
	else if (check_substring_exist(actor.contact_dialogue, special_dialogue[2])) {
		game_status = GameStatus_good_ending;
	}
	else if (check_substring_exist(actor.contact_dialogue, special_dialogue[3])) {
		game_status = GameStatus_bad_ending;
	}
}

void Game1::trigger_nearby_dialogue(Actor& actor)
{
	frame_output << actor.nearby_dialogue << std::endl;
	if (check_substring_exist(actor.nearby_dialogue, special_dialogue[0])) {
		change_player_health(-1);
	}
	else if (check_substring_exist(actor.nearby_dialogue, special_dialogue[1]) && !actor.triggered_score_up) {
		score += 1;
		actor.triggered_score_up = true;
	}
	else if (check_substring_exist(actor.nearby_dialogue, special_dialogue[2])) {
		game_status = GameStatus_good_ending;
	}
	else if (check_substring_exist(actor.nearby_dialogue, special_dialogue[3])) {
		game_status = GameStatus_bad_ending;
	}
}

bool Game1::check_substring_exist(std::string& origin_string, std::string& substring)
{
	return origin_string.find(substring) != std::string::npos;
}

void Game1::check_game_status()
{
	if (game_status != GameStatus_running) {
		if(game_status!=GameStatus_quit)frame_output << "health : " << player_health << ", score : " << score << std::endl;
		is_running = false;
	}	
	if (game_status == GameStatus_bad_ending)frame_output << game_over_bad_message;
	if (game_status == GameStatus_good_ending)frame_output << game_over_good_message;
	if (game_status == GameStatus_quit)frame_output << game_over_bad_message;;
}

void Game1::change_player_health(int change)
{
	player_health += change;
	if (player_health <= 0)game_status = GameStatus_bad_ending;
}

bool Game1::check_out_of_bound(int index_y, int index_x)
{
	return index_y < 0 || index_y >= HARDCODED_MAP_HEIGHT || index_x < 0 || index_x >= HARDCODED_MAP_WIDTH;
}

void Game1::move_actor(int actor_index, int target_y, int target_x)
{
	Actor& actor = hardcoded_actors[actor_index];
	actor_layer[actor.position.y][actor.position.x].erase(actor_index);
	actor_layer[target_y][target_x].insert(actor_index);
	actor.position.x = target_x;
	actor.position.y = target_y;
}

void Game1::cout_frame_output()
{
	std::cout << frame_output.str();
	frame_output.str("");
}

