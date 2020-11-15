/* lua-lexer.l: a simplified lexer for the language lua */

 /* --------- Opening code for generated lexer */

%{
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lua-semantics.h"
#include "lua-parser.tab.h"

// State for strings
static const int String_start_single = -1;
static const int String_start_double = -2;
static int string_start;

// State for comments
static bool start_of_comment = false;
static int comment_start = 0;
static int Comment_start_short = -1;

%}

 /* --------- Lexer declarations */

%option nodefault noyywrap noinput nounput
%option outfile="lua-lexer.c"

NAME           [A-Za-z_][A-Za-z0-9_]*
SINGLE_QUOTE   [']
DOUBLE_QUOTE   ["]
CODE_DECIMAL   [0-9]{1,3}
CODE_HEXA      [0-9A-Fa-f]{2}
OPEN_LONG      \[=*\[
CLOSE_LONG     \]=*\]
NUMBER         ([0-9]*\.?[0-9]+|[0-9]+\.?[0-9]*)([eE][-+]?[0-9]+)?
NUMBER_HEXA    0x[0-9A-Fa-f]+
DOUBLE_HYPHEN  --
WHITESPACE     [ \t]+

%x string_context long_string_context comment_context
%s error_recovery

%% /* --------- Lexer actions */

 /* --- Tokens with fixed contents should simply return the correct TokenType */

and         { yylval.keyword = AND;      return yylval.keyword; }
break       { yylval.keyword = BREAK;    return yylval.keyword; }
do          { yylval.keyword = DO;       return yylval.keyword; }
else        { yylval.keyword = ELSE;     return yylval.keyword; }
elseif      { yylval.keyword = ELSEIF;   return yylval.keyword; }
end         { yylval.keyword = END;      return yylval.keyword; }
false       { yylval.keyword = FALSE;    return yylval.keyword; }
for         { yylval.keyword = FOR;      return yylval.keyword; }
function    { yylval.keyword = FUNCTION; return yylval.keyword; }
if          { yylval.keyword = IF;       return yylval.keyword; }
in          { yylval.keyword = IN;       return yylval.keyword; }
"io.read()" { yylval.keyword = IOREAD;   return yylval.keyword; }
local       { yylval.keyword = LOCAL;    return yylval.keyword; }
nil         { yylval.keyword = NIL;      return yylval.keyword; }
not         { yylval.keyword = NOT;      return yylval.keyword; }
or          { yylval.keyword = OR;       return yylval.keyword; }
print       { yylval.keyword = PRINT;    return yylval.keyword; }
repeat      { yylval.keyword = REPEAT;   return yylval.keyword; }
return      { yylval.keyword = RETURN;   return yylval.keyword; }
then        { yylval.keyword = THEN;     return yylval.keyword; }
true        { yylval.keyword = TRUE;     return yylval.keyword; }
until       { yylval.keyword = UNTIL;    return yylval.keyword; }
while       { yylval.keyword = WHILE;    return yylval.keyword; }

"+"      { yylval.keyword = PLUS;   return yylval.keyword; }
"-"      { yylval.keyword = MINUS;  return yylval.keyword; }
"*"      { yylval.keyword = MULT;   return yylval.keyword; }
"/"      { yylval.keyword = DIV;    return yylval.keyword; }
"%"      { yylval.keyword = MOD;    return yylval.keyword; }
"^"      { yylval.keyword = POW;    return yylval.keyword; }
"=="     { yylval.keyword = EQUAL;  return yylval.keyword; }
"~="     { yylval.keyword = DIFF;   return yylval.keyword; }
"<="     { yylval.keyword = LE;     return yylval.keyword; }
">="     { yylval.keyword = GE;     return yylval.keyword; }
"<"      { yylval.keyword = LT;     return yylval.keyword; }
">"      { yylval.keyword = GT;     return yylval.keyword; }
"#"      { yylval.keyword = LEN;    return yylval.keyword; }
".."     { yylval.keyword = CONCAT; return yylval.keyword; }
"="      { yylval.keyword = SET;    return yylval.keyword; }

"("      return OPEN_PAR;
")"      return CLOSE_PAR;
"{"      return OPEN_CURLY;
"}"      return CLOSE_CURLY;
"["      return OPEN_BRA;
"]"      return CLOSE_BRA;

";"      return SEMICOLON;
":"      return COLON;
","      return COMMA;
"."      return DOT;
"..."    return ELLIPSIS;

 /* --- Tokens with variable contents with simple treatment */

{NAME} {
    yylval.name = check_alloc(strdup(yytext));
    return NAME;
}

{NUMBER}|{NUMBER_HEXA} {
    yylval.number = atof(yytext);
    return NUMBER;
}

 /* --- Strings */

{SINGLE_QUOTE} {
    string_reset();
    string_start = String_start_single;
    BEGIN(string_context);
}
{DOUBLE_QUOTE} {
    string_reset();
    string_start = String_start_double;
    BEGIN(string_context);
}
{OPEN_LONG} {
    string_reset();
    string_start = strlen(yytext);
    BEGIN(long_string_context);
}

<string_context>\\a   string_append("\a");
<string_context>\\b   string_append("\b");
<string_context>\\f   string_append("\f");
<string_context>\\n   string_append("\n");
<string_context>\\r   string_append("\r");
<string_context>\\t   string_append("\t");
<string_context>\\v   string_append("\v");
<string_context>\\["] string_append("\"");
<string_context>\\'   string_append("\'");
<string_context>\\\\  string_append("\\");
<string_context>\\\n  {
    yynewline();
    string_append("\n");
}
<string_context>\\{CODE_DECIMAL} {
    int decimal = atoi(yytext+1);
    if (decimal>255) {
        yyerror("decimal escape too large: %d", decimal);
        return YYUNDEF;
    }
    char decimal_str[2];
    decimal_str[0] = decimal;
    decimal_str[1] = '\0';
    string_append(decimal_str);
}
<string_context>\\x{CODE_HEXA} {
    long hexa = strtol(yytext+2, NULL, 16);
    char hexa_str[2];
    hexa_str[0] = hexa;
    hexa_str[1] = '\0';
    string_append(hexa_str);
}
<string_context>\\. {
    yyerror("invalid escape sequence: %s", yytext);
    return YYUNDEF;
}
<string_context>{SINGLE_QUOTE} {
    if (string_start == String_start_single) {
        yylval.string = string_buffer;
        BEGIN(INITIAL);
        return STRING;
    }
    string_append(yytext);
}
<string_context>{DOUBLE_QUOTE} {
    if (string_start == String_start_double) {
        yylval.string = string_buffer;
        BEGIN(INITIAL);
        return STRING;
    }
    string_append(yytext);
}
<string_context>\n {
    yynewline();
    yyerror("unterminated string");
    BEGIN(INITIAL);
    return YYUNDEF;
}
<string_context>. {
    string_append(yytext);
}
<string_context><<EOF>> {
    yyerror("unterminated string");
    BEGIN(INITIAL);
    return YYUNDEF;
}

<long_string_context>{CLOSE_LONG} {
    if (string_start == strlen(yytext)) {
        yylval.string = string_buffer;
        BEGIN(INITIAL);
        return STRING;
    }
    string_append(yytext);
}
<long_string_context>\n {
    yynewline();
    string_append("\n");
}
<long_string_context>. {
    string_append(yytext);
}
<long_string_context><<EOF>> {
    yyerror("unterminated string");
    BEGIN(INITIAL);
    return YYUNDEF;
}

 /* --- Comments */

{DOUBLE_HYPHEN} {
    BEGIN(comment_context);
    start_of_comment = true;
    comment_start = Comment_start_short;
}
<comment_context>{OPEN_LONG} {
    comment_start = start_of_comment ? strlen(yytext) : comment_start;
    start_of_comment = false;
}
<comment_context>{CLOSE_LONG} {
    if (comment_start == strlen(yytext)) {
        BEGIN(INITIAL);
    }
    start_of_comment = false;
}
<comment_context>\n {
    yynewline();
    if (comment_start == Comment_start_short) {
        BEGIN(INITIAL);
    }
    start_of_comment = false;
}
<comment_context>. {
    start_of_comment = false;
}
<comment_context><<EOF>> {
    if (comment_start == Comment_start_short) {
        BEGIN(INITIAL);
    }
    else {
        yyerror("unterminated comment");
        return YYEOF;
    }
    start_of_comment = false;
}

 /* --- Whitespace */

<error_recovery>\n {
    yynewline();
    return NEWLINE;
}

\n {
    yynewline();
}

{WHITESPACE} /* ignores white spaces */

 /* --- Everything else */

. {
    yyerror("unrecognized symbol: %s", yytext);
    return YYUNDEF;
}

%% /* --------- Closing code for generated lexer */

void scanner_set_interactive(bool interactive) {
    yy_set_interactive(interactive);
}

void scanner_start_recovery(void) {
    BEGIN(error_recovery);
}

void scanner_end_recovery(void) {
    BEGIN(INITIAL);
}
