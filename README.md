# 3D Chunk Processing & Compression

A C project for generating, processing, transforming, and compressing Minecraft-style 3D chunks represented as dynamically allocated three-dimensional arrays.

## Project Overview

The project implements a collection of operations for manipulating three-dimensional chunks of blocks, including structure generation, connected-region processing, transformations, gravity simulation, and lossless compression.

## Task 1 – Block Placement

The `chunk_place_block` function checks whether the given coordinates are within the chunk boundaries. If they are valid, the specified block is placed at the given coordinates. Otherwise, the chunk remains unchanged.

## Task 2 – Cuboid Filling

The `chunk_fill_cuboid` function iterates through all coordinates from `x0` to `x1`, `y0` to `y1`, and `z0` to `z1`, calling the function implemented in Task 1 for each set of coordinates.

The coordinate values are swapped when the first coordinate on a given axis is greater than the second one. The function returns the modified chunk.

## Task 3 – Sphere Filling

The `chunk_fill_sphere` function iterates through all blocks within the cube containing the sphere centered at `(x, y, z)` with the given radius.

A block is placed using `chunk_place_block` if its coordinates satisfy the sphere condition:
(x - center_x)^2 + (y - center_y)^2 + (z - center_z)^2 < radius^2
## Task 4 – Block Shell

The chunk_shell function creates a copy of the chunk and iterates through it to identify blocks of type target_block.

For each identified block, an auxiliary place_shell function places blocks of type shell_block around it, including direct and edge neighbors while excluding diagonal corner positions.

Before returning the modified chunk, the memory allocated for the chunk copy is released.

## Task 5 – XZ Flood Fill

The chunk_fill_xz function calls the recursive helper function fill_xz_recursive.

The recursive function checks whether the current block is within the chunk boundaries and whether it has the same type as the block from which the fill started. If both conditions are satisfied, the block is replaced with the specified block type and the function recursively processes all neighboring blocks in the XZ plane.

The function returns the modified chunk.

## Task 6 – 3D Flood Fill

The chunk_fill function is similar to chunk_fill_xz.

The main difference is that it calls the recursive helper function fill_recursive, which is a modified version of fill_xz_recursive and recursively processes neighboring blocks along all three axes.

## Task 7 – Y-Axis Rotation

The chunk_rotate_y function first allocates memory for a new chunk, rotated_chunk, which represents the original chunk rotated by 90 degrees around the Y axis.

The rotation is performed by iterating through the elements of the original chunk and mapping each position (x, y, z) to:

(depth - 1 - z, y, x)

in the rotated chunk.

After the rotation is completed, the memory used by the original chunk is released and the function returns rotated_chunk.

## Task 8 – Gravity Simulation

The chunk_apply_gravity function simulates the effect of gravity on the connected bodies within a chunk and removes empty layers from the top of the chunk.

The function performs several steps:

Identifies all bodies in the chunk using the auxiliary get_body_from_block function, which is a modified version of fill_recursive and stores relevant information about each body, such as block coordinates and the number of blocks.
Moves all bodies downward and checks whether they have reached the ground (y = 0). When a body reaches the ground, it is marked as grounded using the is_grounded property.
If an ungrounded body overlaps with a grounded body, the ungrounded body becomes grounded and its blocks are moved upward to resolve the overlap.
Step 2 is repeated until all bodies are grounded.
The bodies are placed back into the chunk at their resulting positions.
The function iterates through the planes parallel to the XZ plane from height - 1 downward and determines the new height (new_height) corresponding to the highest non-empty layer. The memory associated with the unused upper layers is then released.
The resulting chunk is returned.

## Task 9 – RLE Compression

The chunk_encode function first flattens the input chunk in Y, Z, X order.

It then iterates through the flattened chunk and determines the lengths of consecutive sequences of blocks of the same type.

If a sequence contains more than 4096 elements, the sequence length is limited to 4095 and the remaining elements are encoded as a separate sequence.

For sequences with a length strictly smaller than 32, a single unsigned char is added to the encoded chunk. The two most significant bits represent the block type, the third bit is set to 0, and the remaining five bits represent the sequence length.

For longer sequences, two unsigned char values are used:

The first byte contains the two most significant bits representing the block type, followed by the encoding flag bits and the four most significant bits of the sequence length.
The second byte contains the eight least significant bits of the sequence length.

Bitwise operations such as shifting, AND, and OR are used to construct the encoded representation.

Finally, the memory allocated for the flattened chunk is released and the encoded chunk is returned.

## Task 10 – RLE Decompression

The chunk_decode function performs the inverse operation of chunk_encode.

Memory is allocated for the decoded chunk and each element of the encoded chunk is processed.

The block type (block_type) is determined from the two most significant bits of the first byte.

If the encoding flag indicates that the current sequence uses two bytes, the next byte is also processed.

The sequence length (run_length) is reconstructed from the relevant bits of the first byte and, when necessary, the second byte.

For each encoded sequence, run_length elements of the corresponding block_type are added to the decoded chunk in the same Y, Z, X order used during encoding.

The function returns the reconstructed three-dimensional chunk.

Technical Concepts
C
Pointers
Dynamic memory allocation
3D data structures
Recursive algorithms
Flood-fill algorithms
Connected-component processing
Coordinate transformations
Bitwise operations
Run-Length Encoding (RLE)
Memory management
Valgrind
Makefile
Project Structure
.
├── libchunk/
│   ├── chunk.h
│   ├── chunk_gen.c
│   ├── chunk_process.c
│   ├── chunk_transform.c
│   └── chunk_compress.c
├── Makefile
└── README.md
