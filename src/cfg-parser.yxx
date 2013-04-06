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

#include "DialectException.hxx"
#include "CFG.hxx"

#include <iostream>
#include <string>
#include <map>
#include <list>

#include <stdlib.h>

int yylex(void);
extern "C" int yyerror(const char * s);
extern "C" FILE *yyin;

%}

%union {
    CFG *testfoo;
    std::string rhs;
    std::string lhs;
}

%token NEWLINE

%type <testfoo> cfg

%start program

%%

program : cfg {
              return new CFG();
          }
        ;

cfg : NEWLINE {
    $$ = new CFG();
};

%%

/* ////////////////////////////////////////////////////////////////////////// */
/* wrapper for yyparse */
int
parserParse(FILE *fp)
{
    /* set to 1 for tons of debug output */
#if 0
    yydebug = 1;
#endif
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
