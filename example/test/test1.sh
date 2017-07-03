# Author Jerry Lundström <jerry@dns-oarc.net>
# Copyright (c) 2017, OARC, Inc.
# All rights reserved.
#
# This file is part of parseconf.
#
# parseconf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# parseconf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with parseconf.  If not, see <http://www.gnu.org/licenses/>.

../example -t "example 1234567890;" \
    "example 12345678901234567890;" \
    "example string string string;" \
    'example "quoted string string string";' \
    "example 0.1234567890;" \
    "example 123456789.1234567890;" \
    "example    string    string    ;" \
    "example	tab	tab 	;" >test1.out

! ../example -t 'example "quote error"";' 2>>test1.out

diff test1.out "$srcdir/test1.gold"
