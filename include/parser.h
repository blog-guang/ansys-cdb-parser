/**
 * @file parser.h
 * @brief CDB file parser
 */

#ifndef CDB_PARSER_H
#define CDB_PARSER_H

#include "archive.h"
#include "types.h"
#include <fstream>
#include <string>

namespace cdb {

/**
 * @brief Parser for ANSYS CDB files
 */
class Parser {
public:
    /**
     * @brief Parse a CDB file into an Archive
     * @param filename Path to CDB file
     * @param archive Archive to populate
     * @return true if successful, false otherwise
     */
    static bool parse(const std::string& filename, Archive& archive);
    
    /**
     * @brief Parse NBLOCK format string
     * @param format_str Format string (e.g., "(3i9,6e21.13e3)")
     * @return Parsed format
     */
    static NBLOCKFormat parse_nblock_format(const std::string& format_str);
    
    /**
     * @brief Parse scientific notation value
     * @param str String to parse (e.g., "1.0000000000000E+000")
     * @return Parsed double value
     */
    static double parse_scientific(const std::string& str);
    
    /**
     * @brief Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);
    
    /**
     * @brief Convert string to uppercase
     * @param str String to convert
     * @return Uppercase string
     */
    static std::string to_upper(const std::string& str);
    
private:
    /**
     * @brief Parse NBLOCK (node block)
     * @param file Input file stream
     * @param archive Archive to populate
     * @param line Current line (NBLOCK command)
     * @return true if successful
     */
    static bool parse_nblock(std::ifstream& file, Archive& archive, const std::string& line);
    
    /**
     * @brief Parse EBLOCK (element block)
     * @param file Input file stream
     * @param archive Archive to populate
     * @param line Current line (EBLOCK command)
     * @return true if successful
     */
    static bool parse_eblock(std::ifstream& file, Archive& archive, const std::string& line);
    
    /**
     * @brief Parse CMBLOCK (component block)
     * @param file Input file stream
     * @param archive Archive to populate
     * @param line Current line (CMBLOCK command)
     * @return true if successful
     */
    static bool parse_cmblock(std::ifstream& file, Archive& archive, const std::string& line);
    
    /**
     * @brief Parse ET command (element type)
     * @param line Current line
     * @param archive Archive to populate
     * @return true if successful
     */
    static bool parse_et(const std::string& line, Archive& archive);
    
    /**
     * @brief Parse RLBLOCK (real constant block)
     * @param file Input file stream
     * @param archive Archive to populate
     * @param line Current line (RLBLOCK command)
     * @return true if successful
     */
    static bool parse_rlblock(std::ifstream& file, Archive& archive, const std::string& line);
    
    /**
     * @brief Parse *SET command (parameter)
     * @param line Current line
     * @param archive Archive to populate
     * @return true if successful
     */
    static bool parse_set(const std::string& line, Archive& archive);
    
    /**
     * @brief Parse D command (displacement constraint)
     * @param line Current line
     * @param archive Archive to populate
     * @return true if successful
     */
    static bool parse_d(const std::string& line, Archive& archive);
    
    /**
     * @brief Parse F command (nodal force)
     * @param line Current line
     * @param archive Archive to populate
     * @return true if successful
     */
    static bool parse_f(const std::string& line, Archive& archive);
};

} // namespace cdb

#endif // CDB_PARSER_H
