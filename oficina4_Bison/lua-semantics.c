#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lua-semantics.h"
#include "lua-parser.tab.h"

// Uncoment for very verbose debug messages on the conditionals stack
// #define DEBUG_CONDITIONALS

// Numeric format used for converting numbers to string
static const int Max_precision = 14; // 14 - gets same results as Lua5.1; 17 - preserves all precision of IEEE float64
static const int Max_size = 32;
static const char *Float_format = "%.*lg";

/* --- Ancillary functions */

void *check_alloc(void *ptr) {
    if (ptr == NULL) {
        fatal("not enough memory");
    }
    return ptr;
}

/* --- String buffer for constants and user inputs */

size_t string_size;
char *string_buffer;

void string_init(void) {
    string_buffer = check_alloc(malloc(Max_string_size+1));
    string_size = 0;
}

void string_reset(void) {
    string_size = 0;
    strcpy(string_buffer, "");
}

void string_append(const char *content) {
    size_t content_size = strlen(content);
    if (string_size+content_size > Max_string_size) {
        yyerror("maximum string size exceeded");
    }
    memcpy(string_buffer+string_size, content, content_size+1);
    string_size += content_size;
}

void string_input(void) {
    char *status = fgets(string_buffer, Max_string_size, stdin);
    if (status == NULL) {
        fatal("error in io.read()");
    }
    size_t len = strlen(string_buffer);
    if (string_buffer[len-1] == '\n') {
        string_buffer[len-1] = '\0';
    }
}

/* --- Values of stored constants and variables */

lua_value_t *make_value(lua_data_type_t type, double number, const char *string) {
    lua_value_t *value = check_alloc(malloc(sizeof(lua_value_t)));
    value->type   = type;
    value->number = number;
    value->string = (string==NULL) ? NULL : check_alloc(strdup(string));
    return value;
}

/* --- Dynamic-sized list */

list_t variable_list;
list_t expression_list;

void list_init(size_t max_size, list_t *list) {
    list->max_size = max_size;
    list->size = 0;
    list->contents = check_alloc(calloc(max_size, sizeof(const void *)));
}

void list_reset(list_t *list) {
    list->size = 0;
}

void list_append(list_t *list, const void *element) {
    if (list->size < list->max_size) {
        list->contents[list->size] = element;
        list->size++;
    }
    else {
        fatal("too many elements on list");
    }
}

/* --- Symbol table */

static const char*  symbol_names[Symbol_table_size];
static lua_value_t* symbol_values[Symbol_table_size];

static size_t symbol_hash(const char *symbol_name) {
    size_t hash = (size_t) 0x7DE00066A8F3C882; // random value
    for (int i=0; symbol_name[i]!='\0'; i++) {
        hash ^= symbol_name[i];
        hash = (hash << 5) | (hash >> (sizeof(hash)*8-5)); // rol(hash, 5)
    }
    return hash;
}

bool get_symbol_(const char *symbol_name, lua_value_t **symbol_value) {
    size_t index = symbol_hash(symbol_name) % Symbol_table_size;
    for (int i=0; i<Symbol_table_size && symbol_names[index] != NULL; i++) {
        if (strcmp(symbol_names[index], symbol_name) == 0) {
            *symbol_value = symbol_values[index];
            return true;
        }
        index = (index+1) % Symbol_table_size;
    }
    yyerror("undefined symbol: %s", symbol_name);
    return false;
}

void set_symbol(const char *symbol_name, lua_value_t *symbol_value) {
    size_t index = symbol_hash(symbol_name) % Symbol_table_size;
    for (int i=0; i<Symbol_table_size && symbol_names[index] != NULL; i++) {
        if (strcmp(symbol_names[index], symbol_name) == 0) {
            symbol_values[index] = symbol_value;
            return;
        }
        index = (index+1) % Symbol_table_size;
    }
    if (symbol_names[index] == NULL) {
        symbol_names[index] = symbol_name;
        symbol_values[index] = symbol_value;
    }
    else {
        fatal("too many symbols!");
        exit(EXIT_FAILURE);
    }
}

bool set_symbols_(list_t *symbol_names, list_t *symbol_values) {
    if (symbol_names->size != symbol_values->size) {
        yyerror("list size mismatch in attribution");
        return false;
    }
    for (int i=0; i<symbol_names->size; i++) {
        const char  *name  = symbol_names->contents[i];
        lua_value_t *value = (lua_value_t *) symbol_values->contents[i];
        if (name != NULL) {
            if (value != NULL) {
                set_symbol(name, value);
            }
            else {
                set_symbol(name, make_value(DTYPE_INVALID, 0, NULL));
            }
        }
    }
    return true;
}

/* --- Operations */

// Ensures that the value has a meaningful number member, returns true if successful
static bool ensure_number(lua_value_t *value) {
    if (value->type == DTYPE_NUMBER) {
        return true;
    }
    if (value->type == DTYPE_STRING) {
        char *number_end;
        double number = strtod(value->string, &(number_end));
        strtok(number_end, " \t");
        if (*number_end == '\0') {
            value->number = number;
            return true;
        }
        yyerror("attempt to do number operation with a string that cannot be converted to number");
        return false;
    }
    yyerror("attempt to do number operation with a non-number");
    return false;
}

// Converts number into a newly allocated string
static char *create_string_with_number(double number) {
    char *buffer = check_alloc(malloc(Max_size));
    snprintf(buffer, Max_size, Float_format, Max_precision, number);
    return buffer;
}

// Ensures that value has a meaningful string member, if require_string is true requires DTYPE_STRING,
// returns true if successful
static bool ensure_string(lua_value_t *symbol, bool require_string) {
    if (symbol->type == DTYPE_STRING) {
        return true;
    }
    if (require_string || symbol->type!=DTYPE_NUMBER) {
        yyerror("attempt to do string operation with a non-string");
        return false;
    }
    symbol->string = create_string_with_number(symbol->number);
    return true;
}

enum value_compare_t {
    VALUE_LESSER  = -1,
    VALUE_EQUALS  = 0,
    VALUE_GREATER = 1,
    VALUE_NUMBER_DIFFERENT = 2, // Values different but not ordered (e.g., nans)
    VALUE_DIFFERENT_TYPES = 3,
    VALUE_NOT_COMPARABLE = 4, // Only Numbers and Strings are comparable in Lua
};

// Compares two operands op1 and op2
static enum value_compare_t value_compare(lua_value_t *op1, lua_value_t *op2) {
    if (op1->type != op2->type) {
        return VALUE_DIFFERENT_TYPES;
    }
    if (op1->type == DTYPE_NUMBER) {
        if (op1->number == op2->number) return VALUE_EQUALS;
        if (op1->number < op2->number)  return VALUE_LESSER;
        if (op1->number > op2->number)  return VALUE_GREATER;
        return VALUE_NUMBER_DIFFERENT;
    }
    if (op1->type == DTYPE_STRING) {
        int comparison = strcmp(op1->string, op2->string);
        if (comparison == 0) return VALUE_EQUALS;
        if (comparison <  0) return VALUE_LESSER;
        if (comparison >  0) return VALUE_GREATER;
        assert(false);
    }
    return VALUE_NOT_COMPARABLE;
}

// Concatenates two strings into a newly allocated string
static char *string_concatenate(const char *string1, const char *string2) {
    size_t length1 = strlen(string1);
    size_t length2 = strlen(string2);
    char *new_string = check_alloc(malloc(length1+length2+1));
    memcpy(new_string,         string1, length1);
    memcpy(new_string+length1, string2, length2+1);
    return new_string;
}

bool get_boolean(lua_value_t *symbol) {
    // Error situations convert to false...
    if (symbol == NULL || symbol->type == DTYPE_INVALID || symbol->type == DTYPE_NONE) {
        return false;
    }
    // ...otherwise, applies Lua rules
    if (symbol->type == DTYPE_BOOLEAN || symbol->type == DTYPE_NUMBER) {
        return (bool) symbol->number;
    }
    else if (symbol->type == DTYPE_NIL) {
        return false;
    }
    else {
        return true;
    }
}

lua_value_t *do_operation(int operation, lua_value_t *op1, lua_value_t *op2) {
    // Propagates invalid operands
    if (op1->type==DTYPE_INVALID || (op2!=NULL && op2->type==DTYPE_INVALID)) {
        return make_value(DTYPE_INVALID, 0, NULL);
    }

    // Gets operands, making appropriate type conversions
    bool bop1, bop2;
    switch (operation) {
    case PLUS:
    case MULT:
    case DIV:
    case MOD:
    case POW:
        if (!ensure_number(op1) || !ensure_number(op2)) {
            return make_value(DTYPE_INVALID, 0, NULL);
        }
    break;
    case MINUS:
        if (!ensure_number(op1) || (op2!=NULL && !ensure_number(op2))) {
            return make_value(DTYPE_INVALID, 0, NULL);
        }
    break;

    case CONCAT:
        if (!ensure_string(op2, false) || !ensure_string(op1, false)) {
            return make_value(DTYPE_INVALID, 0, NULL);
        }
    break;
    case LEN:
        if (!ensure_string(op1, true)) {
            return make_value(DTYPE_INVALID, 0, NULL);
        }
    break;

    case EQUAL:
    case DIFF:
        // do nothing
    break;

    case LE:
    case GE:
    case LT:
    case GT:
        if (op1->type != op2->type) {
            yyerror("attempt to compare operands of different types");
            return make_value(DTYPE_INVALID, 0, NULL);
        }
        if (op1->type != DTYPE_NUMBER && op1->type != DTYPE_STRING) {
            yyerror("attempt to compare operand that is not a number neither a string");
            return make_value(DTYPE_INVALID, 0, NULL);
        }
    break;

    case AND:
    case OR:
        bop2 = get_boolean(op2);
    case NOT:
        bop1 = get_boolean(op1);
    break;
    default:
        assert(false);
    }

    // Performs operation
    switch (operation) {
    case PLUS:
        return make_value(DTYPE_NUMBER, op1->number + op2->number, NULL);
    case MINUS:
        if (op2 == NULL) {
            return make_value(DTYPE_NUMBER, -op1->number, NULL);
        }
        else {
            return make_value(DTYPE_NUMBER, op1->number - op2->number, NULL);
        }
    case MULT:
        return make_value(DTYPE_NUMBER, op1->number * op2->number, NULL);
    case DIV:
        return make_value(DTYPE_NUMBER, op1->number / op2->number, NULL);
    case MOD:
        return make_value(DTYPE_NUMBER, fmod(op1->number, op2->number), NULL);
    case POW:
        return make_value(DTYPE_NUMBER, pow(op1->number, op2->number), NULL);

    case EQUAL:
        return make_value(DTYPE_BOOLEAN, value_compare(op1, op2) == VALUE_EQUALS, NULL);
    case DIFF:
        return make_value(DTYPE_BOOLEAN, value_compare(op1, op2) != VALUE_EQUALS, NULL);
    case LE:
    case GE:
    case LT:
    case GT: {
        enum value_compare_t result = value_compare(op1, op2);
        assert (result != VALUE_DIFFERENT_TYPES && result != VALUE_NOT_COMPARABLE);
        bool bool_result = (result == VALUE_LESSER  && (operation == LE || operation == LT)) ||
                           (result == VALUE_GREATER && (operation == GE || operation == GT)) ||
                           (result == VALUE_EQUALS  && (operation == LE || operation == GE));
        return make_value(DTYPE_BOOLEAN, bool_result, NULL);
    }
    case CONCAT:
        return make_value(DTYPE_STRING, 0, string_concatenate(op1->string, op2->string));
    case LEN:
        return make_value(DTYPE_NUMBER, strlen(op1->string), NULL);

    case AND:
        return bop1 ? op2 : op1;
    case OR:
        return bop1 ? op1 : op2;
    case NOT:
        return make_value(DTYPE_BOOLEAN, !bop1, NULL);
    default:
        assert(false);
    }

}

/* --- Printing control */

static bool print_started;

void print_start(void) {
   print_started = false;
 }

void print_finish(void) {
   printf("\n");
 }

// Prints number formatted for output
static void print_number(double number) {
    printf(Float_format, Max_precision, number);
}

void print_item(lua_value_t *item) {
    if (print_started) {
        printf("\t");
    }
    print_started = true;
    switch (item->type) {
    case DTYPE_INVALID:
        printf("<unimplemented/not executed>");
    break;
    case DTYPE_NONE:
        printf("none");
    break;
    case DTYPE_NIL:
        printf("nil");
    break;
    case DTYPE_BOOLEAN:
        printf(item->number==0. ? "false" : "true");
    break;
    case DTYPE_NUMBER:
        print_number(item->number);
    break;
    case DTYPE_STRING:
        printf("%s", item->string);
    break;
    case DTYPE_FUNCTION:
        printf("<unimplemented:function>");
    break;
    case DTYPE_USERDATA:
        printf("<unimplemented:userdata>");
    break;
    case DTYPE_THREAD:
        printf("<unimplemented:thread>");
    break;
    case DTYPE_TABLE:
        printf("<unimplemented:table>");
    break;
    default:
        assert(false);
    }
}

/* --- Conditional execution control */

#ifdef DEBUG_CONDITIONALS
#define cond_debug(...) { fprintf(stderr, __VA_ARGS__); }
#else
#define cond_debug(...) { }
#endif

static bool cond_stack[Max_nested_controls];
static bool cond_stack_any[Max_nested_controls]; // Has any condition in an if..elseif..else sequence already passed?
static size_t cond_stack_i = 0;

bool cond_push(bool test_exp) {
    if (cond_stack_i >= Max_nested_controls) {
        fatal("stack overflow: too many nested control structures");
    }
    cond_stack[cond_stack_i] = test_exp;
    cond_stack_any[cond_stack_i] = test_exp;
    cond_stack_i++;
    return test_exp;
}

bool cond_pop(void) {
    if (cond_stack_i == 0) {
        fatal("stack underflow: unbalanced pushs() and pops() in control structure rules");
    }
    cond_stack_i--;
    return cond_stack[cond_stack_i];
}

bool cond_enabled(void) {
    cond_debug("cond_stack_i == %zu", cond_stack_i);
    bool enabled = true;
    for (size_t i=0; i<cond_stack_i && enabled; i++) {
        cond_debug(", cond_stack[%zu] == %s", i, cond_stack[i] ? "true" : "false");
        enabled = enabled && cond_stack[i];
    }
    cond_debug(" -> %s\n", enabled ? "enabled" : "disabled");
    return enabled;
}

bool cond_test_elseif(void) {
    if (cond_stack_i == 0) {
        fatal("stack underflow: attempt to set inexisting condition in stack");
    }
    cond_debug("cond_stack_any[top] -> %s", cond_stack_any[cond_stack_i-1] ? "true" : "false");
    bool test_elseif = !cond_stack_any[cond_stack_i-1];
    cond_debug(", test_elseif -> %s\n", test_elseif ? "enabled" : "disabled");
    cond_stack[cond_stack_i-1] = test_elseif;
    return test_elseif;
}

bool cond_elseif(bool elseif_test_exp) {
    if (cond_stack_i == 0) {
        fatal("stack underflow: attempt to set inexisting condition in stack");
    }
    cond_debug("elseif_test_exp -> %s", elseif_test_exp ? "true" : "false");
    cond_debug(", cond_stack_any[top] -> %s", cond_stack_any[cond_stack_i-1] ? "true" : "false");
    bool elseif_enabled = elseif_test_exp && !cond_stack_any[cond_stack_i-1];
    cond_debug(", elseif_enabled -> %s\n", elseif_enabled ? "enabled" : "disabled");
    cond_stack_any[cond_stack_i-1] = cond_stack_any[cond_stack_i-1] || elseif_enabled;
    cond_stack[cond_stack_i-1] = elseif_enabled;
    return elseif_enabled;
}

/* --- Interpreter internals */

bool interactive;
int  line_number;

void yynewline(void) {
    line_number++;
    if (interactive) {
        printf("> ");
    }
}

static const size_t Max_message_size = 1024;

// Prints message to stderr, adding line number if session is not interactive;
// a newline is always appended
static void print_message(const char *message) {
    if (interactive) {
        fprintf(stderr, "%s\n", message);
    }
    else {
        fprintf(stderr, "%s (line %d)\n", message, line_number);
    }
}

void yyerror(const char *message, ...) {
    char partial_message[Max_message_size];
    char formatted_message[Max_message_size];
    va_list arglist;
    va_start(arglist, message);
    vsnprintf(partial_message, Max_message_size, message, arglist);
    snprintf(formatted_message, Max_message_size, "error: %s", partial_message);
    print_message(formatted_message);
    va_end(arglist);
}

void warn(const char *feature) {
    const char *uninmplemented_message = "not implemented: %s -- code will be parsed but will have no effect";
    char formatted_message[Max_message_size];
    snprintf(formatted_message, Max_message_size, uninmplemented_message, feature);
    print_message(formatted_message);
}

