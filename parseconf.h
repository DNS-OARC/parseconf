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

#ifndef __parseconf_h
#define __parseconf_h

#include <stddef.h>
#if PARSECONF_ENABLE_ASSERT
#include <assert.h>
#define parseconf_assert(x) assert(x)
#else
#define parseconf_assert(x)
#endif

#define PARSECONF_VERSION_STR   "1.0.0"
#define PARSECONF_VERSION_MAJOR 1
#define PARSECONF_VERSION_MINOR 0
#define PARSECONF_VERSION_PATCH 0

#define PARSECONF_EEXIST    -4
#define PARSECONF_ENOMEM    -3
#define PARSECONF_EINVAL    -2
#define PARSECONF_ERROR     -1
#define PARSECONF_OK        0
#define PARSECONF_LAST      1
#define PARSECONF_COMMENT   2
#define PARSECONF_EMPTY     3

#define PARSECONF_EEXIST_STR    "Already exists"
#define PARSECONF_ENOMEM_STR    "Out of memory"
#define PARSECONF_EINVAL_STR    "Invalid arguments"
#define PARSECONF_ERROR_STR     "Generic error"

#define PARSECONF_MAX_TOKENS    64

#ifdef __cplusplus
extern "C" {
#endif

const char* parseconf_version_str(void);
int parseconf_version_major(void);
int parseconf_version_minor(void);
int parseconf_version_patch(void);

typedef enum parseconf_token_type parseconf_token_type_t;
#ifdef PARSECONF_USE_SHORT_TOKENS
enum parseconf_token_type {
    END = 0,
    STRING,
    QSTRING,
    NUMBER,
    STRINGS,
    QSTRINGS,
    NUMBERS,
    ANY,
    FLOAT,
    FLOATS,
    NESTED
};
#define PARSECONF_TOKEN_END         END
#define PARSECONF_TOKEN_STRING      STRING
#define PARSECONF_TOKEN_QSTRING     QSTRING
#define PARSECONF_TOKEN_NUMBER      NUMBER
#define PARSECONF_TOKEN_STRINGS     STRINGS
#define PARSECONF_TOKEN_QSTRINGS    QSTRINGS
#define PARSECONF_TOKEN_NUMBERS     NUMBERS
#define PARSECONF_TOKEN_ANY         ANY
#define PARSECONF_TOKEN_FLOAT       FLOAT
#define PARSECONF_TOKEN_FLOATS      FLOATS
#define PARSECONF_TOKEN_NESTED      NESTED
#else
enum parseconf_token_type {
    PARSECONF_TOKEN_END = 0,
    PARSECONF_TOKEN_STRING,
    PARSECONF_TOKEN_QSTRING,
    PARSECONF_TOKEN_NUMBER,
    PARSECONF_TOKEN_STRINGS,
    PARSECONF_TOKEN_QSTRINGS,
    PARSECONF_TOKEN_NUMBERS,
    PARSECONF_TOKEN_ANY,
    PARSECONF_TOKEN_FLOAT,
    PARSECONF_TOKEN_FLOATS,
    PARSECONF_TOKEN_NESTED
};
#endif

typedef struct parseconf_token parseconf_token_t;
struct parseconf_token {
    parseconf_token_type_t  type;
    const char*             token;
    size_t                  length;
};

typedef int (*parseconf_token_callback_t)(void* user, const parseconf_token_t* tokens, const char** errstr);

typedef enum parseconf_error parseconf_error_t;
enum parseconf_error {
    PARSECONF_ERROR_NONE = 0,
    PARSECONF_ERROR_INTERNAL,
    PARSECONF_ERROR_EXPECT_STRING,
    PARSECONF_ERROR_EXPECT_NUMBER,
    PARSECONF_ERROR_EXPECT_QSTRING,
    PARSECONF_ERROR_EXPECT_FLOAT,
    PARSECONF_ERROR_EXPECT_ANY,
    PARSECONF_ERROR_UNKNOWN,
    PARSECONF_ERROR_NO_NESTED,
    PARSECONF_ERROR_NO_CALLBACK,
    PARSECONF_ERROR_CALLBACK,
    PARSECONF_ERROR_FILE_ERRNO,
    PARSECONF_ERROR_TOO_MANY_ARGUMENTS,
    PARSECONF_ERROR_INVALID_SYNTAX
};

typedef void (*parseconf_error_callback_t)(void* user, parseconf_error_t error, size_t line, size_t token, const parseconf_token_t* tokens, const char* errstr);

#define PARSECONF_SYNTAX_END { 0, 0, 0, 0 }
typedef struct parseconf_syntax parseconf_syntax_t;
struct parseconf_syntax {
    const char*                     token;
    parseconf_token_callback_t      callback;
    const parseconf_token_type_t*   syntax;
    const parseconf_syntax_t*       nested;
};

int parseconf_ulongint(const parseconf_token_t* token, unsigned long int* value, const char** errstr);
int parseconf_ulonglongint(const parseconf_token_t* token, unsigned long long int* value, const char** errstr);
int parseconf_double(const parseconf_token_t* token, double* value, const char** errstr);
int parseconf_longdouble(const parseconf_token_t* token, long double* value, const char** errstr);

int parseconf_file(void* user, const char* file, const parseconf_syntax_t* syntax, parseconf_error_callback_t error_callback);
int parseconf_text(void* user, const char* text, const size_t length, const parseconf_syntax_t* syntax, parseconf_error_callback_t error_callback);
const char* parseconf_strerr(int errnum);

#ifdef __cplusplus
}
#endif

#endif /* __parseconf_h */
