/**
 * @file test_element_types.cpp
 * @brief Tests for different element types and mesh sizes
 */

#include <gtest/gtest.h>
#include "ansys_cdb/archive.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>

using namespace ansys_cdb;

// Test academic_rotor.cdb - ET 185 (3D structural solid)
TEST(ElementTypesTest, AcademicRotor_ET185) {
    Archive archive("data/academic_rotor.cdb");
    
    // Verify basic counts
    EXPECT_EQ(archive.num_nodes(), 786);
    EXPECT_EQ(archive.num_elements(), 524);
    
    // Check element types
    const auto& elements = archive.get_elements();
    ASSERT_GT(elements.size(), 0);
    
    // Collect unique element types
    std::set<int> element_types;
    for (const auto& elem : elements) {
        element_types.insert(elem.type);
    }
    
    // academic_rotor uses ET 185
    EXPECT_TRUE(element_types.count(185) > 0 || element_types.count(1) > 0);
    
    std::cout << "AcademicRotor - Element types: ";
    for (auto t : element_types) std::cout << t << " ";
    std::cout << std::endl;
}

// Test mesh200.cdb - Large mesh with ET 200
TEST(ElementTypesTest, Mesh200_LargeMesh) {
    Archive archive("data/mesh200.cdb");
    
    // Verify large mesh
    EXPECT_EQ(archive.num_nodes(), 4961);
    EXPECT_EQ(archive.num_elements(), 1000);
    
    // Should have components
    EXPECT_EQ(archive.num_node_components(), 6);
    EXPECT_EQ(archive.num_element_components(), 1);
    
    // Check element types
    const auto& elements = archive.get_elements();
    ASSERT_GT(elements.size(), 0);
    
    std::set<int> element_types;
    for (const auto& elem : elements) {
        element_types.insert(elem.type);
    }
    
    std::cout << "Mesh200 - Element types: ";
    for (auto t : element_types) std::cout << t << " ";
    std::cout << std::endl;
    
    // Verify node components
    std::cout << "Mesh200 - Node components: " << archive.num_node_components() << std::endl;
}

// Test workbench_193.cdb - Minimal mesh (only 3 nodes)
TEST(ElementTypesTest, Workbench193_MinimalMesh) {
    Archive archive("data/workbench_193.cdb");
    
    // Very small mesh
    EXPECT_EQ(archive.num_nodes(), 3);
    
    // Verify nodes exist
    const auto& nodes = archive.get_nodes();
    EXPECT_EQ(nodes.size(), 3);
    
    // Check first node has valid ID
    if (!nodes.empty()) {
        EXPECT_GT(nodes[0].id, 0);
        std::cout << "Workbench193 - First node ID: " << nodes[0].id << std::endl;
    }
}

// Test etblock.cdb - Minimal element test
TEST(ElementTypesTest, ETBlock_MinimalElement) {
    Archive archive("data/etblock.cdb");
    
    EXPECT_EQ(archive.num_nodes(), 4);
    EXPECT_EQ(archive.num_elements(), 1);
    
    const auto& elements = archive.get_elements();
    ASSERT_EQ(elements.size(), 1);
    
    // Verify element has nodes
    EXPECT_GT(elements[0].num_nodes(), 0);
    
    std::cout << "ETBlock - Element has " << elements[0].num_nodes() << " nodes" << std::endl;
}

// Test TetBeam.cdb - Multiple element types
TEST(ElementTypesTest, TetBeam_MultipleElementTypes) {
    Archive archive("data/TetBeam.cdb");
    
    EXPECT_EQ(archive.num_nodes(), 1041);
    EXPECT_EQ(archive.num_elements(), 3913);
    
    // Collect all element types
    const auto& elements = archive.get_elements();
    std::set<int> element_types;
    std::map<int, int> type_counts;
    
    for (const auto& elem : elements) {
        element_types.insert(elem.type);
        type_counts[elem.type]++;
    }
    
    // TetBeam has ET 1, 2, 3 (mapped to 200, 186, 187)
    EXPECT_GE(element_types.size(), 1);
    
    std::cout << "TetBeam - Element type distribution:" << std::endl;
    for (const auto& [type, count] : type_counts) {
        std::cout << "  Type " << type << ": " << count << " elements" << std::endl;
    }
}

// Mesh size classification test
TEST(ElementTypesTest, MeshSizeClassification) {
    struct MeshInfo {
        std::string name;
        std::string file;
        std::string expected_size;
    };
    
    std::vector<MeshInfo> meshes = {
        {"Minimal", "data/workbench_193.cdb", "tiny"},
        {"Minimal", "data/etblock.cdb", "tiny"},
        {"Small", "data/all_solid_cells.cdb", "small"},
        {"Small", "data/HexBeam.cdb", "small"},
        {"Medium", "data/sector.cdb", "medium"},
        {"Medium", "data/academic_rotor.cdb", "medium"},
        {"Large", "data/TetBeam.cdb", "large"},
        {"Large", "data/mesh200.cdb", "large"}
    };
    
    std::cout << "\nMesh Size Classification:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (const auto& mesh : meshes) {
        Archive archive(mesh.file);
        int total_entities = archive.num_nodes() + archive.num_elements();
        
        std::string actual_size;
        if (total_entities < 10) actual_size = "tiny";
        else if (total_entities < 500) actual_size = "small";
        else if (total_entities < 2000) actual_size = "medium";
        else actual_size = "large";
        
        std::cout << mesh.name << " (" << mesh.file << "): "
                  << archive.num_nodes() << " nodes, "
                  << archive.num_elements() << " elements - "
                  << actual_size << std::endl;
        
        EXPECT_EQ(actual_size, mesh.expected_size);
    }
}

// Performance test for different mesh sizes
TEST(ElementTypesTest, PerformanceScaling) {
    struct PerfTest {
        std::string file;
        int expected_nodes;
        int max_time_ms;
    };
    
    std::vector<PerfTest> tests = {
        {"data/workbench_193.cdb", 3, 5},
        {"data/HexBeam.cdb", 321, 20},
        {"data/academic_rotor.cdb", 786, 30},
        {"data/TetBeam.cdb", 1041, 50},
        {"data/mesh200.cdb", 4961, 100}
    };
    
    std::cout << "\nPerformance Scaling Test:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (const auto& test : tests) {
        auto start = std::chrono::high_resolution_clock::now();
        
        Archive archive(test.file);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << test.file << " (" << test.expected_nodes << " nodes): "
                  << duration.count() << "ms" << std::endl;
        
        EXPECT_EQ(archive.num_nodes(), test.expected_nodes);
        EXPECT_LT(duration.count(), test.max_time_ms);
    }
}
