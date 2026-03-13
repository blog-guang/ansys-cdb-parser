/**
 * @file material_property.h
 * @brief Material property data structures
 */

#ifndef CDB_MATERIAL_PROPERTY_H
#define CDB_MATERIAL_PROPERTY_H

#include <string>
#include <vector>
#include <unordered_map>

namespace cdb {

/**
 * @brief Material property data (MPDATA command)
 */
struct MaterialProperty {
    int material_id;              ///< Material ID
    std::string property_name;    ///< Property name (EX, PRXY, DENS, etc.)
    std::vector<double> values;   ///< Property values (can be temperature-dependent)
    std::vector<double> temps;    ///< Temperature values (from MPTEMP)
    
    MaterialProperty() : material_id(0) {}
    
    MaterialProperty(int mat_id, const std::string& name, double value)
        : material_id(mat_id), property_name(name) {
        values.push_back(value);
    }
    
    /**
     * @brief Get property value at specific temperature
     * @param temp Temperature
     * @return Property value (interpolated if needed)
     */
    double get_value(double temp = 0.0) const {
        if (values.empty()) return 0.0;
        if (temps.empty() || values.size() == 1) return values[0];
        
        // Linear interpolation for temperature-dependent properties
        for (size_t i = 0; i < temps.size() - 1; ++i) {
            if (temp >= temps[i] && temp <= temps[i + 1]) {
                double t1 = temps[i];
                double t2 = temps[i + 1];
                double v1 = values[i];
                double v2 = values[i + 1];
                return v1 + (v2 - v1) * (temp - t1) / (t2 - t1);
            }
        }
        
        // Outside range, return closest value
        if (temp < temps[0]) return values[0];
        return values.back();
    }
};

/**
 * @brief Material database
 */
class MaterialDatabase {
public:
    /**
     * @brief Add material property
     * @param prop Material property to add
     */
    void add_property(const MaterialProperty& prop) {
        properties_.push_back(prop);
    }
    
    /**
     * @brief Get all properties
     * @return Vector of all material properties
     */
    const std::vector<MaterialProperty>& get_properties() const {
        return properties_;
    }
    
    /**
     * @brief Get properties for specific material
     * @param material_id Material ID
     * @return Vector of properties for this material
     */
    std::vector<MaterialProperty> get_material_properties(int material_id) const {
        std::vector<MaterialProperty> result;
        for (const auto& prop : properties_) {
            if (prop.material_id == material_id) {
                result.push_back(prop);
            }
        }
        return result;
    }
    
    /**
     * @brief Get specific property value
     * @param material_id Material ID
     * @param property_name Property name (EX, PRXY, etc.)
     * @param temp Temperature (default 0.0)
     * @return Property value, or 0.0 if not found
     */
    double get_property_value(int material_id, const std::string& property_name, double temp = 0.0) const {
        for (const auto& prop : properties_) {
            if (prop.material_id == material_id && prop.property_name == property_name) {
                return prop.get_value(temp);
            }
        }
        return 0.0;
    }
    
    /**
     * @brief Get number of properties
     * @return Property count
     */
    size_t size() const {
        return properties_.size();
    }
    
    /**
     * @brief Clear all properties
     */
    void clear() {
        properties_.clear();
        temp_table_.clear();
    }
    
    /**
     * @brief Set temperature table (MPTEMP command)
     * @param temps Temperature values
     */
    void set_temp_table(const std::vector<double>& temps) {
        temp_table_ = temps;
    }
    
    /**
     * @brief Get current temperature table
     * @return Temperature table
     */
    const std::vector<double>& get_temp_table() const {
        return temp_table_;
    }

private:
    std::vector<MaterialProperty> properties_;
    std::vector<double> temp_table_;  ///< Current MPTEMP table
};

} // namespace cdb

#endif // CDB_MATERIAL_PROPERTY_H
