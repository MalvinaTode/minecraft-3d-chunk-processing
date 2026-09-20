#include "chunk.h"
#include "stdlib.h"

typedef struct {
    int x, y, z;
} Coordinate;

typedef struct {
    char block_type;   //  Tipul blocului, necesar pentru refacerea chunk-ului
    Coordinate* coordinates;
    int count;  //  Numarul de coordonate
    int is_grounded;  //  1 daca corpul este "impamantat", 0 altfel
} Body;

int is_inside_bounds(int x, int y, int z, int width, int height, int depth);

char*** chunk_rotate_y(
    char*** chunk, int width, int height, int depth) {

        // Alocare de memorie
        int new_width = depth;
        int new_depth = width;
        char ***rotated_chunk = (char ***)malloc(new_width * sizeof(char **));
        for (int i = 0; i < new_width; i++) {
            rotated_chunk[i] = (char **)malloc(height * sizeof(char *));
            for (int j = 0; j < height; j++) {
                rotated_chunk[i][j] = (char *)malloc(new_depth * sizeof(char));
            }
        }

        // Rotatia efectiva
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                for (int z = 0; z < depth; z++) {
                    rotated_chunk[depth - 1 - z][y][x] = chunk[x][y][z];
                }
            }
        }

        // Eliberare de memorie
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                free(chunk[i][j]);
            }
            free(chunk[i]);
        }
        free(chunk);
        return rotated_chunk;
}

void get_body_from_block_recursive(
  char ***chunk, int width, int height, int depth,
  int x, int y, int z, char target_block, Body* body) {
        // Varianta modificata a functiei fill_recursive din chunk_process.c

        if (!is_inside_bounds(x, y, z, width, height, depth) || chunk[x][y][z] != target_block) {
            return;
        }

        chunk[x][y][z] = BLOCK_AIR;

        if (body->is_grounded == 0 && y == 0) {
            body->is_grounded = 1;
        }

        // Folosim realloc pentru a creste dimensiunea vectorului de coordonate
        body->coordinates = realloc(body->coordinates, (body->count + 1) * sizeof(Coordinate));
        body->coordinates[body->count].x = x;
        body->coordinates[body->count].y = y;
        body->coordinates[body->count].z = z;
        body->count++;

        get_body_from_block_recursive(chunk, width, height, depth, x + 1, y, z, target_block, body);
        get_body_from_block_recursive(chunk, width, height, depth, x - 1, y, z, target_block, body);
        get_body_from_block_recursive(chunk, width, height, depth, x, y + 1, z, target_block, body);
        get_body_from_block_recursive(chunk, width, height, depth, x, y - 1, z, target_block, body);
        get_body_from_block_recursive(chunk, width, height, depth, x, y, z + 1, target_block, body);
        get_body_from_block_recursive(chunk, width, height, depth, x, y, z - 1, target_block, body);
}

Body get_body_from_block(
    char*** chunk, int width, int height, int depth,
    int x, int y, int z) {
        // Varianta modificata a functiei chunk_fill din chunk_process.c
        // "Sterge" corpul curent din chunk si returneaza un Body care contine toate coordonatele blockurilor din corp

        Body body;
        body.block_type = '\0';
        body.coordinates = NULL;
        body.count = 0;
        body.is_grounded = 0;

        if (!is_inside_bounds(x, y, z, width, height, depth) || chunk[x][y][z] == BLOCK_AIR) {
            return body;
        }

        body.block_type = chunk[x][y][z];

        get_body_from_block_recursive(chunk, width, height, depth, x, y, z, chunk[x][y][z], &body);

        return body;
}

void move_bodies_down(Body* bodies, int count) {
    for (int i = 0; i < count; i++) {
        if (bodies[i].is_grounded) {
            continue;  // Nu mutam corpurile impamantate
        }

        for (int j = 0; j < bodies[i].count; j++) {
            bodies[i].coordinates[j].y--;
        }
    }
}

void mark_grounded_bodies(Body* bodies, int count) {
    // Marcam corpurile care au cel putin un block la y = 0 ca fiind "impamantate"
    for (int i = 0; i < count; i++) {
        if (bodies[i].is_grounded == 0) {
            for (int j = 0; j < bodies[i].count; j++) {
                if (bodies[i].coordinates[j].y == 0) {
                    bodies[i].is_grounded = 1;
                    break;
                }
            }
        }
    }
}

void move_body_up(Body* body) {
    for (int j = 0; j < body->count; j++) {
        body->coordinates[j].y++;
    }
}

int do_bodies_overlap(Body* body1, Body* body2) {
    // Verificam daca exista cel putin un block comun intre cele doua corpuri
    for (int i = 0; i < body1->count; i++) {
        for (int j = 0; j < body2->count; j++) {
            if (body1->coordinates[i].x == body2->coordinates[j].x &&
                body1->coordinates[i].y == body2->coordinates[j].y &&
                body1->coordinates[i].z == body2->coordinates[j].z) {
                return 1;
            }
        }
    }
    return 0;
}

void remove_empty_top_planes(char*** chunk, int width, int height, int depth, int* new_height) {
    // Pornim de la presupunerea ca nu se va schimba inaltimea
    *new_height = height;

    // Determinam inaltimea noua verificand cate planuri paralele cu xOz goale exista in partea de sus a chunkului
    for (int y = height - 1; y >= 0; y--) {
        int is_empty = 1;
        for (int x = 0; x < width; x++) {
            for (int z = 0; z < depth; z++) {
                if (chunk[x][y][z] != BLOCK_AIR) {
                    is_empty = 0;
                    break;
                }
            }
            if (!is_empty) break;
        }
        if (is_empty) {
            (*new_height)--;
        } else {
            break;
        }
    }

    if (*new_height != height) {
        // Eliberam memoria pentru planurile goale
        for (int x = 0; x < width; x++) {
            for (int y = *new_height; y < height; y++) {
                free(chunk[x][y]);
            }
        }
    }
}

char*** chunk_apply_gravity(
    char*** chunk, int width, int height, int depth, int* new_height) {

    // Identificam toate corpurile din chunk
    Body* bodies = NULL;
    int body_count = 0;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                if (chunk[x][y][z] != BLOCK_AIR) {
                    Body body = get_body_from_block(chunk, width, height, depth, x, y, z);
                    if (body.count > 0) {
                        // Crestem dinamic dimensiunea vectorului de corpuri
                        bodies = realloc(bodies, (body_count + 1) * sizeof(Body));
                        bodies[body_count] = body;
                        body_count++;
                    }
                }
            }
        }
    }

    // Aplicam move_bodies_down si mark_grounded_bodies pana cand toate corpurile sunt "impamantate"
    // Un corp este impamantat daca are cel putin un block cu y = 0 sau daca se afla deasupra altui corp impamantat
    int all_grounded = 1;
    do {
        move_bodies_down(bodies, body_count);

        all_grounded = 1;
        for (int i = 0; i < body_count; i++) {
            if (bodies[i].is_grounded == 0) {  // Cautam overlap doar intre corpuri inpamantate si neimpamantate
                all_grounded = 0;
                for (int j = 0; j < body_count; j++) {
                    if (i != j && do_bodies_overlap(&bodies[i], &bodies[j])) {
                        // Daca un corp neimpamantat se suprapune cu un corp impamantat, inseamna ca acesta
                        // nu poate fi mutat in jos, asa ca il marcam ca fiind impamantat si rezolvam
                        // suprapunerea prin ridicarea acestuia.
                        if (bodies[j].is_grounded == 1) {
                            move_body_up(&bodies[i]);
                            bodies[i].is_grounded = 1;
                            break;
                        }
                    }
                }
            }
        }

        // Marcam corpurile care au cel putin un block cu y = 0 ca fiind "impamantate"
        mark_grounded_bodies(bodies, body_count);
    } while (!all_grounded);

    // Inseram corpurile modificate mai sus inapoi in chunk
    for (int i = 0; i < body_count; i++) {
        for (int j = 0; j < bodies[i].count; j++) {
            int x = bodies[i].coordinates[j].x;
            int y = bodies[i].coordinates[j].y;
            int z = bodies[i].coordinates[j].z;
            chunk[x][y][z] = bodies[i].block_type;
        }
    }

    // Eliberam memoria pentru corpurile temporare
    for (int i = 0; i < body_count; i++) {
        free(bodies[i].coordinates);
    }
    free(bodies);

    remove_empty_top_planes(chunk, width, height, depth, new_height);  // Eliminam planurile goale de sus

    return chunk;
}


