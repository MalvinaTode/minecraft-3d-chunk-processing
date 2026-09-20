#include "chunk.h"
int is_inside_bounds(int x, int y, int z, int width, int height, int depth) {
    return (x >= 0 && x < width && y>= 0 && y < height && z >= 0 && z < depth);
}

char*** chunk_place_block(
    char*** chunk, int width, int height, int depth,
    int x, int y, int z, char block) {
    if (!is_inside_bounds(x, y, z, width, height, depth)) {
       return chunk;
    }
    chunk[x][y][z] = block;
    return chunk;
}

char*** chunk_fill_cuboid(
    char*** chunk, int width, int height, int depth,
    int x0, int y0, int z0, int x1, int y1, int z1, char block) {
        int tmp = 0;
      if (x0 > x1) {
        tmp = x0;
        x0 = x1;
        x1 = tmp;
      }
      if (y0 > y1) {
        tmp = y0;
        y0 = y1;
        y1 = tmp;
      }
      if (z0 > z1) {
        tmp = z0;
        z0 = z1;
        z1 = tmp;
      }
      for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            for (int z = z0; z <= z1; z++) {
                chunk = chunk_place_block(chunk, width, height, depth, x, y, z, block);
            }
        }
      }
      return chunk;
}

char*** chunk_fill_sphere(char*** chunk, int width, int height, int depth,
int x, int y, int z, double radius, char block) {
        int radius_int = (int)radius;
        for (int i = -radius_int; i <= radius_int; i++) {
            for (int j = -radius_int; j <= radius_int; j++) {
              for (int k = -radius_int; k <= radius_int; k++) {
                  //  (x + i - x) ^2 + (y + j -y)^2 + (z + k -z)^2 < radius ^2;
                  if (i * i + j * j + k * k < radius * radius) {
                    chunk = chunk_place_block(chunk, width, height, depth, x+i, y+j, z+k, block);
                  }
                }
            }
        }
    return chunk;
}
