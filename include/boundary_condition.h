/**
 * @file boundary_condition.h
 * @brief Boundary condition data structures
 */

#ifndef CDB_BOUNDARY_CONDITION_H
#define CDB_BOUNDARY_CONDITION_H

#include <string>
#include <vector>

namespace cdb {

/**
 * @brief Displacement constraint (D command)
 */
struct DisplacementBC {
    int node_id;            ///< Node ID
    std::string dof;        ///< Degree of freedom (UX, UY, UZ, ROTX, ROTY, ROTZ, etc.)
    double value;           ///< Constraint value
    
    DisplacementBC() : node_id(0), value(0.0) {}
    DisplacementBC(int nid, const std::string& d, double v) 
        : node_id(nid), dof(d), value(v) {}
};

/**
 * @brief Nodal force (F command)
 */
struct NodalForce {
    int node_id;            ///< Node ID
    std::string direction;  ///< Force direction (FX, FY, FZ, MX, MY, MZ, etc.)
    double value;           ///< Force magnitude
    
    NodalForce() : node_id(0), value(0.0) {}
    NodalForce(int nid, const std::string& dir, double v)
        : node_id(nid), direction(dir), value(v) {}
};

/**
 * @brief Surface load (SF command)
 */
struct SurfaceLoad {
    int element_id;         ///< Element ID
    int key;                ///< Load key (face number)
    std::string type;       ///< Load type (PRES, CONV, etc.)
    std::vector<double> values; ///< Load values
    
    SurfaceLoad() : element_id(0), key(0) {}
};

/**
 * @brief Body force (BF command)
 */
struct BodyForce {
    int node_id;            ///< Node ID
    std::string type;       ///< Body force type (TEMP, HGEN, etc.)
    double value;           ///< Force value
    
    BodyForce() : node_id(0), value(0.0) {}
    BodyForce(int nid, const std::string& t, double v)
        : node_id(nid), type(t), value(v) {}
};

} // namespace cdb

#endif // CDB_BOUNDARY_CONDITION_H
