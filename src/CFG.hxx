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
/* context-free grammar production class */
/* ////////////////////////////////////////////////////////////////////////// */
class CFGProduction {
private:
    static const std::string EPSILON;
    /* left-hand side of production */
    std::string leftHandSide;
    /* right-hand side of production */
    std::string rightHandSide;

public:
    CFGProduction(void) { /* nothing to do */; }

    CFGProduction(std::string lhs,
                  std::string rhs = CFGProduction::EPSILON) :
        leftHandSide(lhs), rightHandSide(rhs) { ; }

    ~CFGProduction(void) { /* nothing to do */; }

    std::string lhs(void) const { return this->leftHandSide; }

    std::string rhs(void) const { return this->rightHandSide; }

    friend std::ostream &operator<<(std::ostream &out,
                                    const CFGProduction &production);
};

/* ////////////////////////////////////////////////////////////////////////// */
/* context-free grammar class */
/* ////////////////////////////////////////////////////////////////////////// */
class CFG {
private:
    /* flag indicating whether or not to emit debug output to stdout */
    bool verbose;
    /* the start of the CFG */
    std::string startSymbol;
    /* list of ALL productions discovered during parse */
    std::vector<CFGProduction> productions;
    /* list of terminals in the grammar */
    std::set<std::string> terminals;
    /* list of non-terminals in the grammar */
    std::set<std::string> nonTerminals;

public:
    CFG(void);

    CFG(std::vector<CFGProduction> productions);

    ~CFG(void) { ; }

    std::set<std::string> getNonTerminals(void) const;

    std::set<std::string> getTerminals(void) const;

    void beVerbose(bool v = true) { this->verbose = v; }

    void emitAllProductions(void) const;

    void emitAllNonTerminals(void) const;

    void emitState(void) const;
};

#endif
