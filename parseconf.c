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

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "parseconf.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Version
 */

static const char* _version = PARSECONF_VERSION_STR;
inline const char* parseconf_version_str(void) {
    return _version;
}

inline int parseconf_version_major(void) {
    return PARSECONF_VERSION_MAJOR;
}

inline int parseconf_version_minor(void) {
    return PARSECONF_VERSION_MINOR;
}

inline int parseconf_version_patch(void) {
    return PARSECONF_VERSION_PATCH;
}

/*
 * Parsing functions
 */

static int parse_token(const char** conf, size_t* length, parseconf_token_t* token) {
    int quoted = 0, end = 0;

    if (!conf || !*conf || !length || !token) {
        return PARSECONF_EINVAL;
    }
    if (!*length) {
        return PARSECONF_ERROR;
    }
    if (**conf == ' ' || **conf == '\t' || **conf == ';' || !**conf || **conf == '\n' || **conf == '\r') {
        return PARSECONF_ERROR;
    }
    if (**conf == '#') {
        return PARSECONF_COMMENT;
    }

    if (**conf == '"') {
        quoted = 1;
        (*conf)++;
        (*length)--;
        token->type = PARSECONF_TOKEN_QSTRING;
    }
    else {
        token->type = PARSECONF_TOKEN_NUMBER;
    }

    token->token = *conf;
    token->length = 0;

    for (; **conf && length; (*conf)++, (*length)--) {
        if (quoted && **conf == '"') {
            end = 1;
            continue;
        }
        else if ((!quoted || end) && (**conf == ' ' || **conf == '\t' || **conf == ';')) {
            if (**conf == ';') {
                (*conf)++;
                (*length)--;
                return PARSECONF_LAST;
            }
            (*conf)++;
            (*length)--;
            return PARSECONF_OK;
        }
        else if (end || **conf == '\n' || **conf == '\r' || !**conf) {
            return PARSECONF_ERROR;
        }

        if (**conf == '.' && token->type == PARSECONF_TOKEN_NUMBER) {
            token->type = PARSECONF_TOKEN_FLOAT;
        }
        else if (**conf < '0' || **conf > '9') {
            switch (token->type) {
                case PARSECONF_TOKEN_NUMBER:
                    token->type = PARSECONF_TOKEN_STRING;
                    break;
                case PARSECONF_TOKEN_FLOAT:
                    return PARSECONF_ERROR;
                default:
                    break;
            }
        }

        token->length++;
    }

    return PARSECONF_ERROR;
}

static int parse_tokens(void* user, const parseconf_syntax_t* syntax, const parseconf_token_t* tokens, size_t token_size, size_t line, parseconf_error_callback_t error_callback) {
    const parseconf_syntax_t* syntaxp;
    const parseconf_token_type_t* type;
    size_t i;
    const char* errstr = "Syntax error or invalid arguments";

    if (!syntax || !tokens || !token_size) {
        if (error_callback)
            error_callback(user, PARSECONF_ERROR_INTERNAL, line, 0, 0, 0);
        return PARSECONF_ERROR;
    }

    if (tokens[0].type != PARSECONF_TOKEN_STRING) {
        if (error_callback)
            error_callback(user, PARSECONF_ERROR_EXPECT_STRING, line, 0, tokens, 0);
        return PARSECONF_ERROR;
    }

    for (syntaxp = syntax; syntaxp->token; syntaxp++) {
        if (!strncmp(tokens[0].token, syntaxp->token, tokens[0].length)) {
            break;
        }
    }
    if (!syntaxp->token) {
        if (error_callback)
            error_callback(user, PARSECONF_ERROR_UNKNOWN, line, 0, tokens, 0);
        return PARSECONF_ERROR;
    }

    for (type = syntaxp->syntax, i = 1; *type != PARSECONF_TOKEN_END && i < token_size; i++) {
        if (*type == PARSECONF_TOKEN_NESTED) {
            if (!syntaxp->nested) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_NO_NESTED, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }

            if (tokens[i].type != PARSECONF_TOKEN_STRING) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_EXPECT_STRING, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }

            for (syntaxp = syntaxp->nested; syntaxp->token; syntaxp++) {
                if (!strncmp(tokens[i].token, syntaxp->token, tokens[i].length)) {
                    break;
                }
            }
            if (!syntaxp->token) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_UNKNOWN, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }

            type = syntaxp->syntax;
            continue;
        }

        if (*type == PARSECONF_TOKEN_STRINGS) {
            if (tokens[i].type != PARSECONF_TOKEN_STRING) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_EXPECT_STRING, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }
            continue;
        }
        if (*type == PARSECONF_TOKEN_QSTRINGS) {
            if (tokens[i].type != PARSECONF_TOKEN_QSTRING) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_EXPECT_QSTRING, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }
            continue;
        }
        if (*type == PARSECONF_TOKEN_NUMBERS) {
            if (tokens[i].type != PARSECONF_TOKEN_NUMBER) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_EXPECT_NUMBER, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }
            continue;
        }
        if (*type == PARSECONF_TOKEN_FLOATS) {
            if (tokens[i].type != PARSECONF_TOKEN_FLOAT) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_EXPECT_FLOAT, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }
            continue;
        }
        if (*type == PARSECONF_TOKEN_ANY) {
            if (tokens[i].type != PARSECONF_TOKEN_STRING && tokens[i].type != PARSECONF_TOKEN_NUMBER && tokens[i].type != PARSECONF_TOKEN_QSTRING && tokens[i].type != PARSECONF_TOKEN_FLOAT) {
                if (error_callback)
                    error_callback(user, PARSECONF_ERROR_EXPECT_ANY, line, i, tokens, 0);
                return PARSECONF_ERROR;
            }
            continue;
        }

        if (tokens[i].type != *type) {
            if (error_callback) {
                error_callback(
                    user,
                    *type == PARSECONF_TOKEN_STRING ? PARSECONF_ERROR_EXPECT_STRING
                        : *type == PARSECONF_TOKEN_NUMBER ? PARSECONF_ERROR_EXPECT_NUMBER
                            : *type == PARSECONF_TOKEN_QSTRING ? PARSECONF_ERROR_EXPECT_QSTRING
                                : *type == PARSECONF_TOKEN_FLOAT ? PARSECONF_ERROR_EXPECT_FLOAT
                                    : PARSECONF_ERROR_EXPECT_ANY,
                    line, i, tokens, 0
                );
            }
            return PARSECONF_ERROR;
        }
        type++;
    }

    if (!syntaxp->callback) {
        if (error_callback)
            error_callback(user, PARSECONF_ERROR_NO_CALLBACK, line, i, tokens, 0);
        return PARSECONF_ERROR;
    }

    if (syntaxp->callback(user, tokens, &errstr)) {
        if (error_callback)
            error_callback(user, PARSECONF_ERROR_CALLBACK, line, i, tokens, errstr);
        return PARSECONF_ERROR;
    }

    return PARSECONF_OK;
}

/*
 * Value helpers
 */

int parseconf_ulongint(const parseconf_token_t* token, unsigned long int* value, const char** errstr) {
    char buf[32];
    char* endptr = 0;

    if (!token) {
        return 1;
    }
    if (!value) {
        return 1;
    }

    if (token->length > (sizeof(buf)-1)) {
        if (errstr)
            *errstr = "Too large value";
        return 1;
    }
    if (!*(token->token)) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, token->token, token->length);
    *value = strtoul(buf, &endptr, 10);

    if (!endptr || *endptr) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    return 0;
}

int parseconf_ulonglongint(const parseconf_token_t* token, unsigned long long int* value, const char** errstr) {
    char buf[64];
    char* endptr = 0;

    if (!token) {
        return 1;
    }
    if (!value) {
        return 1;
    }

    if (token->length > (sizeof(buf)-1)) {
        if (errstr)
            *errstr = "Too large value";
        return 1;
    }
    if (!*(token->token)) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, token->token, token->length);
    *value = strtoull(buf, &endptr, 10);

    if (!endptr || *endptr) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    return 0;
}

int parseconf_double(const parseconf_token_t* token, double* value, const char** errstr) {
    char buf[64];
    char* endptr = 0;

    if (!token) {
        return 1;
    }
    if (!value) {
        return 1;
    }

    if (token->length > (sizeof(buf)-1)) {
        if (errstr)
            *errstr = "Too large value";
        return 1;
    }
    if (!*(token->token)) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, token->token, token->length);
    *value = strtod(buf, &endptr);

    if (!endptr || *endptr) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    return 0;
}

int parseconf_longdouble(const parseconf_token_t* token, long double* value, const char** errstr) {
    char buf[128];
    char* endptr = 0;

    if (!token) {
        return 1;
    }
    if (!value) {
        return 1;
    }

    if (token->length > (sizeof(buf)-1)) {
        if (errstr)
            *errstr = "Too large value";
        return 1;
    }
    if (!*(token->token)) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, token->token, token->length);
    *value = strtold(buf, &endptr);

    if (!endptr || *endptr) {
        if (errstr)
            *errstr = "Invalid value";
        return 1;
    }

    return 0;
}

/*
 * Calls
 */

int parseconf_file(void* user, const char* file, const parseconf_syntax_t* syntax, parseconf_error_callback_t error_callback) {
    FILE* fp;
    char* buffer = 0;
    size_t bufsize = 0;
    const char* buf;
    size_t s, i, line = 0;
    parseconf_token_t tokens[PARSECONF_MAX_TOKENS];
    int ret, ret2;

    if (!file) {
        return PARSECONF_EINVAL;
    }
    if (!syntax) {
        return PARSECONF_EINVAL;
    }

    if (!(fp = fopen(file, "r"))) {
        if (error_callback)
            error_callback(user, PARSECONF_ERROR_FILE_ERRNO, 0, 0, 0, 0);
        return PARSECONF_ERROR;
    }
    ret2 = getline(&buffer, &bufsize, fp);
    buf = buffer;
    s = bufsize;
    line++;
    while (ret2 > 0) {
        memset(tokens, 0, sizeof(tokens));
        /*
         * Go to the first non white-space character
         */
        for (ret = PARSECONF_OK; *buf && s; buf++, s--) {
            if (*buf != ' ' && *buf != '\t') {
                if (*buf == '\n' || *buf == '\r') {
                    ret = PARSECONF_EMPTY;
                }
                break;
            }
        }
        /*
         * Parse all the tokens
         */
        for (i = 0; i < PARSECONF_MAX_TOKENS && ret == PARSECONF_OK; i++) {
            ret = parse_token(&buf, &s, &tokens[i]);
        }

        if (ret == PARSECONF_COMMENT) {
            /*
             * Line ended with comment, reduce the number of tokens
             */
            i--;
        }
        else if (ret == PARSECONF_EMPTY) {
            i = 0;
        }
        else if (ret == PARSECONF_OK) {
            if (error_callback)
                error_callback(user, PARSECONF_ERROR_TOO_MANY_ARGUMENTS, line, 0, tokens, 0);
            free(buffer);
            fclose(fp);
            return PARSECONF_ERROR;
        }
        else if (ret != PARSECONF_LAST) {
            if (error_callback)
                error_callback(user, PARSECONF_ERROR_INVALID_SYNTAX, line, 0, tokens, 0);
            free(buffer);
            fclose(fp);
            return PARSECONF_ERROR;
        }

        if (i) {
            /*
             * Config using the tokens
             */
            if (parse_tokens(user, syntax, tokens, i, line, error_callback) != PARSECONF_OK) {
                free(buffer);
                fclose(fp);
                return PARSECONF_ERROR;
            }
        }

        if (ret == PARSECONF_COMMENT || !s || !*buf || *buf == '\n' || *buf == '\r') {
            ret2 = getline(&buffer, &bufsize, fp);
            buf = buffer;
            s = bufsize;
            line++;
        }
    }
    if (ret2 < 0) {
        long pos;

        pos = ftell(fp);
        if (fseek(fp, 0, SEEK_END)) {
            if (error_callback)
                error_callback(user, PARSECONF_ERROR_FILE_ERRNO, line, 0, 0, 0);
        }
        else if (ftell(fp) < pos) {
            if (error_callback)
                error_callback(user, PARSECONF_ERROR_FILE_ERRNO, line, 0, 0, 0);
        }
    }
    free(buffer);
    fclose(fp);

    return PARSECONF_OK;
}

int parseconf_text(void* user, const char* text, const size_t length, const parseconf_syntax_t* syntax, parseconf_error_callback_t error_callback) {
    const char* buf;
    size_t s, i, line = 0;
    parseconf_token_t tokens[PARSECONF_MAX_TOKENS];
    int ret;

    if (!text) {
        return PARSECONF_EINVAL;
    }
    if (!syntax) {
        return PARSECONF_EINVAL;
    }

    memset(tokens, 0, sizeof(tokens));
    buf = text;
    s = length;
    line++;

    while (1) {
        /*
         * Go to the first non white-space character
         */
        for (ret = PARSECONF_OK; *buf && s; buf++, s--) {
            if (*buf != ' ' && *buf != '\t') {
                if (*buf == '\n' || *buf == '\t') {
                    ret = PARSECONF_EMPTY;
                }
                break;
            }
        }
        /*
         * Parse all the tokens
         */
        for (i = 0; i < PARSECONF_MAX_TOKENS && ret == PARSECONF_OK; i++) {
            ret = parse_token(&buf, &s, &tokens[i]);
        }

        if (ret == PARSECONF_COMMENT) {
            /*
             * Line ended with comment, reduce the number of tokens
             */
            i--;
            if (!i) {
                /*
                 * Comment was the only token so the line is empty
                 */
                return PARSECONF_OK;
            }
        }
        else if (ret == PARSECONF_EMPTY) {
            i = 0;
        }
        else if (ret == PARSECONF_OK) {
            if (error_callback)
                error_callback(user, PARSECONF_ERROR_TOO_MANY_ARGUMENTS, line, 0, tokens, 0);
            return PARSECONF_ERROR;
        }
        else if (ret != PARSECONF_LAST) {
            if (error_callback)
                error_callback(user, PARSECONF_ERROR_INVALID_SYNTAX, line, 0, tokens, 0);
            return PARSECONF_ERROR;
        }

        /*
         * Configure using the tokens
         */
        if (i && parse_tokens(user, syntax, tokens, i, line, error_callback) != PARSECONF_OK) {
            return PARSECONF_ERROR;
        }

        if (ret == PARSECONF_COMMENT || !s || !*buf || *buf == '\n' || *buf == '\r') {
            break;
        }
    }

    return PARSECONF_OK;
}

/*
 * Error strings
 */

const char* parseconf_strerror(int errnum) {
    switch (errnum) {
        case PARSECONF_ERROR:
            return PARSECONF_ERROR_STR;
        case PARSECONF_EINVAL:
            return PARSECONF_EINVAL_STR;
        case PARSECONF_ENOMEM:
            return PARSECONF_ENOMEM_STR;
        case PARSECONF_EEXIST:
            return PARSECONF_EEXIST_STR;
        default:
            break;
    }
    return "Unknown error";
}
