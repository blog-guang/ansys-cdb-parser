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
        else if (upper.find("D,") == 0) {
            parse_d(trimmed, archive);  // Non-critical, don't fail on error
        }
        else if (upper.find("F,") == 0) {
            parse_f(trimmed, archive);  // Non-critical, don't fail on error
        }
        else if (upper.find("MPTEMP") == 0) {
            parse_mptemp(trimmed, archive);  // Non-critical
        }
        else if (upper.find("MPDATA") == 0) {
            parse_mpdata(trimmed, archive);  // Non-critical
        }
        else if (upper.find("N,") == 0) {
            parse_n(trimmed, archive);  // Non-critical
        }
        else if (upper.find("EN,") == 0) {
            parse_en(trimmed, archive);  // Non-critical
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

bool Parser::parse_d(const std::string& line, Archive& archive) {
    // Parse D command (displacement constraint)
    // Format: D,NODE,Lab,VALUE,VALUE2,NEND,NINC,KEXPND,Lab2,Lab3,Lab4,Lab5,Lab6
    // Example: D,1,UX,0.0
    //          D,ALL,UY,0.0
    //          D,10,20,UZ,0.0  (nodes 10 to 20)
    
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    if (tokens.size() < 4) {
        return false;  // Invalid D command
    }
    
    // tokens[0] = "D"
    // tokens[1] = NODE (node ID, or ALL, or start node)
    // tokens[2] = Lab (DOF label like UX, UY, UZ, etc., or end node)
    // tokens[3] = VALUE (constraint value, or DOF label if tokens[2] is end node)
    
    std::string node_str = tokens[1];
    std::string dof_str;
    std::string value_str;
    
    // Check if it's a range (D,start,end,dof,value)
    try {
        int start_node = std::stoi(tokens[1]);
        int end_node = std::stoi(tokens[2]);
        dof_str = tokens[3];
        value_str = (tokens.size() > 4) ? tokens[4] : "0.0";
        
        // Apply constraint to all nodes in range
        for (int nid = start_node; nid <= end_node; ++nid) {
            DisplacementBC bc;
            bc.node_id = nid;
            bc.dof = dof_str;
            bc.value = std::stod(value_str);
            archive.displacement_bcs_.push_back(bc);
        }
        return true;
    } catch (...) {
        // Not a range, continue with normal parsing
    }
    
    // Normal format: D,NODE,Lab,VALUE
    if (node_str == "ALL") {
        // Apply to all nodes
        dof_str = tokens[2];
        value_str = tokens[3];
        
        for (const auto& node : archive.get_nodes()) {
            DisplacementBC bc;
            bc.node_id = node.id;
            bc.dof = dof_str;
            bc.value = std::stod(value_str);
            archive.displacement_bcs_.push_back(bc);
        }
    } else {
        // Single node
        try {
            DisplacementBC bc;
            bc.node_id = std::stoi(tokens[1]);
            bc.dof = tokens[2];
            bc.value = std::stod(tokens[3]);
            archive.displacement_bcs_.push_back(bc);
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

bool Parser::parse_f(const std::string& line, Archive& archive) {
    // Parse F command (nodal force)
    // Format: F,NODE,Lab,VALUE,VALUE2,PHASE
    // Example: F,10,FX,1000.0
    //          F,20,FY,-500.0
    
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    if (tokens.size() < 4) {
        return false;  // Invalid F command
    }
    
    // tokens[0] = "F"
    // tokens[1] = NODE (node ID or ALL)
    // tokens[2] = Lab (force label: FX, FY, FZ, MX, MY, MZ)
    // tokens[3] = VALUE (force magnitude)
    
    std::string node_str = tokens[1];
    std::string direction = tokens[2];
    std::string value_str = tokens[3];
    
    if (node_str == "ALL") {
        // Apply to all nodes
        for (const auto& node : archive.get_nodes()) {
            NodalForce force;
            force.node_id = node.id;
            force.direction = direction;
            force.value = std::stod(value_str);
            archive.nodal_forces_.push_back(force);
        }
    } else {
        // Single node
        try {
            NodalForce force;
            force.node_id = std::stoi(tokens[1]);
            force.direction = tokens[2];
            force.value = std::stod(tokens[3]);
            archive.nodal_forces_.push_back(force);
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

bool Parser::parse_mptemp(const std::string& line, Archive& archive) {
    // Parse MPTEMP command (material temperature table)
    // Format: MPTEMP,R5.0,STLOC,T1,T2,T3,T4,T5,T6
    // Example: MPTEMP,1,0,200,400,600,800,1000
    
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    if (tokens.size() < 3) {
        return false;  // Invalid MPTEMP command
    }
    
    // tokens[0] = "MPTEMP"
    // tokens[1] = R5.0 or table number (usually 1)
    // tokens[2] = starting location (usually 0)
    // tokens[3+] = temperature values
    
    std::vector<double> temps;
    for (size_t i = 3; i < tokens.size(); ++i) {
        try {
            temps.push_back(std::stod(tokens[i]));
        } catch (...) {
            // Skip invalid values
        }
    }
    
    // Set temperature table in material database
    if (!temps.empty()) {
        archive.get_material_database().set_temp_table(temps);
    }
    
    return true;
}

bool Parser::parse_mpdata(const std::string& line, Archive& archive) {
    // Parse MPDATA command (material property data)
    // Format: MPDATA,Lab,MAT,SLOC,C1,C2,C3,C4,C5,C6
    // Example: MPDATA,EX,1,,2.0E11
    //          MPDATA,PRXY,1,,0.3
    //          MPDATA,DENS,1,,7850
    
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    if (tokens.size() < 4) {
        return false;  // Invalid MPDATA command
    }
    
    // tokens[0] = "MPDATA"
    // tokens[1] = Lab (property name: EX, PRXY, DENS, etc.)
    // tokens[2] = MAT (material ID)
    // tokens[3] = SLOC (starting location, usually empty)
    // tokens[4+] = C1, C2, C3... (property values)
    
    std::string property_name = tokens[1];
    int material_id = 0;
    
    try {
        material_id = std::stoi(tokens[2]);
    } catch (...) {
        return false;
    }
    
    // Collect property values
    std::vector<double> values;
    for (size_t i = 4; i < tokens.size() && i < 10; ++i) {  // Max 6 values per line
        if (!tokens[i].empty()) {
            try {
                values.push_back(std::stod(tokens[i]));
            } catch (...) {
                // Skip invalid values
            }
        }
    }
    
    if (values.empty()) {
        return false;
    }
    
    // Create material property
    MaterialProperty prop;
    prop.material_id = material_id;
    prop.property_name = property_name;
    prop.values = values;
    
    // Copy temperature table if available
    const auto& temp_table = archive.get_material_database().get_temp_table();
    if (!temp_table.empty() && values.size() <= temp_table.size()) {
        prop.temps.assign(temp_table.begin(), temp_table.begin() + values.size());
    }
    
    // Add to material database
    archive.get_material_database().add_property(prop);
    
    return true;
}

bool Parser::parse_n(const std::string& line, Archive& archive) {
    // Parse N command (single node definition)
    // Format: N,NODE,X,Y,Z,THXY,THYZ,THZX
    // Example: N,1,0.0,0.0,0.0
    //          N,2,1.0,0.0,0.0
    
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    if (tokens.size() < 4) {
        return false;  // Invalid N command
    }
    
    // tokens[0] = "N"
    // tokens[1] = NODE (node ID)
    // tokens[2] = X
    // tokens[3] = Y
    // tokens[4] = Z (optional)
    // tokens[5-7] = rotation angles (optional)
    
    Node node;
    
    try {
        node.id = std::stoi(tokens[1]);
        node.x = std::stod(tokens[2]);
        node.y = std::stod(tokens[3]);
        
        if (tokens.size() > 4 && !tokens[4].empty()) {
            node.z = std::stod(tokens[4]);
        }
        
        // Rotation angles (optional)
        if (tokens.size() > 5 && !tokens[5].empty()) {
            node.angles[0] = std::stod(tokens[5]);
        }
        if (tokens.size() > 6 && !tokens[6].empty()) {
            node.angles[1] = std::stod(tokens[6]);
        }
        if (tokens.size() > 7 && !tokens[7].empty()) {
            node.angles[2] = std::stod(tokens[7]);
        }
    } catch (...) {
        return false;
    }
    
    // Add node to archive
    size_t index = archive.nodes_.size();
    archive.nodes_.push_back(node);
    archive.node_map_[node.id] = index;
    
    return true;
}

bool Parser::parse_en(const std::string& line, Archive& archive) {
    // Parse EN command (single element definition)
    // Format: EN,ELEM,TYPE,REAL,MAT,SECNUM,ESYS,IXX,IYY,IZZ,MSHKEY,I,J,K,L,M,N,O,P
    // Example: EN,1,1,1,1,0,0,0,0,0,0,1,2,3,4
    //          EN,2,1,1,1,0,0,0,0,0,0,2,3,5,4
    
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split by comma
    while (std::getline(iss, token, ',')) {
        tokens.push_back(trim(token));
    }
    
    if (tokens.size() < 11) {
        return false;  // Invalid EN command
    }
    
    // tokens[0] = "EN"
    // tokens[1] = ELEM (element ID)
    // tokens[2] = TYPE (element type ID)
    // tokens[3] = REAL (real constant ID)
    // tokens[4] = MAT (material ID)
    // tokens[5] = SECNUM (section ID)
    // tokens[6] = ESYS (element coordinate system)
    // tokens[7-9] = IXX, IYY, IZZ (ignored)
    // tokens[10] = MSHKEY (ignored)
    // tokens[11+] = node IDs
    
    Element elem;
    
    try {
        elem.id = std::stoi(tokens[1]);
        elem.type = std::stoi(tokens[2]);
        elem.real_constant_id = std::stoi(tokens[3]);
        elem.material_id = std::stoi(tokens[4]);
        elem.section_id = std::stoi(tokens[5]);
        elem.coordinate_system = std::stoi(tokens[6]);
        
        // Collect node IDs starting from token[11]
        for (size_t i = 11; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) {
                int node_id = std::stoi(tokens[i]);
                if (node_id > 0) {  // Skip zero/negative (unused nodes)
                    elem.node_ids.push_back(node_id);
                }
            }
        }
    } catch (...) {
        return false;
    }
    
    if (elem.node_ids.empty()) {
        return false;  // No valid nodes
    }
    
    // Add element to archive
    size_t index = archive.elements_.size();
    archive.elements_.push_back(elem);
    archive.element_map_[elem.id] = index;
    
    return true;
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
