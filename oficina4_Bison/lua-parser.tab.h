/* A Bison parser, made by GNU Bison 3.7.3.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_LUA_PARSER_TAB_H_INCLUDED
# define YY_YY_LUA_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    OR = 258,                      /* OR  */
    AND = 259,                     /* AND  */
    LT = 260,                      /* LT  */
    GT = 261,                      /* GT  */
    LE = 262,                      /* LE  */
    GE = 263,                      /* GE  */
    DIFF = 264,                    /* DIFF  */
    EQUAL = 265,                   /* EQUAL  */
    CONCAT = 266,                  /* CONCAT  */
    PLUS = 267,                    /* PLUS  */
    MINUS = 268,                   /* MINUS  */
    MULT = 269,                    /* MULT  */
    DIV = 270,                     /* DIV  */
    MOD = 271,                     /* MOD  */
    NOT = 272,                     /* NOT  */
    LEN = 273,                     /* LEN  */
    POW = 274,                     /* POW  */
    BREAK = 275,                   /* BREAK  */
    DO = 276,                      /* DO  */
    ELSE = 277,                    /* ELSE  */
    ELSEIF = 278,                  /* ELSEIF  */
    END = 279,                     /* END  */
    FALSE = 280,                   /* FALSE  */
    FOR = 281,                     /* FOR  */
    FUNCTION = 282,                /* FUNCTION  */
    IF = 283,                      /* IF  */
    IN = 284,                      /* IN  */
    IOREAD = 285,                  /* IOREAD  */
    LOCAL = 286,                   /* LOCAL  */
    NIL = 287,                     /* NIL  */
    REPEAT = 288,                  /* REPEAT  */
    RETURN = 289,                  /* RETURN  */
    THEN = 290,                    /* THEN  */
    TRUE = 291,                    /* TRUE  */
    UNTIL = 292,                   /* UNTIL  */
    WHILE = 293,                   /* WHILE  */
    PRINT = 294,                   /* PRINT  */
    OPEN_BRA = 295,                /* OPEN_BRA  */
    CLOSE_BRA = 296,               /* CLOSE_BRA  */
    OPEN_CURLY = 297,              /* OPEN_CURLY  */
    CLOSE_CURLY = 298,             /* CLOSE_CURLY  */
    OPEN_PAR = 299,                /* OPEN_PAR  */
    CLOSE_PAR = 300,               /* CLOSE_PAR  */
    COLON = 301,                   /* COLON  */
    COMMA = 302,                   /* COMMA  */
    DOT = 303,                     /* DOT  */
    ELLIPSIS = 304,                /* ELLIPSIS  */
    NEWLINE = 305,                 /* NEWLINE  */
    SEMICOLON = 306,               /* SEMICOLON  */
    SET = 307,                     /* SET  */
    NAME = 308,                    /* NAME  */
    STRING = 309,                  /* STRING  */
    NUMBER = 310                   /* NUMBER  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 21 "lua-parser.y"
 int keyword; 
#line 22 "lua-parser.y"
 const char *string; 
#line 23 "lua-parser.y"
 const char *name; 
#line 24 "lua-parser.y"
 double number; 
#line 25 "lua-parser.y"
 lua_value_t *value; 
#line 26 "lua-parser.y"
 list_t *list; 

#line 127 "lua-parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_LUA_PARSER_TAB_H_INCLUDED  */
