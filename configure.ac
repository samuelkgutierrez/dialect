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

AC_PREREQ([2.60])

AC_INIT([dialect], [0.1])

# Check If AM_SILENT_RULES Rule Are Supported
# If So, Build Using AM_SILENT_RULES
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])

# note: -Wall -Werror are automake options.  they are NOT compiler flags
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_CONFIG_HEADER([config.h])

# checks for programs.
AC_PROG_CXX
AC_PROG_LEX
AC_PROG_YACC
AC_GNU_SOURCE
AC_PROG_LIBTOOL

# checks for libraries.
# none at this point

# checks for header files.
AC_CHECK_HEADERS([\
inttypes.h limits.h stdint.h stdlib.h string.h unistd.h
])

# checks for typedefs, structures, and compiler characteristics.

# checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strerror strtoul])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT

# let the user know the configuration 

cat << EOF

dialect configuration
+--------------------
|
| CXX       : $CXX
| CXXFLAGS  : $CXXFLAGS
| LDFLAGS   : $LDFLAGS
| LIBS      : $LIBS
| CPPFLAGS  : $CPPFLAGS
| CPP       : $CPP

EOF
