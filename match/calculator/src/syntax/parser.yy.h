/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_END = 0,
     T_FUNCTION_MOD = 258,
     T_NUMBER = 259,
     T_FUNCTION_SIN = 260,
     T_FUNCTION_COS = 261,
     T_FUNCTION_TAN = 262,
     T_FUNCTION_ARCSIN = 263,
     T_FUNCTION_ARCCOS = 264,
     T_FUNCTION_ARCTAN = 265,
     T_FUNCTION_SINH = 266,
     T_FUNCTION_COSH = 267,
     T_FUNCTION_TANH = 268,
     T_FUNCTION_LOG = 269,
     T_FUNCTION_LOG10 = 270,
     T_FUNCTION_LN = 271,
     T_FUNCTION_POW = 272,
     T_FUNCTION_EXP = 273,
     T_FUNCTION_FACT = 274,
     T_FUNCTION_SQRT = 275,
     T_FUNCTION_CUBEROOT = 276,
     T_FUNCTION_YROOT = 277,
     T_FUNCTION_AVG = 278,
     T_FUNCTION_SUM = 279,
     T_FUNCTION_VAR = 280,
     T_FUNCTION_VARP = 281,
     T_FUNCTION_STDEV = 282,
     T_FUNCTION_STDEVP = 283,
     T_FUNCTION_MAX = 284,
     T_FUNCTION_MIN = 285,
     T_FUNCTION_FLOOR = 286,
     T_FUNCTION_CEIL = 287,
     T_FUNCTION_ROUND = 288,
     T_FUNCTION_RAND = 289,
     T_FUNCTION_A2R = 290,
     T_FUNCTION_R2A = 291
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


