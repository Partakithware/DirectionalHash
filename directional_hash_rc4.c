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

// Flattens the grid using the precomputed weighted order
void flatten(char grid[3][3], char* out, uint8_t byte) {
    int idx = 0;
    for (int i = 0; i < 8; i++) {
        int r = precomputed_coords[byte][i][0];
        int c = precomputed_coords[byte][i][1];
        if (r >= 0 && r < 3 && c >= 0 && c < 3 && grid[r][c] != '\0') {
            out[idx++] = grid[r][c];
        }
    }
    out[idx] = '\0';
}

// Processes a byte with precomputed weighted read pattern
void process_byte(uint8_t byte, char* out) {
    char grid[3][3];
    to_grid(byte, grid);

    flatten(grid, out, byte);
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
        case 256: md = EVP_sha256(); break;
        case 512: md = EVP_sha512(); break;
        case 1024:
        case 2048: md = EVP_shake256(); break;
        default:
            fprintf(stderr, "Unsupported bit size: %d\n", bits);
            fclose(file);
            return;
    }

    EVP_DigestInit_ex(ctx, md, NULL);

    uint8_t buffer[chunk_size];
    size_t read;

    // Allocate a large buffer once (size could be scaled up safely)
    char* bin = malloc(33 * chunk_size * sizeof(char));
    if (!bin) {
        perror("Failed to allocate memory for bin");
        EVP_MD_CTX_free(ctx);
        fclose(file);
        return;
    }

    while ((read = fread(buffer, 1, chunk_size, file)) > 0) {
        // Thread output buffers
        char** processed_array = malloc(read * sizeof(char*));
        if (!processed_array) {
            perror("Failed to allocate processed_array");
            break;
        }

#pragma omp parallel num_threads(max_workers)
        {
            char thread_buf[64]; // Big enough for any processed output
#pragma omp for
            for (size_t j = 0; j < read; j++) {
                process_byte(buffer[j], thread_buf);
                size_t len = strlen(thread_buf);
                processed_array[j] = malloc(len + 1);
                if (processed_array[j]) {
                    memcpy(processed_array[j], thread_buf, len + 1);
                }
            }
        }

        // Efficient direct memory copy to bin buffer
        size_t offset = 0;
        for (size_t j = 0; j < read; j++) {
            if (processed_array[j]) {
                size_t len = strlen(processed_array[j]);
                memcpy(bin + offset, processed_array[j], len);
                offset += len;
                free(processed_array[j]);
            }
        }
        bin[offset] = '\0';
        free(processed_array);

        EVP_DigestUpdate(ctx, bin, offset);
    }

    unsigned char hash_output[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    EVP_DigestFinal_ex(ctx, hash_output, &hash_len);

    for (unsigned int i = 0; i < hash_len; i++) {
        printf("%02x", hash_output[i]);
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
