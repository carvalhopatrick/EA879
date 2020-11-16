/* lua-parser.y: a simplified parser for the language lua */

/* --------- Opening code for generated parser */

%{
// Uncomment this for very verbose parser debug info
// #define YYDEBUG 1
#include "lua-semantics.h"
%}

/* --------- Declarations for generated parser */

%glr-parser
%expect 1
%expect-rr 1
%file-prefix "lua-parser"
%defines

/* --- Type and disposal instructions for semantic symbols (yylval) */

%union { int keyword; }
%union { const char *string; }
%union { const char *name; }
%union { double number; }
%union { lua_value_t *value; }
%union { list_t *list; }

/* --- Operator precedences and associativities */

%left <keyword> OR
%left <keyword> AND
%left <keyword> LT GT LE GE DIFF EQUAL
%right <keyword> CONCAT
%left <keyword> PLUS MINUS
%left <keyword> MULT DIV MOD
%left <keyword> NOT LEN 
%right <keyword> POW
/* <<<complete o código aqui>>> done */

/* --- All other tokens */

%token <keyword> BREAK DO ELSE ELSEIF END FALSE FOR FUNCTION IF IN IOREAD
                 LOCAL NIL REPEAT RETURN THEN TRUE UNTIL WHILE
%token <keyword> PRINT
%token OPEN_BRA CLOSE_BRA OPEN_CURLY CLOSE_CURLY OPEN_PAR CLOSE_PAR
%token COLON COMMA DOT ELLIPSIS NEWLINE SEMICOLON SET

%token <name>   NAME
%token <string> STRING
%token <number> NUMBER

/* --- Types of nonterminals */

%type <name>   var
%type <value>  exp exp_binary exp_unary exp_prefix
%type <list>   var_list exp_list name_list

%% /* --------- Declarations for generated parser */

/* --- Blocks and statements */

/* chunk ::= {stat [`;´]} [laststat [`;´]]
   block ::= chunk
*/

block:
      stat_list stat_last_quasicolon
    | stat_list
    | stat_last_quasicolon
    |
    ;

stat_list:
      stat_list stat_quasicolon
    | stat_quasicolon
    ;

stat_quasicolon:
      stat SEMICOLON
    | stat
    ;

stat_last_quasicolon:
      stat_last SEMICOLON
    | stat_last
    ;

/* stat ::=  varlist `=´ explist |
             functioncall |
             do block end |
             while exp do block end |
             repeat block until exp |
             if exp then block {elseif exp then block} [else block] end |
             for Name `=´ exp `,´ exp [`,´ exp] do block end |
             for namelist in explist do block end |
             function funcname funcbody |
             local function Name funcbody |
             local namelist [`=´ explist]
*/

stat:
      stat_set
    | stat_do
    | stat_if
    | stat_call
    | stat_disabled_control
    | stat_local_var
    | stat_print
    | error   { scanner_start_recovery(); }
      NEWLINE { scanner_end_recovery(); yyerrok; }
    ;

stat_set:
    var_list SET exp_list {  if (cond_enabled()) set_symbols($var_list, $exp_list); }

stat_do: DO block END;

stat_if: IF exp THEN     { cond_push(get_boolean($exp)); }
         block
         elseif_clauses
         else_clause
         END             { cond_pop(); }
         ;

elseif_clauses:
     elseif_clause elseif_clauses
    |
    ;

elseif_clause:
    ELSEIF   { cond_test_elseif();             }
    exp THEN { cond_elseif(get_boolean($exp)); }
    block;

else_clause:
     ELSE { cond_elseif(true); }
     block
    |
    ;

stat_call: function_call;

stat_disabled_control:
    stat_while
    | stat_repeat
    | stat_for
    | stat_forin
    | stat_function
    | stat_local_func
    ;

stat_while: WHILE { cond_push(false); warn("while loop"); } exp DO block END { cond_pop(); };

stat_repeat: REPEAT { cond_push(false); warn("repeat loop"); } block UNTIL  exp { cond_pop(); };
/* <<< ^ */

stat_for: FOR NAME SET { cond_push(false); warn("for loop"); } exp COMMA exp step_spec DO block END { cond_pop(); };

step_spec:
      COMMA exp
    |
    ;

stat_forin: FOR name_list IN { cond_push(false); warn("for in loop"); } exp_list DO block END { cond_pop(); };
/* <<< ^ */

stat_function: FUNCTION { cond_push(false); warn("function"); } function_name function_body { cond_pop(); };

stat_local_func: LOCAL FUNCTION { cond_push(false); warn("function"); } NAME function_body { cond_pop(); };

stat_local_var:
      LOCAL name_list
    | LOCAL name_list SET exp_list { if (cond_enabled()) set_symbols($name_list, $exp_list); }
    ;

/* laststat ::= return [explist] | break */

stat_last: 
        RETURN
      | RETURN exp_list
      | BREAK
      ;
/* <<< ^ */

/* --- Print function calls implemented as a separated statement */

stat_print:
    PRINT     { if (cond_enabled()) print_start(); }
    OPEN_PAR print_list_optional
    CLOSE_PAR { if (cond_enabled()) print_finish(); }
    ;

print_list_optional:
      print_list
    |
    ;

print_list:
      print_list COMMA exp { if (cond_enabled()) print_item($exp); }
    | exp                  { if (cond_enabled()) print_item($exp); }
    ;

/* --- Simple lists */

/* varlist  ::= var {`,´ var}
   explist  ::= {exp `,´} exp
   namelist ::= Name {`,´ Name}
*/

var_list:
      var_list COMMA var { list_append(&variable_list, $var);
                           $$ = &variable_list;
                         }
    | var                { list_reset(&variable_list);
                           list_append(&variable_list, $var);
                           $$ = &variable_list;
                         }
    ;

exp_list:
      exp_list COMMA exp { list_append(&expression_list, $exp);
                           $$ = &expression_list;
                         }
    | exp                { list_reset(&expression_list);
                           list_append(&expression_list, $exp);
                           $$ = &expression_list;
                         }
    ;

name_list:
      name_list COMMA NAME { list_append(&variable_list, $NAME);
                             $$ = &variable_list;
                           }
    | NAME                { list_reset(&variable_list);
                            list_append(&variable_list, $NAME);
                            $$ = &variable_list;
                          }
    ;

/* ---  Qualified names */

/* funcname ::= Name {`.´ Name} [`:´ Name]
   var ::=  Name | prefixexp `[´ exp `]´ | prefixexp `.´ Name
*/

function_name:
      qualified_name
    | qualified_name COLON NAME
    ;

qualified_name:
      NAME DOT qualified_name
    | NAME
    ;

var:
      NAME                              { $$ = $NAME; }
    | exp_prefix OPEN_BRA exp CLOSE_BRA { $$ = NULL; warn("var[item] access"); }
    | exp_prefix DOT NAME               { $$ = NULL; warn("var.field access"); }
    ;

/* ---  Expressions */

/* exp ::=  nil | false | true | Number | String | `...´ | function |
            prefixexp | tableconstructor | exp binop exp | unop exp
*/

exp:
      NIL                  { $$ = !cond_enabled() ? NULL : make_value(DTYPE_NIL,           0,    NULL); }
    | FALSE                { $$ = !cond_enabled() ? NULL : make_value(DTYPE_BOOLEAN,       0,    NULL); }
    | TRUE                 { $$ = !cond_enabled() ? NULL : make_value(DTYPE_BOOLEAN,       1,    NULL); }
    | NUMBER               { $$ = !cond_enabled() ? NULL : make_value(DTYPE_NUMBER,  $NUMBER,    NULL); }
    | STRING               { $$ = !cond_enabled() ? NULL : make_value(DTYPE_STRING,        0, $STRING); }
    | ELLIPSIS             { $$ = !cond_enabled() ? NULL : make_value(DTYPE_INVALID,       0,    NULL); }
    | exp_binary           { $$ = !cond_enabled() ? NULL : $exp_binary; }
    | exp_unary            { $$ = !cond_enabled() ? NULL : $exp_unary;  }
    | exp_prefix %expect 1 { $$ = !cond_enabled() ? NULL : $exp_prefix; }
    | exp_function         { $$ = !cond_enabled() ? NULL : make_value(DTYPE_INVALID, 0, NULL); }
    | table_constructor    { $$ = !cond_enabled() ? NULL : make_value(DTYPE_INVALID, 0, NULL); }
    ;

exp: IOREAD { if (cond_enabled()) {
                   string_input();
                   $$ = make_value(DTYPE_STRING, 0, string_buffer);
               }
               else {
                   $$ = NULL;
               }
            }
    ;

exp_binary:
      exp PLUS exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp MINUS exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp MULT exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp DIV exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp POW exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp MOD exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp CONCAT exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp LT exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp LE exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp GT exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp GE exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp EQUAL exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp DIFF exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp AND exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    | exp OR exp   { $$ = !cond_enabled() ? NULL : do_operation($2, $1, $3); }
    /* <<<complete os demais operadores binários>>> done */
    ;

exp_unary:
      NOT exp { $$ = !cond_enabled() ? NULL : do_operation($1, $2, NULL); }
    | LEN exp { $$ = !cond_enabled() ? NULL : do_operation($1, $2, NULL); }
    | MINUS exp { $$ = !cond_enabled() ? NULL : do_operation($1, $2, NULL); }
    /* <<<complete os demais operadores unários>>> done */
    ;

exp_prefix:
      OPEN_PAR exp CLOSE_PAR     { $$ = !cond_enabled() ? NULL : $exp; }
    | function_call %expect-rr 1 { warn("function call");
                                   $$ = !cond_enabled() ? NULL : make_value(DTYPE_INVALID,  0, NULL); }
    | var                       { if (cond_enabled()) {
                                    if ($var != NULL) {
                                        lua_value_t *variable_value;
                                        get_symbol($var, &variable_value);
                                        $$ = variable_value;
                                    }
                                    else {
                                        $$ = make_value(DTYPE_INVALID, 0, NULL);
                                    }
                                }
                                else {
                                    $$ = NULL;
                                }
                                }
    ;

/* functioncall ::=  prefixexp args | prefixexp `:´ Name args
   args ::=  `(´ [explist] `)´ | tableconstructor | String
   function ::= function funcbody
   funcbody ::= `(´ [parlist] `)´ block end
   parlist ::= namelist [`,´ `...´] | `...´
*/

exp_function: FUNCTION { cond_push(false); warn("function"); } function_body { cond_pop(); }

function_body: OPEN_PAR par_list CLOSE_PAR block END

par_list:
      name_list
    | name_list COMMA ELLIPSIS
    | ELLIPSIS
    |
    ;

function_call:
      exp_prefix arg_list
    | exp_prefix COLON NAME arg_list
    ;

arg_list:
      OPEN_PAR exp_list CLOSE_PAR
    | table_constructor
    | STRING
    | OPEN_PAR CLOSE_PAR
    ;

/* tableconstructor ::= `{´ [fieldlist] `}´
   fieldlist ::= field {fieldsep field} [fieldsep]
   field ::= `[´ exp `]´ `=´ exp | Name `=´ exp | exp
   fieldsep ::= `,´ | `;´
*/

table_constructor: OPEN_CURLY { cond_push(false); warn("table"); } field_list_optional CLOSE_CURLY { cond_pop(); }

field_list_optional:
      field_list
    | 
    ;

field_list:
      field_list field_sep field
    | field_list field_sep
    | field
    ;

field:
      exp
    | NAME SET exp 
    | OPEN_BRA exp CLOSE_BRA SET exp 
    ;

field_sep:
      COMMA
    | SEMICOLON
    ;
/* <<<complete a gramática para reconhecer tabelas --- não é preciso colocar qualquer ação semântica nesta sessão>>> */

%% /* --------- Closing code for generated parser */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char const* argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "usage: lua-parser <source.lua>\n"
                        "       lua-parser --\n\n"
                        "       use the second syntax to read source from standard input.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "--") == 0) {
        yyin = stdin;
    }
    else {
        yyin = fopen(argv[1], "r");
        if (yyin == NULL) {
            interactive = true;
            fatal("error opening source file: %s", argv[1])
        }
    }

    #if YYDEBUG==1
    yydebug=1;
    #endif

    string_init();
    list_init(Max_lua_list_size, &variable_list);
    list_init(Max_lua_list_size, &expression_list);

    interactive = isatty(fileno(yyin));
    scanner_set_interactive(interactive);
    yynewline();

    yyparse();
    return EXIT_SUCCESS;
}

