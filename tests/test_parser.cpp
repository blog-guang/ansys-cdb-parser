/**
 * @file test_parser.cpp
 * @brief Parser basic tests
 */

#include <gtest/gtest.h>
#include "parser.h"
#include "archive.h"

using namespace cdb;

// Test empty file
TEST(ParserTest, EmptyFile) {
    // Create a temporary empty file
    std::ofstream temp("empty_test.cdb");
    temp.close();
    
    Archive archive;
    bool success = archive.load("empty_test.cdb");
    EXPECT_TRUE(success);
    EXPECT_EQ(archive.num_nodes(), 0);
    EXPECT_EQ(archive.num_elements(), 0);
    
    // Clean up
    std::remove("empty_test.cdb");
}

// Test parsing scientific notation
TEST(ParserTest, ParseScientific) {
    EXPECT_DOUBLE_EQ(Parser::parse_scientific(" 1.0000000000000E+000"), 1.0);
    EXPECT_DOUBLE_EQ(Parser::parse_scientific(" 2.5000000000000E-001"), 0.25);
    EXPECT_DOUBLE_EQ(Parser::parse_scientific(" 1.0000000000000E+001"), 10.0);
    EXPECT_DOUBLE_EQ(Parser::parse_scientific("0.0000000000000E+000"), 0.0);
}

// Test string utilities
TEST(ParserTest, Trim) {
    EXPECT_EQ(Parser::trim("  hello  "), "hello");
    EXPECT_EQ(Parser::trim("hello"), "hello");
    EXPECT_EQ(Parser::trim("  "), "");
    EXPECT_EQ(Parser::trim(""), "");
}

TEST(ParserTest, ToUpper) {
    EXPECT_EQ(Parser::to_upper("nblock"), "NBLOCK");
    EXPECT_EQ(Parser::to_upper("NBLOCK"), "NBLOCK");
    EXPECT_EQ(Parser::to_upper("NbLoCk"), "NBLOCK");
}

// Test NBLOCK format parsing
TEST(ParserTest, ParseNBLOCKFormat) {
    NBLOCKFormat fmt = Parser::parse_nblock_format("(3i9,6e21.13e3)");
    EXPECT_EQ(fmt.num_fields, 3);
    EXPECT_EQ(fmt.field_width, 9);
    EXPECT_EQ(fmt.num_coord_fields, 6);
    EXPECT_EQ(fmt.coord_width, 21);
    EXPECT_EQ(fmt.coord_decimal, 13);
    EXPECT_EQ(fmt.coord_exponent, 3);
}
