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
aInFiOfA(CFGProduction &alpha, const Symbol &a)
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
aInFoOfN(CFGProduction &N, const Symbol &a)
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
        dout << "done building LL(1) parse table :: grammar is"
             << (conflict ? " not " : " ") << "strong LL(1) ***"
             << endl;
        dout << endl;
    }
    if (conflict) {
        throw DialectException(DIALECT_WHERE, "", false);
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
LL1Parser::parse(const vector<Symbol> &input)
{
    try {
        StrongLL1Parser sll1(this->_cfg);
        sll1.verbose(this->_verbose);
        sll1.parse(input);
    }
    catch (DialectException &e) {
        /* both approaches failed */
        cerr << e.what() << endl;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::parse(const vector<Symbol> &input)
{
    try {
        this->initTable();
        try {
            /* try strong if grammar is strong-ll(1) */
            this->parseImpl(input, true);
            /* now try experimental dynamic parser */
            this->parseImpl(input , false);
        }
        catch (DialectException &e) {
            throw e;
        }
    }
    catch (DialectException &e) {
        /* not strong ll(1), so try experimental dynamic parser */
        this->parseImpl(input , false);
    }
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
emitParseState(const Symbol &in, const Symbol &tos, const stack<Symbol> &p)
{
    auto savep = p;
    vector<Symbol> q;
    cout << "..." << (Symbol::DEAD == in ? "" : " in: " + in.sym())
         << " top: " << tos << " action: ";
    while (!savep.empty()) {
        q.push_back(savep.top());
        savep.pop();
    }
    for (auto i = q.rbegin(); i != q.rend(); ++i) {
        cout << *i;
    }
    cout << endl;
}

/* ////////////////////////////////////////////////////////////////////////// */
static void
stopParse(void)
{
    string estr = "*** failure: input not recognized by grammar ***";
    throw DialectException(DIALECT_WHERE, estr, false);
}

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::strongParse(const vector<Symbol> &_input)
{
    ParseTable &pt = this->_table;
    stack<Symbol> stk;
    auto input = _input;

    cout << endl << "--- starting strong table-driven parse" << endl;

    stk.push(this->_cfg.startSymbol());

    while (!stk.empty()) {
        Symbol top = stk.top();
        Symbol in;
        if (!input.empty()) in = *input.begin();
        else in = Symbol(Symbol::END);
        CFGProduction cp = pt[top][in];
        if (top.terminal()) {
            stk.pop();
            if (top.epsilon()) continue;
            if (top != in) goto dump;
            cout << "+++ match: " << top << endl;
            if (!input.empty()) input.erase(input.begin());
        }
        else if (Symbol::DEAD == cp.lhs().sym()) goto dump;
        else {
            emitParseState(in, top, cp);
            stk.pop();
            for (auto s = cp.rhs().rbegin(); s != cp.rhs().rend(); ++s) {
                stk.push(*s);
            }
        }
    }
    cout << "--- done with strong table-driven parse" << endl;
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

/* ////////////////////////////////////////////////////////////////////////// */
/* XXX RENAME */
stack<Symbol>
StrongLL1Parser::predict(const Symbol &nont, const Symbol &input)
{
    CFGProductions prods;
    stack<Symbol> res;
    CFGProductions productions = this->_cfg.prods();

    for (CFGProduction &p : productions) {
        if (nont == p.clhs()) {
            if (aInFiOfA(p, input)) {
                prods.push_back(p);
            }
        }
    }
    if (prods.size() == 0) {
        cout << "*** input not recognized by grammar ***" << endl;
        throw;
    }
    if (prods.size() != 1) {
        cout << "*** grammar is not LL(1) ***" << endl;
        throw;
    }
    CFGProduction p = *prods.begin();
    for (auto s = p.rhs().begin(); s != p.rhs().end(); ++s) {
        res.push(*s);
    }
    return res;
}

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::dynamicParse(const vector<Symbol> &_input)
{
    auto input = _input;
    stack<Symbol> stk;

    cout << endl << "--- starting dynamic parse" << endl;

    stk.push(this->_cfg.startSymbol());

    while (!stk.empty()) {
        Symbol top = stk.top();
        Symbol in;
        if (!input.empty()) in = *input.begin();
        else in = Symbol(Symbol::END);
        if (top.terminal()) {
            stk.pop();
            if (top.epsilon()) continue;
            if (top != in) goto dump;
            cout << "+++ match: " << top << endl;
            if (!input.empty()) input.erase(input.begin());
        }
        else {
            stk.pop();
            auto prediction = predict(top, in);
            emitParseState(in, top, prediction);
            while (!prediction.empty()) {
                stk.push(prediction.top());
                prediction.pop();
            }
        }
    }
    cout << "--- done with dynamic parse" << endl;
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

/* ////////////////////////////////////////////////////////////////////////// */
void
StrongLL1Parser::parseImpl(const vector<Symbol> &input , bool strong)
{
    if (strong) this->strongParse(input);
    else this->dynamicParse(input);
}
