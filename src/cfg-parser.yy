/**
 * Copyright (c) 2013 Samuel K. Gutierrez All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%error-verbose

%{

#include "CFG.hxx"

#include <iostream>
#include <string>
#include <cstdlib>

int yylex(void);
extern "C" int yyerror(const char * s);
extern "C" FILE *yyin;

/* a pointer to the newly created context-free grammar instance */
CFG *cfg = NULL;

%}

%union {
    std::string *str;
}

%token <str> RHS
%token <str> LHS
%token <str> ARROW
%token NEWLINE

%start cfg

%%

cfg : productions {
          return 0;
      }
    ;

productions : production {
              }
            | productions production {
              }
            ;

production : LHS ARROW RHS NEWLINE {
                 std::cout << *$1 << *$2 << *$3 << std::endl;
             }
           | LHS ARROW NEWLINE {
                 std::cout << *$1 << *$2 << "epsilon" << std::endl;
             }
           ;

%%

/* ////////////////////////////////////////////////////////////////////////// */
/* wrapper for yyparse */
int
parserParse(FILE *fp = NULL)
{
    if (NULL == fp) {
        yyin = stdin;
    }
    else {
        yyin = fp;
    }
    /* fp closed by caller */
    return yyparse();
}

/* ////////////////////////////////////////////////////////////////////////// */
int
yyerror(const char *s)
{
    /* ignored */
    return 42;
}
