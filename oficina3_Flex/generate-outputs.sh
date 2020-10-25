#!/bin/bash
for f in *.lua
do	
	echo "Criando arquivo out para $f ..."
	./lua-lexer < $f > outputs/$f.out
done