/*
 * Keikaku Programming Language
 * Copyright (c) 2026
 * 
 * "Everything is proceeding according to keikaku."
 * 
 * A calm, omniscient programming language that has already
 * foreseen, permitted, and subtly orchestrated whatever
 * the programmer attempts.
 */

#ifndef KEIKAKU_H
#define KEIKAKU_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Version info */
#define KEIKAKU_VERSION_MAJOR 1
#define KEIKAKU_VERSION_MINOR 0
#define KEIKAKU_VERSION_PATCH 0
#define KEIKAKU_VERSION_STRING "1.0.0"

/* Maximum limits */
#define KEIKAKU_MAX_IDENTIFIER_LENGTH 256
#define KEIKAKU_MAX_STRING_LENGTH 1048576
#define KEIKAKU_MAX_PARAMS 255
#define KEIKAKU_MAX_LOCALS 65536
#define KEIKAKU_MAX_UPVALUES 256
#define KEIKAKU_MAX_CALL_DEPTH 1024
#define KEIKAKU_MAX_LOOP_DEPTH 256

/* Result codes */
typedef enum {
    KEIKAKU_OK = 0,
    KEIKAKU_ERROR_SYNTAX,
    KEIKAKU_ERROR_SEMANTIC,
    KEIKAKU_ERROR_RUNTIME,
    KEIKAKU_ERROR_MEMORY,
    KEIKAKU_ERROR_IO,
    KEIKAKU_ERROR_INTERNAL
} KeikakuResult;

/* Forward declarations */
typedef struct Lexer Lexer;
typedef struct Parser Parser;
typedef struct AST AST;
typedef struct Interpreter Interpreter;
typedef struct Environment Environment;
typedef struct Value Value;

/* Initialize and cleanup */
KeikakuResult keikaku_init(void);
void keikaku_cleanup(void);

/* Run source code */
KeikakuResult keikaku_run_string(const char* source, const char* filename);
KeikakuResult keikaku_run_file(const char* filename);

/* REPL */
KeikakuResult keikaku_repl(void);

/* Error handling */
const char* keikaku_get_error(void);
void keikaku_clear_error(void);

#endif /* KEIKAKU_H */
