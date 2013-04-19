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

#ifndef LL1PARSER_INCLUDED
#define LL1PARSER_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Base.hxx"
#include "CFG.hxx"

#include <stack>
#include <vector>
#include <map>

typedef std::map<Symbol, std::map<Symbol, CFGProduction>> ParseTable;

class LL1Parser {
protected:
    bool _verbose;
    CFG _cfg;

public:
    LL1Parser(void) { this->_verbose = false; }

    ~LL1Parser(void) { ; }

    LL1Parser(const CFG &cfg) : _verbose(false),
                                _cfg(cfg) { ; }

    virtual void parse(const std::vector<Symbol> &input);

    void verbose(bool v = true) { this->_verbose = v; }
};

/* every strong-LL(1) grammar is an LL(1) grammar and vise-versa */
class StrongLL1Parser : public LL1Parser {
private:
    ParseTable _table;

    void initTable(void);

    std::stack<Symbol> predict(const Symbol &nont, const Symbol &input);

    void parseImpl(const std::vector<Symbol> &input, bool strong);

    void strongParse(const std::vector<Symbol> &input);

    void dynamicParse(const std::vector<Symbol> &input);

public:
    StrongLL1Parser(void) : LL1Parser() { ; }

    ~StrongLL1Parser(void) { ; }

    StrongLL1Parser(const CFG &cfg) : LL1Parser(cfg) { ; }

    virtual void parse(const std::vector<Symbol> &input);
};

#endif
