/* grayscale transforms for images */

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void put_byte(int output) {
    int status = putchar(output);
    if (status == EOF) {
        fprintf(stderr, "error writing to output stream\n");
        exit(EXIT_FAILURE);
    }
}

static uint32_t pseudo_random_state[2];
static uint32_t pseudo_random_next(void);

#define MAX_SIZE (8192)

int main(int argc, char *argv[]) {
    int w = -1, h = -1;
    int64_t s = -1;
    if (argc > 3) {
        long arg;
        char *not_number_ptr;
        arg = strtol(argv[1], &not_number_ptr, 10);
        w = *not_number_ptr == '\0' && arg <= INT_MAX ? arg : -1;
        arg = strtol(argv[2], &not_number_ptr, 10);
        h = *not_number_ptr == '\0' && arg <= INT_MAX ? arg : -1;
        arg = strtol(argv[3], &not_number_ptr, 10);
        s = *not_number_ptr == '\0' ? arg : -1;
    }

    if (w <= 0 || w > MAX_SIZE || h <= 0 || h > MAX_SIZE || s < 0) {
        fprintf(stderr,
            "usage: noisetoppm <W> <H> <S> > output.ppm\n"
            "creates a PPM image from pseudo-random noise\n"
            "W => image width, from 1 to %d;\n"
            "H => image height, from 1 to %d;\n"
            "S => random generator seed.\n",
            (int) MAX_SIZE, (int) MAX_SIZE);
        return EXIT_FAILURE;
    }

    pseudo_random_state[0] = s & 0xFFFFFFFF;
    pseudo_random_state[1] = s >> 32;

    // Reads input image...
    // ...reads and parses header...
    const int magic_number_1 = 'P';
    const int magic_number_2 = '6';
    const size_t width = w;
    const size_t height = h;
    const int max_val = 255;
    const int end_of_header = '\n';

    // Writes output header
    put_byte(magic_number_1);
    put_byte(magic_number_2);
    put_byte('\n');
    printf("%zu %zu\n", width, height);
    printf("%d", max_val);
    put_byte(end_of_header);

    // Outputs image
    const size_t pixels_n = height*width;
    for (size_t data=0; data<pixels_n; data++) {
        uint32_t random_pixel = pseudo_random_next();
        put_byte( random_pixel        & 0xFF); // Red
        put_byte((random_pixel >>  8) & 0xFF); // Green
        put_byte((random_pixel >> 16) & 0xFF); // Blue
    }

    return EXIT_SUCCESS;
}


/* Code from Blackman and Vigna's shoshiro64** */

static inline uint32_t rotl(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

static uint32_t pseudo_random_next(void) {
    const uint32_t s0 = pseudo_random_state[0];
          uint32_t s1 = pseudo_random_state[1];
    const uint32_t result = rotl(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
    pseudo_random_state[0] = rotl(s0, 26) ^ s1 ^ (s1 << 9); // a, b
    pseudo_random_state[1] = rotl(s1, 13); // c
    return result;
}
