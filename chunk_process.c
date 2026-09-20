#include "chunk.h"
#include <stdlib.h>
#include <string.h>

int is_inside_bounds(int x, int y, int z, int width, int height, int depth);
void place_shell(
char ***chunk, int width, int height, int depth,
int x, int y, int z,  char target_block, char shell_block) {
for(int dx = -1; dx <= 1; dx++) {
for(int dy = -1; dy <= 1; dy++) {
for(int dz = -1; dz <= 1; dz++) {
  if (dx == 0 && dy == 0 && dz == 0) {  //  ignoram blockul din centru
  continue;
  }
  if (abs(dx) + abs(dy) + abs(dz) == 3) {  //  ignoram diagonalele
    continue;
  }
  int nx = x + dx;  // calculam vecinii
  int ny = y + dy;
  int nz = z + dz;

if (is_inside_bounds(nx, ny, nz, width, height, depth)) {
    if (chunk[nx][ny][nz] != target_block) {
        chunk[nx][ny][nz] = shell_block;
    }
}
}
}
}
}

char*** chunk_shell(
    char*** chunk, int width, int height, int depth,
    char target_block, char shell_block) {
  //  vom folosi o copie a chunkului ca referinta pentru cazul in care target_block == shell_block
  char*** chunk_copy = (char***)malloc(width * sizeof(char**));
  for (int x = 0; x < width; x++) {
    chunk_copy[x] = (char**)malloc(height * sizeof(char**));
    for (int y = 0; y < height; y++) {
       chunk_copy[x][y] = (char*)malloc(depth * sizeof(char*));
       for (int z = 0; z < depth; z++) {
        chunk_copy[x][y][z] = chunk[x][y][z];
      }
    }
  }
// Parcurgem chunkul si plasam shell_block in jurul blocurilor de tip target_block

for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
        for (int z = 0; z < depth; z++) {
            if (chunk_copy[x][y][z] == target_block) {
                place_shell(chunk, width, height, depth, x, y, z, target_block, shell_block);
            }
        }
    }
}

for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
        free(chunk_copy[x][y]);
    }
    free(chunk_copy[x]);
}
  free(chunk_copy);
  return chunk;
}
void fill_xz_recursive(
char ***chunk, int width, int height, int depth,
int x, int y, int z, char block, char target_block) {
  if (!is_inside_bounds(x, y, z, width, height, depth)) {
    return;
}

if (chunk[x][y][z] != target_block) {
    return;
}

chunk[x][y][z] = block;
fill_xz_recursive(chunk, width, height, depth, x+1, y, z, block, target_block);
fill_xz_recursive(chunk, width, height, depth, x-1, y, z, block, target_block);
fill_xz_recursive(chunk, width, height, depth, x, y, z+1, block, target_block);
fill_xz_recursive(chunk, width, height, depth, x, y, z-1, block, target_block);
}


char*** chunk_fill_xz(
    char*** chunk, int width, int height, int depth,
    int x, int y, int z, char block) {
if (!is_inside_bounds(x, y, z, width, height, depth)) {
    return chunk;
}

if (chunk[x][y][z] == block) {
    return chunk;
}

fill_xz_recursive(chunk, width, height, depth, x, y, z, block, chunk[x][y][z]);
return chunk;
}

void fill_recursive(
char*** chunk, int width, int height, int depth,
int x, int y, int z, char block, char target_block) {
    if (!is_inside_bounds(x, y, z, width, height, depth)) {
        return;
    }
    if (chunk[x][y][z] != target_block) {
        return;
    }
    chunk[x][y][z] = block;
fill_recursive(chunk, width, height, depth, x+1, y, z, block, target_block);
fill_recursive(chunk, width, height, depth, x-1, y, z, block, target_block);
fill_recursive(chunk, width, height, depth, x, y+1, z, block, target_block);
fill_recursive(chunk, width, height, depth, x, y-1, z, block, target_block);
fill_recursive(chunk, width, height, depth, x, y, z+1, block, target_block);
fill_recursive(chunk, width, height, depth, x, y, z-1, block, target_block);
}


char*** chunk_fill(
    char*** chunk, int width, int height, int depth,
    int x, int y, int z, char block) {
     if (!is_inside_bounds(x, y, z, width, height, depth)) {
        return chunk;
    }
    char target_block = chunk[x][y][z];
    if (target_block == block) {
        return chunk;
    }
    fill_recursive(chunk, width, height, depth, x, y, z, block, target_block);
    return chunk;
}
