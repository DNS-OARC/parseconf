# Conf parser helper library

[![Build Status](https://travis-ci.org/DNS-OARC/parseconf.svg?branch=develop)](https://travis-ci.org/DNS-OARC/parseconf) [![Coverity Scan Build Status](https://scan.coverity.com/projects/11848/badge.svg)](https://scan.coverity.com/projects/dns-oarc-parseconf)

## About

This is a helper library for parsing configuration files.

## Usage

Please see `example.c` in the example directory.

### git submodule

```shell
git submodule init
git submodule add https://github.com/DNS-OARC/parseconf.git src/parseconf
```

### Makefile.am

```m4
program_SOURCES += parseconf/parseconf.c
dist_program_SOURCES += parseconf/parseconf.h
```

## Author(s)

Jerry Lundström <jerry@dns-oarc.net>

## Copyright

Copyright (c) 2017, OARC, Inc.
All rights reserved.

This file is part of parseconf.

parseconf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

parseconf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with parseconf.  If not, see <http://www.gnu.org/licenses/>.
