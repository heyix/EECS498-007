#pragma once
#include <unordered_map>
#include "SDL2/SDL.h"
#include <vector>
#include <iostream>
enum Input_State {
	Input_State_Up,
	Input_State_Down,
	Input_State_Just_Became_Down,
	Input_State_Just_Became_Up
};
class Input {
public:
	static void Init();
	static bool GetKey(SDL_Scancode keycode);
	static bool GetKeyDown(SDL_Scancode keycode);
	static bool GetKeyUp(SDL_Scancode keycode);
	static void ProcessEvent(const SDL_Event& e);
	static void LateUpdate();
	static bool GetMouseButton(Uint8 button);
	static bool GetMouseButtonDown(Uint8 button);
	static bool GetMouseButtonUp(Uint8 button);
private:
	static inline std::unordered_map<SDL_Scancode, Input_State> key_states;
	static inline std::vector<SDL_Scancode> just_became_up_keys;
	static inline std::vector<SDL_Scancode> just_became_down_keys;

	static inline std::unordered_map<Uint8, Input_State> mouse_button_states;
	static inline std::vector<Uint8> just_became_up_mouse_buttons;
	static inline std::vector<Uint8> just_became_down_mouse_buttons;
};