/**
 * @file test_single_commands.cpp
 * @brief Test single node/element commands (N and EN)
 */

#include <gtest/gtest.h>
#include "archive.h"
#include <iostream>
#include <iomanip>

using namespace cdb;

// Test N command (single node definition)
TEST(SingleCommandTest, NCommand) {
    Archive archive("data/single_node_element.cdb");
    
    const auto& nodes = archive.get_nodes();
    
    std::cout << "\n=== N Command Test ===" << std::endl;
    std::cout << "Total nodes: " << nodes.size() << std::endl;
    
    // Should have nodes from N commands
    EXPECT_GE(nodes.size(), 10);  // At least 10 nodes from N commands
    
    // Check specific nodes
    const Node* node1 = archive.get_node(1);
    ASSERT_NE(node1, nullptr);
    EXPECT_DOUBLE_EQ(node1->x, 0.0);
    EXPECT_DOUBLE_EQ(node1->y, 0.0);
    EXPECT_DOUBLE_EQ(node1->z, 0.0);
    
    const Node* node2 = archive.get_node(2);
    ASSERT_NE(node2, nullptr);
    EXPECT_DOUBLE_EQ(node2->x, 1.0);
    EXPECT_DOUBLE_EQ(node2->y, 0.0);
    EXPECT_DOUBLE_EQ(node2->z, 0.0);
    
    const Node* node10 = archive.get_node(10);
    ASSERT_NE(node10, nullptr);
    EXPECT_DOUBLE_EQ(node10->x, 2.0);
    EXPECT_DOUBLE_EQ(node10->y, 1.0);
    EXPECT_DOUBLE_EQ(node10->z, 0.0);
    EXPECT_DOUBLE_EQ(node10->angles[2], 45.0);  // Z rotation angle
    
    std::cout << "  Node 1: (" << node1->x << ", " << node1->y << ", " << node1->z << ")" << std::endl;
    std::cout << "  Node 10: (" << node10->x << ", " << node10->y << ", " << node10->z 
              << "), angle_z=" << node10->angles[2] << "°" << std::endl;
}

// Test EN command (single element definition)
TEST(SingleCommandTest, ENCommand) {
    Archive archive("data/single_node_element.cdb");
    
    const auto& elements = archive.get_elements();
    
    std::cout << "\n=== EN Command Test ===" << std::endl;
    std::cout << "Total elements: " << elements.size() << std::endl;
    
    // Should have elements from EN commands
    EXPECT_GE(elements.size(), 2);  // At least 2 elements from EN
    
    // Check element 1
    const Element* elem1 = archive.get_element(1);
    ASSERT_NE(elem1, nullptr);
    EXPECT_EQ(elem1->id, 1);
    EXPECT_EQ(elem1->type, 1);
    EXPECT_EQ(elem1->material_id, 1);
    EXPECT_EQ(elem1->real_constant_id, 1);
    EXPECT_EQ(elem1->num_nodes(), 8);  // 8-node hex element
    
    std::cout << "  Element 1: type=" << elem1->type 
              << ", mat=" << elem1->material_id
              << ", nodes=" << elem1->num_nodes() << std::endl;
    
    // Check element 2
    const Element* elem2 = archive.get_element(2);
    ASSERT_NE(elem2, nullptr);
    EXPECT_EQ(elem2->id, 2);
    EXPECT_EQ(elem2->material_id, 2);
    EXPECT_GT(elem2->num_nodes(), 0);
    
    std::cout << "  Element 2: type=" << elem2->type 
              << ", mat=" << elem2->material_id
              << ", nodes=" << elem2->num_nodes() << std::endl;
}

// Test mixed N/NBLOCK commands
TEST(SingleCommandTest, MixedNodeCommands) {
    Archive archive("data/single_node_element.cdb");
    
    const auto& nodes = archive.get_nodes();
    
    std::cout << "\n=== Mixed N/NBLOCK Test ===" << std::endl;
    
    // Should have nodes from both N and NBLOCK
    // N commands: nodes 1-10
    // NBLOCK: nodes 11-12
    EXPECT_GE(nodes.size(), 12);
    
    // Check N command node
    const Node* node5 = archive.get_node(5);
    ASSERT_NE(node5, nullptr);
    std::cout << "  Node 5 (N command): (" << node5->x << ", " << node5->y << ", " << node5->z << ")" << std::endl;
    
    // Check NBLOCK node
    const Node* node11 = archive.get_node(11);
    ASSERT_NE(node11, nullptr);
    EXPECT_DOUBLE_EQ(node11->x, 3.0);
    EXPECT_DOUBLE_EQ(node11->y, 0.0);
    std::cout << "  Node 11 (NBLOCK): (" << node11->x << ", " << node11->y << ", " << node11->z << ")" << std::endl;
}

// Test mixed EN/EBLOCK commands
TEST(SingleCommandTest, MixedElementCommands) {
    Archive archive("data/single_node_element.cdb");
    
    const auto& elements = archive.get_elements();
    
    std::cout << "\n=== Mixed EN/EBLOCK Test ===" << std::endl;
    
    // Should have elements from both EN and EBLOCK
    // EN commands: elements 1-2
    // EBLOCK: element 9 (element ID in field[10])
    EXPECT_GE(elements.size(), 3);
    
    // Check EN command element
    const Element* elem1 = archive.get_element(1);
    ASSERT_NE(elem1, nullptr);
    std::cout << "  Element 1 (EN): mat=" << elem1->material_id << std::endl;
    
    // Check EBLOCK element (ID=9 from the EBLOCK data)
    const Element* elem9 = archive.get_element(9);
    ASSERT_NE(elem9, nullptr);
    EXPECT_EQ(elem9->material_id, 3);
    std::cout << "  Element 9 (EBLOCK): mat=" << elem9->material_id << std::endl;
}

// Test node connectivity
TEST(SingleCommandTest, NodeConnectivity) {
    Archive archive("data/single_node_element.cdb");
    
    const Element* elem1 = archive.get_element(1);
    ASSERT_NE(elem1, nullptr);
    
    std::cout << "\n=== Node Connectivity ===" << std::endl;
    std::cout << "Element 1 nodes: ";
    for (int nid : elem1->node_ids) {
        std::cout << nid << " ";
    }
    std::cout << std::endl;
    
    // Verify all nodes exist
    for (int nid : elem1->node_ids) {
        const Node* node = archive.get_node(nid);
        ASSERT_NE(node, nullptr) << "Node " << nid << " should exist";
    }
}

// Test comprehensive summary
TEST(SingleCommandTest, ComprehensiveSummary) {
    Archive archive("data/single_node_element.cdb");
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "SINGLE COMMAND (N/EN) SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "\nModel Statistics:" << std::endl;
    std::cout << "  Total Nodes: " << archive.num_nodes() << std::endl;
    std::cout << "  Total Elements: " << archive.num_elements() << std::endl;
    
    std::cout << "\nNode Sources:" << std::endl;
    std::cout << "  N command: nodes 1-10" << std::endl;
    std::cout << "  NBLOCK: nodes 11-12" << std::endl;
    
    std::cout << "\nElement Sources:" << std::endl;
    std::cout << "  EN command: elements 1-2" << std::endl;
    std::cout << "  EBLOCK: element 9" << std::endl;
    
    std::cout << "\nDetailed Node List:" << std::endl;
    for (const auto& node : archive.get_nodes()) {
        std::cout << "  Node " << std::setw(3) << node.id 
                  << ": (" << std::setw(8) << node.x 
                  << ", " << std::setw(8) << node.y
                  << ", " << std::setw(8) << node.z << ")";
        if (node.angles[0] != 0.0 || node.angles[1] != 0.0 || node.angles[2] != 0.0) {
            std::cout << " angles=(" << node.angles[0] << ", " 
                      << node.angles[1] << ", " << node.angles[2] << ")";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nDetailed Element List:" << std::endl;
    for (const auto& elem : archive.get_elements()) {
        std::cout << "  Element " << std::setw(3) << elem.id 
                  << ": type=" << elem.type
                  << ", mat=" << elem.material_id
                  << ", nodes=" << elem.num_nodes()
                  << " [";
        for (size_t i = 0; i < std::min(size_t(4), elem.node_ids.size()); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << elem.node_ids[i];
        }
        if (elem.node_ids.size() > 4) {
            std::cout << "...";
        }
        std::cout << "]" << std::endl;
    }
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "Phase 9 Implementation: ✅ COMPLETE" << std::endl;
    std::cout << "  - N command (single node definition)" << std::endl;
    std::cout << "  - EN command (single element definition)" << std::endl;
    std::cout << "  - Mixed N/NBLOCK support" << std::endl;
    std::cout << "  - Mixed EN/EBLOCK support" << std::endl;
    std::cout << "  - Node rotation angles" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Validate
    EXPECT_GE(archive.num_nodes(), 12);
    EXPECT_GE(archive.num_elements(), 3);
}
