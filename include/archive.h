/**
 * @file archive.h
 * @brief Main Archive class for CDB file parsing
 */

#ifndef CDB_ARCHIVE_H
#define CDB_ARCHIVE_H

#include "boundary_condition.h"
#include "component.h"
#include "element.h"
#include "node.h"
#include "types.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace cdb {

/**
 * @brief Main class for reading and storing ANSYS CDB archive data
 */
class Archive {
public:
    /**
     * @brief Default constructor
     */
    Archive() = default;
    
    /**
     * @brief Constructor that loads a CDB file
     * @param filename Path to CDB file
     */
    explicit Archive(const std::string& filename);
    
    /**
     * @brief Load a CDB file
     * @param filename Path to CDB file
     * @return true if successful, false otherwise
     */
    bool load(const std::string& filename);
    
    /**
     * @brief Reserve capacity for nodes and elements (performance optimization)
     * @param num_nodes Expected number of nodes
     * @param num_elements Expected number of elements
     */
    void reserve(size_t num_nodes, size_t num_elements);
    
    /**
     * @brief Get all nodes
     * @return Vector of nodes
     */
    const std::vector<Node>& get_nodes() const { return nodes_; }
    
    /**
     * @brief Get all elements
     * @return Vector of elements
     */
    const std::vector<Element>& get_elements() const { return elements_; }
    
    /**
     * @brief Get node by ID
     * @param id Node ID
     * @return Pointer to node, or nullptr if not found
     */
    const Node* get_node(int id) const;
    
    /**
     * @brief Get element by ID
     * @param id Element ID
     * @return Pointer to element, or nullptr if not found
     */
    const Element* get_element(int id) const;
    
    /**
     * @brief Get node component by name
     * @param name Component name
     * @return Pointer to component, or nullptr if not found
     */
    const Component* get_node_component(const std::string& name) const;
    
    /**
     * @brief Get element component by name
     * @param name Component name
     * @return Pointer to component, or nullptr if not found
     */
    const Component* get_element_component(const std::string& name) const;
    
    /**
     * @brief Get element type by ID
     * @param id Element type ID
     * @return Pointer to element type, or nullptr if not found
     */
    const ElementType* get_element_type(int id) const;
    
    /**
     * @brief Get parameter value by name
     * @param name Parameter name
     * @param default_value Default value if parameter not found
     * @return Parameter value or default_value
     */
    double get_parameter(const std::string& name, double default_value = 0.0) const;
    
    /**
     * @brief Get real constant set by ID
     * @param id Real constant set ID
     * @return Pointer to real constant set, or nullptr if not found
     */
    const RealConstant* get_real_constant(int id) const;
    
    /**
     * @brief Get number of nodes
     * @return Node count
     */
    size_t num_nodes() const { return nodes_.size(); }
    
    /**
     * @brief Get number of elements
     * @return Element count
     */
    size_t num_elements() const { return elements_.size(); }
    
    /**
     * @brief Get number of node components
     * @return Node component count
     */
    size_t num_node_components() const { return node_components_.size(); }
    
    /**
     * @brief Get number of element components
     * @return Element component count
     */
    size_t num_element_components() const { return elem_components_.size(); }
    
    /**
     * @brief Get number of real constant sets
     * @return Real constant set count
     */
    size_t num_real_constants() const { return real_constants_.size(); }
    
    /**
     * @brief Get all displacement boundary conditions
     * @return Vector of displacement BCs
     */
    const std::vector<DisplacementBC>& get_displacement_bcs() const { return displacement_bcs_; }
    
    /**
     * @brief Get all nodal forces
     * @return Vector of nodal forces
     */
    const std::vector<NodalForce>& get_nodal_forces() const { return nodal_forces_; }
    
    /**
     * @brief Get all surface loads
     * @return Vector of surface loads
     */
    const std::vector<SurfaceLoad>& get_surface_loads() const { return surface_loads_; }
    
    /**
     * @brief Get all body forces
     * @return Vector of body forces
     */
    const std::vector<BodyForce>& get_body_forces() const { return body_forces_; }
    
    /**
     * @brief Get number of displacement boundary conditions
     * @return Displacement BC count
     */
    size_t num_displacement_bcs() const { return displacement_bcs_.size(); }
    
    /**
     * @brief Get number of nodal forces
     * @return Nodal force count
     */
    size_t num_nodal_forces() const { return nodal_forces_.size(); }
    
    /**
     * @brief Get number of surface loads
     * @return Surface load count
     */
    size_t num_surface_loads() const { return surface_loads_.size(); }
    
    /**
     * @brief Get number of body forces
     * @return Body force count
     */
    size_t num_body_forces() const { return body_forces_.size(); }
    
    /**
     * @brief Clear all data
     */
    void clear();

private:
    std::vector<Node> nodes_;
    std::unordered_map<int, size_t> node_map_;  ///< ID -> index mapping
    
    std::vector<Element> elements_;
    std::unordered_map<int, size_t> element_map_;  ///< ID -> index mapping
    
    std::unordered_map<std::string, Component> node_components_;
    std::unordered_map<std::string, Component> elem_components_;
    
    std::unordered_map<int, ElementType> element_types_;
    std::unordered_map<int, RealConstant> real_constants_;
    std::unordered_map<std::string, double> parameters_;
    
    std::vector<DisplacementBC> displacement_bcs_;
    std::vector<NodalForce> nodal_forces_;
    std::vector<SurfaceLoad> surface_loads_;
    std::vector<BodyForce> body_forces_;
    
    std::string filename_;
    
    friend class Parser;
};

} // namespace cdb

#endif // CDB_ARCHIVE_H
