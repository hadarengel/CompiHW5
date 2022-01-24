/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_HPP_INCLUDED
# define YY_YY_PARSER_TAB_HPP_INCLUDED
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
    B = 258,                       /* B  */
    BOOL = 259,                    /* BOOL  */
    VOID = 260,                    /* VOID  */
    INT = 261,                     /* INT  */
    BYTE = 262,                    /* BYTE  */
    STRING = 263,                  /* STRING  */
    CONST = 264,                   /* CONST  */
    TRUE = 265,                    /* TRUE  */
    FALSE = 266,                   /* FALSE  */
    IF = 267,                      /* IF  */
    RETURN = 268,                  /* RETURN  */
    WHILE = 269,                   /* WHILE  */
    BREAK = 270,                   /* BREAK  */
    CONTINUE = 271,                /* CONTINUE  */
    SC = 272,                      /* SC  */
    COMMA = 273,                   /* COMMA  */
    NUM = 274,                     /* NUM  */
    ID = 275,                      /* ID  */
    ASSIGN = 276,                  /* ASSIGN  */
    OR = 277,                      /* OR  */
    AND = 278,                     /* AND  */
    EQUALOP = 279,                 /* EQUALOP  */
    RELOP = 280,                   /* RELOP  */
    ADD = 281,                     /* ADD  */
    SUB = 282,                     /* SUB  */
    MULT = 283,                    /* MULT  */
    DIV = 284,                     /* DIV  */
    NOT = 285,                     /* NOT  */
    LPAREN = 286,                  /* LPAREN  */
    RPAREN = 287,                  /* RPAREN  */
    LBRACE = 288,                  /* LBRACE  */
    RBRACE = 289,                  /* RBRACE  */
    ELSE = 290                     /* ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_HPP_INCLUDED  */
