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

#include "LL1Parser.hxx"
#include "Base.hxx"
#include "Constants.hxx"
#include "DialectException.hxx"

#include <iostream>
#include <string>
#include <stack>

using namespace std;

/* XXX add const & where needed at some point */

/* ////////////////////////////////////////////////////////////////////////// */
static bool
aInFiOfA(CFGProduction &alpha,
       const Symbol &a)
{
    /* first of alpha set */
    set<Symbol> foa;
    auto rhs = alpha.rhs();
    auto rhss = rhs.begin();

    /* now figure out FIRST(alpha) */
    while (rhs.end() != rhss) {
        foa.insert(rhss->firsts().begin(), rhss->firsts().end());
        if (!rhss->nullable()) break;
        else ++rhss;
    }
    return (foa.end() != foa.find(a));
}

/* ////////////////////////////////////////////////////////////////////////// */
static bool
aInFoOfN(CFGProduction &N,
         const Symbol &a)
{
    return (N.lhs().follows().end() != N.lhs().follows().find(a));
}

/* ////////////////////////////////////////////////////////////////////////// */
/* XXX move to CFGProduction and rm similar routine in CFG */
static bool
alphaNullable(CFGProduction &alpha)
{
    for (const Symbol &s : alpha.rhs()) {
        if (!s.nullable() && !s.epsilon()) return false;
    }
    return true;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
emitTableEntry(const Symbol &nt,
               const Symbol &t,
               const CFGProduction &p)
{
    dout << "[" << nt << "]" << "[" << t << "] = " << p << endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
static bool
tCellOccupied(CFGProduction &p)
{
    return Symbol::DEAD != p.lhs().sym();
}

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::initTable(void)
{
    bool verbose = this->_verbose;
    auto nonTerminals = this->_cfg.getNonTerminals();
    auto terminals = this->_cfg.getTerminals();
    auto &pt = this->_table;
    bool conflict = false;

    if (verbose) dout << "building LL(1) parse table ***" << endl;
    for (CFGProduction &p : this->_cfg.prods()) {
        for (const Symbol &nont : nonTerminals) {
            if (nont == p.lhs()) {
                for (const Symbol &t : terminals) {
                    if (aInFiOfA(p, t)) {
                        if (tCellOccupied(pt[nont][t])) {
                            conflict = true;
                            if (this->_verbose) {
                                dout << "*** CONFLICT ***" << endl;
                            }
                        }
                        pt[nont][t] = p;
                        if (verbose) emitTableEntry(nont, t, p);
                    }
                    else if (alphaNullable(p) && aInFoOfN(p, t)) {
                        if (tCellOccupied(pt[nont][t])) {
                            conflict = true;
                            if (verbose) dout << "*** CONFLICT ***" << endl;
                        }
                        pt[nont][t] = p;
                        if (this->_verbose) emitTableEntry(nont, t, p);
                    }
                }
            }
        }
    }
    if (verbose) {
        dout << "done building LL(1) parse table ***" << endl;
        dout << endl;
    }
    if (conflict) {
        string estr = "*** grammar not strong LL(1) ***";
        throw DialectException(DIALECT_WHERE, estr, false);
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
LL1Parser::parse(void)
{
    try {
        StrongLL1Parser sll1(this->_cfg, this->_input);
        sll1.verbose(this->_verbose);
        sll1.parse();
    }
    catch (DialectException &e) {
        cerr << e.what() << endl;
        /* now try a different approach */
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::parse(void)
{
    this->initTable();
    this->parseImpl();
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
emitParseState(const Symbol &in, const Symbol &tos, const CFGProduction &p)
{
    cout << "..." << (Symbol::DEAD == in ? "" : " in: " + in.sym())
         << " top: " << tos << " action: " << p << endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
stopParse(void)
{
    string estr = "*** STOP: input string not recognized by grammar ***";
    throw DialectException(DIALECT_WHERE, estr, false);
}

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::parseImpl(void)
{
    ParseTable &pt = this->_table;
    vector<Symbol> &input = this->_input;
    stack<Symbol> stk;

    cout << endl << "--- starting table-driven parse" << endl;

    stk.push(this->_cfg.startSymbol());

    while (!stk.empty()) {
        Symbol top = stk.top();
        Symbol in;
        if (!input.empty()) in = *input.begin();
        else in = Symbol(Symbol::END);
        CFGProduction cp = pt[top][in];
        if (top.terminal()) {
            if (!input.empty()) {
                if (top == *input.begin()) {
                    input.erase(input.begin());
                    cout << "+++ match: " << top << endl;
                }
            }
            stk.pop();
        }
        else if (Symbol::DEAD == cp.lhs().sym()) {
            goto dump;
        }
        else {
            emitParseState(in, top, cp);
            stk.pop();
            for (auto s = cp.rhs().rbegin(); s != cp.rhs().rend(); ++s) {
                stk.push(*s);
            }
        }
    }
    cout << "--- done with table-driven parse" << endl;
    if (stk.size() == 0 && input.empty()) {
        cout << "*** success: input recognized by grammar ***" << endl;
    }
    else {
dump:
        cout << "*** failure: input not recognized by grammar ***" << endl;
        cout << "*** begin state dump ***" << endl;
        cout << "input empty: " << (input.empty() ? "yes" : "no") << endl;
        while (!input.empty()) {
            cout << " -- " << *input.begin() << endl;
            input.erase(input.begin());
        }
        cout << "stack empty: " << (stk.empty() ? "yes" : "no") << endl;
        while (!stk.empty()) {
            cout << " -- " << stk.top() << endl;
            stk.pop();
        }
        cout << "*** end state dump ***" << endl;
        stopParse();
    }
}
