#include "chunk.h"
#include <stdlib.h>

enum {
    MAX_RUN_LENGTH_SMALL = 32,
    MAX_RUN_LENGTH_LARGE = 4096,
    BLOCK_TYPE_SHIFT = 6,
    RUN_LENGTH_LARGE_SHIFT = 8,
    SMALL_RUN_LENGTH_MASK = 0b00011111,
    LARGE_RUN_LENGTH_MASK = 0b00001111,
    LARGE_RUN_FLAG = 0b00100000
};

unsigned char* chunk_encode(
    char*** chunk, int width, int height, int depth,
    int* length) {

        int chunk_size = width * height * depth;
        int index = 0, index_encoded = 0;
        unsigned char* flattened = (unsigned char*)malloc(chunk_size * sizeof(unsigned char));

        // Aplatizam chunkul in ordinea y, z, x
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                for (int x = 0; x < width; x++) {
                    flattened[index++] = (unsigned char)chunk[x][y][z];
                }
            }
        }

        // Alocam suficienta memorie pentru cel mai rau caz
        unsigned char* encoded = (unsigned char*)malloc(chunk_size * 2 * sizeof(unsigned char));

        index = 0;

        while (index < chunk_size) {
            unsigned char block_type = flattened[index];
            int run_length = 1;  // Lungimea secventei de blockuri de acelasi tip

            while (index + run_length < chunk_size
            && flattened[index + run_length] == block_type
            && run_length < MAX_RUN_LENGTH_LARGE - 1) {
                // Daca un run are run_length >= 4096, incheiem run-ul curent la 4095
                run_length++;
            }

            if (run_length < MAX_RUN_LENGTH_SMALL) {
                // run_length < 32, asa ca folosim un singur byte
                // Dupa shiftarea la stanga cu 6 biti a block_type, cei mai semnificativi 2 biti
                // vor deveni tipul blockului. Aplicam AND intre run_length si masca 00011111
                // pentru a pastra doar cei mai putin semnificativi 5 biti
                // Apoi concatenam valorile rezultate prin aplicarea operatiei OR
                encoded[index_encoded++] = (block_type << BLOCK_TYPE_SHIFT)
                | (run_length & SMALL_RUN_LENGTH_MASK);
            } else {
                // run_length >= 32, asa ca folosim doi bytes
                // Cei mai seminificativi 2 biti ai primului byte se obtin la fel ca mai sus
                // Shiftam la dreapta run_length cu 8 biti pentru a obtine cei mai semnificativi 4 biti
                // (Nu e nevoie sa aplicam AND cu masca 00001111 deoarece run_length < 4096)
                // Apoi concatenam valorile rezultate prin aplicarea operatiei OR,
                // alaturi de flag-ul LARGE_RUN_FLAG = 00100000, pentru ca
                // bitii 3 si 4 din primul byte sa fie 10
                encoded[index_encoded++] = (block_type << BLOCK_TYPE_SHIFT)
                | LARGE_RUN_FLAG
                | (run_length >> RUN_LENGTH_LARGE_SHIFT);
                // Al doilea byte va contine cei mai putin semnificativi 8 biti ai run_length
                // Trunchierea la cei mai putin semnificativi 8 biti se face automat in C
                encoded[index_encoded++] = run_length;
            }

            index += run_length;
        }

        free(flattened);  // Eliberam memoria alocata pentru chunkul aplatizat
        *length = index_encoded;

        return encoded;
}

char*** chunk_decode(
    unsigned char* code, int width, int height, int depth) {

        // Alocam memorie pentru chunk
        char*** chunk = (char***)malloc(width * sizeof(char**));
        for (int x = 0; x < width; x++) {
            chunk[x] = (char**)malloc(height * sizeof(char*));
            for (int y = 0; y < height; y++) {
                chunk[x][y] = (char*)malloc(depth * sizeof(char));
            }
        }

        int index_chunk = 0;
        int index_compressed = 0;
        int chunk_size = width * height * depth;

        // Decodificam datele comprimate
        while (index_chunk < chunk_size) {
            unsigned char first_byte = code[index_compressed++];
            // Extragem tipul blockului din cei mai semnificativi 2 biti ai primului byte
            unsigned char block_type = first_byte >> BLOCK_TYPE_SHIFT;
            int run_length = 0;

            // Daca al treilea bit din primul byte este 1, inseamna ca lungimea secventei de blockuri >= 32
            // Altfel, operatia AND va rezulta 0 (lungimea secventei va fi < 32)
            if (first_byte & LARGE_RUN_FLAG) {
                unsigned char second_byte = code[index_compressed++];
                // Preluam lungimea secventei prin concatenarea celor doi bytes cu operatia OR
                // Scapam de bitii irelevanti din primul byte prin operatia AND cu masca 00001111
                run_length = ((first_byte & LARGE_RUN_LENGTH_MASK) << RUN_LENGTH_LARGE_SHIFT) | second_byte;
            } else {
                // Preluam lungimea secventei din cei mai putin semnificativi 5 biti ai primului byte
                run_length = first_byte & SMALL_RUN_LENGTH_MASK;
            }

            // Umplem chunkul cu blockuri de tipul block_type, in ordinea y, z, x
            for (int i = 0; i < run_length; i++) {
                int x = index_chunk % width;
                int y = index_chunk / (width * depth);
                int z = (index_chunk / width) % depth;
                chunk[x][y][z] = (char)block_type;
                index_chunk++;
            }
        }

        return chunk;
}

