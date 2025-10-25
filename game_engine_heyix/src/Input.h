#pragma once
#include <unordered_map>
#include "SDL2/SDL.h"
#include <vector>
#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include "Vector2.h"
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
	static Vector2 GetMousePosition();
public:
	static bool Lua_Get_Key(const std::string& keycode);
	static bool Lua_Get_Key_Down(const std::string& keycode);
	static bool Lua_Get_Key_Up(const std::string& keycode);
	static glm::vec2 Lua_Get_Mouse_Position();
	static bool Lua_Get_Mouse_Button(int buttom_num);
	static bool Lua_Get_Mouse_Button_Down(int buttom_num);
	static bool Lua_Get_Mouse_Button_Up(int buttom_num);
	static float Lua_Get_Mouse_Scroll_Delta();
	static void Lua_Hide_Cursor();
	static void Lua_Show_Cursor();
private:
	static inline std::vector<Input_State> key_states;
	static inline std::vector<SDL_Scancode> just_became_up_keys;
	static inline std::vector<SDL_Scancode> just_became_down_keys;

	static inline std::unordered_map<Uint8, Input_State> mouse_button_states;
	static inline std::vector<Uint8> just_became_up_mouse_buttons;
	static inline std::vector<Uint8> just_became_down_mouse_buttons;
	static inline glm::vec2 mouse_position{ 0,0 };
	static inline float mouse_scroll_delta = 0;
private:
	const static inline std::unordered_map<std::string, SDL_Scancode> __keycode_to_scancode = {
		// Directional (arrow) Keys
		{"up", SDL_SCANCODE_UP},
		{"down", SDL_SCANCODE_DOWN},
		{"right", SDL_SCANCODE_RIGHT},
		{"left", SDL_SCANCODE_LEFT},

		// Misc Keys
		{"escape", SDL_SCANCODE_ESCAPE},

		// Modifier Keys
		{"lshift", SDL_SCANCODE_LSHIFT},
		{"rshift", SDL_SCANCODE_RSHIFT},
		{"lctrl", SDL_SCANCODE_LCTRL},
		{"rctrl", SDL_SCANCODE_RCTRL},
		{"lalt", SDL_SCANCODE_LALT},
		{"ralt", SDL_SCANCODE_RALT},

		// Editing Keys
		{"tab", SDL_SCANCODE_TAB},
		{"return", SDL_SCANCODE_RETURN},
		{"enter", SDL_SCANCODE_RETURN},
		{"backspace", SDL_SCANCODE_BACKSPACE},
		{"delete", SDL_SCANCODE_DELETE},
		{"insert", SDL_SCANCODE_INSERT},

		// Character Keys
		{"space", SDL_SCANCODE_SPACE},
		{"a", SDL_SCANCODE_A},
		{"b", SDL_SCANCODE_B},
		{"c", SDL_SCANCODE_C},
		{"d", SDL_SCANCODE_D},
		{"e", SDL_SCANCODE_E},
		{"f", SDL_SCANCODE_F},
		{"g", SDL_SCANCODE_G},
		{"h", SDL_SCANCODE_H},
		{"i", SDL_SCANCODE_I},
		{"j", SDL_SCANCODE_J},
		{"k", SDL_SCANCODE_K},
		{"l", SDL_SCANCODE_L},
		{"m", SDL_SCANCODE_M},
		{"n", SDL_SCANCODE_N},
		{"o", SDL_SCANCODE_O},
		{"p", SDL_SCANCODE_P},
		{"q", SDL_SCANCODE_Q},
		{"r", SDL_SCANCODE_R},
		{"s", SDL_SCANCODE_S},
		{"t", SDL_SCANCODE_T},
		{"u", SDL_SCANCODE_U},
		{"v", SDL_SCANCODE_V},
		{"w", SDL_SCANCODE_W},
		{"x", SDL_SCANCODE_X},
		{"y", SDL_SCANCODE_Y},
		{"z", SDL_SCANCODE_Z},
		{"0", SDL_SCANCODE_0},
		{"1", SDL_SCANCODE_1},
		{"2", SDL_SCANCODE_2},
		{"3", SDL_SCANCODE_3},
		{"4", SDL_SCANCODE_4},
		{"5", SDL_SCANCODE_5},
		{"6", SDL_SCANCODE_6},
		{"7", SDL_SCANCODE_7},
		{"8", SDL_SCANCODE_8},
		{"9", SDL_SCANCODE_9},
		{"/", SDL_SCANCODE_SLASH},
		{";", SDL_SCANCODE_SEMICOLON},
		{"=", SDL_SCANCODE_EQUALS},
		{"-", SDL_SCANCODE_MINUS},
		{".", SDL_SCANCODE_PERIOD},
		{",", SDL_SCANCODE_COMMA},
		{"[", SDL_SCANCODE_LEFTBRACKET},
		{"]", SDL_SCANCODE_RIGHTBRACKET},
		{"\\", SDL_SCANCODE_BACKSLASH},
		{"'", SDL_SCANCODE_APOSTROPHE}
	};
	const static inline std::unordered_map<int, Uint8> __buttom_num_to_button = {
		{1,SDL_BUTTON_LEFT},
		{2,SDL_BUTTON_MIDDLE },
		{3,SDL_BUTTON_RIGHT}
	};
};