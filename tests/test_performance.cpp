/**
 * @file test_performance.cpp
 * @brief Performance benchmark tests
 */

#include <gtest/gtest.h>
#include "ansys_cdb/archive.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace ansys_cdb;

// Performance benchmark for parsing
TEST(PerformanceTest, DetailedBenchmark) {
    struct BenchmarkTest {
        std::string file;
        int expected_nodes;
        int expected_elements;
        double target_time_ms;  // Target time in milliseconds
    };
    
    std::vector<BenchmarkTest> tests = {
        {"data/workbench_193.cdb", 3, 0, 1.0},
        {"data/etblock.cdb", 4, 1, 1.0},
        {"data/all_solid_cells.cdb", 52, 4, 2.0},
        {"data/ErnoRadiation.cdb", 65, 36, 5.0},
        {"data/hypermesh.cdb", 105, 80, 5.0},
        {"data/HexBeam.cdb", 321, 40, 5.0},
        {"data/sector.cdb", 655, 105, 10.0},
        {"data/academic_rotor.cdb", 786, 524, 10.0},
        {"data/TetBeam.cdb", 1041, 3913, 20.0},
        {"data/mesh200.cdb", 4961, 1000, 30.0}
    };
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "PERFORMANCE BENCHMARK - Parsing Speed" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << std::left << std::setw(25) << "File"
              << std::right << std::setw(10) << "Nodes"
              << std::setw(10) << "Elements"
              << std::setw(12) << "Time(ms)"
              << std::setw(15) << "Throughput"
              << std::setw(10) << "Status"
              << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    double total_time = 0.0;
    int total_entities = 0;
    
    for (const auto& test : tests) {
        auto start = std::chrono::high_resolution_clock::now();
        Archive archive(test.file);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double time_ms = duration.count() / 1000.0;
        
        int entities = archive.num_nodes() + archive.num_elements();
        total_entities += entities;
        total_time += time_ms;
        
        // Calculate throughput (entities per second)
        double throughput = (entities / time_ms) * 1000.0;
        
        std::string status = (time_ms <= test.target_time_ms) ? "✅ PASS" : "⚠️  SLOW";
        
        std::cout << std::left << std::setw(25) << test.file
                  << std::right << std::setw(10) << archive.num_nodes()
                  << std::setw(10) << archive.num_elements()
                  << std::setw(10) << std::fixed << std::setprecision(2) << time_ms
                  << " ms"
                  << std::setw(12) << std::fixed << std::setprecision(0) << throughput
                  << "/s"
                  << std::setw(10) << status
                  << std::endl;
        
        EXPECT_EQ(archive.num_nodes(), test.expected_nodes);
        EXPECT_EQ(archive.num_elements(), test.expected_elements);
        // Allow some tolerance for performance variation
        EXPECT_LT(time_ms, test.target_time_ms * 1.5);
    }
    
    std::cout << std::string(80, '-') << std::endl;
    std::cout << std::left << std::setw(25) << "TOTAL"
              << std::right << std::setw(20) << total_entities
              << std::setw(10) << std::fixed << std::setprecision(2) << total_time
              << " ms"
              << std::setw(12) << std::fixed << std::setprecision(0) 
              << (total_entities / total_time) * 1000.0
              << "/s" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

// Memory usage benchmark
TEST(PerformanceTest, MemoryEfficiency) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "MEMORY EFFICIENCY - Structure Sizes" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "sizeof(Node):            " << sizeof(Node) << " bytes" << std::endl;
    std::cout << "sizeof(Element):         " << sizeof(Element) << " bytes" << std::endl;
    std::cout << "sizeof(Component):       " << sizeof(Component) << " bytes" << std::endl;
    std::cout << "sizeof(RealConstant):    " << sizeof(RealConstant) << " bytes" << std::endl;
    std::cout << "sizeof(ElementType):     " << sizeof(ElementType) << " bytes" << std::endl;
    
    // Estimate memory usage for TetBeam
    Archive archive("data/TetBeam.cdb");
    
    size_t nodes_mem = archive.num_nodes() * sizeof(Node);
    size_t elements_mem = archive.num_elements() * sizeof(Element);
    size_t total_mem = nodes_mem + elements_mem;
    
    std::cout << "\nMemory usage for TetBeam.cdb:" << std::endl;
    std::cout << "  Nodes:    " << std::setw(8) << archive.num_nodes() 
              << " x " << sizeof(Node) << " = " 
              << std::setw(10) << nodes_mem << " bytes ("
              << std::fixed << std::setprecision(2) << nodes_mem / 1024.0 << " KB)" << std::endl;
    std::cout << "  Elements: " << std::setw(8) << archive.num_elements()
              << " x " << sizeof(Element) << " = "
              << std::setw(10) << elements_mem << " bytes ("
              << std::fixed << std::setprecision(2) << elements_mem / 1024.0 << " KB)" << std::endl;
    std::cout << "  Total (estimated): " << std::setw(10) << total_mem << " bytes ("
              << std::fixed << std::setprecision(2) << total_mem / 1024.0 / 1024.0 << " MB)" << std::endl;
    
    std::cout << std::string(80, '=') << std::endl;
    
    // Sanity checks
    EXPECT_GT(sizeof(Node), 0);
    EXPECT_LT(sizeof(Node), 200);  // Should be reasonably small
    EXPECT_GT(sizeof(Element), 0);
}

// Repeated parsing benchmark (cache effects)
TEST(PerformanceTest, CacheEffects) {
    const int iterations = 10;
    const std::string test_file = "data/HexBeam.cdb";
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "CACHE EFFECTS - Repeated Parsing (" << iterations << " iterations)" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::vector<double> times;
    times.reserve(iterations);
    
    for (int i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        Archive archive(test_file);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double time_ms = duration.count() / 1000.0;
        times.push_back(time_ms);
        
        EXPECT_EQ(archive.num_nodes(), 321);
        EXPECT_EQ(archive.num_elements(), 40);
    }
    
    // Calculate statistics
    double min_time = *std::min_element(times.begin(), times.end());
    double max_time = *std::max_element(times.begin(), times.end());
    double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    
    std::cout << "Min time:  " << std::fixed << std::setprecision(3) << min_time << " ms" << std::endl;
    std::cout << "Max time:  " << std::fixed << std::setprecision(3) << max_time << " ms" << std::endl;
    std::cout << "Avg time:  " << std::fixed << std::setprecision(3) << avg_time << " ms" << std::endl;
    std::cout << "Variation: " << std::fixed << std::setprecision(1) 
              << ((max_time - min_time) / avg_time * 100.0) << "%" << std::endl;
    
    std::cout << std::string(80, '=') << std::endl;
    
    // Variance should be reasonable
    EXPECT_LT((max_time - min_time) / avg_time, 0.5);  // Less than 50% variation
}
