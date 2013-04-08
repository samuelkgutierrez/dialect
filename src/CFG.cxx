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

#include "Base.hxx"
#include "CFG.hxx"
#include "Constants.hxx"
#include "DialectException.hxx"

#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const string CFGProduction::EPSILON = " ";

/* ////////////////////////////////////////////////////////////////////////// */
ostream &
operator<<(ostream &out, const CFGProduction &production)
{
    out << production.leftHandSide << " --> " << production.rightHandSide;
    return out;
}

/* ////////////////////////////////////////////////////////////////////////// */
CFG::CFG(void)
{
    this->verbose = false;
}

/* ////////////////////////////////////////////////////////////////////////// */
CFG::CFG(vector<CFGProduction> productions)
{
    CFGProduction firstProduction;

    this->verbose = false;
    this->productions = productions;
    firstProduction = *this->productions.begin();
    this->startSymbol = firstProduction.lhs();
    this->nonTerminals = this->getNonTerminals();
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::emitAllProductions(void) const
{
    vector<CFGProduction>::const_iterator production;
    for (production = this->productions.begin();
         this->productions.end() != production;
         ++production) {
        dout << "  " << *production << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::emitAllNonTerminals(void) const
{
    set<string>::const_iterator nonTerm;

    for (nonTerm = this->nonTerminals.begin();
         this->nonTerminals.end() != nonTerm;
         ++nonTerm) {
        dout << "  " << *nonTerm << endl;
    }
}


/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::emitState(void) const
{
    dout << endl;
    dout << "start symbol: " << this->startSymbol << endl;
    dout << "non-terminals begin" << endl;
    this->emitAllNonTerminals();
    dout << "non-terminals end" << endl;
    dout << "productions begin" << endl;
    this->emitAllProductions();
    dout << "productions end" << endl;
    dout << endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* this is easy because we know that only non-terminals are going to be on the
 * left-hand side of all of our productions. just iterate over all the
 * productions and stash the left-hand sides. */
set<string>
CFG::getNonTerminals(void) const
{
    set<string> nonTerms;
    vector<CFGProduction>::const_iterator production;

    for (production = this->productions.begin();
         this->productions.end() != production;
         ++production) {
        nonTerms.insert(production->lhs());
    }
    return nonTerms;
}
