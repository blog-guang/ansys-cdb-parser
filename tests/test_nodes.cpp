/**
 * @file test_nodes.cpp
 * @brief Node-related tests
 */

#include <gtest/gtest.h>
#include "ansys_cdb/node.h"
#include "ansys_cdb/archive.h"

using namespace ansys_cdb;

// Test Node construction
TEST(NodeTest, Construction) {
    Node node;
    EXPECT_EQ(node.id, 0);
    EXPECT_DOUBLE_EQ(node.x, 0.0);
    EXPECT_DOUBLE_EQ(node.y, 0.0);
    EXPECT_DOUBLE_EQ(node.z, 0.0);
    
    Node node2(1, 1.0, 2.0, 3.0);
    EXPECT_EQ(node2.id, 1);
    EXPECT_DOUBLE_EQ(node2.x, 1.0);
    EXPECT_DOUBLE_EQ(node2.y, 2.0);
    EXPECT_DOUBLE_EQ(node2.z, 3.0);
}

// Test Node coordinates
TEST(NodeTest, Coordinates) {
    Node node(1, 1.5, 2.5, 3.5);
    auto coords = node.coordinates();
    EXPECT_DOUBLE_EQ(coords[0], 1.5);
    EXPECT_DOUBLE_EQ(coords[1], 2.5);
    EXPECT_DOUBLE_EQ(coords[2], 3.5);
}

// Test with actual HexBeam CDB file
TEST(NodeTest, HexBeamNodes) {
    Archive archive("data/HexBeam.cdb");
    
    // From Python reference: 321 nodes
    EXPECT_EQ(archive.num_nodes(), 321);
    
    // Check first node (id=1, coords=[0,0,0])
    const Node* node1 = archive.get_node(1);
    ASSERT_NE(node1, nullptr);
    EXPECT_EQ(node1->id, 1);
    EXPECT_DOUBLE_EQ(node1->x, 0.0);
    EXPECT_DOUBLE_EQ(node1->y, 0.0);
    EXPECT_DOUBLE_EQ(node1->z, 0.0);
    
    // Check second node (id=2, coords=[1,0,0])
    const Node* node2 = archive.get_node(2);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node2->id, 2);
    EXPECT_DOUBLE_EQ(node2->x, 1.0);
    EXPECT_DOUBLE_EQ(node2->y, 0.0);
    EXPECT_DOUBLE_EQ(node2->z, 0.0);
}
