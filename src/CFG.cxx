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
        p.lhs().mark(p.lhs().isEpsilon());
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
            if (this->verbose) dout << "  rm " << *p << endl;
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
            if (this->verbose) dout << "  rm " << *p << endl;
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
        for (CFGProduction &p : productions) {
            if (p.lhs().marked() && !p.rhsMarked()) {
                if (this->verbose) {
                    dout << "  marking: ";
                    CFG::emitAllMembers(p.rhs(), false);
                }
                for (Symbol &sym : p.rhs()) {
                    CFG::markAllSymbols(productions, sym.sym());
                }
                hadUpdate = true;
            }
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
CFG::CFG(const CFGProductions &productions)
{
    this->verbose = false;

    /* we can't assume that the vector of productions that we are being passed
     * is completely valid. that is, some symbol information may be incorrect.
     * so, first take the incomplete productions vector and generate a new,
     * fully populated production vector that we can trust. this vector will
     * be built from the parsed CFG, so it may not be "clean." */
    this->productions = this->buildFullyPopulatedGrammar(productions);
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

    /* first mark all non-terminals on the lhs and add to nonTerminals */
    for (CFGProduction &prod : fpp) {
        prod.lhs().setIsTerminal(false);
        nonTerminals.insert(prod.lhs());
        prod.lhs().setIsStart(startSymbol == prod.lhs());
    }
    /* now that we know about all the non-terminals, finish setup by updating
     * the symbols on the right-hand side. */
    for (CFGProduction &prod : fpp) {
        /* iterate over all the symbols on the rhs */
        for (Symbol &sym : prod.rhs()) {
            /* if not in set of non-terminals, so must be a terminal
             * else must be a non-terminal on the rhs */
            sym.setIsTerminal(nonTerminals.end() == nonTerminals.find(sym));
            if (startSymbol == sym) sym.setIsStart(true);
        }
    }
    return fpp;
}

/* ////////////////////////////////////////////////////////////////////////// */
template <typename T>
void
CFG::emitAllMembers(const T &t, bool nls)
{
    typename T::const_iterator member;

    if (nls) {
        for (member = t.begin(); t.end() != member; ++member) {
            dout << "  " << *member << endl;
        }
    }
    else {
        cout << "{";
        for (member = t.begin(); t.end() != member; ++member) {
            if (member != t.begin()) cout << ", ";
            cout << *member;
        }
        cout << "}" << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
Symbol
CFG::startSymbol(void) const
{
    CFGProduction fp = *this->productions.begin();
    return fp.lhs();
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::emitState(void) const
{
    dout << endl;
    dout << "start symbol: " << this->startSymbol() << endl;

    dout << "non-terminals begin" << endl;
    CFG::emitAllMembers(this->getNonTerminals());
    dout << "non-terminals end" << endl;

    dout << "terminals begin" << endl;
    CFG::emitAllMembers(this->getTerminals());
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
/* XXX FIXME rm firstSet */
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
                      const Symbol &s)
{
    for (CFGProduction &p : productions) {
        if (s == p.lhs()) {
            p.lhs().follows().insert(const_cast<Symbol &>(s).follows().begin(),
                                     const_cast<Symbol &>(s).follows().end());
        }
        for (Symbol &sym : p.rhs()) {
            if (s == sym) {
                sym.follows().insert(const_cast<Symbol &>(s).follows().begin(),
                                     const_cast<Symbol &>(s).follows().end());
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
    this->productions = this->clean(gMarker, gEraser,
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
    this->productions = this->clean(rMarker, rEraser,
                                    rHygiene, this->productions);
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
    //CFGProductions pCopy = this->productions;
    NullableMarker marker; marker.beVerbose(this->verbose);
    bool hadUpdate;

    if (this->verbose) {
        dout << __func__ << ": nullable fixed-point begin ***" << endl;
    }
    /* init symbol markers for nullable calculation */
    marker.mark(this->productions);
    /* start the fixed-point calculation */
    do {
        hadUpdate = false;
        for (CFGProduction &p : this->productions) {
            if (p.rhsMarked() && !p.lhs().marked()) {
                CFG::markAllSymbols(this->productions, p.lhs().sym());
                hadUpdate = true;
                /* add this symbol to the nullable set */
                /* XXX RM this */
                this->nullableSet.insert(p.lhs().sym());
            }
        }
    } while (hadUpdate);

    if (this->verbose) {
        if (0 != this->nullableSet.size()) {
            dout << __func__ << ": here are the nullable non-terminals: ";
            CFG::emitAllMembers(this->nullableSet, false);
        }
        else {
            dout << __func__ << ": did not find nullable non-terminals!"
                 << endl;
        }
        dout << __func__ << ": nullable fixed-point end ***" << endl;
        dout << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::refreshFirstSets(void)
{
    for (CFGProduction &p : this->productions) {
        for (Symbol &sym : p.rhs()) {
            /* add myself to my firsts if not epsilon 8-| */
            if (sym.isTerminal() && !sym.isEpsilon()) {
                sym.firsts().insert(sym);
            }
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
emitFirstSet(const CFGProductions &prods)
{
    set<Symbol> lhsSet;
    for (const CFGProduction &p : prods) {
        lhsSet.insert(const_cast<CFGProduction &>(p).lhs());
        auto rhs = const_cast<CFGProduction &>(p).rhs();
        lhsSet.insert(rhs.begin(), rhs.end());
    }
    for (const Symbol &sym : lhsSet) {
        dout << "FIRST(" << sym << ") = ";
        CFG::emitAllMembers(const_cast<Symbol &>(sym).firsts(), false);
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::computeFirstSets(void)
{
    size_t nelems = 0;
    bool hadUpdate;

    if (this->verbose) dout << __func__ << ": fixed-point begin ***" << endl;

    this->refreshFirstSets();
    /* start the fixed-point calculation */
    do {
        hadUpdate = false;
        for (CFGProduction &p : this->productions) {
            nelems = p.lhs().firsts().size();
            Symbol alpha = *p.rhs().begin();
            /* if alpha is nullable */
            /* XXX FIX */
            if (alpha.nullable()) {
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
            CFG::propagateFirsts(this->productions,
                                 p.lhs().sym(),
                                 p.lhs().firsts());
            hadUpdate = (nelems != p.lhs().firsts().size());
        }
    } while (hadUpdate);

    if (this->verbose) {
        dout << __func__ << ": here are the first sets:" << endl;
        emitFirstSet(this->productions);
        dout << __func__ << ": fixed-point end ***" << endl;
        dout << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static bool
lastElem(const vector<Symbol> &l, vector<Symbol>::iterator i)
{
    bool res = false;

    advance(i, 1);
    res = (l.end() == i);
    advance(i, -1);
    return res;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::followsetPrep(void)
{
    /* add a new, special terminal, $ and new start production */
    CFGProduction newp(Symbol::START, this->startSymbol().sym() + Symbol::END);
    this->productions.insert(this->productions.begin(), newp);
    /* init S''s follow set to include $ */
    this->productions.begin()->lhs().follows().insert(Symbol(Symbol::END));
    /* refresh productions */
    this->productions = this->buildFullyPopulatedGrammar(this->productions);
    this->refreshFirstSets();
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::computeFollowSets(void)
{
    size_t nelems = 0;
    bool hadUpdate = false;
    FollowSetMarker marker;

    if (this->verbose) dout << __func__ << ": begin ***" << endl;

    this->followsetPrep();

    /* reset markers before we start calculating follow sets */
    marker.mark(this->productions);

    do {
        hadUpdate = false;
        for (CFGProduction &p : this->productions) {
            auto rhss = p.rhs().begin();
            while (p.rhs().end() != rhss) {
                if (!rhss->isTerminal()) {
                    nelems = rhss->follows().size();
                    /* for the case where beta is empty */
                    if (lastElem(p.rhs(), rhss)) {
                        rhss->follows().insert(p.lhs().follows().begin(),
                                               p.lhs().follows().end());
                    }
                    else {
                        /* safe because rhss not last element */
                        Symbol rn;
                        advance(rhss, 1);
                        rn = *rhss;
                        advance(rhss, -1);
                        rhss->follows().insert(rn.firsts().begin(),
                                               rn.firsts().end());
                        /* XXX FIX */
                        if (rn.nullable()) {
                            rhss->follows().insert(p.lhs().follows().begin(),
                                                   p.lhs().follows().end());
                        }
                    }
                    hadUpdate = (nelems != rhss->follows().size());
                    CFG::propagateFollows(this->productions, *rhss);
                }
                ++rhss;
            }
        }
    } while (hadUpdate);

    if (this->verbose) {
        dout << __func__ << ": here are the follow sets:" << endl;
        set<Symbol> lhsSet;
        for (const CFGProduction &p : this->productions) {
            lhsSet.insert(const_cast<CFGProduction &>(p).lhs());
            vector<Symbol> rhs = const_cast<CFGProduction &>(p).rhs();
            lhsSet.insert(rhs.begin(), rhs.end());
        }
        for (const Symbol &sym : lhsSet) {
            dout << "FOLLOW(" << sym << ") = ";
            CFG::emitAllMembers(const_cast<Symbol &>(sym).follows(), false);
        }
        dout << __func__ << ": end ***" << endl;
        dout << endl;
    }
}
