/**
 * @file test_additional_files.cpp
 * @brief Additional CDB file tests for broader validation
 */

#include <gtest/gtest.h>
#include "ansys_cdb/archive.h"
#include <chrono>
#include <vector>
#include <string>
#include <iostream>

using namespace ansys_cdb;

// Test TetBeam.cdb (tetrahedral elements)
TEST(AdditionalFilesTest, TetBeam) {
    Archive archive("data/TetBeam.cdb");
    
    // TetBeam should have nodes and elements
    EXPECT_GT(archive.num_nodes(), 0);
    EXPECT_GT(archive.num_elements(), 0);
    
    // Should be able to access first node
    const auto& nodes = archive.get_nodes();
    if (!nodes.empty()) {
        EXPECT_GE(nodes[0].id, 0);
    }
    
    // Should be able to access first element
    const auto& elements = archive.get_elements();
    if (!elements.empty()) {
        EXPECT_GT(elements[0].id, 0);
        EXPECT_GT(elements[0].num_nodes(), 0);
    }
}

// Test sector.cdb (sector geometry with components)
TEST(AdditionalFilesTest, Sector) {
    Archive archive("data/sector.cdb");
    
    // Sector should have nodes, elements, and at least 1 component
    EXPECT_GT(archive.num_nodes(), 0);
    EXPECT_GT(archive.num_elements(), 0);
    EXPECT_GE(archive.num_node_components() + archive.num_element_components(), 1);
    
    std::cout << "Sector - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << ", Components: " << (archive.num_node_components() + archive.num_element_components())
              << std::endl;
}

// Test all_solid_cells.cdb (various solid element types)
TEST(AdditionalFilesTest, AllSolidCells) {
    Archive archive("data/all_solid_cells.cdb");
    
    // Should have nodes and elements
    EXPECT_GT(archive.num_nodes(), 0);
    EXPECT_GT(archive.num_elements(), 0);
    
    // Should have multiple element types
    const auto& elements = archive.get_elements();
    if (!elements.empty()) {
        // Just verify we can access element type
        EXPECT_GE(elements[0].type, 0);
    }
    
    std::cout << "AllSolidCells - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << std::endl;
}

// Test parm.cdb (parameter-heavy file)
TEST(AdditionalFilesTest, Parameters) {
    Archive archive("data/parm.cdb");
    
    // parm.cdb may not have geometry, but should parse without error
    // Just verify it loads successfully
    EXPECT_GE(archive.num_nodes(), 0);
    EXPECT_GE(archive.num_elements(), 0);
    
    std::cout << "Parm - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements()
              << std::endl;
}

// Test file loading error handling
TEST(AdditionalFilesTest, NonExistentFile) {
    // Should throw exception for non-existent file
    EXPECT_THROW({
        Archive archive("data/nonexistent.cdb");
    }, std::runtime_error);
}

// Benchmark test - measure parsing time
TEST(AdditionalFilesTest, ParsingPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Archive archive("data/TetBeam.cdb");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "TetBeam.cdb parsing time: " << duration.count() << "ms" << std::endl;
    
    // Should parse in reasonable time (< 1 second for medium-sized file)
    EXPECT_LT(duration.count(), 1000);
}

// Integration test - verify all test files can be loaded
TEST(AdditionalFilesTest, AllFilesLoadable) {
    std::vector<std::string> test_files = {
        "data/HexBeam.cdb",
        "data/TetBeam.cdb",
        "data/sector.cdb",
        "data/all_solid_cells.cdb",
        "data/ErnoRadiation.cdb",
        "data/hypermesh.cdb"
    };
    
    int total_nodes = 0;
    int total_elements = 0;
    
    for (const auto& file : test_files) {
        Archive archive(file);
        total_nodes += archive.num_nodes();
        total_elements += archive.num_elements();
        
        std::cout << "Loaded " << file << ": "
                  << archive.num_nodes() << " nodes, "
                  << archive.num_elements() << " elements" << std::endl;
    }
    
    std::cout << "Total across all files: " 
              << total_nodes << " nodes, " 
              << total_elements << " elements" << std::endl;
    
    EXPECT_GT(total_nodes, 0);
    EXPECT_GT(total_elements, 0);
}
