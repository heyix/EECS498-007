#pragma once
#include "Vector2.h"
#include <array>
#include <vector>
#include <memory>
#include <deque>
namespace FlatPhysics {
	class FlatBody;
	class FlatFixture;
	class FlatManifold;
	struct ContactPointsOld {
	public:
		ContactPointsOld(const Vector2& point1_, const Vector2& point2_)
			:point1(point1_), point2(point2_), points_num(2)
		{}
		ContactPointsOld(const Vector2& point1_)
			:point1(point1_), points_num(1)
		{}
		ContactPointsOld()
			:points_num(0)
		{}
	public:
		void SetPoint(const Vector2& point) {
			point1 = point;
			points_num = 1;
		}
		void SetPoints(const Vector2& point1_, const Vector2& point2_) {
			point1 = point1_;
			point2 = point2_;
			points_num = 2;
		}
		void ClearPoints() {
			points_num = 0;
		}
	public:
		Vector2 point1{ 0,0 };
		Vector2 point2{ 0,0 };
		int points_num = 0;
	};






	struct ContactPoint {
		Vector2 start;
		Vector2 end;
		float depth = 0;
		Vector2 normal;
	};






	template <typename T, std::size_t Capacity>
	class FixedSizeContainer {
	public:
		FixedSizeContainer() = default;

		FixedSizeContainer(std::initializer_list<T> init) {
			count_ = static_cast<int>(std::min(init.size(), Capacity));
			std::copy_n(init.begin(), count_, data_.begin());
		}

		bool Empty() const { return count_ == 0; }
		int  Size()  const { return count_; }
		constexpr std::size_t MaxSize() const { return Capacity; }

		void Clear() { count_ = 0; }

		bool Push_Back(const T& value) {
			if (count_ >= static_cast<int>(Capacity)) return false;
			data_[count_++] = value;
			return true;
		}

		T& operator[](int idx) { return data_[idx]; }
		const T& operator[](int idx) const { return data_[idx]; }

		T* begin() { return data_.data(); }
		T* end() { return data_.data() + count_; }
		const T* begin() const { return data_.data(); }
		const T* end()   const { return data_.data() + count_; }

	private:
		std::array<T, Capacity> data_{};
		int count_{ 0 };
	};

	struct FlatContactEdge {
		FlatBody* other{ nullptr };
		int contact_index{ -1 };
		FlatContactEdge* prev = nullptr;
		FlatContactEdge* next = nullptr;
		int pool_index = -1;
	};
	struct ContactEdgePool {
		std::deque<FlatContactEdge> edges;
		std::vector<int> free_list;
		FlatContactEdge* Allocate();
		void Free(FlatContactEdge* e);
		void Clear();
	};










	class IContactFilter {
	public:
		virtual ~IContactFilter() {}
		virtual bool ShouldCollide(const FlatFixture* a, const FlatFixture* b) = 0;
	};

	struct ContactPair {
		FlatFixture* fixture_a;
		FlatFixture* fixture_b;
	};












	class IContactListener {
	public:
		virtual ~IContactListener() {}
		virtual void BeginContact(FlatFixture* fixture_a, FlatFixture* fixture_b) {}
		virtual void EndContact(FlatFixture* fixture_a, FlatFixture* fixture_b) {}
		virtual void PreSolve(FlatFixture* fixture_a, FlatFixture* fixture_b, const FlatManifold& manifold) {}
		virtual void EndSolve(FlatFixture* fixture_a, FlatFixture* fixture_b, const FlatManifold& manifold) {}
	};
}