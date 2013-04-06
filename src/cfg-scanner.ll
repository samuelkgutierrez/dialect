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

/* context-free scanner */

%option interactive noyywrap nounput

%{

#include "DialectException.hxx"

#include <iostream>
#include <cstdlib>
#include <string.h>

#if 0
#include "y.tab.h"
#endif

%}

/* the ascii characters that we care about in octal */
ASCII [\41-\176]

%%

[ \t] { ; }

{ASCII} { yylval.id = strdup(yytext); return VARID; }

"\n" { return NEWLINE; }
[\n\v\f\r] { ; }

. { throw DialectException("invalid input encountered during CFG scan."); }

%%
