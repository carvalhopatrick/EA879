/* lua.l: a simplified lexer for the language lua */

 /* --------- Opening code for generated lexer */

%{
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum TokenType {
    END_OF_INPUT = 0,
    // reserved words
    AND, BREAK, DO, ELSE, ELSEIF, END, FALSE, FOR, FUNCTION, IF, IN, LOCAL, NIL,
    NOT, OR, REPEAT, RETURN, THEN, TRUE, UNTIL, WHILE,
    // operators
    PLUS, MINUS, MULT, DIV, MOD, EXP,
    EQUAL, DIFF, LE, GE, LT, GT, LEN, CONCAT, SET,
    // brackets
    OPEN_PAR, CLOSE_PAR, OPEN_CURLY, CLOSE_CURLY, OPEN_BRA, CLOSE_BRA,
    // delimiters
    SEMICOLON, COLON, COMMA, DOT, ELLIPSIS,
    // tokens with variable contents
    IDENTIFIER, STRING, NUMBER
} TokenType;

int line_number  = 0;

// Variables to keep state and data for strings
int string_start = 0;
const int String_start_single = -1;
const int String_start_double = -2;
const size_t Max_string_size = 1024*1024;
size_t string_size;
char *string_buffer;

void start_string(int delimiter);
void accumulate_string(const char *content);
void stop_string(void);


// Variables to keep state for comments
bool start_of_comment = false;
int comment_start = 0;
int Comment_start_short = -1;

void lex_error(const char *message, ...);

%}

 /* --------- Lexer declarations */

%option nodefault noyywrap
%option outfile="lua-lexer.c"

IDENTIFIER      [_a-zA-Z][_a-zA-Z0-9]*
ESCAPE          \\[abftvrn\\"'\[\]]
ESCAPE_NEWLINE  \\\n
SINGLE_QUOTE    \'
DOUBLE_QUOTE    \"
OPEN_LONG       \[[=]*\[
CLOSE_LONG      \][=]*\]
NUMBER          -?([0-9]+\.[0-9]+|[0-9]+\.?|\.[0-9]+)([eE][-+]?[0-9]+)?
NUMBER_HEXA     -?0[xX][a-fA-F0-9]+
DOUBLE_HYPHEN   --
WHITESPACE      [ \t\v]+
/* " */

%x string_context comment_context long_comment_context short_comment_context

%% /* --------- Lexer actions */

 /* --- Tokens with fixed contents should simply return the correct TokenType */

and      return AND;
break    return BREAK;
do       return DO;
else     return ELSE;
elseif   return ELSEIF;
end      return END;
false    return FALSE;
for      return FOR;
function return FUNCTION;
if       return IF;
in       return IN;
local    return LOCAL;
nil      return NIL;
not      return NOT;
or       return OR;
repeat   return REPEAT;
return   return RETURN;
then     return THEN;
true     return TRUE;
until    return UNTIL;
while    return WHILE;

"+"      return PLUS;
"-"      return MINUS;
"*"      return MULT;
"/"      return DIV;
"%"      return MOD;
"^"      return EXP;
"=="     return EQUAL;
"~="     return DIFF;
"<="     return LE;
">="     return GE;
"<"      return LT;
">"      return GT;
"#"      return LEN;
".."     return CONCAT;
"="      return SET;

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

{IDENTIFIER} return IDENTIFIER;

 /* --- Numbers */

{NUMBER}      return NUMBER;
{NUMBER_HEXA} return NUMBER;

 /* --- Strings */

{SINGLE_QUOTE} {
    start_string(String_start_single);
    BEGIN(string_context);
}
{DOUBLE_QUOTE} {
    start_string(String_start_double);
    BEGIN(string_context);
}
{OPEN_LONG} {
    start_string(strlen(yytext));
    BEGIN(string_context);
}

<string_context>{ESCAPE} {
    accumulate_string(yytext);
}
<string_context>{ESCAPE_NEWLINE} {
    line_number++;
    accumulate_string("\\n");
}
<string_context>{SINGLE_QUOTE} {
    if (string_start == String_start_single) {
        BEGIN(INITIAL);
        stop_string();
        return STRING;
    }
    else {
        accumulate_string(yytext);
    }
}
<string_context>{DOUBLE_QUOTE} {
    if (string_start == String_start_double) {
        BEGIN(INITIAL);
        stop_string();
        return STRING;
    }
    else {
        accumulate_string(yytext);
    }
}
<string_context>{CLOSE_LONG} {
    if (string_start == strlen(yytext)) {
        BEGIN(INITIAL);
        stop_string();
        return STRING;
    }
    else {
        accumulate_string(yytext);
    }
}
<string_context>\n {
    if (string_start >= 0) {
        accumulate_string("\\n");
        line_number++;
    }
    else {
        lex_error("unterminated string");
    }
    
}
<string_context>\\. {
    lex_error("invalid escape sequence: %s", yytext);
}
<string_context>. {
    accumulate_string(yytext);
}
<string_context><<EOF>> {
    lex_error("unterminated string");
}

 /* --- Comments */

{DOUBLE_HYPHEN} {
    BEGIN(comment_context);
    start_of_comment = true;
    comment_start = Comment_start_short;
}
<comment_context>{OPEN_LONG} {
    if (start_of_comment) {
        comment_start = (strlen(yytext) - 1);
    }
    start_of_comment = false;
}
<comment_context>{CLOSE_LONG} {
    start_of_comment = false;
    if ((strlen(yytext) - 1) == comment_start) {
        // end comment??
        BEGIN(INITIAL);
    }
}
<comment_context>\n { 
    start_of_comment = false;
    if (comment_start <= 0){
        // end comment??
        line_number++;
        BEGIN(INITIAL);
    } else {
        line_number++;
    }
}
<comment_context>. {
    start_of_comment = false;
}
<comment_context><<EOF>> {
    if (comment_start > 0){
        lex_error("unterminated long comment");
    } else {
        return END_OF_INPUT;
    }
}

 /* --- Whitespace */

\n    line_number++;

{WHITESPACE} /* não faz nada */ 

 /* --- Everything else */

. {
    lex_error("unrecognized symbol: %s", yytext);
}

%% /* --------- Closing code for generated lexer */

void start_string(int delimiter) {
    string_start = delimiter;
    string_size = 0;
    strcpy(string_buffer, "");
    accumulate_string(yytext);
}

void accumulate_string(const char *content) {
    size_t content_size = strlen(content);
    if (string_size+content_size > Max_string_size) {
        lex_error("maximum string size exceeded");
    }
    strcat(string_buffer, content);
    string_size += content_size;
}

void stop_string(void) {
    accumulate_string(yytext);
}

void lex_error(const char *message, ...) {
    const size_t Max_message_size = 1024;
    char formatted_message[Max_message_size];

    va_list arglist;
    va_start(arglist, message);
    vsnprintf(formatted_message, Max_message_size, message, arglist);

    fprintf(stderr, "error in line %d - %s\n", line_number, formatted_message);
    va_end(arglist);
    exit(EXIT_FAILURE);
}

int main(void) {
    /* Initialize variables */
    string_buffer = malloc(Max_string_size+1);
    if (string_buffer == NULL) {
        lex_error("not enough memory");
    }
    line_number = 1;

    /* Reads and prints all tokens */
    TokenType token_type;
    do {
        /* Prints basic info about the token */
        token_type = yylex();
        printf("line %d - token: %d - content: %s\n", line_number,
               (int) token_type, yytext);

        /* Prints contents of string, cut to fit in line */
        if (token_type == STRING) {
            const size_t Max_line_width = 100;
            char line[Max_line_width+1];
            strcpy(line, "");
            char *message = "    string content: ";
            size_t printed = 0;
            while (true) {
                printed += strlen(line);
                if (printed >= string_size) {
                    break;
                }
                const int Line_width = Max_line_width - strlen(message);
                strncpy(line, string_buffer+printed, Line_width);
                line[Line_width] = '\0';
                printf("%s%s\n", message, line);
                message = "    ";
            }
        }
    }
    while (token_type != END_OF_INPUT);

    return EXIT_SUCCESS;
}
