#include "Input.h"

void Input::Init()
{
	key_states = std::vector<Input_State>(SDL_NUM_SCANCODES, Input_State_Up);
	for (int button = SDL_BUTTON_LEFT; button <= SDL_BUTTON_X2; button++) {
		mouse_button_states[static_cast<Uint8>(button)] = Input_State_Up;
	}
}

bool Input::GetKey(SDL_Scancode keycode)
{
	return key_states[keycode] == Input_State_Down || key_states[keycode] == Input_State_Just_Became_Down;
}

bool Input::GetKeyDown(SDL_Scancode keycode)
{
	return key_states[keycode] == Input_State_Just_Became_Down;
}

bool Input::GetKeyUp(SDL_Scancode keycode)
{
	return key_states[keycode] == Input_State_Just_Became_Up;
}

void Input::ProcessEvent(const SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN && key_states[e.key.keysym.scancode] != Input_State_Down) {
		key_states[e.key.keysym.scancode] = Input_State_Just_Became_Down;
		just_became_down_keys.push_back(e.key.keysym.scancode);
	}
	else if (e.type == SDL_KEYUP) {
		key_states[e.key.keysym.scancode] = Input_State_Just_Became_Up;
		just_became_up_keys.push_back(e.key.keysym.scancode);
	}

	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		mouse_button_states[e.button.button] = Input_State_Just_Became_Down;
		just_became_down_mouse_buttons.push_back(e.button.button);
	}
	else if (e.type == SDL_MOUSEBUTTONUP) {
		mouse_button_states[e.button.button] = Input_State_Just_Became_Up;
		just_became_up_mouse_buttons.push_back(e.button.button);
	}
	else if (e.type == SDL_MOUSEMOTION) {
		mouse_position = { static_cast<float>(e.motion.x),static_cast<float>(e.motion.y) };
	}
	else if (e.type == SDL_MOUSEWHEEL) {
		mouse_scroll_delta = e.wheel.preciseY;
	}
}

void Input::LateUpdate()
{
	for (auto& code : just_became_down_keys) {
		key_states[code] = Input_State_Down;
	}
	just_became_down_keys.clear();
	for (auto& code : just_became_up_keys) {
		key_states[code] = Input_State_Up;
	}
	just_became_up_keys.clear();
	for (auto& button : just_became_down_mouse_buttons) {
		mouse_button_states[button] = Input_State_Down;
	}
	just_became_down_mouse_buttons.clear();
	for (auto& button : just_became_up_mouse_buttons) {
		mouse_button_states[button] = Input_State_Up;
	}
	just_became_up_mouse_buttons.clear();
	mouse_scroll_delta = 0;
}

bool Input::GetMouseButton(Uint8 button)
{
	return mouse_button_states[button] == Input_State_Down || mouse_button_states[button] == Input_State_Just_Became_Down;
}

bool Input::GetMouseButtonDown(Uint8 button)
{
	return mouse_button_states[button] == Input_State_Just_Became_Down;
}

bool Input::GetMouseButtonUp(Uint8 button)
{
	return mouse_button_states[button] == Input_State_Just_Became_Up;
}

bool Input::Lua_Get_Key(const std::string& keycode)
{
	auto it = __keycode_to_scancode.find(keycode);
	if (it == __keycode_to_scancode.end()) {
		return false;
	}
	return Input::GetKey(it->second);
}

bool Input::Lua_Get_Key_Down(const std::string& keycode)
{
	auto it = __keycode_to_scancode.find(keycode);
	if (it == __keycode_to_scancode.end()) {
		return false;
	}
	return Input::GetKeyDown(it->second);
	
}

bool Input::Lua_Get_Key_Up(const std::string& keycode)
{
	auto it = __keycode_to_scancode.find(keycode);
	if (it == __keycode_to_scancode.end()) {
		return false;
	}
	return Input::GetKeyUp(it->second);
}

glm::vec2 Input::Lua_Get_Mouse_Position()
{
	return mouse_position;
}

bool Input::Lua_Get_Mouse_Button(int buttom_num)
{
	auto it = __buttom_num_to_button.find(buttom_num);
	if (it == __buttom_num_to_button.end()) {
		return false;
	}
	return Input::GetMouseButton(it->second);
}

bool Input::Lua_Get_Mouse_Button_Down(int buttom_num)
{
	auto it = __buttom_num_to_button.find(buttom_num);
	if (it == __buttom_num_to_button.end()) {
		return false;
	}
	return Input::GetMouseButtonDown(it->second);
}

bool Input::Lua_Get_Mouse_Button_Up(int buttom_num)
{
	auto it = __buttom_num_to_button.find(buttom_num);
	if (it == __buttom_num_to_button.end()) {
		return false;
	}
	return Input::GetMouseButtonUp(it->second);
}

float Input::Lua_Get_Mouse_Scroll_Delta()
{
	return mouse_scroll_delta;
}

void Input::Lua_Hide_Cursor()
{
	SDL_ShowCursor(SDL_DISABLE);
}

void Input::Lua_Show_Cursor()
{
	SDL_ShowCursor(SDL_ENABLE);
}
