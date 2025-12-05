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

        static std::unordered_map<std::string, std::deque<Sample>>& GetMap() {
            static std::unordered_map<std::string, std::deque<Sample>> m;
            return m;
        }

        static void AddSample(const std::string& name, double elapsed_ms) {
            auto& dq = GetMap()[name];
            auto now = Clock::now();

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
