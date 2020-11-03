#!/bin/bash
for f in lua-examples/*.lua
do
	new="$(echo $f | perl -pe 's/(.+\/)(.+)\.lua/\2\.out/')"
	echo "####### Comparando saida de $f com reference-outputs/$new" ... 
	./lua-lexer < $f | diff reference-outputs/$new -
	echo "####### Done ... ###########"
done