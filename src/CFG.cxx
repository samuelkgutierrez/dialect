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

/* this is okay because our parser makes sure that we can never get a space on
 * either side of any production rule. if this ever changes, then this "special"
 * character will need to be changed. */
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

    /* the order here matters. we need to first calculate the non-terminals
     * because the getTerminals code uses nonTerminals to determine the
     * terminals set. */
    this->verbose = false;
    /* initially start with given productions */
    this->productions = this->cleanProductions = productions;
    firstProduction = *this->productions.begin();
    this->startSymbol = firstProduction.lhs();
    this->nonTerminals = this->getNonTerminals();
    this->terminals = this->getTerminals();
}

/* ////////////////////////////////////////////////////////////////////////// */
template <typename T>
void
CFG::emitAllMembers(const T &t) const
{
    typename T::const_iterator member;

    for (member = t.begin(); t.end() != member; ++member) {
        dout << "  " << *member << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::emitState(void) const
{
    dout << endl;
    dout << "start symbol: " << this->startSymbol << endl;

    dout << "non-terminals begin" << endl;
    this->emitAllMembers(this->nonTerminals);
    dout << "non-terminals end" << endl;

    dout << "terminals begin" << endl;
    this->emitAllMembers(this->terminals);
    dout << "terminals end" << endl;

    dout << "productions begin" << endl;
    this->emitAllMembers(this->productions);
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

/* ////////////////////////////////////////////////////////////////////////// */
/* this is also pretty easy because once we have all of our non-terminals,
 * everything that is left must be terminals. */
set<string>
CFG::getTerminals(void) const
{
    vector<CFGProduction>::const_iterator production;
    /* set of all right-hand side symbols */
    set<string> allRHSSymbols;
    /* the result */
    set<string> terms;

    for (production = this->productions.begin();
         this->productions.end() != production;
         ++production) {
        /* the right-hand side is going to be a string of symbols. we need to
         * split those up first because each right-hand side can contain many
         * symbols. */
        string rhsString = production->rhs();
        for (unsigned c = 0; c < rhsString.length(); ++c) {
            allRHSSymbols.insert(string(&rhsString[c], 1));
        }
    }
    /* now that we have all the rhs symbols in a set, get the set of terminals
     * by simply taking the set difference of allRHSSymbols and nonTerminals */
    set_difference(allRHSSymbols.begin(), allRHSSymbols.end(),
                   this->nonTerminals.begin(), this->nonTerminals.end(),
                   inserter(terms, terms.end()));

    return terms;
}

/* ////////////////////////////////////////////////////////////////////////// */
vector<CFGProduction>
CFG::rmNonGeneratingVars(const vector<CFGProduction> &old)
{
    vector<CFGProduction> newProds;

    return newProds;
}

/* ////////////////////////////////////////////////////////////////////////// */
vector<CFGProduction>
CFG::rmUnreachableVars(const vector<CFGProduction> &old)
{
    vector<CFGProduction> newProds;

    return newProds;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::clean(void)
{
    /* the order of this matters. first we find and remove non-generating
     * productions and their rules and then we do the same for non-reachable
     * variables. */
    this->cleanProductions = this->rmNonGeneratingVars(this->productions);
    this->cleanProductions = this->rmUnreachableVars(this->cleanProductions);
}
