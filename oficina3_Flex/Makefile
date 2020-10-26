build_lexer:
	flex lua-lexer.l
	gcc-7 lua-lexer.c -o lua-lexer

generate-outputs:
	bash generate-outputs.sh

compare-outputs:
	bash compare-outputs.sh

compare-outputs-professor:
	bash compare-outputs-professor.sh
	