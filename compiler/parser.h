/*
 * Keikaku Programming Language - Parser Header
 *
 * "Your intentions are being structured."
 */

#ifndef KEIKAKU_PARSER_H
#define KEIKAKU_PARSER_H

#include "ast.h"
#include "lexer.h"
#include <stdbool.h>

/* ============================================================================
 * Parser Structure
 * ============================================================================
 */

typedef struct Parser {
  Token *tokens;
  size_t token_count;
  size_t current;

  /* Error handling */
  bool has_error;
  bool panic_mode;
  char error_buffer[1024];

  /* Source info for error messages */
  const char *source;
  const char *filename;
} Parser;

/* ============================================================================
 * Parser Functions
 * ============================================================================
 */

/* Lifecycle */
Parser *parser_create(Token *tokens, size_t count, const char *source,
                      const char *filename);
void parser_destroy(Parser *parser);

/* Parsing */
ASTNode *parser_parse(Parser *parser);

/* Error handling */
bool parser_has_error(const Parser *parser);
const char *parser_get_error(const Parser *parser);

#endif /* KEIKAKU_PARSER_H */
