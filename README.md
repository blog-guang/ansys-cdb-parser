# ANSYS CDB Parser

A modern C++17 library for parsing ANSYS MAPDL archive (CDB) files.

## Features

- ✅ Parse NBLOCK (nodes)
- 🚧 Parse EBLOCK (elements) - In Progress
- 🚧 Parse CMBLOCK (components) - Planned
- ✅ Parse ET (element types)
- 🚧 Parse RLBLOCK (real constants) - Planned
- ✅ Parse parameters (*SET)
- ✅ STL containers (vector, unordered_map)
- ✅ Header-only friendly design
- ✅ Comprehensive tests with Google Test

## Requirements

- CMake 3.15+
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Google Test (automatically downloaded)

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Run Tests

```bash
ctest --output-on-failure
```

### Usage Example

```cpp
#include <ansys_cdb/archive.h>
#include <iostream>

int main() {
    ansys_cdb::Archive archive("mesh.cdb");
    
    std::cout << "Nodes: " << archive.num_nodes() << std::endl;
    std::cout << "Elements: " << archive.num_elements() << std::endl;
    
    // Access nodes
    for (const auto& node : archive.get_nodes()) {
        std::cout << "Node " << node.id 
                  << ": (" << node.x << ", " << node.y << ", " << node.z << ")" 
                  << std::endl;
    }
    
    // Get specific node by ID
    const auto* node = archive.get_node(1);
    if (node) {
        std::cout << "Found node 1 at: " << node->x << ", " 
                  << node->y << ", " << node->z << std::endl;
    }
    
    return 0;
}
```

## Project Structure

```
ansys-cdb-parser/
├── include/ansys_cdb/   # Public headers
│   ├── archive.h        # Main Archive class
│   ├── parser.h         # CDB parser
│   ├── node.h           # Node data structure
│   ├── element.h        # Element data structure
│   ├── component.h      # Component (groups)
│   └── types.h          # Common types
├── src/                 # Implementation
├── tests/               # Unit tests
└── examples/            # Usage examples
```

## Development Status

This project follows Test-Driven Development (TDD) principles.

### Phase 1: Project Setup ✅
- [x] CMake build system
- [x] Google Test integration
- [x] Basic data structures
- [x] Initial project structure

### Phase 2: NBLOCK Parser (Current)
- [x] Parse NBLOCK format specification
- [x] Read node data
- [x] Handle scientific notation
- [ ] Test with HexBeam.cdb
- [ ] Test with all reference CDB files

### Phase 3: EBLOCK Parser (Next)
- [ ] Parse EBLOCK format
- [ ] Read element data
- [ ] Support multiple element types
- [ ] Test with reference files

## Testing

All test files from [mapdl-archive](https://github.com/akaszynski/mapdl-archive) repository are used for validation:

- HexBeam.cdb
- TetBeam.cdb
- sector.cdb
- academic_rotor.cdb
- and more...

## Reference

This implementation follows the format specifications from:
- [mapdl-archive](https://github.com/akaszynski/mapdl-archive) - Python reference implementation
- ANSYS MAPDL documentation

## License

MIT License

## Contributing

1. Fork the repository
2. Create a feature branch
3. Write tests first (TDD)
4. Implement the feature
5. Submit a pull request

## Roadmap

- Week 1: Complete NBLOCK and EBLOCK parsing
- Week 2: Add component, element type, and parameter support
- Week 3: Full test coverage and optimization
- Week 4: Documentation and release

---

**Status**: 🚧 In Development - Phase 1 Complete
