#!/bin/bash

bison lua-parser.y
if [ "$?" != "0" ]; then
    echo Error creating parser!
    exit 1
fi
flex lua-lexer.l
if [ "$?" != "0" ]; then
    echo Error creating scanner!
    exit 1
fi
gcc -o lua-parser lua-lexer.c lua-parser.tab.c lua-semantics.c -Wall -lm -std=gnu99
if [ "$?" != "0" ]; then
    echo Error creating executable!
    exit 1
fi

cd tests/

echo Testing hello.lua
../lua-parser hello.lua > hello.out
diff hello.out hello.ref

echo Testing numbers.lua
../lua-parser numbers.lua > numbers.out
diff numbers.out numbers.ref

echo Testing strings.lua
../lua-parser strings.lua > strings.out
diff strings.out strings.ref

echo Testing operators.lua
../lua-parser operators.lua > operators.out
diff operators.out operators.ref

echo Testing markov.lua
../lua-parser markov.lua > markov.out 2> markov.err
diff markov.out markov.ref
diff markov.err markov.errref

echo Testing controls.lua
../lua-parser controls.lua > controls.out 2> controls.err
diff controls.out controls.ref
diff controls.err controls.errref

for i in `seq 1 8`; do
    echo Testing sort.lua on input "$i"
    ../lua-parser sort.lua < "sort.in$i" > "sort.out$i"
    diff "sort.out$i" "sort.ref$i"
done

for i in `seq 1 9`; do
    echo Testing find_ascii_code.lua on input "$i"
    ../lua-parser find_ascii_code.lua < "find_ascii_code.in$i" > "find_ascii_code.out$i"
    diff "find_ascii_code.out$i" "find_ascii_code.ref$i"
done

echo done!
cd ../
