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

#include <vector>
#include <map>

typedef std::map<Symbol, std::map<Symbol, CFGProduction>> ParseTable;

class LL1Parser {
protected:
    bool _verbose;
    CFG _cfg;
    std::vector<Symbol> _input;

public:
    LL1Parser(void) { this->_verbose = false; }

    ~LL1Parser(void) { ; }

    LL1Parser(const CFG &cfg,
              const std::vector<Symbol> &input) : _verbose(false),
                                                  _cfg(cfg),
                                                  _input(input) { ; }

    virtual void parse(void);

    void verbose(bool v = true) { this->_verbose = v; }
};

class StrongLL1Parser : public LL1Parser {
private:
    ParseTable _table;

    void initTable(void);

    void parseImpl(void);

public:
    StrongLL1Parser(void) : LL1Parser() { ; }

    ~StrongLL1Parser(void) { ; }

    StrongLL1Parser(const CFG &cfg,
                    const std::vector<Symbol> &input) :
        LL1Parser(cfg, input) { ; }

    virtual void parse(void);
};

#endif
