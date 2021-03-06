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
    bool _terminal;
    /* flag indicating whether or not this symbol is the start symbol */
    bool _start;
    /* flag indicating whether or not this symbol is epsilon */
    bool _epsilon;
    /* flag indicating whether or not this symbol is nullable */
    bool _nullable;
    /* first set for symbol */
    std::set<Symbol> firstSet;
    /* follow set for symbol */
    std::set<Symbol> followSet;

public:
    /* nothing */
    static const std::string DEAD;
    /* epsilon string representation */
    static const std::string EPSILON;
    /* real start symbol string */
    static const std::string START;
    /* special terminal symbol string */
    static const std::string END;
    /* all "valid" symbols will be exactly one character in length */
    Symbol(void) : marker(false),
                   symbol(Symbol::DEAD),
                   _terminal(false),
                   _start(false),
                   _epsilon(false),
                   _nullable(false) { ; }

    Symbol(const std::string &sym,
           bool marked = false,
           bool terminal = true,
           bool start = false) : marker(marked),
                                 symbol(sym),
                                 _terminal(terminal),
                                 _start(start),
                                 _epsilon(Symbol::EPSILON == symbol),
                                 _nullable(false) { ; }

    ~Symbol(void) { ; }

    std::string sym(void) const { return this->symbol; }

    bool marked(void) const { return this->marker; }

    void mark(bool m = true) { this->marker = m; }

    bool terminal(void) const { return this->_terminal; }

    void terminal(bool is) { this->_terminal = is; }

    void start(bool is) { this->_start = is; }

    bool start(void) const { return this->_start; }

    bool epsilon(void) const { return this->_epsilon; }

    bool nullable(void) const { return this->_nullable; }

    void nullable(bool n) { this->_nullable = n; }

    std::set<Symbol> &firsts(void) { return this->firstSet; }

    std::set<Symbol> &follows(void) { return this->followSet; }
    /* == */
    friend bool operator==(const Symbol &s1,
                           const Symbol &s2);
    /* != */
    friend bool operator!=(const Symbol &s1,
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

    Symbol clhs(void) const { return this->leftHandSide; }

    std::vector<Symbol> &rhs(void) { return this->rightHandSide; }

    std::vector<Symbol> crhs(void) const { return this->rightHandSide; }

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

class FollowSetMarker : public CFGProductionMarker {
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
    /* grammar productions */
    CFGProductions productions;
    /* refresh some internal state */
    void refresh(void);
    /* compute nullable set */
    void computeNullable(void);
    /* first step when computing first sets */
    void refreshFirstSets(void);
    /* compute first sets */
    void computeFirstSets(void);
    /* performs prep work for computeFollowSets */
    void followsetPrep(void);
    /* compute follow sets */
    void computeFollowSets(void);
    /* performs prep work for parse table creation */
    void parseTablePrep(void);

public:
    CFG(void) { this->verbose = false; }

    CFG(const CFGProductions &productions);

    ~CFG(void) { ; }

    std::set<Symbol> getNonTerminals(void) const;

    std::set<Symbol> getTerminals(void) const;

    Symbol startSymbol(void) const;

    void beVerbose(bool v = true) { this->verbose = v; }

    void emitState(void) const;

    void crunch(void);

    static CFGProductions refresh(const CFGProductions &prods);

    CFGProductions &prods(void) { return this->productions; }

    /* cleans cfg based on marker, eraser, and algo behavior */
    void clean(const CFGProductionMarker &marker,
               const CFGProductionEraser &eraser,
               const CFGProductionHygieneAlgo &algo);

    void clean(void);
};

#endif
