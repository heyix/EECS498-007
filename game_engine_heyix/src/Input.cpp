#include "Input.h"

void Input::Init()
{
	for (int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++) {
		key_states[static_cast<SDL_Scancode>(code)] = Input_State_Up;
	}
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
