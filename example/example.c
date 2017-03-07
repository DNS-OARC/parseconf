/*
 * Author Jerry Lundström <jerry@dns-oarc.net>
 * Copyright (c) 2017, OARC, Inc.
 * All rights reserved.
 *
 * This file is part of parseconf.
 *
 * parseconf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * parseconf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with parseconf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "parseconf.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void usage(void) {
    printf(
"usage: example [options] <config...>\n"
" -f                 the config is a file (default)\n"
" -t                 the config is text\n"
"                    multiple config options can be given but each command\n"
"                    line argument is parse separate\n"
" -V                 display version and exit\n"
" -h                 this\n"
    );
}

static parseconf_token_type_t example_tokens[] = {
    PARSECONF_TOKEN_ANY, PARSECONF_TOKEN_END
};

static int parse_example(void* user, const parseconf_token_t* tokens, const char** errstr) {
    unsigned long long int num = 0;
    long double dbl = 0.;

    if (!tokens) {
        return 1;
    }
    if (!errstr) {
        return 1;
    }

    switch (tokens[1].type) {
        case PARSECONF_TOKEN_NUMBER:
            if (parseconf_ulonglongint(&tokens[1], &num, errstr))
                return 1;
            printf("example number: %llu\n", num);
            break;

        case PARSECONF_TOKEN_STRING:
            printf("example string: %.*s\n", (int)tokens[1].length, tokens[1].token);
            break;

        case PARSECONF_TOKEN_QSTRING:
            printf("example quoted string: %.*s\n", (int)tokens[1].length, tokens[1].token);
            break;

        case PARSECONF_TOKEN_FLOAT:
            if (parseconf_longdouble(&tokens[1], &dbl, errstr))
                return 1;
            printf("example number: %Le\n", dbl);
            break;

        default:
            *errstr = "Unknown token type";
            return 1;
    }

    return 0;
}

static parseconf_syntax_t syntax[] = {
    { "example", parse_example, example_tokens, 0 },
    PARSECONF_SYNTAX_END
};

static void error_callback(void* user, parseconf_error_t error, size_t line, size_t token, const parseconf_token_t* tokens, const char* errstr) {
    switch (error) {
        case PARSECONF_ERROR_INTERNAL:
            fprintf(stderr, "Internal conf error at line %lu\n", line);
            break;

        case PARSECONF_ERROR_EXPECT_STRING:
            fprintf(stderr, "Conf error at line %lu for argument %lu, expected a string\n", line, token);
            break;

        case PARSECONF_ERROR_EXPECT_NUMBER:
            fprintf(stderr, "Conf error at line %lu for argument %lu, expected a number\n", line, token);
            break;

        case PARSECONF_ERROR_EXPECT_QSTRING:
            fprintf(stderr, "Conf error at line %lu for argument %lu, expected a quoted string\n", line, token);
            break;

        case PARSECONF_ERROR_EXPECT_FLOAT:
            fprintf(stderr, "Conf error at line %lu for argument %lu, expected a float\n", line, token);
            break;

        case PARSECONF_ERROR_EXPECT_ANY:
            fprintf(stderr, "Conf error at line %lu for argument %lu, expected any type\n", line, token);
            break;

        case PARSECONF_ERROR_UNKNOWN:
            fprintf(stderr, "Conf error at line %lu for argument %lu, unknown configuration\n", line, token);
            break;

        case PARSECONF_ERROR_NO_NESTED:
            fprintf(stderr, "Internal conf error at line %lu for argument %lu, no nested conf\n", line, token);
            break;

        case PARSECONF_ERROR_NO_CALLBACK:
            fprintf(stderr, "Internal conf error at line %lu for argument %lu, no callback\n", line, token);
            break;

        case PARSECONF_ERROR_CALLBACK:
            fprintf(stderr, "Conf error at line %lu, %s\n", line, errstr);
            break;

        case PARSECONF_ERROR_FILE_ERRNO:
            fprintf(stderr, "Internal conf error at line %lu for argument %lu, errno: %s\n", line, token, strerror(errno));
            break;

        case PARSECONF_ERROR_TOO_MANY_ARGUMENTS:
            fprintf(stderr, "Conf error at line %lu, too many arguments\n", line);
            break;

        case PARSECONF_ERROR_INVALID_SYNTAX:
            fprintf(stderr, "Conf error at line %lu, invalid syntax\n", line);
            break;

        default:
            fprintf(stderr, "Unknown conf error %d at %lu\n", error, line);
            break;
    }
}

int main(int argc, char** argv) {
    int opt, file = 1, err;

    while ((opt = getopt(argc, argv, "fthV")) != -1) {
        switch (opt) {
        case 'f':
            file = 1;
            break;
        case 't':
            file = 0;
            break;
        case 'h':
            usage();
            return 0;
        case 'V':
            printf("example version %s (parseconf version %s)\n", PACKAGE_VERSION, PARSECONF_VERSION_STR);
            return 0;
        default:
            usage();
            return 1;
        }
    }

    while (optind < argc) {
        if (file)
            err = parseconf_file(0, argv[optind], syntax, error_callback);
        else
            err = parseconf_text(0, argv[optind], strlen(argv[optind]), syntax, error_callback);

        if (err != PARSECONF_OK) {
            fprintf(stderr, file ? "parseconf_file(%s): %s" : "parseconf_text(%s): %s\n", argv[optind], parseconf_strerror(err));
            return 2;
        }

        optind++;
    }

    return 0;
}
