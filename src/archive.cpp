/**
 * @file archive.cpp
 * @brief Archive implementation
 */

#include "ansys_cdb/archive.h"
#include "ansys_cdb/parser.h"
#include <stdexcept>

namespace ansys_cdb {

Archive::Archive(const std::string& filename) 
    : filename_(filename) {
    if (!load(filename)) {
        throw std::runtime_error("Failed to load CDB file: " + filename);
    }
}

bool Archive::load(const std::string& filename) {
    clear();
    filename_ = filename;
    return Parser::parse(filename, *this);
}

const Node* Archive::get_node(int id) const {
    auto it = node_map_.find(id);
    if (it != node_map_.end()) {
        return &nodes_[it->second];
    }
    return nullptr;
}

const Element* Archive::get_element(int id) const {
    auto it = element_map_.find(id);
    if (it != element_map_.end()) {
        return &elements_[it->second];
    }
    return nullptr;
}

const Component* Archive::get_node_component(const std::string& name) const {
    auto it = node_components_.find(name);
    if (it != node_components_.end()) {
        return &it->second;
    }
    return nullptr;
}

const Component* Archive::get_element_component(const std::string& name) const {
    auto it = elem_components_.find(name);
    if (it != elem_components_.end()) {
        return &it->second;
    }
    return nullptr;
}

const ElementType* Archive::get_element_type(int id) const {
    auto it = element_types_.find(id);
    if (it != element_types_.end()) {
        return &it->second;
    }
    return nullptr;
}

double Archive::get_parameter(const std::string& name, double default_value) const {
    auto it = parameters_.find(name);
    if (it != parameters_.end()) {
        return it->second;
    }
    return default_value;
}

void Archive::clear() {
    nodes_.clear();
    node_map_.clear();
    elements_.clear();
    element_map_.clear();
    node_components_.clear();
    elem_components_.clear();
    element_types_.clear();
    real_constants_.clear();
    parameters_.clear();
}

} // namespace ansys_cdb
