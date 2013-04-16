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
/* static utility functions */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////// */
static void
propagateMark(CFGProductions &productions,
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
propagateNullable(CFGProductions &productions,
                  const Symbol &symbol)
{
    for (CFGProduction &p : productions) {
        if (symbol == p.lhs()) p.lhs().nullable(true);
        for (Symbol &sym : p.rhs()) {
            if (symbol == sym) sym.nullable(true);
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
propagateFirsts(CFGProductions &productions,
                const Symbol &symbol,
                const set<Symbol> &fset)
{
    for (CFGProduction &p : productions) {
        if (symbol == p.lhs()) {
            p.lhs().firsts().insert(fset.begin(), fset.end());
        }
        for (Symbol &sym : p.rhs()) {
            if (symbol == sym) {
                sym.firsts().insert(fset.begin(), fset.end());
            }
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
template <typename T>
static bool
lastElem(const T &l)
{
    typename T::const_iterator i = l.begin();
    bool res = false;

    advance(i, 1);
    res = (l.end() == i);
    advance(i, -1);
    return res;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
emitNullables(const CFGProductions &productions)
{
    set<Symbol> nullables;

    for (CFGProduction p : productions) {
        if (p.lhs().nullable() && !p.lhs().terminal()) {
            nullables.insert(p.lhs());
        }
        for (const Symbol &s : p.rhs()) {
            if (s.nullable() && !s.terminal()) nullables.insert(s);
        }
    }
    if (0 != nullables.size()) {
        dout << "here are the nullable non-terminals: ";
        CFG::emitAllMembers(nullables, false);
    }
    else {
        dout << "did not find nullable non-terminals!" << endl;
    }
}

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
const string Symbol::START   = "S'";
/* our scanner doesn't accept $s, so this is okay */
const string Symbol::END     = "$";

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
     * everything similarly in this path because we really can't telly anything
     * about this grammar at this point only given production strings. */
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
        p.lhs().mark(p.lhs().terminal());
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.terminal());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
ReachabilityMarker::mark(CFGProductions &productions) const
{
    /* this one is easy. mark the start symbol. */
    for (CFGProduction &p : productions) {
        p.lhs().mark(p.lhs().start());
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.start());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
NullableMarker::mark(CFGProductions &productions) const
{
    for (CFGProduction &p : productions) {
        p.lhs().mark(p.lhs().epsilon());
        for (Symbol &sym : p.rhs()) {
            sym.mark(sym.epsilon());
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
NonGeneratingEraser::erase(CFGProductions &productions) const
{
    bool rm = false;
    if (this->verbose) {
        dout << "removing non-generating symbols..." << endl;
    }
    for (auto p = productions.begin(); productions.end() != p;) {
        if (!p->lhs().marked() || !p->rhsMarked()) {
            if (this->verbose) dout << "  rm " << *p << endl;
            rm = true;
            p = productions.erase(p);
        }
        else ++p;
    }
    if (!rm && this->verbose) dout << "  none found" << endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
UnreachableEraser::erase(CFGProductions &productions) const
{
    bool rm = false;
    if (this->verbose) {
        dout << "removing unreachable symbols..." << endl;
    }
    for (auto p = productions.begin(); productions.end() != p;) {
        if (!p->lhs().marked()) {
            if (this->verbose) dout << "  rm " << *p << endl;
            rm = true;
            p = productions.erase(p);
        }
        else ++p;
    }
    if (!rm && this->verbose) dout << "  none found" << endl;
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
                for (Symbol &sym : p.rhs()) {
                    propagateMark(productions, sym.sym());
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
                /* make sure that we update all instances of lhs()->sym() */
                propagateMark(productions, p.lhs().sym());
                hadUpdate = true;
            }
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
    this->productions = productions;
    this->refresh();
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
refreshSymbolTypes(CFGProductions &prods)
{
    set<Symbol> nonTerms;
    Symbol start = prods.begin()->lhs();

    for (CFGProduction &p : prods) {
        p.lhs().terminal(false);
        p.lhs().start(p.lhs() == start);
        nonTerms.insert(p.lhs());
    }
    for (CFGProduction &p : prods) {
        for (Symbol &s : p.rhs()) {
            s.terminal(nonTerms.end() == nonTerms.find(s));
            s.start(s == start);
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::refresh(void)
{
    refreshSymbolTypes(this->productions);
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
    dout << endl;

    dout << "terminals begin" << endl;
    CFG::emitAllMembers(this->getTerminals());
    dout << "terminals end" << endl;
    dout << endl;

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

    for (CFGProduction p : this->productions) {
        if (!p.lhs().terminal()) nonTerms.insert(p.lhs());
        for (const Symbol &s : p.rhs()) {
            if (!s.terminal()) nonTerms.insert(s);
        }
    }
    return nonTerms;
}

/* ////////////////////////////////////////////////////////////////////////// */
set<Symbol>
CFG::getTerminals(void) const
{
    set<Symbol> terms;

    for (CFGProduction p : this->productions) {
        if (p.lhs().terminal()) terms.insert(p.lhs());
        for (const Symbol &s : p.rhs()) {
            if (s.terminal()) terms.insert(s);
        }
    }
    return terms;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::propagateFollows(CFGProductions &productions,
                      const Symbol &s)
{
    Symbol t = s;

    for (CFGProduction &p : productions) {
        if (t == p.lhs()) {
            p.lhs().follows().insert(t.follows().begin(), t.follows().end());
        }
        for (Symbol &sym : p.rhs()) {
            if (t == sym) {
                sym.follows().insert(t.follows().begin(), t.follows().end());
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
                p.lhs().nullable(true);
                propagateMark(this->productions, p.lhs().sym());
                propagateNullable(this->productions, p.lhs().sym());
                hadUpdate = true;
            }
        }
    } while (hadUpdate);

    if (this->verbose) {
        emitNullables(this->productions);
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
            if (sym.terminal() && !sym.epsilon()) {
                sym.firsts().insert(sym);
            }
        }
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
emitFirstSets(const CFGProductions &prods)
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
static set<Symbol>
getAlphaUBeta(const vector<Symbol> &in)
{
    vector<Symbol> cin = in;
    set<Symbol> aub;
    vector<Symbol>::iterator s = cin.begin();

    /* alpha's firsts are always going to be in the set */
    aub.insert(s->firsts().begin(), s->firsts().end());
    ++s;
    /* now figure out FIRST(beta) */
    while (cin.end() != s) {
        aub.insert(s->firsts().begin(), s->firsts().end());
        if (!s->nullable()) break;
        else ++s;
    }
    return aub;
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
            if (alpha.terminal() || !alpha.nullable()) {
                p.lhs().firsts().insert(alpha.firsts().begin(),
                                        alpha.firsts().end());
            }
            else {
                set<Symbol> aub = getAlphaUBeta(p.rhs());
                p.lhs().firsts().insert(aub.begin(), aub.end());
            }
            propagateFirsts(this->productions, p.lhs().sym(), p.lhs().firsts());
            if (nelems != p.lhs().firsts().size()) hadUpdate = true;
        }
    } while (hadUpdate);

    if (this->verbose) {
        dout << __func__ << ": here are the first sets:" << endl;
        emitFirstSets(this->productions);
        dout << __func__ << ": fixed-point end ***" << endl;
        dout << endl;
    }
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
    this->refresh();
    this->refreshFirstSets();
}

/* ////////////////////////////////////////////////////////////////////////// */
static bool
nullableFromHere(const vector<Symbol> &l, vector<Symbol>::iterator i)
{
    for (; l.end() != i; ++i) {
        if (!i->nullable()) return false;
    }
    return true;
}

/* ////////////////////////////////////////////////////////////////////////// */
static set<Symbol>
firstOfBeta(const vector<Symbol> &in)
{
    vector<Symbol> cin = in;
    vector<Symbol>::iterator i = cin.begin();
    set<Symbol> fob;

    /* now figure out FIRST(beta) */
    while (cin.end() != i) {
        fob.insert(i->firsts().begin(), i->firsts().end());
        if (!i->nullable()) break;
        else ++i;
    }
    return fob;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
CFG::computeFollowSets(void)
{
    size_t nelems = 0;
    bool hadUpdate = false;

    if (this->verbose) dout << __func__ << ": begin ***" << endl;

    this->followsetPrep();

    do {
        hadUpdate = false;
        for (CFGProduction &p : this->productions) {
            auto rhss = p.rhs().begin();
            while (p.rhs().end() != rhss) {
                if (!rhss->terminal()) {
                    nelems = rhss->follows().size();
                    /* for the case where beta is empty */
                    vector<Symbol> slice(rhss, p.rhs().end());
                    if (lastElem(slice)) {
                        rhss->follows().insert(p.lhs().follows().begin(),
                                               p.lhs().follows().end());
                    }
                    else {
                        /* safe because rhss not last element */
                        advance(rhss, 1);
                        vector<Symbol> slice(rhss, p.rhs().end());
                        set<Symbol> fob = firstOfBeta(slice);
                        if (nullableFromHere(p.rhs(), rhss)) {
                            advance(rhss, -1);
                            rhss->follows().insert(p.lhs().follows().begin(),
                                                   p.lhs().follows().end());
                            advance(rhss, 1);
                        }
                        advance(rhss, -1);
                        rhss->follows().insert(fob.begin(), fob.end());
                    }
                    if (nelems != rhss->follows().size()) hadUpdate = true;
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
