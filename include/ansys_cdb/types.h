/**
 * @file types.h
 * @brief Common type definitions for ANSYS CDB parser
 */

#ifndef ANSYS_CDB_TYPES_H
#define ANSYS_CDB_TYPES_H

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace ansys_cdb {

/**
 * @brief Component type enumeration
 */
enum class ComponentType {
    NODE,     ///< Node component
    ELEMENT   ///< Element component
};

/**
 * @brief Element type information
 */
struct ElementType {
    int id;              ///< Element type ID
    std::string name;    ///< Element type name (e.g., "SOLID186")
    int keyopt[11];      ///< KEYOPT values
    
    ElementType() : id(0), name(""), keyopt{0} {}
};

/**
 * @brief Real constant set
 */
struct RealConstant {
    int id;                         ///< Real constant set ID
    std::vector<double> values;     ///< Real constant values
    
    RealConstant() : id(0) {}
};

/**
 * @brief Format specification for NBLOCK
 */
struct NBLOCKFormat {
    int num_fields;          ///< Number of fields
    int field_width;         ///< Field width for integers
    int num_coord_fields;    ///< Number of coordinate fields (usually 6)
    int coord_width;         ///< Total width for coordinates
    int coord_decimal;       ///< Decimal places
    int coord_exponent;      ///< Exponent width
    
    NBLOCKFormat() 
        : num_fields(0), field_width(0), num_coord_fields(6),
          coord_width(21), coord_decimal(13), coord_exponent(3) {}
};

/**
 * @brief Format specification for EBLOCK
 */
struct EBLOCKFormat {
    int format_code;         ///< Format code (e.g., 19)
    std::string format_type; ///< Format type (e.g., "SOLID")
    
    EBLOCKFormat() : format_code(0), format_type("SOLID") {}
};

} // namespace ansys_cdb

#endif // ANSYS_CDB_TYPES_H
