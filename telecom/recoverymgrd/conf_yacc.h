/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     PID = 258,
     APPHB_HUP_L = 259,
     APPHB_NOHB_L = 260,
     APPHB_HBAGAIN_L = 261,
     APPHB_HBWARN_L = 262,
     APPHB_HBUNREG_L = 263,
     STRING = 264,
     FILENAME = 265,
     OPEN_CURLY = 266,
     CLOSE_CURLY = 267,
     COLON = 268,
     EQUALS = 269,
     WORD = 270
   };
#endif
/* Tokens.  */
#define PID 258
#define APPHB_HUP_L 259
#define APPHB_NOHB_L 260
#define APPHB_HBAGAIN_L 261
#define APPHB_HBWARN_L 262
#define APPHB_HBUNREG_L 263
#define STRING 264
#define FILENAME 265
#define OPEN_CURLY 266
#define CLOSE_CURLY 267
#define COLON 268
#define EQUALS 269
#define WORD 270




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

