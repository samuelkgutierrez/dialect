# Copyright (c) 2013 Samuel K. Gutierrez All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


AM_YFLAGS=-d -t

BUILT_SOURCES = \
CFGParser.cc CFGParser.h CFGScanner.cc

CLEANFILES = \
${BUILT_SOURCES} CFGParser.h

bin_PROGRAMS = \
dialect

PARSER_FILES = CFGParser.yy CFGParser.h CFGScanner.ll

BASE_SRC = \
Constants.hxx \
Base.hxx Base.cxx \
DialectException.hxx DialectException.cxx \
CFG.hxx CFG.cxx \
LL1Parser.hxx LL1Parser.cxx \
UserInputParser.hxx UserInputParser.cxx \
${PARSER_FILES}

dialect_SOURCES = \
${BASE_SRC} \
Dialect.cxx
