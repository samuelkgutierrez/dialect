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

%option interactive noyywrap nounput

%{

#include <iostream>
#include <cstdlib>
#include <string.h>

#include "Expressions.hxx"
#include "y.tab.h"

%}

DIGIT [0-9]
LETTER [a-zA-Z]

%%

[ \t\f\r] { ; }

Add { return PLUS; }

Mul { return MUL; }

Const { return CONST; }

Var { return VAR; }

{DIGIT}+ { yylval.num = atoi(yytext); return NUMBER; }

{LETTER} { yylval.id = strdup(yytext); return VARID; }

"\n" { return NEWLINE; }

"-" { return UMINUS; }

"(" { return LPAREN; }

")" { return RPAREN; }

"\"" { return DQ; }

. {   /* everything else */
      std::cout << "invalid input -- cannot continue..." << std::endl;
      exit(EXIT_FAILURE);
  }

%%
