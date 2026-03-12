/**
 * @file test_realconstants.cpp
 * @brief Real constant tests
 */

#include <gtest/gtest.h>
#include "ansys_cdb/types.h"
#include "ansys_cdb/archive.h"

using namespace ansys_cdb;

// Test RealConstant construction
TEST(RealConstantTest, Construction) {
    RealConstant rc;
    EXPECT_EQ(rc.id, 0);
    EXPECT_EQ(rc.values.size(), 0);
}

// Test RealConstant values
TEST(RealConstantTest, Values) {
    RealConstant rc;
    rc.id = 1;
    rc.values.push_back(1.0);
    rc.values.push_back(0.5669e-7);
    rc.values.push_back(0.0);
    
    EXPECT_EQ(rc.id, 1);
    EXPECT_EQ(rc.values.size(), 3);
    EXPECT_DOUBLE_EQ(rc.values[0], 1.0);
    EXPECT_DOUBLE_EQ(rc.values[1], 0.5669e-7);
    EXPECT_DOUBLE_EQ(rc.values[2], 0.0);
}

// Test with ErnoRadiation CDB file
TEST(RealConstantTest, ErnoRadiationFile) {
    Archive archive("data/ErnoRadiation.cdb");
    
    // Should have at least 1 real constant set
    EXPECT_GE(archive.num_real_constants(), 1);
    
    // Check real constant set 2 (from the file)
    const RealConstant* rc = archive.get_real_constant(2);
    ASSERT_NE(rc, nullptr);
    EXPECT_EQ(rc->id, 2);
    
    // Should have 6 values based on the file
    EXPECT_EQ(rc->values.size(), 6);
    
    // Check first value
    EXPECT_DOUBLE_EQ(rc->values[0], 1.0);
    
    // Check second value (0.566900000E-07)
    EXPECT_NEAR(rc->values[1], 0.5669e-7, 1e-12);
    
    // Remaining values should be 0.0
    for (size_t i = 2; i < rc->values.size(); ++i) {
        EXPECT_DOUBLE_EQ(rc->values[i], 0.0);
    }
}

// Test with HyperMesh CDB file (has 12 real constants)
TEST(RealConstantTest, HyperMeshFile) {
    Archive archive("data/hypermesh.cdb");
    
    // Should have at least 1 real constant set
    EXPECT_GE(archive.num_real_constants(), 1);
    
    // Check real constant set 1
    const RealConstant* rc = archive.get_real_constant(1);
    ASSERT_NE(rc, nullptr);
    EXPECT_EQ(rc->id, 1);
    
    // Should have 12 values based on the file
    EXPECT_EQ(rc->values.size(), 12);
    
    // First value should be 0.375
    EXPECT_DOUBLE_EQ(rc->values[0], 0.375);
    
    // Remaining values should be 0.0
    for (size_t i = 1; i < rc->values.size(); ++i) {
        EXPECT_DOUBLE_EQ(rc->values[i], 0.0);
    }
}
