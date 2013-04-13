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
    /* flag indicating whether or not this symbol is the start symbol */
    bool start;
    /* flag indicating whether or not this symbol is epsilon */
    bool epsilon;

public:
    static const std::string EPSILON;
    /* all "valid" symbols will be exactly one character in length */
    Symbol(void) : marker(false),
                   symbol("_0xDEADBEEF_"),
                   terminal(false),
                   start(false),
                   epsilon(false) { ; }

    Symbol(const std::string &sym,
           bool marked = false,
           bool isTerminal = false,
           bool isStart = false) : marker(marked),
                                   symbol(sym),
                                   terminal(isTerminal),
                                   start(isStart),
                                   epsilon(Symbol::EPSILON == symbol) { ; }

    ~Symbol(void) { ; }

    std::string sym(void) const { return this->symbol; }

    bool marked(void) const { return this->marker; }

    void mark(bool m = true) { this->marker = m; }

    bool isTerminal(void) const { return this->terminal; }

    void setIsTerminal(bool is) { this->terminal = is; }

    void setIsStart(bool is = true) { this->start = is; }

    bool isStart(void) const { return this->start; }

    bool isEpsilon(void) const { return Symbol::EPSILON == this->symbol; }
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
    
    bool rhsMarked(void) const;

    friend std::ostream &operator<<(std::ostream &out,
                                    const CFGProduction &production);

};
/* convenience typedefs */
typedef std::vector<CFGProduction> CFGProductions;

/* ////////////////////////////////////////////////////////////////////////// */
/* production hygiene marker, eraser, and algorithm classes */
/* ////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */
class CFGProductionMarker {
protected:
    bool verbose;
public:
    virtual void mark(CFGProductions &productions) const = 0;

    void beVerbose(bool v = true) { this->verbose = v; }
};

class GeneratingMarker : public CFGProductionMarker {
public:
    virtual void mark(CFGProductions &productions) const;
};

class ReachabilityMarker : public CFGProductionMarker {
public:
    virtual void mark(CFGProductions &productions) const;
};

class NullableMarker : public CFGProductionMarker {
public:
    virtual void mark(CFGProductions &productions) const;
};

/* ////////////////////////////////////////////////////////////////////////// */
class CFGProductionEraser {
protected:
    bool verbose;
public:
    virtual void erase(CFGProductions &productions) const = 0;

    void beVerbose(bool v = true) { this->verbose = v; }
};

class NonGeneratingEraser : public CFGProductionEraser {
public:
    virtual void erase(CFGProductions &productions) const;
};

class UnreachableEraser : public CFGProductionEraser {
public:
    virtual void erase(CFGProductions &productions) const;
};

/* ////////////////////////////////////////////////////////////////////////// */
class CFGProductionHygieneAlgo {
protected:
    bool verbose;
public:
    virtual void go(CFGProductions &productions) const = 0;

    void beVerbose(bool v = true) { this->verbose = v; }
};

class NonGeneratingHygiene : public CFGProductionHygieneAlgo {
public:
    virtual void go(CFGProductions &productions) const;
};

class UnreachableHygiene : public CFGProductionHygieneAlgo {
public:
    virtual void go(CFGProductions &productions) const;
};

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
    /* nullable set */
    std::set<Symbol> nullableSet;
    /* performs prep work for parse table creation */
    void parseTablePrep(void);

public:
    CFG(void);

    CFG(const CFGProductions &productions);

    ~CFG(void) { ; }

    std::set<Symbol> getNonTerminals(void) const;

    std::set<Symbol> getTerminals(void) const;

    void beVerbose(bool v = true) { this->verbose = v; }

    void emitState(void) const;

    void createParseTable(void);

    CFGProductions
    buildFullyPopulatedGrammar(const CFGProductions &prods) const;
    /* XXX move this to Base */
    template <typename T> static void emitAllMembers(const T &t);
    /* cleans old based on marker, eraser, and algo behavior */
    CFGProductions clean(const CFGProductionMarker &marker,
                         const CFGProductionEraser &eraser,
                         const CFGProductionHygieneAlgo &algo,
                         const CFGProductions &old) const;
    /* performs grammar hygiene operations on the calling instance */
    static void markAllSymbols(CFGProductions &productions,
                               const Symbol &symbol);

    void clean(void);

    void computeNullable(void);
};

#endif
