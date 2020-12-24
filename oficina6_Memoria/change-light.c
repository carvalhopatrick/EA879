/* grayscale transforms for images */

#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "benchmark.h"


static inline uint32_t get_byte() {
    int input = getchar();
    if (input == EOF) {
        fprintf(stderr, "unexpected EOF in byte input stream\n");
        exit(EXIT_FAILURE);
    }
    return (uint32_t) input;
}

static inline uint32_t get_word() {
    int input_most  = getchar();
    int input_least = getchar();
    if (input_most == EOF || input_least == EOF) {
        fprintf(stderr, "unexpected EOF in word input stream\n");
        exit(EXIT_FAILURE);
    }
    return (uint32_t) (input_most << 8) | input_least;
}

static inline uint32_t get_qword() {
    uint32_t input_most  = get_word();
    uint32_t input_least = get_word();
    return (input_most << 16) | input_least;
}


static inline void put_byte(uint32_t output) {
    int status = putchar((int) output);
    if (status == EOF) {
        fprintf(stderr, "error writing to output stream\n");
        exit(EXIT_FAILURE);
    }
}

static inline void put_word(uint32_t output) {
    int status_most  = putchar((int) output >> 8);
    int status_least = putchar((int) output & 0xFF);
    if (status_most == EOF || status_least == EOF) {
        fprintf(stderr, "error writing to output stream\n");
        exit(EXIT_FAILURE);
    }
}

static inline void put_qword(uint32_t output) {
    put_word(output >> 16);
    put_word(output & 0xFFFF);
}


int main(int argc, char *argv[]) {
    float g = -2.0f, c = -2.0f, b = -2.0f;
    if (argc > 3) {
        float arg;
        char *not_number_ptr;
        arg = strtod(argv[1], &not_number_ptr);
        g = *not_number_ptr == '\0' ? arg : -2.0f;
        arg = strtod(argv[2], &not_number_ptr);
        c = *not_number_ptr == '\0' ? arg : -2.0f;
        arg = strtod(argv[3], &not_number_ptr);
        b = *not_number_ptr == '\0' ? arg : -2.0f;
    }

    if (!(g > 0.0 && c >= -1.0 && c <= 1.0 && b >= -1.0 && b <= 1.0)) {
        fprintf(stderr,
            "usage: change-light <G> <C> <B> < input.pgm > output.pgm\n"
            "applies change-light transformations to PGM/PPM image:\n"
            "G => gamma, greater than 0.0 (1.0 = no change);\n"
            "C => contrast, from -1.0 to 1.0 (0.0 = no change);\n"
            "B => brightness, from -1.0 to 1.0 (0.0 => no change);\n"
            "The operations are applied in that order. If the image has more\n"
            "than one channel (e.g., PPM), the transformations apply to the\n"
            "channels independently.\n");
        return EXIT_FAILURE;
    }

    benchmark_push();

    fprintf(stderr, "runtimes...\n");

    // Reads input image...
    benchmark_push();
    // ...reads and parses header...
    const int magic_number_1 = get_byte();
    const int magic_number_2 = get_byte();
    if (magic_number_1!='P' || (magic_number_2!='5' && magic_number_2!='6')) {
        fprintf(stderr, "invalid file type in header\n");
        return EXIT_FAILURE;
    }
    size_t channels = magic_number_2=='5' ? 1 : 3;
    size_t width;
    if (scanf("%zu", &width) != 1 || width <= 0) {
        fprintf(stderr, "invalid width in header\n");
        return EXIT_FAILURE;
    }
    size_t height;
    if (scanf("%zu", &height) != 1 || height <= 0) {
        fprintf(stderr, "invalid width in header\n");
        return EXIT_FAILURE;
    }
    long max_val;
    if (scanf("%ld", &max_val) != 1 || max_val <= 0 || max_val >= 4294967296) {
        fprintf(stderr, "invalid max_val in header\n");
        return EXIT_FAILURE;
    }
    int end_of_header = get_byte();
    if (!isspace(end_of_header)) {
        fprintf(stderr, "invalid character terminating header\n");
        return EXIT_FAILURE;
    }
    int bytes = max_val < 256 ? 1 : (max_val < 65536 ? 2 : 4);
    uint32_t (*get_pixel)(void);
    void     (*put_pixel)(uint32_t);
    if (bytes == 1) {
        get_pixel = get_byte;
        put_pixel = put_byte;
    }
    else if (bytes == 2) {
        get_pixel = get_word;
        put_pixel = put_word;
    }
    else {
        get_pixel = get_qword;
        put_pixel = put_qword;
    }
    // ..allocates memory in the heap
    const size_t subpixels_n = height*width*channels;
    uint32_t *image = malloc(subpixels_n*sizeof(uint32_t));
    if (image == NULL) {
        fprintf(stderr, "not enough memory\n");
        return EXIT_FAILURE;
    }
    // ..reads input pixels
    uint32_t *subpixels_ptr = image;
    for (size_t row=0; row<height; row++) {
        for (size_t col=0; col<width; col++) {
            for (size_t channel=0; channel<channels; channel++) {
                *subpixels_ptr = get_pixel();
                subpixels_ptr++;
            }
        }
    }
    fprintf(stderr, "input      (ns): %.0lf\n", benchmark_pop());

    // Processes image
    benchmark_push();
    subpixels_ptr = image;
    float max_val_f = max_val;
    for (size_t row=0; row<height; row++) {
        for (size_t col=0; col<width; col++) {
            for (size_t channel=0; channel<channels; channel++) {
                float input_f    = ((float) *subpixels_ptr)/max_val;
                float gamma      = powf(input_f, g);
                float contrast   = gamma*(1.0f+c);
                float brightness = contrast+b;
                float output_f   = rintf(brightness*max_val);
                output_f = output_f < 0.0f      ? 0.0f      : output_f;
                output_f = output_f > max_val_f ? max_val_f : output_f;
                uint32_t output  = (uint32_t) output_f;
                *subpixels_ptr = output;
                subpixels_ptr++;
            }
        }
    }
    fprintf(stderr, "processing (ns): %.0lf\n", benchmark_pop());

    // Writes output image...
    benchmark_push();
    // ...writes header
    put_byte(magic_number_1);
    put_byte(magic_number_2);
    put_byte('\n');
    printf("%zu %zu\n", width, height);
    printf("%ld", max_val);
    put_byte(end_of_header);
    // ..writes output pixels
    subpixels_ptr = image;
    for (size_t row=0; row<height; row++) {
        for (size_t col=0; col<width; col++) {
            for (size_t channel=0; channel<channels; channel++) {
                put_pixel(*subpixels_ptr);
                subpixels_ptr++;
            }
        }
    }
    fprintf(stderr, "output     (ns): %.0lf\n", benchmark_pop());
    fprintf(stderr, "total      (ns): %.0lf\n", benchmark_pop());
    fprintf(stderr, "resolution (ns): %.0lf\n", benchmark_resolution());

    return EXIT_SUCCESS;
}
