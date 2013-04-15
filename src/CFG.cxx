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

const string Symbol::DEAD = "_0xDEADBEEF_";
/* this is okay because our parser makes sure that we can never get a space on
 * either side of any production rule. if this ever changes, then this "special"
 * character will need to be changed. */
const string Symbol::EPSILON = " ";
/* our parser doesn't allow multi-char string, so this is okay */
const string Symbol::START = "S'";
/* our scanner doesn't accept $s, so this is okay */
const string Symbol::END = "$";

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
    for (const Symbol &p : production.rightHandSide) {
        out << p;
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
    for (const Symbol &sym : this->rightHandSide) {
        if (!sym.marked()) return false;
    }
    return true;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* production marker, eraser classes */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */
void
GeneratingMarker::mark(CFGProductions &productions) const
{
    /* init the symbol markers by marking all terminals and making sure that
     * non-terminals aren't marked at this point. */
    for (CFGProduction &p : productions) {
        p.lhs().mark(false);
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.isTerminal());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
ReachabilityMarker::mark(CFGProductions &productions) const
{
    /* this one is easy. mark the start symbol. */
    for (CFGProduction &p : productions) {
        if (p.lhs().isStart()) p.lhs().mark(true);
        else p.lhs().mark(false);
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.isStart());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
NullableMarker::mark(CFGProductions &productions) const
{
    /* start by marking all epsilons */
    for (CFGProduction &p : productions) {
        /* the lhs can't be a terminal, so it can't be epsilon */
        p.lhs().mark(false);
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.isEpsilon());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
FollowSetMarker::mark(CFGProductions &productions) const
{
    /* this one is easy, just mark all terminals */
    for (CFGProduction &p : productions) {
        /* left-hand sides are always non-terminals */
        p.lhs().mark(false);
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.isTerminal());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
NonGeneratingEraser::erase(CFGProductions &productions) const
{
    if (this->verbose) {
        dout << __func__ << ": removing non-generating symbols..." << endl;
    }
    for (auto p = productions.begin(); productions.end() != p;) {
        if (!p->lhs().marked() || !p->rhsMarked()) {
            if (this->verbose) {
                dout << "  rm " << *p << endl;
            }
            p = productions.erase(p);
        }
        else ++p;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
UnreachableEraser::erase(CFGProductions &productions) const
{
    if (this->verbose) {
        dout << __func__ << ": removing unreachable symbols..." << endl;
    }
    for (auto p = productions.begin(); productions.end() != p;) {
        if (!p->lhs().marked()) {
            if (this->verbose) {
                dout << "  rm " << *p << endl;
            }
            p = productions.erase(p);
        }
        else ++p;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
UnreachableHygiene::go(CFGProductions &productions) const
{
    bool hadUpdate;
    do {
        hadUpdate = false;
        if (this->verbose) {
            dout << __func__ << ": in main loop" << endl;
        }
        for (CFGProduction &p : productions) {
            if (p.lhs().marked() && !p.rhsMarked()) {
                if (this->verbose) {
                    dout << "  marking " << endl;
                    CFG::emitAllMembers(p.rhs());
                }
                for (Symbol &sym : p.rhs()) {
                    CFG::markAllSymbols(productions, sym.sym());
                }
                hadUpdate = true;
            }
        }
        if (!hadUpdate) {
            if (this->verbose) dout << "  done!" << endl;
        }
    } while (hadUpdate);
}

/* ////////////////////////////////////////////////////////////////////////// */
void
NonGeneratingHygiene::go(CFGProductions &productions) const
{
    bool hadUpdate;
    do {
        hadUpdate = false;
        if (this->verbose) {
            dout << __func__ << ": in main loop" << endl;
        }
        for (CFGProduction &p : productions) {
            if (!p.lhs().marked() && p.rhsMarked()) {
                if (this->verbose) {
                    dout << "  marking " << p.lhs() << endl;
                }
                /* make sure that we update all instances of lhs()->sym() */
                CFG::markAllSymbols(productions, p.lhs().sym());
                hadUpdate = true;
            }
        }
        if (!hadUpdate) {
            if (this->verbose) dout << "  done!" << endl;
        }
    } while (hadUpdate);
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

    /* stash the start symbol */
    CFGProduction firstProduction = *productions.begin();
    Symbol startSymbol = firstProduction.lhs();

    /* add a new, special terminal, $ and new start production */
    fpp.insert(fpp.begin(), CFGProduction(Symbol::START,
                                          startSymbol.sym() + Symbol::END));

    startSymbol = Symbol(Symbol::START);

    /* first mark all non-terminals on the lhs and add to nonTerminals */
    for (CFGProduction &prod : fpp) {
        prod.lhs().setIsTerminal(false);
        nonTerminals.insert(prod.lhs());
        if (startSymbol == prod.lhs()) prod.lhs().setIsStart(true);
    }
    /* now that we know about all the non-terminals, finish setup by updating
     * the symbols on the right-hand side. */
    for (CFGProduction &prod : fpp) {
        /* iterate over all the symbols on the rhs */
        for (Symbol &sym : prod.rhs()) {
            /* not in set of non-terminals, so must be a terminal */
            if (nonTerminals.end() == nonTerminals.find(sym)) {
                sym.setIsTerminal(true);
            }
            /* must be a non-terminal on the rhs */
            else {
                sym.setIsTerminal(false);
            }
            if (startSymbol == sym) sym.setIsStart(true);
        }
    }
    return fpp;
}

/* ////////////////////////////////////////////////////////////////////////// */
template <typename T>
void
CFG::emitAllMembers(const T &t)
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
    CFG::emitAllMembers(this->nonTerminals);
    dout << "non-terminals end" << endl;

    dout << "terminals begin" << endl;
    CFG::emitAllMembers(this->terminals);
    dout << "terminals end" << endl;

    dout << "productions begin" << endl;
    CFG::emitAllMembers(this->productions);
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

    for (const CFGProduction &p : this->productions) {
        nonTerms.insert(const_cast<CFGProduction &>(p).lhs());
    }
    return nonTerms;
}

/* ////////////////////////////////////////////////////////////////////////// */
set<Symbol>
CFG::getTerminals(void) const
{
    set<Symbol> terms;

    for (const CFGProduction &production : this->productions) {
        for (const Symbol &s : const_cast<CFGProduction &>(production).rhs()) {
            if (s.isTerminal()) terms.insert(s);
        }
    }
    return terms;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::markAllSymbols(CFGProductions &productions,
                    const Symbol &symbol)
{
    for (CFGProduction &p : productions) {
        if (symbol == p.lhs()) p.lhs().mark(true);
        for (Symbol &sym : p.rhs()) {
            if (symbol == sym) sym.mark(true);
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
markAllRHS(CFGProductions &productions,
           const Symbol &symbol)
{
    for (CFGProduction &p : productions) {
        for (Symbol &sym : p.rhs()) {
            if (symbol == sym) sym.mark(true);
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::propagateFirsts(CFGProductions &productions,
                     const Symbol &symbol,
                     const set<Symbol> &firstSet)
{
    for (CFGProduction &p : productions) {
        if (symbol == p.lhs()) {
            p.lhs().firsts().insert(firstSet.begin(), firstSet.end());
        }
        for (Symbol &sym : p.rhs()) {
            if (symbol == sym) {
                sym.firsts().insert(firstSet.begin(), firstSet.end());
            }
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::propagateFollows(CFGProductions &productions,
                      const vector<Symbol> &what)
{
    for (CFGProduction &p : productions) {
        vector<Symbol>::const_iterator i;
        if (what.end() != (i = find(what.begin(), what.end(), p.lhs()))) {
            p.lhs().follows().insert(const_cast<Symbol &>(*i).follows().begin(),
                                     const_cast<Symbol &>(*i).follows().end());
        }
        for (Symbol &sym : p.rhs()) {
            if (what.end() != (i = find(what.begin(), what.end(), sym))) {
                sym.follows().insert(const_cast<Symbol &>(*i).follows().begin(),
                                     const_cast<Symbol &>(*i).follows().end());

            }
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
CFGProductions
CFG::clean(const CFGProductionMarker &marker,
           const CFGProductionEraser &eraser,
           const CFGProductionHygieneAlgo &algo,
           const CFGProductions &old) const
{
    CFGProductions newProds = old;

    if (this->verbose) {
        dout << __func__ << ": grammar hygiene begin ***" << endl;
    }
    /* start by marking all symbols */
    marker.mark(newProds);
    /* run the hygiene algo */
    algo.go(newProds);
    /* erase unproductive productions */
    eraser.erase(newProds);

    if (this->verbose) {
        dout << __func__ << ": here is the new cfg:" << endl;
        CFG::emitAllMembers(newProds);
        dout << __func__ << ": grammar hygiene end ***" << endl;
        dout << endl;
    }
    return newProds;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::clean(void)
{
    /* the order of this matters. first we find and remove non-generating
     * productions and their rules and then we do the same for non-reachable
     * variables. */
    GeneratingMarker     gMarker;  gMarker.beVerbose(this->verbose);
    NonGeneratingEraser  gEraser;  gEraser.beVerbose(this->verbose);
    NonGeneratingHygiene gHygiene; gHygiene.beVerbose(this->verbose);

    ReachabilityMarker rMarker;  rMarker.beVerbose(this->verbose);
    UnreachableEraser  rEraser;  rEraser.beVerbose(this->verbose);
    UnreachableHygiene rHygiene; rHygiene.beVerbose(this->verbose);

    /**
     * general algorithm for removing non-generating symbols:
     * mark all terminals
     * while no more markable non-terminals
     *     if (all symbols on the rhs are marked)
     *         mark lhs
     *     fi
     * done
     */
    this->cleanProductions = this->clean(gMarker, gEraser,
                                         gHygiene, this->productions);
    /**
     * general algorithm for removing unreachable symbols
     * mark the start symbol
     * while no more markable non-terminals
     *     if (rhs is marked)
     *         mark rhs
     *     fi
     * done
     */
    this->cleanProductions = this->clean(rMarker, rEraser,
                                         rHygiene, this->cleanProductions);
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::createParseTable(void)
{
    this->parseTablePrep();
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::parseTablePrep(void)
{
    /* the order of this matters. */
    this->computeNullable();
    this->computeFirstSets();
    this->computeFollowSets();
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::computeNullable(void)
{
    CFGProductions pCopy = this->cleanProductions;
    NullableMarker marker; marker.beVerbose(this->verbose);
    bool hadUpdate;

    if (this->verbose) {
        dout << __func__ << ": nullable fixed-point begin ***" << endl;
    }
    /* init symbol markers for nullable calculation */
    marker.mark(pCopy);
    /* start the fixed-point calculation */
    do {
        hadUpdate = false;
        if (this->verbose) {
            dout << __func__ << ": in main loop" << endl;
        }
        for (CFGProduction &p : pCopy) {
            if (p.rhsMarked() && !p.lhs().marked()) {
                if (this->verbose) {
                    dout << "  marking " << p.lhs() << endl;
                }
                CFG::markAllSymbols(pCopy, p.lhs().sym());
                hadUpdate = true;
                /* add this symbol to the nullable set */
                this->nullableSet.insert(p.lhs().sym());
            }
        }
        if (!hadUpdate) if (this->verbose) dout << "  done!" << endl;
    } while (hadUpdate);

    if (this->verbose) {
        dout << __func__ << ": here are the nullable non-terminals:" << endl;
        CFG::emitAllMembers(this->nullableSet);
        dout << __func__ << ": nullable fixed-point end ***" << endl;
        dout << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::initFirstSets(void)
{
    for (CFGProduction &p : this->cleanProductions) {
        p.lhs().mark(false);
        for (Symbol &sym : p.rhs()) {
            if (sym.isTerminal()) {
                /* mark all terminals */
                sym.mark(true);
                /* add myself to my firsts if not epsilon 8-| */
                if (!sym.isEpsilon()) sym.firsts().insert(sym);
            }
            else sym.mark(false);
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::computeFirstSets(void)
{
    bool hadUpdate;

    if (this->verbose) dout << __func__ << ": fixed-point begin ***" << endl;

    this->initFirstSets();
    /* start the fixed-point calculation */
    do {
        hadUpdate = false;
        if (this->verbose) dout << __func__ << ": in main loop" << endl;

        for (CFGProduction &p : this->cleanProductions) {
            if (!p.lhs().marked()) {
                Symbol alpha = *p.rhs().begin();
                /* this must mean that the rhs hasn't been updated yet for this
                 * particular production. so, just continue. */
                if (0 == alpha.firsts().size() && !alpha.isTerminal()) {
                    hadUpdate = true;
                    continue;
                }
                /* if alpha is nullable */
                if (this->nullableSet.end() != this->nullableSet.find(alpha)) {
                    /* need FIRST(alpha) U FIRST(beta) */
                    for (Symbol &sym : p.rhs()) {
                        p.lhs().firsts().insert(sym.firsts().begin(),
                                                sym.firsts().end());
                    }
                }
                /* else alpha is not nullable, so FIRST(alpha) is all we need */
                else {
                    p.lhs().firsts().insert(alpha.firsts().begin(),
                                            alpha.firsts().end());
                }
                if (this->verbose) dout << "  marking " << p.lhs() << endl;
                p.lhs().mark(true);
                CFG::propagateFirsts(this->cleanProductions,
                                     p.lhs().sym(),
                                     p.lhs().firsts());
                markAllRHS(this->cleanProductions, p.lhs().sym());
                hadUpdate = true;
            }
        }
        if (!hadUpdate) if (this->verbose) dout << "  done!" << endl;
    } while (hadUpdate);

    if (this->verbose) {
        dout << __func__ << ": here are the first sets:" << endl;
        set<Symbol> lhsSet;
        for (const CFGProduction &p : this->cleanProductions) {
            lhsSet.insert(const_cast<CFGProduction &>(p).lhs());
            vector<Symbol> rhs = const_cast<CFGProduction &>(p).rhs();
            lhsSet.insert(rhs.begin(), rhs.end());
        }
        for (const Symbol &sym : lhsSet) {
            dout << sym << " begin" << endl;
            CFG::emitAllMembers(const_cast<Symbol &>(sym).firsts());
            dout << sym << " end" << endl;
        }
        dout << __func__ << ": fixed-point end ***" << endl;
        dout << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::followSetPrep(void)
{
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::computeFollowSets(void)
{
    bool hadUpdate;
    FollowSetMarker marker;

    if (this->verbose) dout << __func__ << ": begin ***" << endl;

    /* reset markers before we start calculating follow sets */
    marker.mark(this->cleanProductions);

    do {
        if (this->verbose) dout << __func__ << ": in main loop" << endl;
        hadUpdate = false;
        vector<Symbol> updates;
        vector<Symbol>::iterator update;
        for (CFGProduction &p : this->cleanProductions) {
            if (!p.lhs().marked()) {
                auto sym = p.rhs().begin();
                while (sym != p.rhs().end()) {
                    if (!sym->isTerminal()) {
                        /* safe because we know that sym != rhs().end() */
                        advance(sym, 1);
                        if (p.rhs().end() == sym) {
                            continue;
                        }
                        Symbol rneighbor = *sym;
                        advance(sym, -1);
                        sym->follows().insert(rneighbor.firsts().begin(),
                                              rneighbor.firsts().end());
                        if (this->nullable(rneighbor)) {
                            sym->follows().insert(p.lhs().follows().begin(),
                                                  p.lhs().follows().end());
                        }
                        if (updates.end() == (update = find(updates.begin(),
                                                            updates.end(),
                                                            *sym))) {
                            updates.push_back(*sym);
                        }
                        /* sym is already in the updates vector, so just add to
                         * the follow set. */
                        else {
                            update->follows().insert(sym->follows().begin(),
                                                     sym->follows().end());
                        }
                        sym->mark();
                    }
                    ++sym;
                }
                if (this->verbose) dout << "  marking " << p.lhs() << endl;
                p.lhs().mark(true);
                CFG::propagateFollows(this->cleanProductions, updates);
                hadUpdate = true;
            }
        }
        if (!hadUpdate) if (this->verbose) dout << "  done!" << endl;
    } while (hadUpdate);
    if (this->verbose) {
        dout << __func__ << ": here are the follow sets:" << endl;
        set<Symbol> lhsSet;
        for (const CFGProduction &p : this->cleanProductions) {
            lhsSet.insert(const_cast<CFGProduction &>(p).lhs());
            vector<Symbol> rhs = const_cast<CFGProduction &>(p).rhs();
            lhsSet.insert(rhs.begin(), rhs.end());
        }
        for (const Symbol &sym : lhsSet) {
            dout << sym << " begin" << endl;
            CFG::emitAllMembers(const_cast<Symbol &>(sym).follows());
            dout << sym << " end" << endl;
        }
        dout << __func__ << ": end ***" << endl;
        dout << endl;
    }
}
