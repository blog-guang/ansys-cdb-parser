/**
 * @file simple_read.cpp
 * @brief Simple example of reading a CDB file
 */

#include <ansys_cdb/archive.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <cdb_file>" << std::endl;
        return 1;
    }
    
    try {
        ansys_cdb::Archive archive(argv[1]);
        
        std::cout << "Successfully loaded: " << argv[1] << std::endl;
        std::cout << "Number of nodes: " << archive.num_nodes() << std::endl;
        std::cout << "Number of elements: " << archive.num_elements() << std::endl;
        std::cout << "Number of node components: " << archive.num_node_components() << std::endl;
        std::cout << "Number of element components: " << archive.num_element_components() << std::endl;
        
        // Print first few nodes
        std::cout << "\nFirst 5 nodes:" << std::endl;
        const auto& nodes = archive.get_nodes();
        for (size_t i = 0; i < std::min(size_t(5), nodes.size()); ++i) {
            const auto& node = nodes[i];
            std::cout << "  Node " << node.id << ": (" 
                      << node.x << ", " << node.y << ", " << node.z << ")" 
                      << std::endl;
        }
        
        // Print first few elements
        std::cout << "\nFirst 5 elements:" << std::endl;
        const auto& elements = archive.get_elements();
        for (size_t i = 0; i < std::min(size_t(5), elements.size()); ++i) {
            const auto& elem = elements[i];
            std::cout << "  Element " << elem.id << " (type " << elem.type << "): " 
                      << elem.num_nodes() << " nodes" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
