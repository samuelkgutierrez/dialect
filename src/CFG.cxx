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

#include "CFG.hxx"
#include "Base.hxx"
#include "Constants.hxx"
#include "DialectException.hxx"

#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* Symbol */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/* this is okay because our parser makes sure that we can never get a space on
 * either side of any production rule. if this ever changes, then this "special"
 * character will need to be changed. */
const string Symbol::EPSILON = " ";

/* ////////////////////////////////////////////////////////////////////////// */
bool
operator==(const Symbol &s1,
           const Symbol &s2)
{
    return s1.symbol == s2.symbol;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* < */
bool
operator<(const Symbol &s1,
          const Symbol &s2)
{
    return s1.symbol < s2.symbol;
}

/* ////////////////////////////////////////////////////////////////////////// */
ostream &
operator<<(ostream &out,
           const Symbol &symbol)
{
    out << symbol.symbol;
    return out;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* CFGProduction */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
ostream &
operator<<(ostream &out,
           const CFGProduction &production)
{
    out << production.leftHandSide << " --> ";
    vector<Symbol>::const_iterator p;
    for (p = production.rhs().begin(); production.rhs().end() != p; ++p) {
        cout << *p;
    }
    return out;
}

/* ////////////////////////////////////////////////////////////////////////// */
CFGProduction::CFGProduction(const string &lhs,
                             const string &rhs)
{
    /* left-hand side symbols are always non-terminals, but we'll just init
     * everything with similarly. */
    this->leftHandSide = Symbol(lhs);
    for (unsigned i = 0; i < rhs.length(); ++i) {
        /* at this point we don't know what type the symbols are. */
        this->rightHandSide.push_back(Symbol(string(&rhs[i], 1)));
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
bool
CFGProduction::rhsMarked(void) const
{
    for (vector<Symbol>::const_iterator sym = this->rhs().begin();
         this->rhs().end() != sym;
         ++sym) {
        if (!sym->marked()) return false;
    }
    return true;
}


/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* CFG */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */
CFG::CFG(void)
{
    this->verbose = false;
}

/* ////////////////////////////////////////////////////////////////////////// */
CFG::CFG(const CFGProductions &productions)
{
    this->verbose = false;

    /* we can't assume that the vector of productions that we are being passed
     * is completely valid. that is, some symbol information may be incorrect.
     * so, first take the incomplete productions vector and generate a new,
     * fully populated production vector that we can trust. this vector will
     * be built from the parsed CFG, so it may not be "clean." */
    this->productions = this->buildFullyPopulatedGrammar(productions);
    /* init clean with all productions */
    this->cleanProductions = this->productions;
    /* get the start symbol */
    CFGProduction firstProduction = *this->productions.begin();
    this->startSymbol = firstProduction.lhs();
    /* now capture some extra information for nice output */
    this->nonTerminals = this->getNonTerminals();
    this->terminals = this->getTerminals();
}

/* ////////////////////////////////////////////////////////////////////////// */
CFGProductions
CFG::buildFullyPopulatedGrammar(const CFGProductions &productions) const
{
    /* fully populated productions */
    CFGProductions fpp = productions;
    /* set of non-terminals */
    set<Symbol> nonTerminals;

    /* first mark all non-terminals on the lhs and add to nonTerminals */
    for (CFGProductions::iterator prod = fpp.begin();
         fpp.end() != prod;
         ++prod) {
        prod->lhs().setIsTerminal(false);
        nonTerminals.insert(prod->lhs());
    }
    /* now that we know about all the non-terminals, finish setup by updating
     * the symbols on the right-hand side. */
    for (CFGProductions::iterator prod = fpp.begin();
         fpp.end() != prod;
         ++prod) {
        vector<Symbol> &rhs = prod->rhs();
        /* iterate over all the symbols on the rhs */
        for (vector<Symbol>::iterator sym = rhs.begin();
             rhs.end() != sym;
             ++sym) {
            /* not in set of non-terminals, so must be a terminal */
            if (nonTerminals.end() == nonTerminals.find(*sym)) {
                sym->setIsTerminal(true);
            }
            /* must be a non-terminal on the rhs */
            else {
                sym->setIsTerminal(false);
            }
        }
    }
    return fpp;
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
    dout << "start symbol: " << this->startSymbol.sym() << endl;

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
set<Symbol>
CFG::getNonTerminals(void) const
{
    set<Symbol> nonTerms;

    for (CFGProductions::const_iterator p = this->productions.begin();
         this->productions.end() != p;
         ++p) {
        nonTerms.insert(p->lhs());
    }
    return nonTerms;
}

/* ////////////////////////////////////////////////////////////////////////// */
set<Symbol>
CFG::getTerminals(void) const
{
    CFGProductions::const_iterator production;
    /* the result */
    set<Symbol> terms;

    for (production = this->productions.begin();
         this->productions.end() != production;
         ++production) {
        vector<Symbol> rhs = production->rhs();
        for (vector<Symbol>::const_iterator sym = rhs.begin();
             rhs.end() != sym;
             ++sym) {
            if (sym->isTerminal()) {
                terms.insert(*sym);
            }
        }
    }
    return terms;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
markAllSymbols(CFGProductions &productions,
               const Symbol &symbol)
{
    for (CFGProductions::iterator p = productions.begin();
         productions.end() != p;
         ++p) {
        if (symbol == p->lhs()) p->lhs().mark(true);
        vector<Symbol> &rhs = p->rhs();
        for (vector<Symbol>::iterator sym = rhs.begin();
             rhs.end() != sym;
             ++sym) {
            if (symbol == *sym) sym->mark(true);
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
/**
 * general algorithm for removing non-generating symbols:
 * mark all terminals
 * while no more markable non-terminals
 *     if (all symbols on the rhs are marked)
 *         mark lhs
 *     fi
 * done
 */
CFGProductions
CFG::rmNonGeneratingSyms(const CFGProductions &old) const
{
    CFGProductions newProds = old;
    /* init the symbol markers by marking all terminals and making sure that
     * non-terminals aren't marked at this point. */
    for (CFGProductions::iterator p = newProds.begin();
         newProds.end() != p;
         ++p) {
        p->lhs().mark(false);
        vector<Symbol> &rhs = p->rhs();
        for (vector<Symbol>::iterator sym = rhs.begin();
             rhs.end() != sym;
             ++sym) {
            if (sym->isTerminal()) {
                sym->mark(true);
            }
            else sym->mark(false);
        }
    }
    bool hadUpdate;
    do {
        hadUpdate = false;
        if (this->verbose) {
            dout << __func__ << ": in main loop" << endl;
        }
        for (CFGProductions::iterator p = newProds.begin();
             newProds.end() != p;
             ++p) {
            if (!p->lhs().marked() && p->rhsMarked()) {
                if (this->verbose) {
                    dout << "marking " << p->lhs() << endl;
                }
                /* make sure that we update all instances of lhs()->sym() */
                markAllSymbols(newProds, p->lhs().sym());
                hadUpdate = true;
            }
        }
        if (!hadUpdate) {
            dout << "done!" << endl;
        }
    } while (hadUpdate);
    if (this->verbose) {
        dout << __func__ << ": removing non-generating symbols..." << endl;
    }
    for (CFGProductions::iterator p = newProds.begin(); newProds.end() != p;) {
        if (!p->lhs().marked()) {
            dout << __func__ << ": rm " << *p << endl;
            p = newProds.erase(p);
        }
        else ++p;
    }
    if (this->verbose) {
        dout << __func__ << ": done removing non-generating symbols..." << endl;
    }

    return newProds;
}

/* ////////////////////////////////////////////////////////////////////////// */
CFGProductions
CFG::rmUnreachableVars(const CFGProductions &old)
{
    CFGProductions newProds;

    return newProds;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::clean(void)
{
    /* the order of this matters. first we find and remove non-generating
     * productions and their rules and then we do the same for non-reachable
     * variables. */
    this->cleanProductions = this->rmNonGeneratingSyms(this->productions);
    this->cleanProductions = this->rmUnreachableVars(this->cleanProductions);
}
