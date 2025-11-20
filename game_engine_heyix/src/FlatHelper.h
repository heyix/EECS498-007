#pragma once
#include <string>
#include <functional>
#include <chrono>
namespace FlatPhysics{
    template<typename Func>
    void MeasureTime(const std::string& name, Func&& func)
    {
        using namespace std::chrono;

        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();

        double ms = duration<double, std::milli>(end - start).count();
        std::cout << name << " took " << ms << " ms\n";
    }
}