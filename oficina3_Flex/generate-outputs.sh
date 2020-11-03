#!/bin/bash
for f in lua-examples/*.lua
do	
	echo "Criando arquivo out para $f ..."
	new="$(echo $f | perl -pe 's/(.+\/)(.+)\.lua/\2\.out/')"
	echo outputs/$new
	./lua-lexer < $f > outputs/$new
done