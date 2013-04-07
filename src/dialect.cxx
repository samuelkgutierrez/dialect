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


#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "Constants.hxx"
#include "DialectException.hxx"
#include "cfg-parser.h"

extern int parserParse(FILE *fp);

using namespace std;

/* ////////////////////////////////////////////////////////////////////////// */
static void
echoHeader(void)
{
    cout                                                         << endl <<
    "         _/  _/            _/                        _/"    << endl <<
    "    _/_/_/        _/_/_/  _/    _/_/      _/_/_/  _/_/_/_/" << endl <<
    " _/    _/  _/  _/    _/  _/  _/_/_/_/  _/          _/"      << endl <<
    "_/    _/  _/  _/    _/  _/  _/        _/          _/"       << endl <<
    " _/_/_/  _/    _/_/_/  _/    _/_/_/    _/_/_/      _/_/"    << endl <<
                                                                 endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
usage(void)
{
    cout << endl << "usage:" << endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
parseCFG(string what)
{
    FILE *fp = NULL;

    if (0 != parserParse(stdin)) {
        string estr = "error encountered during CFG parse. cannot continue.";
        throw DialectException(DIALECT_WHERE, estr);
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
int
main(int argc, char **argv)
{
    bool verboseMode = false;
    string cfgDescription, fileToParse;

    if (3 != argc && 4 != argc) {
        usage();
        return EXIT_FAILURE;
    }
    else if (3 == argc) {
        cfgDescription = string(argv[1]);
        fileToParse = string(argv[2]);
    }
    else {
        if ("-v" != string(argv[1])) {
            usage();
            return EXIT_FAILURE;
        }
        cfgDescription = string(argv[2]);
        fileToParse = string(argv[3]);
        verboseMode = true;
    }
    try {
        echoHeader();
        parseCFG(cfgDescription);
    }
    catch (DialectException &e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
