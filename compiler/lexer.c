/*
 * Keikaku Programming Language - Lexer Implementation
 *
 * "Your characters have been observed and classified."
 */

#include "lexer.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Keyword Table
 * ============================================================================
 */

typedef struct {
  const char *keyword;
  K_TokenType type;
} KeywordEntry;

static const KeywordEntry keywords[] = {
    /* Core */
    {"designate", TOKEN_DESIGNATE},
    {"foresee", TOKEN_FORESEE},
    {"alternate", TOKEN_ALTERNATE},
    {"otherwise", TOKEN_OTHERWISE},
    {"cycle", TOKEN_CYCLE},
    {"while", TOKEN_WHILE},
    {"through", TOKEN_THROUGH},
    {"from", TOKEN_FROM},
    {"to", TOKEN_TO},
    {"as", TOKEN_AS},
    {"protocol", TOKEN_PROTOCOL},
    {"yield", TOKEN_YIELD},
    {"and", TOKEN_AND},
    {"or", TOKEN_OR},
    {"not", TOKEN_NOT},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},

    /* Special constructs */
    {"scheme", TOKEN_SCHEME},
    {"execute", TOKEN_EXECUTE},
    {"preview", TOKEN_PREVIEW},
    {"override", TOKEN_OVERRIDE},
    {"absolute", TOKEN_ABSOLUTE},
    {"anomaly", TOKEN_ANOMALY},

    /* Advanced features */
    {"attempt", TOKEN_ATTEMPT},
    {"recover", TOKEN_RECOVER},
    {"incorporate", TOKEN_INCORPORATE},
    {"entity", TOKEN_ENTITY},
    {"manifest", TOKEN_MANIFEST},
    {"self", TOKEN_SELF},
    {"inherits", TOKEN_INHERITS},
    {"situation", TOKEN_SITUATION},
    {"alignment", TOKEN_ALIGNMENT},
    {"ascend", TOKEN_ASCEND},
    {"sequence", TOKEN_SEQUENCE},
    {"delegate", TOKEN_DELEGATE},
    {"for", TOKEN_FOR},
    {"where", TOKEN_WHERE},
    {"async", TOKEN_ASYNC},
    {"await", TOKEN_AWAIT},

    {NULL, TOKEN_EOF}};

/* ============================================================================
 * Token Type Names
 * ============================================================================
 */

static const char *token_type_names[] = {
    "INTEGER",    "FLOAT",       "STRING",     "TRUE",      "FALSE",
    "IDENTIFIER", "DESIGNATE",   "FORESEE",    "ALTERNATE", "OTHERWISE",
    "CYCLE",      "WHILE",       "THROUGH",    "FROM",      "TO",
    "AS",         "PROTOCOL",    "YIELD",      "AND",       "OR",
    "NOT",        "BREAK",       "CONTINUE",   "SCHEME",    "EXECUTE",
    "PREVIEW",    "OVERRIDE",    "ABSOLUTE",   "ANOMALY",   "ATTEMPT",
    "RECOVER",    "INCORPORATE", "ENTITY",     "MANIFEST",  "SELF",
    "INHERITS",   "SITUATION",   "ALIGNMENT",  "ASCEND",    "SEQUENCE",
    "DELEGATE",   "FOR",         "WHERE",      "ASYNC",     "AWAIT",
    "PLUS",       "MINUS",       "STAR",       "SLASH",     "DOUBLE_SLASH",
    "PERCENT",    "DOUBLE_STAR", "ASSIGN",     "WALRUS",    "EQUAL",
    "NOT_EQUAL",  "LESS",        "LESS_EQUAL", "GREATER",   "GREATER_EQUAL",
    "ARROW",      "LPAREN",      "RPAREN",     "LBRACKET",  "RBRACKET",
    "LBRACE",     "RBRACE",      "COMMA",      "COLON",     "DOT",
    "NEWLINE",    "INDENT",      "DEDENT",     "EOF",       "ERROR"};

const char *token_type_name(K_TokenType type) {
  if (type >= 0 && type < TOKEN_COUNT) {
    return token_type_names[type];
  }
  return "UNKNOWN";
}

/* ============================================================================
 * Lexer Lifecycle
 * ============================================================================
 */

Lexer *lexer_create(const char *source, const char *filename) {
  Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
  if (!lexer)
    return NULL;

  lexer->source = source;
  lexer->filename = filename ? filename : "<input>";
  lexer->length = strlen(source);
  lexer->current = 0;
  lexer->start = 0;
  lexer->line = 1;
  lexer->column = 1;

  /* Indentation stack */
  lexer->indent_stack_capacity = 64;
  lexer->indent_stack =
      (int *)malloc(sizeof(int) * lexer->indent_stack_capacity);
  lexer->indent_stack[0] = 0;
  lexer->indent_stack_size = 1;
  lexer->pending_dedents = 0;
  lexer->at_line_start = true;

  /* Token buffer */
  lexer->token_capacity = 1024;
  lexer->tokens = (Token *)malloc(sizeof(Token) * lexer->token_capacity);
  lexer->token_count = 0;

  lexer->has_error = false;
  lexer->error_buffer[0] = '\0';

  return lexer;
}

void lexer_destroy(Lexer *lexer) {
  if (!lexer)
    return;

  if (lexer->indent_stack)
    free(lexer->indent_stack);
  if (lexer->tokens) {
    for (size_t i = 0; i < lexer->token_count; i++) {
      if (lexer->tokens[i].value.string_value &&
          lexer->tokens[i].type == TOKEN_STRING) {
        free(lexer->tokens[i].value.string_value);
      }
    }
    free(lexer->tokens);
  }
  free(lexer);
}

/* ============================================================================
 * Character Helpers
 * ============================================================================
 */

static inline char peek(Lexer *lexer) {
  if (lexer->current >= lexer->length)
    return '\0';
  return lexer->source[lexer->current];
}

static inline char peek_next(Lexer *lexer) {
  if (lexer->current + 1 >= lexer->length)
    return '\0';
  return lexer->source[lexer->current + 1];
}

static inline char advance(Lexer *lexer) {
  if (lexer->current >= lexer->length)
    return '\0';
  char c = lexer->source[lexer->current++];
  if (c == '\n') {
    lexer->line++;
    lexer->column = 1;
  } else {
    lexer->column++;
  }
  return c;
}

static inline bool at_end(Lexer *lexer) {
  return lexer->current >= lexer->length;
}

static inline bool match(Lexer *lexer, char expected) {
  if (at_end(lexer))
    return false;
  if (lexer->source[lexer->current] != expected)
    return false;
  advance(lexer);
  return true;
}

/* ============================================================================
 * Error Handling
 * ============================================================================
 */

static void lexer_error(Lexer *lexer, const char *format, ...) {
  lexer->has_error = true;
  va_list args;
  va_start(args, format);
  vsnprintf(lexer->error_buffer, sizeof(lexer->error_buffer), format, args);
  va_end(args);
}

bool lexer_has_error(const Lexer *lexer) { return lexer->has_error; }

const char *lexer_get_error(const Lexer *lexer) { return lexer->error_buffer; }

/* ============================================================================
 * Token Creation
 * ============================================================================
 */

static Token make_token(Lexer *lexer, K_TokenType type) {
  Token token;
  token.type = type;
  token.start = lexer->source + lexer->start;
  token.length = lexer->current - lexer->start;
  token.line = lexer->line;
  token.column = lexer->column - (int)token.length;
  token.has_error = false;
  token.error_message = NULL;
  memset(&token.value, 0, sizeof(token.value));
  return token;
}

static Token error_token(Lexer *lexer, const char *message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = strlen(message);
  token.line = lexer->line;
  token.column = lexer->column;
  token.has_error = true;
  token.error_message = (char *)message;
  return token;
}

/* ============================================================================
 * Whitespace and Comments
 * ============================================================================
 */

static void skip_whitespace_inline(Lexer *lexer) {
  while (!at_end(lexer)) {
    char c = peek(lexer);
    if (c == ' ' || c == '\t' || c == '\r') {
      advance(lexer);
    } else {
      break;
    }
  }
}

static void skip_comment(Lexer *lexer) {
  /* Line comment: # ... */
  while (!at_end(lexer) && peek(lexer) != '\n') {
    advance(lexer);
  }
}

/* ============================================================================
 * Indentation Handling
 * ============================================================================
 */

static int measure_indent(Lexer *lexer) {
  int indent = 0;
  size_t pos = lexer->current;

  while (pos < lexer->length) {
    char c = lexer->source[pos];
    if (c == ' ') {
      indent++;
      pos++;
    } else if (c == '\t') {
      indent += 4;
      pos++;
    } else {
      break;
    }
  }

  return indent;
}

static void push_indent(Lexer *lexer, int indent) {
  if (lexer->indent_stack_size >= lexer->indent_stack_capacity) {
    lexer->indent_stack_capacity *= 2;
    lexer->indent_stack = (int *)realloc(
        lexer->indent_stack, sizeof(int) * lexer->indent_stack_capacity);
  }
  lexer->indent_stack[lexer->indent_stack_size++] = indent;
}

static int current_indent(Lexer *lexer) {
  return lexer->indent_stack[lexer->indent_stack_size - 1];
}

/* ============================================================================
 * Number Parsing
 * ============================================================================
 */

static Token read_number(Lexer *lexer) {
  bool is_float = false;

  while (isdigit(peek(lexer))) {
    advance(lexer);
  }

  /* Decimal point */
  if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
    is_float = true;
    advance(lexer); /* consume . */
    while (isdigit(peek(lexer))) {
      advance(lexer);
    }
  }

  /* Exponent */
  if (peek(lexer) == 'e' || peek(lexer) == 'E') {
    is_float = true;
    advance(lexer);
    if (peek(lexer) == '+' || peek(lexer) == '-') {
      advance(lexer);
    }
    while (isdigit(peek(lexer))) {
      advance(lexer);
    }
  }

  Token token = make_token(lexer, is_float ? TOKEN_FLOAT : TOKEN_INTEGER);

  /* Parse value */
  char *lexeme = token_lexeme(&token);
  if (is_float) {
    token.value.float_value = strtod(lexeme, NULL);
  } else {
    token.value.int_value = strtoll(lexeme, NULL, 10);
  }
  free(lexeme);

  return token;
}

/* ============================================================================
 * String Parsing
 * ============================================================================
 */

static Token read_string(Lexer *lexer, char quote) {
  advance(lexer); /* consume opening quote */
  size_t start_pos = lexer->current;

  while (!at_end(lexer) && peek(lexer) != quote) {
    if (peek(lexer) == '\n') {
      return error_token(lexer, "Unterminated string. Your words trail off...");
    }
    if (peek(lexer) == '\\') {
      advance(lexer); /* skip backslash */
      if (!at_end(lexer))
        advance(lexer); /* skip escaped char */
    } else {
      advance(lexer);
    }
  }

  if (at_end(lexer)) {
    return error_token(lexer,
                       "Unterminated string. The message remains incomplete.");
  }

  size_t str_len = lexer->current - start_pos;

  advance(lexer); /* consume closing quote */

  Token token = make_token(lexer, TOKEN_STRING);

  /* Process escape sequences */
  char *value = (char *)malloc(str_len + 1);
  size_t j = 0;
  for (size_t i = 0; i < str_len; i++) {
    char c = lexer->source[start_pos + i];
    if (c == '\\' && i + 1 < str_len) {
      i++;
      switch (lexer->source[start_pos + i]) {
      case 'n':
        value[j++] = '\n';
        break;
      case 't':
        value[j++] = '\t';
        break;
      case 'r':
        value[j++] = '\r';
        break;
      case '\\':
        value[j++] = '\\';
        break;
      case '\'':
        value[j++] = '\'';
        break;
      case '"':
        value[j++] = '"';
        break;
      default:
        value[j++] = lexer->source[start_pos + i];
        break;
      }
    } else {
      value[j++] = c;
    }
  }
  value[j] = '\0';
  token.value.string_value = value;

  return token;
}

/* ============================================================================
 * Identifier / Keyword
 * ============================================================================
 */

static K_TokenType check_keyword(const char *start, size_t length) {
  for (int i = 0; keywords[i].keyword != NULL; i++) {
    if (strlen(keywords[i].keyword) == length &&
        strncmp(start, keywords[i].keyword, length) == 0) {
      return keywords[i].type;
    }
  }
  return TOKEN_IDENTIFIER;
}

static Token read_identifier(Lexer *lexer) {
  while (isalnum(peek(lexer)) || peek(lexer) == '_') {
    advance(lexer);
  }

  const char *start = lexer->source + lexer->start;
  size_t length = lexer->current - lexer->start;

  K_TokenType type = check_keyword(start, length);
  Token token = make_token(lexer, type);

  if (type == TOKEN_TRUE) {
    token.value.bool_value = true;
  } else if (type == TOKEN_FALSE) {
    token.value.bool_value = false;
  }

  return token;
}

/* ============================================================================
 * Main Tokenization
 * ============================================================================
 */

Token lexer_next_token(Lexer *lexer) {
  /* Handle pending dedents */
  if (lexer->pending_dedents > 0) {
    lexer->pending_dedents--;
    Token token;
    token.type = TOKEN_DEDENT;
    token.start = lexer->source + lexer->current;
    token.length = 0;
    token.line = lexer->line;
    token.column = lexer->column;
    token.has_error = false;
    return token;
  }

  /* Handle indentation at line start */
  if (lexer->at_line_start && !at_end(lexer)) {
    lexer->at_line_start = false;

    int indent = 0;
    while (peek(lexer) == ' ' || peek(lexer) == '\t') {
      if (peek(lexer) == ' ')
        indent++;
      else
        indent += 4;
      advance(lexer);
    }

    /* Skip blank lines and comments */
    if (peek(lexer) == '\n' || peek(lexer) == '#' || at_end(lexer)) {
      if (peek(lexer) == '#')
        skip_comment(lexer);
      /* Don't process indentation for blank lines */
    } else {
      int curr = current_indent(lexer);

      if (indent > curr) {
        push_indent(lexer, indent);
        Token token;
        token.type = TOKEN_INDENT;
        token.start = lexer->source + lexer->current;
        token.length = 0;
        token.line = lexer->line;
        token.column = 1;
        token.has_error = false;
        return token;
      } else if (indent < curr) {
        while (lexer->indent_stack_size > 1 &&
               indent < lexer->indent_stack[lexer->indent_stack_size - 1]) {
          lexer->indent_stack_size--;
          lexer->pending_dedents++;
        }
        if (lexer->pending_dedents > 0) {
          lexer->pending_dedents--;
          Token token;
          token.type = TOKEN_DEDENT;
          token.start = lexer->source + lexer->current;
          token.length = 0;
          token.line = lexer->line;
          token.column = 1;
          token.has_error = false;
          return token;
        }
      }
    }
  }

  /* Skip inline whitespace */
  skip_whitespace_inline(lexer);

  /* Skip comments */
  if (peek(lexer) == '#') {
    skip_comment(lexer);
  }

  lexer->start = lexer->current;

  if (at_end(lexer)) {
    /* Emit remaining dedents */
    if (lexer->indent_stack_size > 1) {
      lexer->indent_stack_size--;
      Token token;
      token.type = TOKEN_DEDENT;
      token.start = lexer->source + lexer->current;
      token.length = 0;
      token.line = lexer->line;
      token.column = lexer->column;
      token.has_error = false;
      return token;
    }
    return make_token(lexer, TOKEN_EOF);
  }

  char c = advance(lexer);

  /* Newline */
  if (c == '\n') {
    lexer->at_line_start = true;
    return make_token(lexer, TOKEN_NEWLINE);
  }

  /* Numbers */
  if (isdigit(c)) {
    lexer->current--; /* back up */
    lexer->column--;
    return read_number(lexer);
  }

  /* Identifiers and keywords */
  if (isalpha(c) || c == '_') {
    lexer->current--;
    lexer->column--;
    return read_identifier(lexer);
  }

  /* Strings */
  if (c == '"' || c == '\'') {
    lexer->current--;
    lexer->column--;
    return read_string(lexer, c);
  }

  /* Operators and delimiters */
  switch (c) {
  case '+':
    return make_token(lexer, TOKEN_PLUS);
  case '-':
    return make_token(lexer, TOKEN_MINUS);
  case '*':
    if (match(lexer, '*'))
      return make_token(lexer, TOKEN_DOUBLE_STAR);
    return make_token(lexer, TOKEN_STAR);
  case '.':
    if (match(lexer, '.')) {
      if (match(lexer, '.')) {
        return make_token(lexer, TOKEN_ELLIPSIS);
      }
      /* Fallback or handle .. */
    }
    return make_token(lexer, TOKEN_DOT);
  case '/':
    if (match(lexer, '/'))
      return make_token(lexer, TOKEN_DOUBLE_SLASH);
    return make_token(lexer, TOKEN_SLASH);
  case '%':
    return make_token(lexer, TOKEN_PERCENT);
  case '=':
    if (match(lexer, '='))
      return make_token(lexer, TOKEN_EQUAL);
    if (match(lexer, '>'))
      return make_token(lexer, TOKEN_ARROW);
    return make_token(lexer, TOKEN_ASSIGN);
  case ':':
    if (match(lexer, '='))
      return make_token(lexer, TOKEN_WALRUS);
    return make_token(lexer, TOKEN_COLON);
  case '!':
    if (match(lexer, '='))
      return make_token(lexer, TOKEN_NOT_EQUAL);
    return error_token(lexer, "Unexpected '!'. Did you intend '!='?");
  case '<':
    if (match(lexer, '='))
      return make_token(lexer, TOKEN_LESS_EQUAL);
    return make_token(lexer, TOKEN_LESS);
  case '>':
    if (match(lexer, '='))
      return make_token(lexer, TOKEN_GREATER_EQUAL);
    return make_token(lexer, TOKEN_GREATER);
  case '(':
    return make_token(lexer, TOKEN_LPAREN);
  case ')':
    return make_token(lexer, TOKEN_RPAREN);
  case '[':
    return make_token(lexer, TOKEN_LBRACKET);
  case ']':
    return make_token(lexer, TOKEN_RBRACKET);
  case '{':
    return make_token(lexer, TOKEN_LBRACE);
  case '}':
    return make_token(lexer, TOKEN_RBRACE);
  case ',':
    return make_token(lexer, TOKEN_COMMA);
  }

  return error_token(
      lexer,
      "Unexpected character. The system does not recognize this symbol.");
}

/* ============================================================================
 * Tokenize All
 * ============================================================================
 */

Token *lexer_tokenize_all(Lexer *lexer, size_t *count) {
  size_t capacity = 256;
  Token *tokens = (Token *)malloc(sizeof(Token) * capacity);
  size_t num = 0;

  while (true) {
    if (num >= capacity) {
      capacity *= 2;
      tokens = (Token *)realloc(tokens, sizeof(Token) * capacity);
    }

    Token token = lexer_next_token(lexer);
    tokens[num++] = token;

    if (token.type == TOKEN_EOF || token.type == TOKEN_ERROR) {
      break;
    }
  }

  *count = num;
  return tokens;
}

void lexer_free_tokens(Token *tokens, size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (tokens[i].type == TOKEN_STRING && tokens[i].value.string_value) {
      free(tokens[i].value.string_value);
    }
  }
  free(tokens);
}

/* ============================================================================
 * Token Utilities
 * ============================================================================
 */

char *token_lexeme(const Token *token) {
  char *lexeme = (char *)malloc(token->length + 1);
  memcpy(lexeme, token->start, token->length);
  lexeme[token->length] = '\0';
  return lexeme;
}

void token_print(const Token *token) {
  char *lexeme = token_lexeme(token);
  printf("[%3d:%3d] %-16s '%s'\n", token->line, token->column,
         token_type_name(token->type), lexeme);
  free(lexeme);
}
