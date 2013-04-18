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

#include "UserInputParser.hxx"
#include "Constants.hxx"
#include "DialectException.hxx"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <locale>
#include <map>
#include <vector>

#include <errno.h>
#include <string.h>

using namespace std;

/* ////////////////////////////////////////////////////////////////////////// */
UserInputParser::UserInputParser(const string &fileToParse)
{
    string line;

    if ("-" == fileToParse) {
        cout << "dialect: ";
        istream *in = &cin;
        getline(*in, line);
        /* break it up into pieces */
        for (unsigned long c = 0; c < line.length(); ++c) {
            char *cp = &line[c];
            this->_input.push_back(Symbol(string(cp, 1)));
        }
    }
    else {
        ifstream *file;
        file = new ifstream(fileToParse.c_str());
        /* problem opening the file */
        if (!file->is_open()) {
            int err = errno;
            string eStr = "cannot open " + fileToParse +
                          ". " + strerror(err) + ".\n";
            throw DialectException(DIALECT_WHERE, eStr);
        }
        /* start building the strings */
        while (file->good()) {
            getline(*file, line);
            /* break it up into pieces */
            for (unsigned long c = 0; c < line.length(); ++c) {
                char *cp = &line[c];
                this->_input.push_back(Symbol(string(cp, 1)));
            }
        }
        /* close the file */
        file->close();
        delete file;
    }
}
