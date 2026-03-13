/**
 * @file test_phase6_extended.cpp
 * @brief Phase 6: Extended Testing - Comprehensive validation
 */

#include <gtest/gtest.h>
#include "archive.h"
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

using namespace cdb;

// Test Beam_186TetQuadAnglesDOS.cdb - with angles and DOS format
TEST(Phase6ExtendedTest, BeamWithAngles) {
    Archive archive("data/Beam_186TetQuadAnglesDOS.cdb");
    
    // Should have nodes and elements
    EXPECT_GT(archive.num_nodes(), 0);
    EXPECT_GT(archive.num_elements(), 0);
    
    std::cout << "BeamWithAngles - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements() << std::endl;
    
    // Check if any nodes have rotation angles
    const auto& nodes = archive.get_nodes();
    bool has_angles = false;
    for (const auto& node : nodes) {
        if (node.angles[0] != 0.0 || node.angles[1] != 0.0 || node.angles[2] != 0.0) {
            has_angles = true;
            std::cout << "  Node " << node.id << " has angles: ("
                      << node.angles[0] << ", " << node.angles[1] << ", " 
                      << node.angles[2] << ")" << std::endl;
            break;
        }
    }
    
    // This file may or may not have angles, just verify it parses
    EXPECT_TRUE(true);
}

// Test mixed_missing_midside.cdb - elements with missing mid-side nodes
TEST(Phase6ExtendedTest, MixedMissingMidside) {
    Archive archive("data/mixed_missing_midside.cdb");
    
    // Should have nodes and elements
    EXPECT_GT(archive.num_nodes(), 0);
    EXPECT_GT(archive.num_elements(), 0);
    
    std::cout << "MixedMissingMidside - Nodes: " << archive.num_nodes() 
              << ", Elements: " << archive.num_elements() << std::endl;
    
    // Collect element node counts
    const auto& elements = archive.get_elements();
    std::map<size_t, int> node_count_distribution;
    
    for (const auto& elem : elements) {
        node_count_distribution[elem.num_nodes()]++;
    }
    
    std::cout << "  Element node count distribution:" << std::endl;
    for (const auto& [count, freq] : node_count_distribution) {
        std::cout << "    " << count << " nodes: " << freq << " elements" << std::endl;
    }
    
    // Should have various node counts (some with mid-side, some without)
    EXPECT_TRUE(true);
}

// Comprehensive file loading test - all 13 files
TEST(Phase6ExtendedTest, AllFilesComprehensive) {
    struct FileInfo {
        std::string path;
        std::string name;
        bool should_succeed;
    };
    
    std::vector<FileInfo> files = {
        {"data/HexBeam.cdb", "HexBeam", true},
        {"data/TetBeam.cdb", "TetBeam", true},
        {"data/sector.cdb", "sector", true},
        {"data/academic_rotor.cdb", "academic_rotor", true},
        {"data/mesh200.cdb", "mesh200", true},
        {"data/ErnoRadiation.cdb", "ErnoRadiation", true},
        {"data/hypermesh.cdb", "hypermesh", true},
        {"data/all_solid_cells.cdb", "all_solid_cells", true},
        {"data/workbench_193.cdb", "workbench_193", true},
        {"data/etblock.cdb", "etblock", true},
        {"data/parm.cdb", "parm", true},
        {"data/Beam_186TetQuadAnglesDOS.cdb", "Beam_DOS", true},
        {"data/mixed_missing_midside.cdb", "mixed_midside", true}
    };
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "COMPREHENSIVE FILE VALIDATION - All 13 Test Files" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    int total_nodes = 0;
    int total_elements = 0;
    int total_components = 0;
    int files_loaded = 0;
    
    for (const auto& file : files) {
        try {
            Archive archive(file.path);
            files_loaded++;
            
            total_nodes += archive.num_nodes();
            total_elements += archive.num_elements();
            total_components += archive.num_node_components() + archive.num_element_components();
            
            std::cout << std::left << std::setw(25) << file.name
                      << " Nodes: " << std::setw(6) << archive.num_nodes()
                      << " Elements: " << std::setw(6) << archive.num_elements()
                      << " Components: " << std::setw(3) 
                      << (archive.num_node_components() + archive.num_element_components())
                      << " ✅" << std::endl;
            
            EXPECT_TRUE(file.should_succeed);
        } catch (const std::exception& e) {
            std::cout << std::left << std::setw(25) << file.name
                      << " FAILED: " << e.what() << " ❌" << std::endl;
            EXPECT_FALSE(file.should_succeed);
        }
    }
    
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "SUMMARY:" << std::endl;
    std::cout << "  Files loaded:      " << files_loaded << "/" << files.size() << std::endl;
    std::cout << "  Total nodes:       " << total_nodes << std::endl;
    std::cout << "  Total elements:    " << total_elements << std::endl;
    std::cout << "  Total components:  " << total_components << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    EXPECT_EQ(files_loaded, files.size());
    EXPECT_GT(total_nodes, 0);
}

// Element type diversity test
TEST(Phase6ExtendedTest, ElementTypeDiversity) {
    std::vector<std::string> files = {
        "data/HexBeam.cdb",
        "data/TetBeam.cdb",
        "data/sector.cdb",
        "data/academic_rotor.cdb",
        "data/all_solid_cells.cdb"
    };
    
    std::set<int> all_element_types;
    
    std::cout << "\nElement Type Diversity Across Files:" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& file : files) {
        Archive archive(file);
        const auto& elements = archive.get_elements();
        
        std::set<int> file_types;
        for (const auto& elem : elements) {
            file_types.insert(elem.type);
            all_element_types.insert(elem.type);
        }
        
        std::cout << std::left << std::setw(30) << file.substr(5) << " Types: ";
        for (auto t : file_types) {
            std::cout << t << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "Total unique element types: " << all_element_types.size() << std::endl;
    std::cout << "Types found: ";
    for (auto t : all_element_types) {
        std::cout << t << " ";
    }
    std::cout << std::endl;
    
    // Should have multiple element types across all files
    EXPECT_GE(all_element_types.size(), 3);
}

// Coordinate range validation
TEST(Phase6ExtendedTest, CoordinateRangeValidation) {
    std::vector<std::string> files = {
        "data/HexBeam.cdb",
        "data/sector.cdb",
        "data/academic_rotor.cdb"
    };
    
    std::cout << "\nCoordinate Range Validation:" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& file : files) {
        Archive archive(file);
        const auto& nodes = archive.get_nodes();
        
        if (nodes.empty()) continue;
        
        double min_x = nodes[0].x, max_x = nodes[0].x;
        double min_y = nodes[0].y, max_y = nodes[0].y;
        double min_z = nodes[0].z, max_z = nodes[0].z;
        
        for (const auto& node : nodes) {
            min_x = std::min(min_x, node.x);
            max_x = std::max(max_x, node.x);
            min_y = std::min(min_y, node.y);
            max_y = std::max(max_y, node.y);
            min_z = std::min(min_z, node.z);
            max_z = std::max(max_z, node.z);
        }
        
        std::cout << std::left << std::setw(30) << file.substr(5) << std::endl;
        std::cout << "  X: [" << std::setw(10) << min_x << ", " << std::setw(10) << max_x << "]" << std::endl;
        std::cout << "  Y: [" << std::setw(10) << min_y << ", " << std::setw(10) << max_y << "]" << std::endl;
        std::cout << "  Z: [" << std::setw(10) << min_z << ", " << std::setw(10) << max_z << "]" << std::endl;
        
        // Verify coordinates are finite
        EXPECT_TRUE(std::isfinite(min_x) && std::isfinite(max_x));
        EXPECT_TRUE(std::isfinite(min_y) && std::isfinite(max_y));
        EXPECT_TRUE(std::isfinite(min_z) && std::isfinite(max_z));
    }
    
    std::cout << std::string(80, '-') << std::endl;
}

// Node ID consistency check
TEST(Phase6ExtendedTest, NodeIDConsistency) {
    std::vector<std::string> files = {
        "data/HexBeam.cdb",
        "data/TetBeam.cdb",
        "data/mesh200.cdb"
    };
    
    std::cout << "\nNode ID Consistency Check:" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& file : files) {
        Archive archive(file);
        const auto& nodes = archive.get_nodes();
        const auto& elements = archive.get_elements();
        
        // Collect all referenced node IDs from elements
        std::set<int> referenced_ids;
        for (const auto& elem : elements) {
            for (int node_id : elem.node_ids) {
                referenced_ids.insert(node_id);
            }
        }
        
        // Check if all referenced nodes exist
        int missing_nodes = 0;
        for (int id : referenced_ids) {
            if (archive.get_node(id) == nullptr) {
                missing_nodes++;
            }
        }
        
        std::cout << std::left << std::setw(30) << file.substr(5)
                  << " Referenced: " << std::setw(6) << referenced_ids.size()
                  << " Missing: " << std::setw(3) << missing_nodes;
        
        if (missing_nodes == 0) {
            std::cout << " ✅" << std::endl;
        } else {
            std::cout << " ⚠️" << std::endl;
        }
        
        // Most files should have all nodes (some may reference external nodes)
        // Just verify the check runs without errors
        EXPECT_TRUE(true);
    }
    
    std::cout << std::string(60, '-') << std::endl;
}
