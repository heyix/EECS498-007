#pragma once
#include "FlatDefs.h"
namespace FlatPhysics {
	class IPairCallback {
	public:
		virtual ~IPairCallback() = default;
		virtual void AddPair(void* user_data_a, void* user_data_b) = 0;
	};

	class IQueryCallback {
	public:
		virtual ~IQueryCallback() = default;
		virtual bool ReportProxy(ProxyID id, void* user_data) = 0;
	};

	class IRaycastCallback {
	public:
		virtual ~IRaycastCallback() = default;
		virtual float ReportRaycastHit(ProxyID* id, void* user_data, float fraction) = 0;
	};
}