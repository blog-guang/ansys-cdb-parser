/**
 * @file test_elements.cpp
 * @brief Element-related tests
 */

#include <gtest/gtest.h>
#include "element.h"
#include "archive.h"

using namespace cdb;

// Test Element construction
TEST(ElementTest, Construction) {
    Element elem;
    EXPECT_EQ(elem.id, 0);
    EXPECT_EQ(elem.type, 0);
    EXPECT_EQ(elem.num_nodes(), 0);
    
    std::vector<int> nodes = {1, 2, 3, 4, 5, 6, 7, 8};
    Element elem2(1, 186, nodes);
    EXPECT_EQ(elem2.id, 1);
    EXPECT_EQ(elem2.type, 186);
    EXPECT_EQ(elem2.num_nodes(), 8);
}

// Test with actual HexBeam CDB file
TEST(ElementTest, HexBeamElements) {
    Archive archive("data/HexBeam.cdb");
    
    // From Python reference: 40 elements
    EXPECT_EQ(archive.num_elements(), 40);
    
    // Check element type
    const ElementType* et = archive.get_element_type(1);
    ASSERT_NE(et, nullptr);
    EXPECT_EQ(et->id, 1);
    EXPECT_EQ(et->name, "186");
}
