#include "EventBus.h"

void EventBus::Process_Subscription()
{
	for (auto& p : pending_adding_subscription) {
		registered_events[p.first].push_back(p.second);
	}
	pending_adding_subscription.clear();
	for (auto& p : pending_removing_subscription) {
		auto& subs = registered_events[p.first];
		subs.erase(std::remove_if(subs.begin(), subs.end(), [&](const Subscription& sub) {
			return sub.first == p.second.first && sub.second == p.second.second;
			}), subs.end());

		if (subs.empty()) {
			registered_events.erase(p.first);
		}
	}
	pending_removing_subscription.clear();
}

void EventBus::Lua_Publish(std::string event_type, luabridge::LuaRef event_object)
{
	auto it = registered_events.find(event_type);
	if (it != registered_events.end()) {
		for (auto& [component, function] : it->second) {
			if (function.isFunction()) {
				function(component, event_object);
			}
		}
	}
}

void EventBus::Lua_Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef func)
{
	pending_adding_subscription.push_back({ event_type,{component,func} });
}

void EventBus::Lua_Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef func)
{
	pending_removing_subscription.push_back({ event_type,{component,func} });
}
