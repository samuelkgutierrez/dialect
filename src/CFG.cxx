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

using namespace std;

const string CFGProduction::EPSILON = "\b";

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
CFG::emitState(void) const
{
    dout << endl;
    dout << "start symbol: " << this->startSymbol << endl;
    dout << "productions begin" << endl;
    this->emitAllProductions();
    dout << "productions end" << endl;
    dout << endl;
}
