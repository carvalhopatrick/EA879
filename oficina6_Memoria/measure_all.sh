#!/bin/bash
for (( replica=1; replica<=3; replica++ )); do
    for (( bits=8; bits<=29; bits++ )); do
        for imagem in noise carpet arctic; do
            for programa in change-light change-light-lut; do
                ./${programa} 0.8 -0.2 0.1 < ${imagem}-${bits}.pgm > temp.pgm 2> out
                time=$(grep processing < out | cut -d : -f 2 | tr -d ' ')
                echo $programa,$imagem-$bits,$replica,$time
                rm temp.pgm out
            done
        done
    done
done

