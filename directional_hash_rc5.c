#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <omp.h>
#include <unistd.h>
#include <time.h>

#define CHUNK_SIZE 512

typedef struct {
    int r, c;
    int weight;
} CoordWeight;

// Forward declaration of to_grid
void to_grid(uint8_t byte, char grid[3][3]);
// Precompute weighted patterns for all 256 byte values
uint8_t precomputed_coords[256][8][2];

// Precompute weighted patterns
void precompute_weighted_patterns() {
    for (int byte = 0; byte < 256; byte++) {
        char grid[3][3];
        to_grid(byte, grid);

        CoordWeight coords[9];
        int count = 0;

        int position_bias[3][3] = {
            {3, 2, 3},
            {2, 4, 2},
            {3, 2, 3}
        };

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (grid[r][c] != '\0') {
                    coords[count].r = r;
                    coords[count].c = c;
                    coords[count].weight = (grid[r][c] == '1' ? 10 : 5) + position_bias[r][c];
                    count++;
                }
            }
        }

        // Sort by weight descending
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (coords[j].weight > coords[i].weight) {
                    CoordWeight temp = coords[i];
                    coords[i] = coords[j];
                    coords[j] = temp;
                }
            }
        }

        for (int i = 0; i < 8; i++) {
            precomputed_coords[byte][i][0] = (i < count) ? coords[i].r : -1;  // Prevent out of bounds
            precomputed_coords[byte][i][1] = (i < count) ? coords[i].c : -1;  // Prevent out of bounds
        }
    }
}

// Converts a byte to a 3x3 grid with 1 blank (None)
void to_grid(uint8_t byte, char grid[3][3]) {
    char bits[9] = {0};
    for (int i = 0; i < 8; i++) {
        bits[i] = ((byte >> (7 - i)) & 1) + '0';
    }
    bits[8] = '\0';

    int k = 0;
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (k < 8)
                grid[r][c] = bits[k++];
            else
                grid[r][c] = '\0'; // blank
        }
    }
}

// Generate a seed based on weighted pattern modulation
int generate_shift_seed(uint8_t byte, uint8_t prev, uint8_t next) {
    // Example: Combine byte, previous byte, and next byte into a single value
    return (byte + prev + next) % 9; // Seed within the 0-8 range for index shifting
}

// Shuffle the coordinates in a non-linear, weighted way based on the seed
void shuffle_grid_coords(const uint8_t original_coords[8][2], int seed, uint8_t shuffled_coords[8][2]) {
    // Simple seed-based rotation of the precomputed coordinates
    for (int i = 0; i < 8; i++) {
        int new_index = (i + seed) % 8;
        shuffled_coords[i][0] = original_coords[new_index][0];
        shuffled_coords[i][1] = original_coords[new_index][1];
    }
}

// Processes a byte with the new shuffled pattern
void process_byte(uint8_t byte, char* out, uint8_t prev, uint8_t next) {
    char grid[3][3];
    to_grid(byte, grid);

    // Generate a seed based on the neighboring bytes
    int seed = generate_shift_seed(byte, prev, next);

    // Shuffle the precomputed coordinates based on the seed
    uint8_t shuffled_coords[8][2];
    shuffle_grid_coords(precomputed_coords[byte], seed, shuffled_coords);

    int idx = 0;
    // Flatten the grid using the shuffled coordinate order
    for (int i = 0; i < 8; i++) {
        int r = shuffled_coords[i][0];
        int c = shuffled_coords[i][1];
        if (r >= 0 && r < 3 && c >= 0 && c < 3 && grid[r][c] != '\0') {
            out[idx++] = grid[r][c];
        }
    }
    out[idx] = '\0'; // Null-terminate the output string
}

void directional_hash_file(const char* filename, int bits, int chunk_size, int max_workers) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = NULL;

    switch (bits) {
        case 256:
            md = EVP_sha256(); break;
        case 512:
            md = EVP_sha512(); break;
        case 1024:
        case 2048:
            md = EVP_shake256(); break;
        default:
            fprintf(stderr, "Unsupported bit size: %d\n", bits);
            fclose(file);
            return;
    }

    EVP_DigestInit_ex(ctx, md, NULL);

    uint8_t buffer[chunk_size];
    size_t read;
    char* bin = malloc(33 * chunk_size * sizeof(char));
    size_t bin_index = 0;

    if (!bin) {
        perror("Failed to allocate memory for bin");
        EVP_MD_CTX_free(ctx);
        fclose(file);
        return;
    }

    uint8_t prev_byte = 0;
    uint8_t next_byte = 0;

    while ((read = fread(buffer, 1, chunk_size, file)) > 0) {
        // Allocate thread-local buffers outside parallel region
        char** processed_array = malloc(read * sizeof(char*));

#define MAX_BINARY_LENGTH_PER_BYTE 9 // 8 bits + null
#pragma omp parallel for num_threads(max_workers)
        for (size_t j = 0; j < read; j++) {
            processed_array[j] = malloc(MAX_BINARY_LENGTH_PER_BYTE);
            //processed_array[j] = malloc(1024); // allocate per thread
            processed_array[j][0] = '\0';

            // Determine previous and next bytes
            uint8_t prev = (j > 0) ? buffer[j - 1] : prev_byte;
            uint8_t next = (j < read - 1) ? buffer[j + 1] : next_byte;

            process_byte(buffer[j], processed_array[j], prev, next);
        }

        //bin[0] = '\0';
        //for (size_t j = 0; j < read; j++) {
        //    strcat(bin, processed_array[j]);
        //    free(processed_array[j]);
        //}
        //free(processed_array);
        size_t offset = 0;
        for (size_t j = 0; j < read; j++) {
            size_t len = strlen(processed_array[j]);
            memcpy(bin + offset, processed_array[j], len);
            offset += len;
        }
        bin[offset] = '\0';
        free(processed_array);

        size_t len = strlen(bin);
        size_t byte_len = (len + 7) / 8;
        uint8_t* byte_buf = malloc(byte_len);
        memset(byte_buf, 0, byte_len);

        for (size_t i = 0; i < len; i++) {
            if (bin[i] == '1') {
                byte_buf[i / 8] |= 1 << (7 - (i % 8));
            }
        }

        EVP_DigestUpdate(ctx, byte_buf, byte_len);
        free(byte_buf);
        bin[0] = '\0';

        // Store the last byte as previous for the next chunk
        if (read > 0) {
            prev_byte = buffer[read - 1];
        }

        // Attempt to peek at the next byte for the next iteration's first byte
        long current_pos = ftell(file);
        if (fread(&next_byte, 1, 1, file) != 1) {
            next_byte = 0; // End of file
        }
        fseek(file, current_pos, SEEK_SET); // Reset file pointer
    }

    size_t hash_output_size = bits / 8;
    unsigned char hash[EVP_MAX_MD_SIZE * 4];

    if (bits == 1024 || bits == 2048) {
        EVP_DigestFinalXOF(ctx, hash, hash_output_size);
    } else {
        unsigned int hash_len = 0;
        EVP_DigestFinal_ex(ctx, hash, &hash_len);
        hash_output_size = (size_t)hash_len;
    }

    for (size_t i = 0; i < hash_output_size; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    free(bin);
    EVP_MD_CTX_free(ctx);
    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [bits=256|512|1024|2048] [chunk_size=8192] [max_workers=4] [--time]\n", argv[0]);
        return 1;
    }

    int bits = 256;
    int chunk_size = 512;
    int max_workers = 4;
    int time_flag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--time") == 0) {
            time_flag = 1;
        }
    }

    if (argc > 2) bits = atoi(argv[2]);
    if (argc > 3) chunk_size = atoi(argv[3]);
    if (argc > 4) max_workers = atoi(argv[4]);

    // Precompute all weight orders
    precompute_weighted_patterns();

    struct timespec start, end;
    if (time_flag) {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    directional_hash_file(argv[1], bits, chunk_size, max_workers);

    if (time_flag) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Time elapsed: %.6f seconds\n", elapsed);
    }
    return 0;
}