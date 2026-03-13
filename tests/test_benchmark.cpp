/**
 * @file test_benchmark.cpp
 * @brief Benchmark tests for large meshes
 */

#include <gtest/gtest.h>
#include "archive.h"
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace cdb;

// Benchmark test for medium mesh (10K nodes)
TEST(BenchmarkTest, Medium10K) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Archive archive("data/medium_10k.cdb");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Medium 10K - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << ", Time: " << duration.count() << "ms" << std::endl;
    
    EXPECT_EQ(archive.num_nodes(), 10000);
    EXPECT_GT(archive.num_elements(), 0);
    EXPECT_LT(duration.count(), 200);  // Should parse in < 200ms
}

// Benchmark test for medium mesh (50K nodes)
TEST(BenchmarkTest, Medium50K) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Archive archive("data/medium_50k.cdb");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Medium 50K - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << ", Time: " << duration.count() << "ms" << std::endl;
    
    EXPECT_EQ(archive.num_nodes(), 50000);
    EXPECT_GT(archive.num_elements(), 0);
    EXPECT_LT(duration.count(), 1000);  // Should parse in < 1s
}

// Benchmark test for large mesh (100K nodes)
TEST(BenchmarkTest, Large100K) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Archive archive("data/large_100k.cdb");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Large 100K - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << ", Time: " << duration.count() << "ms" << std::endl;
    
    EXPECT_EQ(archive.num_nodes(), 100000);
    EXPECT_GT(archive.num_elements(), 0);
    EXPECT_LT(duration.count(), 2000);  // Should parse in < 2s
}

// Benchmark test for large mesh (200K nodes)
TEST(BenchmarkTest, Large200K) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Archive archive("data/large_200k.cdb");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Large 200K - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << ", Time: " << duration.count() << "ms" << std::endl;
    
    EXPECT_EQ(archive.num_nodes(), 200000);
    EXPECT_GT(archive.num_elements(), 0);
    EXPECT_LT(duration.count(), 4000);  // Should parse in < 4s
}

// Benchmark test for very large mesh (500K nodes)
TEST(BenchmarkTest, VeryLarge500K) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Archive archive("data/benchmark_500k.cdb");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double throughput = (archive.num_nodes() + archive.num_elements()) / (duration.count() / 1000.0);
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "BENCHMARK: 500K Nodes Mesh" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Nodes:        " << archive.num_nodes() << std::endl;
    std::cout << "Elements:     " << archive.num_elements() << std::endl;
    std::cout << "Parse time:   " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput:   " << std::fixed << std::setprecision(0) 
              << throughput << " entities/sec" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    EXPECT_EQ(archive.num_nodes(), 500000);
    EXPECT_GT(archive.num_elements(), 0);
    EXPECT_LT(duration.count(), 10000);  // Should parse in < 10s
    
    // Verify data integrity
    const auto& nodes = archive.get_nodes();
    EXPECT_EQ(nodes.size(), 500000);
    
    // Check first and last nodes
    EXPECT_GT(nodes[0].id, 0);
    EXPECT_GT(nodes[nodes.size()-1].id, 0);
}

// Comprehensive benchmark summary
TEST(BenchmarkTest, ScalingSummary) {
    struct BenchmarkFile {
        std::string path;
        int expected_nodes;
        std::string label;
    };
    
    std::vector<BenchmarkFile> files = {
        {"data/medium_10k.cdb", 10000, "10K"},
        {"data/medium_50k.cdb", 50000, "50K"},
        {"data/large_100k.cdb", 100000, "100K"},
        {"data/large_200k.cdb", 200000, "200K"},
        {"data/benchmark_500k.cdb", 500000, "500K"}
    };
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "BENCHMARK SCALING ANALYSIS" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << std::left << std::setw(12) << "Mesh Size"
              << std::right << std::setw(12) << "Nodes"
              << std::setw(12) << "Elements"
              << std::setw(12) << "Time(ms)"
              << std::setw(16) << "Throughput"
              << std::setw(12) << "ms/1K nodes"
              << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& file : files) {
        auto start = std::chrono::high_resolution_clock::now();
        Archive archive(file.path);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        double throughput = (archive.num_nodes() + archive.num_elements()) / (time_ms / 1000.0);
        double ms_per_1k_nodes = time_ms / (archive.num_nodes() / 1000.0);
        
        std::cout << std::left << std::setw(12) << file.label
                  << std::right << std::setw(12) << archive.num_nodes()
                  << std::setw(12) << archive.num_elements()
                  << std::setw(12) << std::fixed << std::setprecision(0) << time_ms
                  << std::setw(16) << std::fixed << std::setprecision(0) << throughput << "/s"
                  << std::setw(12) << std::fixed << std::setprecision(2) << ms_per_1k_nodes
                  << std::endl;
        
        EXPECT_EQ(archive.num_nodes(), file.expected_nodes);
    }
    
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Analysis: Performance should scale linearly with mesh size" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}
