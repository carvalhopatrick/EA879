#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

int main() {
	char yytext[10];
	scanf("%s", yytext);
	printf("entrada:\t%s\n", yytext);

  	char* strAscii;
    strAscii = malloc((strlen(yytext)-1) * sizeof(char));
    for (int i=1; i<strlen(yytext); i++) {
        strAscii[i-1] = yytext[i];
    }
	printf("strAscii:\t%s\n", strAscii);

    int ascii = atoi(strAscii);
	printf("ascii: \t%c\t%d\n", ascii, ascii);

	char c[2];
	c[0] = (char)ascii;
	c[1] = '\0';
	printf("accumulate:\t %s", c);
	return 0;
}