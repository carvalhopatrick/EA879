#ifndef LUA_SEMANTICS_H
#define LUA_SEMANTICS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/* --- Symbols exported by the lexical analyser */

extern char *yytext;
extern FILE *yyin;
int yylex(void);
void scanner_set_interactive(bool interactive);
void scanner_start_recovery(void);
void scanner_end_recovery(void);

/* --- Ancillary functions */

// If ptr is NULL aborts program with "not enough error" message, otherwise returns ptr
void *check_alloc(void *ptr);

/* --- String buffer for constants and user inputs */

static const size_t Max_string_size = 1024*1024;
extern size_t string_size;
extern char *string_buffer;

// Initializes string buffer - should be called only once per program execution
extern void string_init(void);
// Resets string buffer to empty string
void string_reset(void);
// Appends value to string buffer
extern void string_append(const char *content);
// Inputs string from stdin to string buffer
extern void string_input(void);

/* --- Values of stored constants and variables */

typedef enum lua_data_type_t {
    // Invalid data type to indicate unsupported operations
    DTYPE_INVALID = 0,
    // Data types from lua
    DTYPE_NONE, DTYPE_NIL, DTYPE_BOOLEAN, DTYPE_NUMBER, DTYPE_STRING,
    DTYPE_FUNCTION, DTYPE_USERDATA, DTYPE_THREAD, DTYPE_TABLE,
} lua_data_type_t;

typedef struct lua_value_t {
    lua_data_type_t type;
    double          number;
    char*           string;
} lua_value_t;

lua_value_t *make_value(lua_data_type_t type, double number, const char *string);

/* --- Dynamic-sized list */

typedef struct list_t {
    size_t        max_size;
    size_t        size;
    const void * *contents;
} list_t;

static const size_t Max_lua_list_size = 1024;
extern list_t variable_list;
extern list_t expression_list;

// Initializes a list with a given capacity
void list_init(size_t max_size, list_t *list);
// Removes all elements from the list
void list_reset(list_t *list);
// Appends (pointer to) element at the end of the list
void list_append(list_t *list, const void *element);

/* --- Symbol table */

static const size_t Symbol_table_size = 1024;

// Gets symbol symbol_name, symbol_value should be a pointer to a (lua_value_t *)
// triggers syntactic error if symbol is not found
#define get_symbol(symbol_name, symbol_value) { if (!get_symbol_(symbol_name, symbol_value)) YYERROR; }
bool get_symbol_(const char *symbol_name, lua_value_t **symbol_value);
// Sets symbol symbol_name to, symbol_value
void set_symbol(const char *symbol_name, lua_value_t *symbol_value);
// Sets each symbol with name (of type char *) in the list symbol_names to value (of type lua_value_t *)
// in the list symbol_values, the two lists must have the same size
#define set_symbols(symbol_names, symbol_values) { if (!set_symbols_(symbol_names, symbol_values)) YYERROR; }
bool set_symbols_(list_t *symbol_names, list_t *symbol_values);

/* --- Operations */

// Gets the Boolean value of symbol (using Lua's conventions for casting to Boolean)
bool get_boolean(lua_value_t *symbol);
// Performs the operation given by token operation, on values op1 and op2 (set op2=NULL for unary operations)
lua_value_t *do_operation(int operation, lua_value_t *op1, lua_value_t *op2);

/* --- Printing control */

// Starts printing output
void print_start(void);
// Prints a single item in the print function call
void print_item(lua_value_t *item);
// Concludes printing output
void print_finish(void);

/* --- Conditional execution control */

static const size_t Max_nested_controls = 1024; // How deep can we nest conditionals? (size of conditionals stack)

// Pushed one nested conditional onto conditionals stack;
// test_exp should be the Boolean result of the conditional test
bool cond_push(bool test_exp);
// Pops one nested conditional from conditionals stack
bool cond_pop(void);
// Checks if execution is currently enabled by nested conditionals
bool cond_enabled(void);
// Checks if the test_expression for an elseif (in an if..elseif..else..end chain) should be computed
bool cond_test_elseif(void);
// Checks if the execution is enabled for an elseif or else block (in an if..elseif..else..end chain)
// elseif_test_exp should be the Boolean result of the elseif test (use the true constant for an else)
bool cond_elseif(bool elseif_test_exp);

/* --- Interpreter internals */

extern bool interactive; // True for interactive parsers (stdin is terminal)
extern int  line_number;

// Counts lines and, if interactive, shows interpreter prompt
void yynewline(void);

// Formats error messages, uses same parameters as printf
void yyerror(const char *message, ...);
#define fatal(...) { yyerror(__VA_ARGS__); exit(EXIT_FAILURE); }
// Shows warning for unimplemented feature
void warn(const char *feature);

#endif
