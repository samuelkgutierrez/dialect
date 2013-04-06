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

BUILT_SOURCES = \
cfg-scanner.cxx

AM_YFLAGS= -d -t -y
AM_LFLAGS= -8

bin_PROGRAMS = \
dialect

base_src = \
Constants.hxx \
Base.hxx Base.cxx \
DialectException.hxx DialectException.cxx \
CFG.hxx CFG.cxx \
cfg-scanner.lxx cfg-scanner.yxx

dialect_SOURCES = \
${base_src} \
dialect.cxx
