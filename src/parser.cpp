/**
 * @file parser.cpp
 * @brief CDB file parser implementation
 */

#include "ansys_cdb/parser.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>

namespace ansys_cdb {

bool Parser::parse(const std::string& filename, Archive& archive) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Trim and convert to uppercase for command matching
        std::string trimmed = trim(line);
        std::string upper = to_upper(trimmed);
        
        // Skip empty lines and comments
        if (trimmed.empty() || trimmed[0] == '!') {
            continue;
        }
        
        // Parse different command types
        if (upper.find("NBLOCK") == 0) {
            if (!parse_nblock(file, archive, trimmed)) {
                std::cerr << "Error parsing NBLOCK" << std::endl;
                return false;
            }
        }
        else if (upper.find("EBLOCK") == 0) {
            if (!parse_eblock(file, archive, trimmed)) {
                std::cerr << "Error parsing EBLOCK" << std::endl;
                return false;
            }
        }
        else if (upper.find("CMBLOCK") == 0) {
            if (!parse_cmblock(file, archive, trimmed)) {
                std::cerr << "Error parsing CMBLOCK" << std::endl;
                return false;
            }
        }
        else if (upper.find("ET,") == 0) {
            if (!parse_et(trimmed, archive)) {
                std::cerr << "Error parsing ET" << std::endl;
                return false;
            }
        }
        else if (upper.find("RLBLOCK") == 0) {
            if (!parse_rlblock(file, archive, trimmed)) {
                std::cerr << "Error parsing RLBLOCK" << std::endl;
                return false;
            }
        }
        else if (upper.find("*SET") == 0) {
            parse_set(trimmed, archive);  // Non-critical, don't fail on error
        }
    }
    
    return true;
}

bool Parser::parse_nblock(std::ifstream& file, Archive& archive, const std::string& line) {
    // Parse NBLOCK command line
    // Format: NBLOCK,<fields>,<format>,<numnp>,<numnp>
    // Example: NBLOCK,6,SOLID,       321,       321
    
    std::istringstream iss(line);
    std::string token;
    std::getline(iss, token, ',');  // Skip "NBLOCK"
    
    std::string format_line;
    if (!std::getline(file, format_line)) {
        return false;
    }
    
    // Parse format specification, e.g., "(3i9,6e21.13e3)"
    NBLOCKFormat format = parse_nblock_format(format_line);
    
    // Read node data until terminator (N,R... or -1)
    std::string data_line;
    while (std::getline(file, data_line)) {
        // DON'T trim the line - it's fixed-width format!
        // Only trim for terminator check
        std::string trimmed_check = trim(data_line);
        
        // Check for terminator (N,R5.3,LOC,       -1, or just -1)
        if (trimmed_check.empty()) {
            continue;
        }
        if (trimmed_check[0] == 'N' || trimmed_check[0] == 'n' || 
            trimmed_check.find("-1") == 0 || trimmed_check == "-1") {
            break;
        }
        
        // Parse node data from ORIGINAL line (not trimmed)
        // Format: node_id, solid, coord_sys, x, [y], [z], [angles...]
        // Coordinates are optional - missing ones default to 0
        
        Node node;
        node.x = 0.0;
        node.y = 0.0;
        node.z = 0.0;
        
        // Parse node ID and skip fields (first num_fields integers)
        size_t pos = 0;
        size_t int_width = static_cast<size_t>(format.field_width);
        size_t total_int_width = int_width * format.num_fields;
        
        if (data_line.length() < int_width) {
            continue;  // Skip malformed line
        }
        
        std::string id_str = data_line.substr(pos, int_width);
        try {
            node.id = std::stoi(trim(id_str));
        } catch (const std::exception& e) {
            continue;  // Skip unparseable line
        }
        
        // Move past all integer fields
        pos = total_int_width;
        
        // Extract remaining coordinate data
        // Some lines may have fewer coordinates than the full width specifies
        if (pos >= data_line.length()) {
            // No coordinate data, use defaults (0,0,0)
            size_t index = archive.nodes_.size();
            archive.nodes_.push_back(node);
            archive.node_map_[node.id] = index;
            continue;
        }
        
        // Split the coordinate section by whitespace
        std::string coord_section = data_line.substr(pos);
        std::istringstream coord_stream(coord_section);
        std::vector<double> coords;
        std::string value;
        
        while (coord_stream >> value) {
            try {
                double parsed = parse_scientific(value);
                coords.push_back(parsed);
            } catch (...) {
                break;  // Stop on parse error
            }
        }
        
        // Assign coordinates
        if (coords.size() > 0) node.x = coords[0];
        if (coords.size() > 1) node.y = coords[1];
        if (coords.size() > 2) node.z = coords[2];
        if (coords.size() > 3) node.angles[0] = coords[3];
        if (coords.size() > 4) node.angles[1] = coords[4];
        if (coords.size() > 5) node.angles[2] = coords[5];
        
        // Add node to archive
        size_t index = archive.nodes_.size();
        archive.nodes_.push_back(node);
        archive.node_map_[node.id] = index;
    }
    
    return true;
}

bool Parser::parse_eblock(std::ifstream& file, Archive& archive, const std::string& line) {
    (void)archive;  // TODO: Use in Phase 3
    (void)line;     // TODO: Use in Phase 3
    
    // TODO: Implement EBLOCK parsing in Phase 3
    // For now, just skip to the terminator
    std::string data_line;
    while (std::getline(file, data_line)) {
        if (trim(data_line).find("-1") == 0) {
            break;
        }
    }
    return true;
}

bool Parser::parse_cmblock(std::ifstream& file, Archive& archive, const std::string& line) {
    (void)archive;  // TODO: Use in Phase 4
    (void)line;     // TODO: Use in Phase 4
    
    // TODO: Implement CMBLOCK parsing in Phase 4
    std::string data_line;
    while (std::getline(file, data_line)) {
        if (trim(data_line).find("-1") == 0) {
            break;
        }
    }
    return true;
}

bool Parser::parse_et(const std::string& line, Archive& archive) {
    // Parse ET command: ET,<itype>,<ename>
    // Example: ET,        1,186
    
    std::istringstream iss(line);
    std::string token;
    
    std::getline(iss, token, ',');  // Skip "ET"
    
    ElementType et;
    
    if (std::getline(iss, token, ',')) {
        et.id = std::stoi(trim(token));
    }
    
    if (std::getline(iss, token, ',')) {
        et.name = trim(token);
    }
    
    archive.element_types_[et.id] = et;
    return true;
}

bool Parser::parse_rlblock(std::ifstream& file, Archive& archive, const std::string& line) {
    (void)archive;  // TODO: Use in Phase 5
    (void)line;     // TODO: Use in Phase 5
    
    // TODO: Implement RLBLOCK parsing in Phase 5
    std::string data_line;
    while (std::getline(file, data_line)) {
        if (trim(data_line).find("-1") == 0) {
            break;
        }
    }
    return true;
}

bool Parser::parse_set(const std::string& line, Archive& archive) {
    // Parse *SET command: *SET,<name>,<value>
    // Example: *SET,_BUTTON ,  1.000000000000
    
    std::istringstream iss(line);
    std::string token;
    
    std::getline(iss, token, ',');  // Skip "*SET"
    
    std::string name;
    if (std::getline(iss, token, ',')) {
        name = trim(token);
    }
    
    double value = 0.0;
    if (std::getline(iss, token, ',')) {
        try {
            value = std::stod(trim(token));
        } catch (...) {
            // Ignore parsing errors for parameters
            return false;
        }
    }
    
    if (!name.empty()) {
        archive.parameters_[name] = value;
    }
    
    return true;
}

NBLOCKFormat Parser::parse_nblock_format(const std::string& format_str) {
    // Parse format like "(3i9,6e21.13e3)"
    NBLOCKFormat format;
    
    // Match integer format: <num>i<width>
    std::regex int_pattern(R"((\d+)i(\d+))");
    std::smatch int_match;
    if (std::regex_search(format_str, int_match, int_pattern)) {
        format.num_fields = std::stoi(int_match[1]);
        format.field_width = std::stoi(int_match[2]);
    }
    
    // Match floating point format: <num>e<width>.<decimal>e<exponent>
    std::regex float_pattern(R"((\d+)e(\d+)\.(\d+)e(\d+))");
    std::smatch float_match;
    if (std::regex_search(format_str, float_match, float_pattern)) {
        format.num_coord_fields = std::stoi(float_match[1]);
        format.coord_width = std::stoi(float_match[2]);
        format.coord_decimal = std::stoi(float_match[3]);
        format.coord_exponent = std::stoi(float_match[4]);
    }
    
    return format;
}

double Parser::parse_scientific(const std::string& str) {
    std::string trimmed = trim(str);
    if (trimmed.empty()) {
        return 0.0;
    }
    
    try {
        // Handle FORTRAN-style scientific notation (e.g., "1.0E+00" or "1.0+00")
        // Replace 'D' with 'E' for double precision
        std::string normalized = trimmed;
        std::replace(normalized.begin(), normalized.end(), 'D', 'E');
        std::replace(normalized.begin(), normalized.end(), 'd', 'e');
        
        return std::stod(normalized);
    } catch (...) {
        std::cerr << "Warning: Failed to parse scientific notation: '" << str << "'" << std::endl;
        return 0.0;
    }
}

std::string Parser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string Parser::to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

} // namespace ansys_cdb
