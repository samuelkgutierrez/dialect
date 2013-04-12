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

#ifndef CFG_H_INCLUDED
#define CFG_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <string>
#include <vector>
#include <set>

/* ////////////////////////////////////////////////////////////////////////// */
/* symbol class */
/* ////////////////////////////////////////////////////////////////////////// */
class Symbol {
private:
    /* general marker for a symbol */
    bool marker;
    /* string representation of the grammar symbol */
    std::string symbol;
    /* flag indicating whether or not this symbol is a terminal symbol */
    bool terminal;

public:
    static const std::string EPSILON;
    /* all "valid" symbols will be exactly one character in length */
    Symbol(void) : marker(false),
                   symbol("_0xDEADBEEF_"),
                   terminal(false) { ; }

    Symbol(const std::string &sym,
           bool marked = false,
           bool isTerminal = false) : marker(marked),
                                      symbol(sym),
                                      terminal(isTerminal) { ; }

    ~Symbol(void) { ; }

    std::string sym(void) const { return this->symbol; }

    bool marked(void) const { return this->marker; }

    void mark(bool m = true) { this->marker = m; }

    bool isTerminal(void) const { return this->terminal; }

    void setIsTerminal(bool is) { this->terminal = is; }
    /* == */
    friend bool operator==(const Symbol &s1,
                           const Symbol &s2);
    /* < */
    friend bool operator<(const Symbol &s1,
                          const Symbol &s2);
    /* << */
    friend std::ostream &operator<<(std::ostream &out,
                                    const Symbol &symbol);
};

/* ////////////////////////////////////////////////////////////////////////// */
/* context-free grammar production class */
/* ////////////////////////////////////////////////////////////////////////// */
class CFGProduction {
private:
    /* left-hand side of production */
    Symbol leftHandSide;
    /* right-hand side of production */
    std::vector<Symbol> rightHandSide;

public:
    CFGProduction(void) { /* nothing to do */; }

    CFGProduction(const std::string &lhs,
                  const std::string &rhs = Symbol::EPSILON);

    ~CFGProduction(void) { /* nothing to do */; }

    Symbol &lhs(void) { return this->leftHandSide; }

    Symbol lhs(void) const { return this->leftHandSide; }

    std::vector<Symbol> &rhs(void) { return this->rightHandSide; }

    std::vector<Symbol> rhs(void) const { return this->rightHandSide; }

    friend std::ostream &operator<<(std::ostream &out,
                                    const CFGProduction &production);

};
/* convenience typedefs */
typedef std::vector<CFGProduction> CFGProductions;

/* ////////////////////////////////////////////////////////////////////////// */
/* context-free grammar class */
/* ////////////////////////////////////////////////////////////////////////// */
class CFG {
private:
    /* flag indicating whether or not to emit debug output to stdout */
    bool verbose;
    /* the start of the CFG */
    Symbol startSymbol;
    /* list of ALL productions discovered during parse */
    CFGProductions productions;
    /* list of productions after grammar hygiene */
    CFGProductions cleanProductions;
    /* list of terminals in the grammar */
    std::set<Symbol> terminals;
    /* list of non-terminals in the grammar */
    std::set<Symbol> nonTerminals;

public:
    CFG(void);

    CFG(const CFGProductions &productions);

    ~CFG(void) { ; }

    std::set<Symbol> getNonTerminals(void) const;

    std::set<Symbol> getTerminals(void) const;

    void beVerbose(bool v = true) { this->verbose = v; }

    void emitState(void) const;

    CFGProductions
    buildFullyPopulatedGrammar(const CFGProductions &prods) const;

    template <typename T> void emitAllMembers(const T &t) const;
    /* removes non-generating variables from the instance */
    CFGProductions
    rmNonGeneratingVars(const CFGProductions &old);
    /* removes unreachable variables from the instance */
    CFGProductions
    rmUnreachableVars(const CFGProductions &old);
    /* performs grammar hygiene operations on the calling instance */
    void clean(void);
};

#endif
