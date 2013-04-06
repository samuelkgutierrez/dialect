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
    CFG cfg;
    std::string rhs;
    std::string lhs;
}

%token ARROW NEWLINE

%type <expnode> exp
%type <stmts> stmtlist
%type <st> stmt
%type <cfg> cfg

%start program

%%

cfg : 

program : stmtlist {
              $$ = new Program($1);
              root = $$;
              return 0;
          }
        ;

stmtlist : stmt NEWLINE {
              $$ = new std::list<Statement *>();
              $$->push_back($1);
           }
         ;

stmt : exp {
           $$ = new StatementAssign("$", $1);
       }
     | error {
           std::cerr << "unrecognized grammar -- cannot continue..."
                     << std::endl;
           return 1;
     }
 ;

exp : LPAREN exp RPAREN {
          $$ = $2;
      }
    | PLUS exp exp {
          /* if both are leaves, then evaluate and create a new EXPNodeNum */
          if ($2->constLeaf() && $3->constLeaf()) {
              $$ = new EXPNodeNum($2->evaluate() + $3->evaluate());
          }
          else {
              if ($2->print() < $3->print()) {
                  $$ = new EXPNodeOpAdd($2, $3);
              }
              else {
                  $$ = new EXPNodeOpAdd($3, $2);
              }
          }
      }
    | MUL exp exp {
          /* if both are leaves, then evaluate and create a new EXPNodeNum */
          if ($2->constLeaf() && $3->constLeaf()) {
              $$ = new EXPNodeNum($2->evaluate() * $3->evaluate());
          }
          else {
              if ($2->print() < $3->print()) {
                  $$ = new EXPNodeOpMul($2, $3);
              }
              else {
                  $$ = new EXPNodeOpMul($3, $2);
              }
          }
      }
    | CONST NUMBER {
          $$ = new EXPNodeNum($2);
      }
    | CONST LPAREN NUMBER RPAREN {
          $$ = new EXPNodeNum($3);
      }
    | CONST UMINUS NUMBER {
          $$ = new EXPNodeNum(-$3);
      }
    | CONST LPAREN UMINUS NUMBER RPAREN {
          $$ = new EXPNodeNum(-$4);
      }
    | VAR DQ VARID DQ {
          $$ = new EXPNodeVar($3);
      }
    ;

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
