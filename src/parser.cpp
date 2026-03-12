/**
 * @file parser.cpp
 * @brief CDB file parser implementation
 */

#include "parser.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>

namespace cdb {

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
    
    // Try to extract node count for pre-allocation
    std::vector<std::string> tokens;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    // Token[2] usually contains node count (numnp)
    if (tokens.size() >= 3) {
        try {
            size_t node_count = std::stoul(tokens[2]);
            if (node_count > 0) {
                archive.reserve(node_count, archive.num_elements());
            }
        } catch (...) {
            // Ignore parse errors, just skip pre-allocation
        }
    }
    
    std::string format_line;
    if (!std::getline(file, format_line)) {
        return false;
    }
    
    // Parse format specification, e.g., "(3i9,6e21.13e3)"
    NBLOCKFormat format = parse_nblock_format(format_line);
    
    // Read node data until terminator (N,R... or -1)
    std::string data_line;
    std::vector<double> coords;  // Reuse this vector
    coords.reserve(6);  // Max 6 values: x, y, z, angle1, angle2, angle3
    
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
        coords.clear();  // Clear instead of creating new vector
        std::string coord_section = data_line.substr(pos);
        std::istringstream coord_stream(coord_section);
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
    // Parse EBLOCK command line
    // Format: EBLOCK,<format_code>,<format_type>,<numelem>,<numelem>
    // Example: EBLOCK,19,SOLID,        40,        40
    
    // Try to extract element count for pre-allocation
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    // Token[3] usually contains element count
    if (tokens.size() >= 4) {
        try {
            size_t elem_count = std::stoul(tokens[3]);
            if (elem_count > 0) {
                archive.reserve(archive.num_nodes(), elem_count);
            }
        } catch (...) {
            // Ignore parse errors
        }
    }
    
    std::string format_line;
    if (!std::getline(file, format_line)) {
        return false;
    }
    
    // Parse format specification, e.g., "(19i10)"
    std::regex format_regex(R"(\((\d+)i(\d+)\))");
    std::smatch format_match;
    int num_fields = 19;  // Default
    int field_width = 10;  // Default
    
    if (std::regex_search(format_line, format_match, format_regex)) {
        num_fields = std::stoi(format_match[1]);
        field_width = std::stoi(format_match[2]);
    }
    
    // Read element data
    std::string data_line;
    bool reading_element = false;
    Element current_element;
    std::vector<int> fields;  // Reuse this vector instead of creating new one each iteration
    fields.reserve(static_cast<size_t>(num_fields));
    
    while (std::getline(file, data_line)) {
        std::string trimmed_check = trim(data_line);
        
        // Check for terminator
        if (trimmed_check.find("-1") == 0 || trimmed_check == "-1") {
            break;
        }
        
        if (trimmed_check.empty()) {
            continue;
        }
        
        // Parse fields from fixed-width line
        fields.clear();  // Clear instead of creating new vector
        for (int i = 0; i < num_fields && i * field_width < static_cast<int>(data_line.length()); ++i) {
            size_t pos = i * field_width;
            size_t width = std::min(static_cast<size_t>(field_width), data_line.length() - pos);
            std::string field_str = data_line.substr(pos, width);
            try {
                fields.push_back(std::stoi(trim(field_str)));
            } catch (...) {
                break;  // Stop on parse error
            }
        }
        
        if (fields.empty()) {
            continue;
        }
        
        // Check if this is the start of a new element
        // New element line has at least 11 fields and field[8] is the node count (reasonable value <= 30)
        bool is_new_element = false;
        if (fields.size() >= 11 && fields[8] > 0 && fields[8] <= 30) {
            is_new_element = true;
        }
        
        if (!reading_element || is_new_element) {
            // Save previous element if any
            if (reading_element && current_element.id != 0) {
                size_t index = archive.elements_.size();
                archive.elements_.push_back(current_element);
                archive.element_map_[current_element.id] = index;
            }
            
            // Start new element
            if (fields.size() < 11) {
                continue;  // Invalid element line
            }
            
            current_element = Element();
            current_element.material_id = fields[0];
            current_element.type = fields[1];
            current_element.real_constant_id = fields[2];
            current_element.section_id = fields[3];
            current_element.coordinate_system = fields[4];
            // fields[8] contains number of nodes (for validation)
            current_element.id = fields[10];  // Element ID
            
            // Collect node IDs starting from field 11
            for (size_t i = 11; i < fields.size(); ++i) {
                current_element.node_ids.push_back(fields[i]);
            }
            
            reading_element = true;
        } else {
            // Continuation line - just node IDs
            for (const auto& field : fields) {
                current_element.node_ids.push_back(field);
            }
        }
    }
    
    // Save last element
    if (reading_element && current_element.id != 0) {
        size_t index = archive.elements_.size();
        archive.elements_.push_back(current_element);
        archive.element_map_[current_element.id] = index;
    }
    
    return true;
}

bool Parser::parse_cmblock(std::ifstream& file, Archive& archive, const std::string& line) {
    // Parse CMBLOCK command line
    // Format: CMBLOCK,<name>,<type>,<count>
    // Example: CMBLOCK,ECOMP1  ,ELEM,       4
    
    std::istringstream iss(line);
    std::string token;
    std::getline(iss, token, ',');  // Skip "CMBLOCK"
    
    Component comp;
    
    // Parse component name
    if (std::getline(iss, token, ',')) {
        comp.name = trim(token);
    }
    
    // Parse component type (NODE or ELEM)
    if (std::getline(iss, token, ',')) {
        std::string type_str = trim(token);
        if (type_str == "NODE") {
            comp.type = ComponentType::NODE;
        } else if (type_str == "ELEM" || type_str == "ELEMENT") {
            comp.type = ComponentType::ELEMENT;
        } else {
            return false;  // Unknown type
        }
    }
    
    // Read format line (e.g., "(8i10)")
    std::string format_line;
    if (!std::getline(file, format_line)) {
        return false;
    }
    
    // Parse format to get field width
    std::regex format_regex(R"(\((\d+)i(\d+)\))");
    std::smatch format_match;
    int num_fields = 8;  // Default
    int field_width = 10;  // Default
    
    if (std::regex_search(format_line, format_match, format_regex)) {
        num_fields = std::stoi(format_match[1]);
        field_width = std::stoi(format_match[2]);
    }
    
    // Read component IDs
    std::string data_line;
    std::vector<int> raw_ids;
    
    // Store file position before reading each line
    std::streampos last_pos;
    
    while (true) {
        last_pos = file.tellg();
        if (!std::getline(file, data_line)) {
            break;
        }
        
        std::string trimmed_check = trim(data_line);
        
        // Check for terminator or next command
        if (trimmed_check.empty()) {
            continue;  // Skip empty lines
        }
        
        // If we hit a new command, restore position and exit
        std::string upper = to_upper(trimmed_check);
        if (upper.find("CMBLOCK") == 0 ||
            upper.find("MPTEMP") == 0 ||
            upper.find("MPDATA") == 0 ||
            upper.find("EXTOPT") == 0 ||
            upper.find("NBLOCK") == 0 ||
            upper.find("EBLOCK") == 0 ||
            upper.find("ET,") == 0) {
            // Restore file position so main parser can read this line
            file.seekg(last_pos);
            break;
        }
        
        // Parse IDs from fixed-width line
        for (int i = 0; i < num_fields && i * field_width < static_cast<int>(data_line.length()); ++i) {
            size_t pos = i * field_width;
            size_t width = std::min(static_cast<size_t>(field_width), data_line.length() - pos);
            std::string field_str = data_line.substr(pos, width);
            std::string trimmed_field = trim(field_str);
            
            if (!trimmed_field.empty()) {
                try {
                    raw_ids.push_back(std::stoi(trimmed_field));
                } catch (...) {
                    break;
                }
            }
        }
    }
    
    // Expand range notation (negative numbers indicate range end)
    size_t i = 0;
    while (i < raw_ids.size()) {
        int id = raw_ids[i];
        
        if (id > 0) {
            // Positive: start of range or single ID
            if (i + 1 < raw_ids.size() && raw_ids[i + 1] < 0) {
                // This is a range: id to -raw_ids[i+1]
                int range_end = -raw_ids[i + 1];
                for (int j = id; j <= range_end; ++j) {
                    comp.add_id(j);
                }
                i += 2;  // Skip both the start and the negative end marker
            } else {
                // Single ID
                comp.add_id(id);
                ++i;
            }
        } else {
            // Shouldn't happen (negative without preceding positive)
            ++i;
        }
    }
    
    // Store component in archive
    if (comp.type == ComponentType::NODE) {
        archive.node_components_[comp.name] = comp;
    } else {
        archive.elem_components_[comp.name] = comp;
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
    (void)line;  // Command line parameters are in format lines, not needed here
    
    // Parse RLBLOCK command line
    // Format: RLBLOCK,<set_id>,<num_values>,<fields_line1>,<fields_line2>
    // Example: RLBLOCK,       1,       2,       6,       7
    
    // Read first format line (e.g., "(2i8,6g16.9)")
    std::string format_line1;
    if (!std::getline(file, format_line1)) {
        return false;
    }
    
    // Read second format line (e.g., "(7g16.9)")
    std::string format_line2;
    if (!std::getline(file, format_line2)) {
        return false;
    }
    
    // Read data line(s)
    std::string data_line;
    std::streampos last_pos;
    
    while (true) {
        last_pos = file.tellg();
        if (!std::getline(file, data_line)) {
            break;
        }
        
        std::string trimmed = trim(data_line);
        if (trimmed.empty()) {
            continue;
        }
        
        // Check if this is the start of a new command
        std::string upper = to_upper(trimmed);
        if (upper.find("NBLOCK") == 0 || upper.find("EBLOCK") == 0 ||
            upper.find("CMBLOCK") == 0 || upper.find("RLBLOCK") == 0 ||
            upper.find("ET,") == 0 || upper.find("!!") == 0) {
            // Restore position and exit
            file.seekg(last_pos);
            break;
        }
        
        // Parse real constant data
        RealConstant rc;
        
        // Split line by whitespace and parse
        std::istringstream iss(data_line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.size() < 2) {
            continue;  // Invalid line
        }
        
        // First two tokens are set_id and num_values
        try {
            rc.id = std::stoi(tokens[0]);
            int num_values = std::stoi(tokens[1]);
            
            // Remaining tokens are real constant values
            for (size_t i = 2; i < tokens.size(); ++i) {
                rc.values.push_back(parse_scientific(tokens[i]));
            }
            
            // Read additional lines if needed
            while (rc.values.size() < static_cast<size_t>(num_values)) {
                last_pos = file.tellg();
                if (!std::getline(file, data_line)) {
                    break;
                }
                
                trimmed = trim(data_line);
                if (trimmed.empty()) {
                    continue;
                }
                
                // Check for new command
                upper = to_upper(trimmed);
                if (upper.find("NBLOCK") == 0 || upper.find("EBLOCK") == 0 ||
                    upper.find("CMBLOCK") == 0 || upper.find("RLBLOCK") == 0 ||
                    upper.find("ET,") == 0 || upper.find("!!") == 0) {
                    file.seekg(last_pos);
                    break;
                }
                
                // Parse continuation line
                std::istringstream iss2(data_line);
                while (iss2 >> token) {
                    rc.values.push_back(parse_scientific(token));
                    if (rc.values.size() >= static_cast<size_t>(num_values)) {
                        break;
                    }
                }
            }
            
            // Store real constant
            archive.real_constants_[rc.id] = rc;
            
        } catch (...) {
            continue;  // Skip invalid data
        }
        
        // RLBLOCK typically has only one set per block
        break;
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
        // Check if we need to normalize (contains 'D' or 'd')
        bool needs_normalization = false;
        for (char c : trimmed) {
            if (c == 'D' || c == 'd') {
                needs_normalization = true;
                break;
            }
        }
        
        if (needs_normalization) {
            // Replace 'D' with 'E' for FORTRAN-style double precision
            std::string normalized = trimmed;
            for (char& c : normalized) {
                if (c == 'D') c = 'E';
                else if (c == 'd') c = 'e';
            }
            return std::stod(normalized);
        } else {
            // No normalization needed, use directly
            return std::stod(trimmed);
        }
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

} // namespace cdb
