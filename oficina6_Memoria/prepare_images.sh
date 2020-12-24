#!/bin/bash
for imagem in carpet arctic; do
    for (( bits=8; bits<=29; bits++ )); do
        echo Preparing ${imagem}-$bits.pgm
        ./grayscale $bits < ${imagem}.ppm > ${imagem}-$bits.pgm
    done
done

for (( bits=8; bits<=29; bits++ )); do
    echo Preparing noise-$bits.pgm
    ./noisetoppm 4096 2160 13 | ./grayscale $bits > noise-$bits.pgm
done
