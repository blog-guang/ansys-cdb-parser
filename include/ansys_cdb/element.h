/**
 * @file element.h
 * @brief Element data structure definition
 */

#ifndef ANSYS_CDB_ELEMENT_H
#define ANSYS_CDB_ELEMENT_H

#include <vector>

namespace ansys_cdb {

/**
 * @brief Represents a finite element
 */
struct Element {
    int id;                      ///< Element ID
    int type;                    ///< Element type ID
    int material_id;             ///< Material ID
    int real_constant_id;        ///< Real constant set ID
    int section_id;              ///< Section ID
    int coordinate_system;       ///< Element coordinate system
    std::vector<int> node_ids;   ///< Node connectivity list
    
    /**
     * @brief Default constructor
     */
    Element() 
        : id(0), type(0), material_id(0), real_constant_id(0), 
          section_id(0), coordinate_system(0) {}
    
    /**
     * @brief Constructor with basic parameters
     * @param elem_id Element ID
     * @param elem_type Element type ID
     * @param nodes Node connectivity
     */
    Element(int elem_id, int elem_type, const std::vector<int>& nodes)
        : id(elem_id), type(elem_type), material_id(0), real_constant_id(0),
          section_id(0), coordinate_system(0), node_ids(nodes) {}
    
    /**
     * @brief Get number of nodes
     * @return Number of nodes in element
     */
    size_t num_nodes() const {
        return node_ids.size();
    }
};

} // namespace ansys_cdb

#endif // ANSYS_CDB_ELEMENT_H
