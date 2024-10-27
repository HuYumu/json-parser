#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <string.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char *t, lept_type t_ype) {
    const char *p = c->json;
    size_t i;
    for(i = 0; i < strlen(t); ++i) {
        if(p[i] != t[i]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }

    c->json += strlen(t);
    v->type = t_ype;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    if(!ISDIGIT(*(c->json)) && *(c->json) != '-') {
        return LEPT_PARSE_INVALID_VALUE;
    }

    const char *p = c->json;
    if(*p == '-')
        ++p;

    if(*p == '0') {
        if(p[1] != 'E' && p[1] != 'e' && p[1] != '.' && p[1] != '\0') {
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
        ++p;
    } else {
        while(ISDIGIT(*p))
            ++p;
    }

    if(*p == '.') {
        if(!ISDIGIT(p[1]))
            return LEPT_PARSE_INVALID_VALUE;
        ++p;
        while(ISDIGIT(*p))
            ++p;
    }

    if(*p == 'e' || *p == 'E') {
        if(p[1] != '+' && p[1] != '-' && !ISDIGIT(p[1]))
            return LEPT_PARSE_INVALID_VALUE;
        if(p[1] == '+' || p[1] == '-')
            ++p;
        while(ISDIGIT(*p))
            ++p;
    }

    const char* end;
    v->n = strtod(c->json, &end);
    if (errno == ERANGE) {
        v->n = 0;
        errno = 0;
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
