#pragma once
#include <string>
#include <functional>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <deque>

namespace FlatPhysics {

    class TimeAverager {
    public:
        using Clock = std::chrono::high_resolution_clock;
        using ms = std::chrono::duration<double, std::milli>;
        using sec = std::chrono::duration<double>;

        static constexpr double WINDOW_SECONDS = 0.5;

        struct Sample {
            Clock::time_point t;
            double ms;
        };

        struct AggregateStats {
            double sum_ms = 0.0;
            std::uint64_t count = 0;
        };

        // --- per-name sliding window (for live debugging, optional) ---
        static std::unordered_map<std::string, std::deque<Sample>>& GetWindowMap() {
            static std::unordered_map<std::string, std::deque<Sample>> m;
            return m;
        }

        // --- per-name global aggregates (for end-of-run stats) ---
        static std::unordered_map<std::string, AggregateStats>& GetAggregateMap() {
            static std::unordered_map<std::string, AggregateStats> m;
            return m;
        }

        // Control whether we print live sliding-window averages
        static bool& LivePrintEnabled() {
            static bool enabled = false; // turn off by default for MPI
            return enabled;
        }

        static void AddSample(const std::string& name, double elapsed_ms) {
            auto now = Clock::now();

            // 1) Update aggregate stats (for final averages)
            {
                auto& agg = GetAggregateMap()[name];
                agg.sum_ms += elapsed_ms;
                agg.count += 1;
            }

            // 2) Optional sliding-window behavior (for real-time debugging)
            if (LivePrintEnabled()) {
                auto& dq = GetWindowMap()[name];
                dq.push_back({ now, elapsed_ms });

                while (!dq.empty()) {
                    double age = sec(now - dq.front().t).count();
                    if (age > WINDOW_SECONDS) dq.pop_front();
                    else break;
                }

                double sum = 0.0;
                for (auto& s : dq) sum += s.ms;
                double avg = dq.empty() ? 0.0 : sum / dq.size();

                std::cout << name << " avg over last "
                    << WINDOW_SECONDS << "s = "
                    << avg << " ms\n";
            }
        }

        // Helper to query a single stat
        static bool GetAggregate(const std::string& name, double& avg_ms, std::uint64_t& count) {
            auto& map = GetAggregateMap();
            auto it = map.find(name);
            if (it == map.end() || it->second.count == 0) {
                avg_ms = 0.0;
                count = 0;
                return false;
            }
            count = it->second.count;
            avg_ms = it->second.sum_ms / static_cast<double>(it->second.count);
            return true;
        }

        // Helper to get all aggregates (for iteration at the end)
        static const std::unordered_map<std::string, AggregateStats>& GetAllAggregates() {
            return GetAggregateMap();
        }
    };


    template<typename Func>
    void MeasureTime(const std::string& name, Func&& func)
    {
        using namespace std::chrono;

        auto start = TimeAverager::Clock::now();
        func();
        auto end = TimeAverager::Clock::now();

        double ms = duration<double, std::milli>(end - start).count();
        TimeAverager::AddSample(name, ms);
    }

} 
