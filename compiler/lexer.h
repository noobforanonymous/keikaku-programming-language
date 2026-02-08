/*
 * Keikaku Programming Language - Lexer Header
 *
 * "Your tokens have been anticipated."
 */

#ifndef KEIKAKU_LEXER_H
#define KEIKAKU_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ============================================================================
 * Token Types
 * ============================================================================
 */

typedef enum {
  /* Literals */
  TOKEN_INTEGER,
  TOKEN_FLOAT,
  TOKEN_STRING,
  TOKEN_TRUE,
  TOKEN_FALSE,

  /* Identifier */
  TOKEN_IDENTIFIER,

  /* Core Keywords */
  TOKEN_DESIGNATE, /* designate */
  TOKEN_FORESEE,   /* foresee */
  TOKEN_ALTERNATE, /* alternate */
  TOKEN_OTHERWISE, /* otherwise */
  TOKEN_CYCLE,     /* cycle */
  TOKEN_WHILE,     /* while */
  TOKEN_THROUGH,   /* through */
  TOKEN_FROM,      /* from */
  TOKEN_TO,        /* to */
  TOKEN_AS,        /* as */
  TOKEN_PROTOCOL,  /* protocol */
  TOKEN_YIELD,     /* yield */
  TOKEN_AND,       /* and */
  TOKEN_OR,        /* or */
  TOKEN_NOT,       /* not */
  TOKEN_BREAK,     /* break */
  TOKEN_CONTINUE,  /* continue */

  /* Special Construct Keywords */
  TOKEN_SCHEME,   /* scheme */
  TOKEN_EXECUTE,  /* execute */
  TOKEN_PREVIEW,  /* preview */
  TOKEN_OVERRIDE, /* override */
  TOKEN_ABSOLUTE, /* absolute */
  TOKEN_ANOMALY,  /* anomaly */

  /* Advanced Feature Keywords */
  TOKEN_ATTEMPT,     /* attempt (try) */
  TOKEN_RECOVER,     /* recover (catch) */
  TOKEN_INCORPORATE, /* incorporate (import) */
  TOKEN_ENTITY,      /* entity (class) */
  TOKEN_MANIFEST,    /* manifest (new instance) */
  TOKEN_SELF,        /* self (this) */
  TOKEN_INHERITS,    /* inherits (extends) */
  TOKEN_SITUATION,   /* situation (match) */
  TOKEN_ALIGNMENT,   /* alignment (case) */
  TOKEN_ASCEND,      /* ascend (super) */
  TOKEN_SEQUENCE,    /* sequence (generator) */
  TOKEN_DELEGATE,    /* delegate (yield from) */
  TOKEN_FOR,         /* for (generator expression) */
  TOKEN_WHERE,       /* where (filter in gen expr) */
  TOKEN_ASYNC,       /* async */
  TOKEN_AWAIT,       /* await */

  /* Operators */
  TOKEN_PLUS,          /* + */
  TOKEN_MINUS,         /* - */
  TOKEN_STAR,          /* * */
  TOKEN_SLASH,         /* / */
  TOKEN_DOUBLE_SLASH,  /* // */
  TOKEN_PERCENT,       /* % */
  TOKEN_DOUBLE_STAR,   /* ** */
  TOKEN_ASSIGN,        /* = */
  TOKEN_WALRUS,        /* := */
  TOKEN_EQUAL,         /* == */
  TOKEN_NOT_EQUAL,     /* != */
  TOKEN_LESS,          /* < */
  TOKEN_LESS_EQUAL,    /* <= */
  TOKEN_GREATER,       /* > */
  TOKEN_GREATER_EQUAL, /* >= */
  TOKEN_ARROW,         /* => */
  TOKEN_ELLIPSIS,      /* ... */

  /* Delimiters */
  TOKEN_LPAREN,   /* ( */
  TOKEN_RPAREN,   /* ) */
  TOKEN_LBRACKET, /* [ */
  TOKEN_RBRACKET, /* ] */
  TOKEN_LBRACE,   /* { */
  TOKEN_RBRACE,   /* } */
  TOKEN_COMMA,    /* , */
  TOKEN_COLON,    /* : */
  TOKEN_DOT,      /* . */

  /* Structure */
  TOKEN_NEWLINE,
  TOKEN_INDENT,
  TOKEN_DEDENT,
  TOKEN_EOF,
  TOKEN_ERROR,

  TOKEN_COUNT
} K_TokenType;

/* ============================================================================
 * Token Structure
 * ============================================================================
 */

typedef struct {
  K_TokenType type;
  const char *start; /* Pointer to start in source */
  size_t length;     /* Length of lexeme */
  int line;
  int column;

  /* Parsed values */
  union {
    int64_t int_value;
    double float_value;
    char *string_value;
    bool bool_value;
  } value;

  /* Error info */
  bool has_error;
  char *error_message;
} Token;

/* ============================================================================
 * Lexer Structure
 * ============================================================================
 */

typedef struct Lexer {
  const char *source;
  const char *filename;
  size_t length;
  size_t current;
  size_t start;
  int line;
  int column;

  /* Indentation tracking */
  int *indent_stack;
  size_t indent_stack_size;
  size_t indent_stack_capacity;
  int pending_dedents;
  bool at_line_start;

  /* Token buffer */
  Token *tokens;
  size_t token_count;
  size_t token_capacity;

  /* Error state */
  bool has_error;
  char error_buffer[512];
} Lexer;

/* ============================================================================
 * Lexer Functions
 * ============================================================================
 */

/* Lifecycle */
Lexer *lexer_create(const char *source, const char *filename);
void lexer_destroy(Lexer *lexer);
void lexer_reset(Lexer *lexer, const char *source, const char *filename);

/* Tokenization */
Token lexer_next_token(Lexer *lexer);
Token *lexer_tokenize_all(Lexer *lexer, size_t *count);
void lexer_free_tokens(Token *tokens, size_t count);

/* Token utilities */
const char *token_type_name(K_TokenType type);
char *token_lexeme(const Token *token);
void token_print(const Token *token);

/* Error handling */
bool lexer_has_error(const Lexer *lexer);
const char *lexer_get_error(const Lexer *lexer);

#endif /* KEIKAKU_LEXER_H */
