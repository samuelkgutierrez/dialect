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
};

/* ////////////////////////////////////////////////////////////////////////// */
/* context-free grammar class */
/* ////////////////////////////////////////////////////////////////////////// */
class CFG {
private:
    /* the start of the CFG */
    std::string startSymbol;
    /* list of ALL productions */
    std::vector<CFGProduction> productions;

public:
    CFG(void);
    ~CFG(void);
};

#endif
