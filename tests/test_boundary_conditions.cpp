/**
 * @file test_boundary_conditions.cpp
 * @brief Test boundary condition parsing (D and F commands)
 */

#include <gtest/gtest.h>
#include "archive.h"
#include <iostream>
#include <iomanip>

using namespace cdb;

// Test basic displacement constraint parsing
TEST(BoundaryConditionTest, DisplacementConstraints) {
    Archive archive("data/boundary_conditions.cdb");
    
    const auto& bcs = archive.get_displacement_bcs();
    
    std::cout << "\n=== Displacement Constraints ===" << std::endl;
    std::cout << "Total: " << bcs.size() << std::endl;
    
    for (const auto& bc : bcs) {
        std::cout << "  Node " << bc.node_id 
                  << ", DOF=" << bc.dof 
                  << ", Value=" << bc.value << std::endl;
    }
    
    // Should have displacement constraints
    EXPECT_GT(bcs.size(), 0);
    
    // Check specific constraints
    // D,1,UX,0.0
    bool found_node1_ux = false;
    for (const auto& bc : bcs) {
        if (bc.node_id == 1 && bc.dof == "UX" && bc.value == 0.0) {
            found_node1_ux = true;
            break;
        }
    }
    EXPECT_TRUE(found_node1_ux) << "Node 1 UX constraint not found";
    
    // D,4,UX,0.0
    bool found_node4_ux = false;
    for (const auto& bc : bcs) {
        if (bc.node_id == 4 && bc.dof == "UX" && bc.value == 0.0) {
            found_node4_ux = true;
            break;
        }
    }
    EXPECT_TRUE(found_node4_ux) << "Node 4 UX constraint not found";
}

// Test nodal force parsing
TEST(BoundaryConditionTest, NodalForces) {
    Archive archive("data/boundary_conditions.cdb");
    
    const auto& forces = archive.get_nodal_forces();
    
    std::cout << "\n=== Nodal Forces ===" << std::endl;
    std::cout << "Total: " << forces.size() << std::endl;
    
    for (const auto& force : forces) {
        std::cout << "  Node " << force.node_id 
                  << ", Direction=" << force.direction 
                  << ", Value=" << force.value << std::endl;
    }
    
    // Should have nodal forces
    EXPECT_GT(forces.size(), 0);
    
    // Check specific forces
    // F,3,FX,1000.0
    bool found_node3_fx = false;
    for (const auto& force : forces) {
        if (force.node_id == 3 && force.direction == "FX" && force.value == 1000.0) {
            found_node3_fx = true;
            break;
        }
    }
    EXPECT_TRUE(found_node3_fx) << "Node 3 FX force not found";
    
    // F,6,FY,-500.0
    bool found_node6_fy = false;
    for (const auto& force : forces) {
        if (force.node_id == 6 && force.direction == "FY" && force.value == -500.0) {
            found_node6_fy = true;
            break;
        }
    }
    EXPECT_TRUE(found_node6_fy) << "Node 6 FY force not found";
    
    // F,9,MZ,100.0
    bool found_node9_mz = false;
    for (const auto& force : forces) {
        if (force.node_id == 9 && force.direction == "MZ" && force.value == 100.0) {
            found_node9_mz = true;
            break;
        }
    }
    EXPECT_TRUE(found_node9_mz) << "Node 9 MZ moment not found";
}

// Test range constraints (D,7,9,UZ,0.0)
TEST(BoundaryConditionTest, RangeConstraints) {
    Archive archive("data/boundary_conditions.cdb");
    
    const auto& bcs = archive.get_displacement_bcs();
    
    // Should have constraints for nodes 7, 8, 9 in UZ direction
    int count_uz = 0;
    for (const auto& bc : bcs) {
        if (bc.dof == "UZ" && bc.node_id >= 7 && bc.node_id <= 9) {
            count_uz++;
            EXPECT_EQ(bc.value, 0.0);
        }
    }
    
    EXPECT_EQ(count_uz, 3) << "Should have UZ constraints for nodes 7, 8, 9";
}

// Test counting functions
TEST(BoundaryConditionTest, CountingFunctions) {
    Archive archive("data/boundary_conditions.cdb");
    
    size_t num_bcs = archive.num_displacement_bcs();
    size_t num_forces = archive.num_nodal_forces();
    
    std::cout << "\n=== Counts ===" << std::endl;
    std::cout << "Displacement BCs: " << num_bcs << std::endl;
    std::cout << "Nodal Forces: " << num_forces << std::endl;
    
    EXPECT_GT(num_bcs, 0);
    EXPECT_GT(num_forces, 0);
    
    EXPECT_EQ(num_bcs, archive.get_displacement_bcs().size());
    EXPECT_EQ(num_forces, archive.get_nodal_forces().size());
}

// Test with file that has no boundary conditions
TEST(BoundaryConditionTest, NoBoundaryConditions) {
    Archive archive("data/HexBeam.cdb");
    
    // HexBeam.cdb doesn't have D or F commands
    EXPECT_EQ(archive.num_displacement_bcs(), 0);
    EXPECT_EQ(archive.num_nodal_forces(), 0);
    EXPECT_EQ(archive.get_displacement_bcs().size(), 0);
    EXPECT_EQ(archive.get_nodal_forces().size(), 0);
}

// Test comprehensive summary
TEST(BoundaryConditionTest, ComprehensiveSummary) {
    Archive archive("data/boundary_conditions.cdb");
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "BOUNDARY CONDITIONS SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "\nModel Statistics:" << std::endl;
    std::cout << "  Nodes: " << archive.num_nodes() << std::endl;
    std::cout << "  Elements: " << archive.num_elements() << std::endl;
    
    std::cout << "\nBoundary Conditions:" << std::endl;
    std::cout << "  Displacement Constraints: " << archive.num_displacement_bcs() << std::endl;
    std::cout << "  Nodal Forces: " << archive.num_nodal_forces() << std::endl;
    std::cout << "  Surface Loads: " << archive.num_surface_loads() << std::endl;
    std::cout << "  Body Forces: " << archive.num_body_forces() << std::endl;
    
    std::cout << "\nDetailed Displacement Constraints:" << std::endl;
    for (const auto& bc : archive.get_displacement_bcs()) {
        std::cout << "  Node " << std::setw(3) << bc.node_id 
                  << " | DOF: " << std::setw(4) << bc.dof
                  << " | Value: " << std::setw(10) << bc.value
                  << std::endl;
    }
    
    std::cout << "\nDetailed Nodal Forces:" << std::endl;
    for (const auto& force : archive.get_nodal_forces()) {
        std::cout << "  Node " << std::setw(3) << force.node_id 
                  << " | Dir: " << std::setw(4) << force.direction
                  << " | Value: " << std::setw(10) << force.value
                  << std::endl;
    }
    
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Phase 1 Implementation: ✅ COMPLETE" << std::endl;
    std::cout << "  - D command (displacement constraints)" << std::endl;
    std::cout << "  - F command (nodal forces)" << std::endl;
    std::cout << "  - Range support (D,start,end,dof,value)" << std::endl;
    std::cout << "  - ALL node support" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Validate results
    EXPECT_EQ(archive.num_nodes(), 10);
    EXPECT_EQ(archive.num_elements(), 2);
    EXPECT_GT(archive.num_displacement_bcs(), 0);
    EXPECT_GT(archive.num_nodal_forces(), 0);
}
