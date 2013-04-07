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

/* context-free grammar scanner */

%option interactive noyywrap nounput

%{

#include <iostream>
#include <cstdlib>
#include <string>

#include "cfg-parser.h"

#define SAVE_TOKEN                                                             \
do {                                                                           \
    yylval.str = new std::string(yytext, yyleng);                              \
} while (0)

%}

/* the ascii characters that we care about in octal */
ASCII [\41-\176]

%%

{ASCII} { SAVE_TOKEN; return LHS; }

"-->" { return ARROW; }

{ASCII}+ { SAVE_TOKEN; return RHS; }

[ \t] { ; }

"\n" { return NEWLINE; }

. { std::cerr << "invalid token encountered during CFG scan... bye!"
              << std::endl;
    yyterminate();
  }

%%
