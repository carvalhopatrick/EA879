%{
  #include <stdbool.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include "arithmetic.tab.h"
  bool lex_in_error = false;
%}

%option noyywrap noinput nounput

NUMBER         [0-9]+
IDENTIFIER     [A-Za-z_][A-Za-z0-9_]*
RESERVED       [-+/*%()=?:&|~^;]
WHITESPACE     [ \t\r]+

%%

dir          return DIR;
exit         return EXIT;
"=="         return EQ;
"!="         return NE;
">"          return GT;
">="         return GE;
"<"          return LT;
"<="         return LE;
{IDENTIFIER} { yylval.IDENTIFIER = strdup(yytext); return IDENTIFIER; }
{NUMBER}     { yylval.NUMBER = atoi(yytext); return NUMBER; }
{RESERVED}   return yytext[0];
{WHITESPACE} /* ignores white spaces */
\n           { lex_in_error = false; return '\n'; }
.            { if (!lex_in_error) {
                   fprintf(stderr, "unrecognized symbol: %s\n", yytext);
                   lex_in_error = true;
               }
               return YYUNDEF;
             }
