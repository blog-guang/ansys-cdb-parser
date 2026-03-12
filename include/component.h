/**
 * @file component.h
 * @brief Component (groups) data structure definition
 */

#ifndef CDB_COMPONENT_H
#define CDB_COMPONENT_H

#include "types.h"
#include <string>
#include <vector>

namespace cdb {

/**
 * @brief Represents a named component (group of nodes or elements)
 */
struct Component {
    std::string name;         ///< Component name
    ComponentType type;       ///< Component type (NODE or ELEMENT)
    std::vector<int> ids;     ///< Member IDs
    
    /**
     * @brief Default constructor
     */
    Component() : name(""), type(ComponentType::NODE) {}
    
    /**
     * @brief Constructor with parameters
     * @param comp_name Component name
     * @param comp_type Component type
     */
    Component(const std::string& comp_name, ComponentType comp_type)
        : name(comp_name), type(comp_type) {}
    
    /**
     * @brief Add an ID to the component
     * @param id ID to add
     */
    void add_id(int id) {
        ids.push_back(id);
    }
    
    /**
     * @brief Get number of members
     * @return Number of IDs in component
     */
    size_t size() const {
        return ids.size();
    }
};

} // namespace cdb

#endif // CDB_COMPONENT_H
