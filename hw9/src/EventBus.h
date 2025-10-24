#pragma once
#include <unordered_map>
#include <string>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <vector>
class EventBus {
private:
	using Subscription = std::pair<luabridge::LuaRef, luabridge::LuaRef>;
	static inline std::unordered_map < std::string, std::vector<Subscription> > registered_events;
	static inline std::vector<std::pair<std::string,Subscription>> pending_adding_subscription;
	static inline std::vector<std::pair<std::string,Subscription>> pending_removing_subscription;
public:
	static void Process_Subscription();
public:
	static void Lua_Publish(std::string event_type, luabridge::LuaRef event_object);
	static void Lua_Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef func);
	static void Lua_Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef func);
};