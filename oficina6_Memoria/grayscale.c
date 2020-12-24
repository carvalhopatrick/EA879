/* grayscale transforms for images */

#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "benchmark.h"

int get_byte() {
    int input = getchar();
    if (input == EOF) {
        fprintf(stderr, "unexpected EOF in byte input stream\n");
        exit(EXIT_FAILURE);
    }
    return input;
}

int get_word() {
    int input_most  = getchar();
    int input_least = getchar();
    if (input_most == EOF || input_least == EOF) {
        fprintf(stderr, "unexpected EOF in word input stream\n");
        exit(EXIT_FAILURE);
    }
    return (input_most << 8) | input_least;
}

void put_byte(int64_t output) {
    int status = putchar((int) output);
    if (status == EOF) {
        fprintf(stderr, "error writing to output stream\n");
        exit(EXIT_FAILURE);
    }
}

void put_word(int64_t output) {
    int status_most  = putchar((int) (output >> 8));
    int status_least = putchar((int) (output & 0xFF));
    if (status_most == EOF || status_least == EOF) {
        fprintf(stderr, "error writing to output stream\n");
        exit(EXIT_FAILURE);
    }
}

void put_qword(int64_t output) {
    put_word(output >> 16);
    put_word(output & 0xFFFF);
}


int main(int argc, char *argv[]) {
    int b = -1;
    if (argc > 1) {
        long arg;
        char *not_number_ptr;
        arg = strtol(argv[1], &not_number_ptr, 10);
        b = *not_number_ptr == '\0' && arg <= INT_MAX ? arg : -1;
    }

    if (b <= 0 || b > 32) {
        fprintf(stderr,
            "usage: grayscale <B> < input.ppm > output.pgm\n"
            "transforma a color PPM image into higher or lower-bit a grayscale "
            "PGM image\n"
            "B => bit-depth of the output, between 1 and 32 (usually 8).\n"
            "     Note: values above 16 create non-standard PGM files!\n");
        return EXIT_FAILURE;
    }

    benchmark_push();

    fprintf(stderr, "runtimes...\n");

    // Reads input image...
    // ...reads and parses header...
    const int magic_number_1 = get_byte();
    const int magic_number_2 = get_byte();
    if (magic_number_1!='P' || (magic_number_2!='5' && magic_number_2!='6')) {
        fprintf(stderr, "invalid file type in header\n");
        return EXIT_FAILURE;
    }
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
    int max_val;
    if (scanf("%d", &max_val) != 1 || max_val <= 0 || max_val >= 65536) {
        fprintf(stderr, "invalid max_val in header\n");
        return EXIT_FAILURE;
    }
    const int end_of_header = get_byte();
    if (!isspace(end_of_header)) {
        fprintf(stderr, "invalid character terminating header\n");
        return EXIT_FAILURE;
    }
    const int bytes = max_val < 256 ? 1 : 2;

    // Writes output header
    const int     out_magic_number_2 = '5';
    const int64_t out_bound_val      = 1L << b;
    const int64_t out_max_val        = out_bound_val - 1;
    const int64_t out_bytes          = b/8 + (b%8 != 0 ? 1 : 0);
    put_byte(magic_number_1);
    put_byte(out_magic_number_2);
    put_byte('\n');
    printf("%zu %zu\n", width, height);
    printf("%" PRId64, out_max_val);
    put_byte(end_of_header);

    int  (* const get_pixel)(void) = bytes == 1 ? get_byte : get_word;
    void (* const put_pixel)(int64_t) =  out_bytes == 1 ? put_byte :
                                        (out_bytes == 2 ? put_word : put_qword);

    // Inputs, processes, and outputs image
    const size_t pixels_n = height*width;

    // No colorspace consideration, just averages the colors
    // #define TRANSFER_TO_LINEAR(v) (v)
    // #define LINEAR_TO_TRANSFER(v) (v)
    // #define LUMINANCE_R_COEF (0.33333333f)
    // #define LUMINANCE_G_COEF (0.33333333f)
    // #define LUMINANCE_B_COEF (0.33333333f)

    // sRGB
    // #define TRANSFER_TO_LINEAR(v) ((v)<0.04045f ? \
    //                                (v)/12.92f : \
    //                                powf(((v)+0.055f)/1.055f, 2.4f))
    // #define LINEAR_TO_TRANSFER(v) ((v)<=0.0031308f ? \
    //                                (v)*12.92f : \
    //                                powf((v),1.0f/2.4f)*1.055f-0.055f)
    // #define LUMINANCE_R_COEF (0.21263901f)
    // #define LUMINANCE_G_COEF (0.71516868f)
    // #define LUMINANCE_B_COEF (0.07219231f)

    // ITU-R Recommendation BT. 709
    // #define TRANSFER_TO_LINEAR(v) ((v)<0.018f ? \
    //                                (v)/4.5f : \
    //                                powf(((v)+0.099f)/1.099f, 1.0f/0.45f))
    // #define LINEAR_TO_TRANSFER(v) ((v)<=0.081f ? \
    //                                (v)*4.5f : \
    //                                powf((v),0.45f)*1.099f-0.099f)
    // #define LUMINANCE_R_COEF (0.2126f)
    // #define LUMINANCE_G_COEF (0.7152f)
    // #define LUMINANCE_B_COEF (0.0722f)

    // ITU-R Recommendation BT. 601 + NTSC
    #define TRANSFER_TO_LINEAR(v) ((v)<0.018f ? \
                                   (v)/4.5f : \
                                   powf(((v)+0.099f)/1.099f, 1.0f/0.45f))
    #define LINEAR_TO_TRANSFER(v) ((v)<=0.081f ? \
                                   (v)*4.5f : \
                                   powf((v),0.45f)*1.099f-0.099f)
    #define LUMINANCE_R_COEF (0.299f)
    #define LUMINANCE_G_COEF (0.587f)
    #define LUMINANCE_B_COEF (0.114f)

    for (size_t data=0; data<pixels_n; data++) {
        float input_r = ((float) get_pixel()) / max_val;
        float input_g = ((float) get_pixel()) / max_val;
        float input_b = ((float) get_pixel()) / max_val;
        float linear_r = TRANSFER_TO_LINEAR(input_r);
        float linear_g = TRANSFER_TO_LINEAR(input_g);
        float linear_b = TRANSFER_TO_LINEAR(input_b);
        float linear_gray = LUMINANCE_R_COEF*linear_r +
                            LUMINANCE_G_COEF*linear_g +
                            LUMINANCE_B_COEF*linear_b;
        float transfer_gray = LINEAR_TO_TRANSFER(linear_gray);
        int64_t output = (int64_t) rintf(transfer_gray*out_max_val);
        output = output < 0           ? 0           : output;
        output = output > out_max_val ? out_max_val : output;
        put_pixel(output);
    }

    fprintf(stderr, "total      (ns): %.0lf\n", benchmark_pop());
    fprintf(stderr, "resolution (ns): %.0lf\n", benchmark_resolution());

    return EXIT_SUCCESS;
}

