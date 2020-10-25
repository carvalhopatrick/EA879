#!/bin/bash
for f in *.lua
do
	echo "######### Comparando $f.out com $f ... #########"
	./lua-lexer < $f | diff outputs/$f.out -
	echo "######### Done ... ###########"
done