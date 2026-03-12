/**
 * @file node.h
 * @brief Node data structure definition
 */

#ifndef CDB_NODE_H
#define CDB_NODE_H

#include <array>
#include <vector>

namespace cdb {

/**
 * @brief Represents a finite element node
 */
struct Node {
    int id;                      ///< Node ID
    double x;                    ///< X coordinate
    double y;                    ///< Y coordinate
    double z;                    ///< Z coordinate
    std::array<double, 3> angles; ///< Rotation angles (optional)
    
    /**
     * @brief Default constructor
     */
    Node() : id(0), x(0.0), y(0.0), z(0.0), angles{0.0, 0.0, 0.0} {}
    
    /**
     * @brief Constructor with coordinates
     * @param node_id Node ID
     * @param x_coord X coordinate
     * @param y_coord Y coordinate
     * @param z_coord Z coordinate
     */
    Node(int node_id, double x_coord, double y_coord, double z_coord)
        : id(node_id), x(x_coord), y(y_coord), z(z_coord), angles{0.0, 0.0, 0.0} {}
    
    /**
     * @brief Get coordinates as array
     * @return Array of [x, y, z]
     */
    std::array<double, 3> coordinates() const {
        return {x, y, z};
    }
};

} // namespace cdb

#endif // CDB_NODE_H
