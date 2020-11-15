%{
  #include <stdbool.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  int yylex(void);
  void yyerror(const char *s);

  bool check_zero(int value);

  // Symbol table
  #define MAX_SYMBOLS 1024
  const char *symbol_names[MAX_SYMBOLS];
  int symbol_values[MAX_SYMBOLS];
  bool get_symbol(const char *symbol_name, int *symbol_value);
  void set_symbol(const char *symbol_name, int symbol_value);
  void print_symbols(void);
%}

%define api.value.type union

%token DIR
%token EXIT

%token <char *> IDENTIFIER;
%destructor { free($$); } <char *>;

%token <int> NUMBER
%nterm <int> exp
%destructor { } <int>;

%destructor { } <>;

%nonassoc ':'
%nonassoc '?'
%left '|' '^'
%left '&'
%left '~'
%nonassoc EQ NE GT GE LT LE
%left '+' '-'
%left '*' '/' '%'
%precedence UMINUS

%%

statements:
      statement statements
    | statement
    ;

statement:
      exp eol                { printf("%d\n", $exp);          }
    | IDENTIFIER '=' exp eol { set_symbol($IDENTIFIER, $exp); }
    | DIR eol                { print_symbols();               }
    | EXIT eol               { exit(EXIT_SUCCESS);            }
    | error eol              { yyerrok;                       }
    | eol
    ;

eol: '\n' | ';';

exp:
      exp '+' exp            { $$ = $1 + $3; }
    | exp '-' exp            { $$ = $1 - $3; }
    | exp '*' exp            { $$ = $1 * $3; }
    | '-' exp  %prec UMINUS  { $$ = -$2;     }
    ;

exp:
      exp '/' exp { if (check_zero($3)) { YYERROR; } else { $$ = $1 / $3; } }
    | exp '%' exp { if (check_zero($3)) { YYERROR; } else { $$ = $1 % $3; } }
    ;

exp:
      exp '|' exp { $$ = $1 | $3; }
    | exp '^' exp { $$ = $1 ^ $3; }
    | exp '&' exp { $$ = $1 & $3; }
    | '~' exp     { $$ = ~$2;     }
    ;

exp:
      exp EQ exp { $$ = $1 == $3 ? -1 : 0; }
    | exp NE exp { $$ = $1 != $3 ? -1 : 0; }
    | exp GT exp { $$ = $1 >  $3 ? -1 : 0; }
    | exp GE exp { $$ = $1 >= $3 ? -1 : 0; }
    | exp LT exp { $$ = $1 <  $3 ? -1 : 0; }
    | exp LE exp { $$ = $1 <= $3 ? -1 : 0; }
    ;

exp: exp '?' exp ':' exp { $$ = $1 ? $3 : $5; }
    ;

exp:  '(' exp ')'   { $$ = $2; }
    | NUMBER        { $$ = $1; }
    ;

exp: IDENTIFIER { int symbol_value = 0;
                  if (!get_symbol($1, &symbol_value)) YYERROR;
                  $$ = symbol_value;
                }
    ;

%%

bool check_zero(int value) {
    if (value == 0) {
        fprintf(stderr, "division by zero!\n");
        return true;
    }
    return false;
}

size_t symbol_hash(const char *symbol_name) {
    size_t hash = 0x18462937; // random value
    for (int i=0; symbol_name[i]!='\0'; i++) {
        hash ^= symbol_name[i];
        hash = (hash << 5) | (hash >> (sizeof(hash)*8-5)); // rol(hash, 5)
    }
    return hash;
}

bool get_symbol(const char *symbol_name, int *symbol_value) {
    size_t index = symbol_hash(symbol_name) % MAX_SYMBOLS;
    for (int i=0; i<MAX_SYMBOLS && symbol_names[index] != NULL; i++) {
        if (strcmp(symbol_names[index], symbol_name) == 0) {
            *symbol_value = symbol_values[index];
            return true;
        }
        index = (index+1) % MAX_SYMBOLS;
    }
    fprintf(stderr, "undefined variable: %s\n", symbol_name);
    return false;
}

void set_symbol(const char *symbol_name, int symbol_value) {
    size_t index = symbol_hash(symbol_name) % MAX_SYMBOLS;
    for (int i=0; i<MAX_SYMBOLS && symbol_names[index] != NULL; i++) {
        if (strcmp(symbol_names[index], symbol_name) == 0) {
            symbol_values[index] = symbol_value;
            return;
        }
        index = (index+1) % MAX_SYMBOLS;
    }
    if (symbol_names[index] == NULL) {
        symbol_names[index] = symbol_name;
        symbol_values[index] = symbol_value;
    }
    else {
        fprintf(stderr, "too many variables!\n");
        exit(EXIT_FAILURE);
    }
}

void print_symbols(void) {
    for (size_t index=0; index<MAX_SYMBOLS; index++) {
        if (symbol_names[index] != NULL) {
            printf("%s = %d\n", symbol_names[index], symbol_values[index]);
        }
    }
}

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}

int main(void) {
    yyparse();
    return EXIT_SUCCESS;
}
