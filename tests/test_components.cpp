/**
 * @file test_components.cpp
 * @brief Component (groups) tests
 */

#include <gtest/gtest.h>
#include "ansys_cdb/component.h"
#include "ansys_cdb/archive.h"

using namespace ansys_cdb;

// Test Component construction
TEST(ComponentTest, Construction) {
    Component comp;
    EXPECT_EQ(comp.name, "");
    EXPECT_EQ(comp.type, ComponentType::NODE);
    EXPECT_EQ(comp.size(), 0);
    
    Component comp2("TEST_COMP", ComponentType::ELEMENT);
    EXPECT_EQ(comp2.name, "TEST_COMP");
    EXPECT_EQ(comp2.type, ComponentType::ELEMENT);
    EXPECT_EQ(comp2.size(), 0);
}

// Test adding IDs to component
TEST(ComponentTest, AddIds) {
    Component comp("TEST", ComponentType::NODE);
    
    comp.add_id(1);
    comp.add_id(5);
    comp.add_id(10);
    
    EXPECT_EQ(comp.size(), 3);
    EXPECT_EQ(comp.ids[0], 1);
    EXPECT_EQ(comp.ids[1], 5);
    EXPECT_EQ(comp.ids[2], 10);
}

// Test with HexBeam CDB file
TEST(ComponentTest, HexBeamComponents) {
    Archive archive("data/HexBeam.cdb");
    
    // Should have 2 node components and 2 element components
    EXPECT_EQ(archive.num_node_components(), 2);
    EXPECT_EQ(archive.num_element_components(), 2);
    
    // Check ECOMP1 (element component)
    // IDs: 17 -18 21 -40
    // Expanded: 17-18 (2) + 21-40 (20) = 22 elements
    const Component* ecomp1 = archive.get_element_component("ECOMP1");
    ASSERT_NE(ecomp1, nullptr);
    EXPECT_EQ(ecomp1->name, "ECOMP1");
    EXPECT_EQ(ecomp1->type, ComponentType::ELEMENT);
    EXPECT_EQ(ecomp1->size(), 22);
    EXPECT_EQ(ecomp1->ids[0], 17);
    EXPECT_EQ(ecomp1->ids[1], 18);
    EXPECT_EQ(ecomp1->ids[2], 21);
    EXPECT_EQ(ecomp1->ids[21], 40);  // Last element
    
    // Check NCOMP2 (node component)
    // 42 ID entries, but expands to 98 actual nodes
    // Ranges: 1-21(21) + 43-47(5) + 62-64(3) + ... = 98 total
    const Component* ncomp2 = archive.get_node_component("NCOMP2");
    ASSERT_NE(ncomp2, nullptr);
    EXPECT_EQ(ncomp2->name, "NCOMP2");
    EXPECT_EQ(ncomp2->type, ComponentType::NODE);
    EXPECT_EQ(ncomp2->size(), 98);  // Total after range expansion
    EXPECT_EQ(ncomp2->ids[0], 1);   // First node
    EXPECT_EQ(ncomp2->ids[20], 21); // End of first range
    
    // Check NODE_SELECTION (node component)
    // 44 ID entries, expands to 164 actual nodes
    const Component* node_sel = archive.get_node_component("NODE_SELECTION");
    ASSERT_NE(node_sel, nullptr);
    EXPECT_EQ(node_sel->size(), 164);  // Total after range expansion
}
