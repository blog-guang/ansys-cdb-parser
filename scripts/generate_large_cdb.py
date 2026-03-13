#!/usr/bin/env python3
"""
Generate a large CDB file for benchmark testing
"""

import sys
import os

def generate_large_cdb(filename, num_nodes=500000, nodes_per_element=8):
    """
    Generate a large CDB file with specified number of nodes
    Creates a regular grid of hex elements
    """
    num_elements = (num_nodes // nodes_per_element) if nodes_per_element > 0 else 0
    
    print(f"Generating {filename} with {num_nodes} nodes and {num_elements} elements...")
    
    with open(filename, 'w') as f:
        # Header
        f.write("/COM,ANSYS BENCHMARK FILE - AUTO-GENERATED\n")
        f.write("/PREP7\n")
        f.write("/NOPR\n")
        f.write("/TITLE,Large Benchmark Mesh\n")
        f.write("*SET,_RETURN ,  0.000000000000\n")
        f.write("*SET,_STATUS ,  0.000000000000\n")
        f.write("DOF,DELETE\n")
        f.write("ET,        1,186\n")
        
        # NBLOCK - nodes
        f.write(f"NBLOCK,6,SOLID,{num_nodes},{num_nodes}\n")
        f.write("(3i9,6e21.13e3)\n")
        
        # Generate nodes in a regular grid
        nx = int(num_nodes ** (1/3)) + 1  # Cube root for 3D grid
        ny = nx
        nz = nx
        
        node_id = 1
        for k in range(nz):
            for j in range(ny):
                for i in range(nx):
                    if node_id > num_nodes:
                        break
                    x = i * 1.0
                    y = j * 1.0
                    z = k * 1.0
                    # Format: node_id, solid=0, coord_sys=0, x, y, z
                    f.write(f"{node_id:9d}{0:9d}{0:9d}")
                    f.write(f"{x:21.13E}{y:21.13E}{z:21.13E}\n")
                    node_id += 1
                if node_id > num_nodes:
                    break
            if node_id > num_nodes:
                break
        
        f.write("N,R5.3,LOC,       -1,\n")
        
        # EBLOCK - elements
        if num_elements > 0:
            f.write(f"EBLOCK,19,SOLID,{num_elements},{num_elements}\n")
            f.write("(19i9)\n")
            
            # Generate hex elements
            elem_id = 1
            for k in range(nz - 1):
                for j in range(ny - 1):
                    for i in range(nx - 1):
                        if elem_id > num_elements:
                            break
                        
                        # 8-node hex element connectivity
                        n1 = k * nx * ny + j * nx + i + 1
                        n2 = n1 + 1
                        n3 = n1 + nx + 1
                        n4 = n1 + nx
                        n5 = n1 + nx * ny
                        n6 = n5 + 1
                        n7 = n5 + nx + 1
                        n8 = n5 + nx
                        
                        # Check if all nodes exist
                        if max(n1, n2, n3, n4, n5, n6, n7, n8) > num_nodes:
                            continue
                        
                        # Format: mat, type, real, secnum, esys, death, ?, ?, numNodes, ?, elemId, nodes...
                        f.write(f"{1:9d}{1:9d}{0:9d}{0:9d}{0:9d}{0:9d}{0:9d}{0:9d}")
                        f.write(f"{8:9d}{0:9d}{elem_id:9d}")
                        f.write(f"{n1:9d}{n2:9d}{n3:9d}{n4:9d}{n5:9d}{n6:9d}{n7:9d}{n8:9d}\n")
                        elem_id += 1
                        
                    if elem_id > num_elements:
                        break
                if elem_id > num_elements:
                    break
            
            f.write("       -1\n")
        
        f.write("FINISH\n")
    
    file_size = os.path.getsize(filename) / (1024 * 1024)  # MB
    print(f"Generated {filename} successfully!")
    print(f"  Nodes: {num_nodes}")
    print(f"  Elements: {num_elements}")
    print(f"  File size: {file_size:.2f} MB")

if __name__ == "__main__":
    output_file = sys.argv[1] if len(sys.argv) > 1 else "large_benchmark.cdb"
    num_nodes = int(sys.argv[2]) if len(sys.argv) > 2 else 500000
    
    generate_large_cdb(output_file, num_nodes)
