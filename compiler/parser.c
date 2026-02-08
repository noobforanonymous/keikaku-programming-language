/*
 * Keikaku Programming Language - Parser Implementation
 *
 * "Your code is being assembled into the grand design."
 */

#include "parser.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Parser Lifecycle
 * ============================================================================
 */

Parser *parser_create(Token *tokens, size_t count, const char *source,
                      const char *filename) {
  Parser *parser = (Parser *)malloc(sizeof(Parser));
  if (!parser)
    return NULL;

  parser->tokens = tokens;
  parser->token_count = count;
  parser->current = 0;
  parser->has_error = false;
  parser->panic_mode = false;
  parser->error_buffer[0] = '\0';
  parser->source = source;
  parser->filename = filename;

  return parser;
}

void parser_destroy(Parser *parser) {
  if (parser) {
    free(parser);
  }
}

/* ============================================================================
 * Token Access
 * ============================================================================
 */

static Token *current(Parser *parser) {
  if (parser->current >= parser->token_count) {
    return &parser->tokens[parser->token_count - 1];
  }
  return &parser->tokens[parser->current];
}

static Token *previous(Parser *parser) {
  if (parser->current == 0)
    return &parser->tokens[0];
  return &parser->tokens[parser->current - 1];
}

static Token *peek(Parser *parser, int offset) {
  size_t pos = parser->current + offset;
  if (pos >= parser->token_count) {
    return &parser->tokens[parser->token_count - 1];
  }
  return &parser->tokens[pos];
}

static bool at_end(Parser *parser) {
  return current(parser)->type == TOKEN_EOF;
}

static bool check(Parser *parser, K_TokenType type) {
  if (at_end(parser))
    return type == TOKEN_EOF;
  return current(parser)->type == type;
}

static Token *advance(Parser *parser) {
  if (!at_end(parser)) {
    parser->current++;
  }
  return previous(parser);
}

static bool match(Parser *parser, K_TokenType type) {
  if (check(parser, type)) {
    advance(parser);
    return true;
  }
  return false;
}

static void skip_newlines(Parser *parser) {
  while (match(parser, TOKEN_NEWLINE)) {
    /* consume */
  }
}

/* ============================================================================
 * Error Handling
 * ============================================================================
 */

static void error_at(Parser *parser, Token *token, const char *message) {
  if (parser->panic_mode)
    return;
  parser->panic_mode = true;
  parser->has_error = true;

  snprintf(parser->error_buffer, sizeof(parser->error_buffer),
           "  ⚠ Structural anomaly at line %d. %s\n"
           "    Your intent was... misaligned. The scenario adjusts.",
           token->line, message);
}

static void error(Parser *parser, const char *message) {
  error_at(parser, current(parser), message);
}

static Token *expect(Parser *parser, K_TokenType type, const char *message) {
  if (check(parser, type)) {
    return advance(parser);
  }
  error(parser, message);
  return NULL;
}

bool parser_has_error(const Parser *parser) { return parser->has_error; }

const char *parser_get_error(const Parser *parser) {
  return parser->error_buffer;
}

/* ============================================================================
 * Forward Declarations
 * ============================================================================
 */

static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_statement(Parser *parser);
static ASTNodeArray parse_block(Parser *parser);

/* Helper for AST node creation */
static ASTNode *create_node(ASTNodeType type, int line, int col) {
  ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
  node->type = type;
  node->line = line;
  node->column = col;
  return node;
}

/* ============================================================================
 * Expression Parsing
 * ============================================================================
 */

static ASTNode *parse_primary(Parser *parser) {
  Token *token = current(parser);

  if (match(parser, TOKEN_INTEGER)) {
    return ast_create_int(token->value.int_value, token->line, token->column);
  }

  if (match(parser, TOKEN_FLOAT)) {
    return ast_create_float(token->value.float_value, token->line,
                            token->column);
  }

  if (match(parser, TOKEN_STRING)) {
    return ast_create_string(token->value.string_value, token->line,
                             token->column);
  }

  if (match(parser, TOKEN_TRUE)) {
    return ast_create_bool(true, token->line, token->column);
  }

  if (match(parser, TOKEN_FALSE)) {
    return ast_create_bool(false, token->line, token->column);
  }

  if (match(parser, TOKEN_IDENTIFIER)) {
    char *name = token_lexeme(token);
    ASTNode *node = ast_create_identifier(name, token->line, token->column);
    free(name);
    return node;
  }

  /* List literal or Comprehension */
  if (match(parser, TOKEN_LBRACKET)) {
    Token *lbracket = previous(parser);

    if (match(parser, TOKEN_RBRACKET)) {
      return ast_create_list(lbracket->line, lbracket->column);
    }

    /* Could be a regular list or a comprehension */
    ASTNode *first = parse_expression(parser);

    if (match(parser, TOKEN_CYCLE)) {
      /* Comprehension: [expr cycle through list as var foresee cond] */
      expect(parser, TOKEN_THROUGH,
             "Expected 'through' after 'cycle' in list comprehension.");
      ASTNode *iterable = parse_expression(parser);
      expect(parser, TOKEN_AS,
             "Expected 'as' before iteration variable in list comprehension.");
      Token *var_tok =
          expect(parser, TOKEN_IDENTIFIER, "Expected iteration variable name.");
      char *var_name = var_tok ? token_lexeme(var_tok) : strdup("_");

      ASTNode *condition = NULL;
      if (match(parser, TOKEN_FORESEE)) {
        condition = parse_expression(parser);
      }

      expect(parser, TOKEN_RBRACKET, "Expected ']' after list comprehension.");

      ASTNode *comp =
          create_node(AST_LIST_COMP, lbracket->line, lbracket->column);
      comp->data.list_comp.expr = first;
      comp->data.list_comp.iterable = iterable;
      comp->data.list_comp.var_name = var_name;
      comp->data.list_comp.condition = condition;
      return comp;
    }

    /* Regular list */
    ASTNode *list = ast_create_list(lbracket->line, lbracket->column);
    ast_array_push(&list->data.list.elements, first);

    while (match(parser, TOKEN_COMMA)) {
      if (check(parser, TOKEN_RBRACKET))
        break;
      ast_array_push(&list->data.list.elements, parse_expression(parser));
    }

    expect(parser, TOKEN_RBRACKET, "Expected ']' after list elements.");
    return list;
  }

  /* Dict literal */
  if (match(parser, TOKEN_LBRACE)) {
    ASTNode *dict = ast_create_dict(token->line, token->column);

    if (!check(parser, TOKEN_RBRACE)) {
      do {
        ASTNode *key = parse_expression(parser);
        expect(parser, TOKEN_COLON, "Expected ':' after dictionary key.");
        ASTNode *value = parse_expression(parser);
        if (key && value) {
          ast_kv_array_push(&dict->data.dict.pairs, key, value);
        }
      } while (match(parser, TOKEN_COMMA));
    }

    expect(parser, TOKEN_RBRACE, "Expected '}' after dictionary pairs.");
    return dict;
  }

  /* Parenthesized expression or Generator Expression */
  if (match(parser, TOKEN_LPAREN)) {
    Token *lparen = previous(parser);
    ASTNode *expr = parse_expression(parser);

    /* Check for generator expression: (expr for var in iterable [where cond])
     */
    if (match(parser, TOKEN_FOR)) {
      Token *var_tok =
          expect(parser, TOKEN_IDENTIFIER,
                 "Expected variable name after 'for' in generator expression.");
      char *var_name = var_tok ? token_lexeme(var_tok) : strdup("_");

      expect(parser, TOKEN_THROUGH,
             "Expected 'through' after variable in generator expression.");
      ASTNode *iterable = parse_expression(parser);

      ASTNode *condition = NULL;
      if (match(parser, TOKEN_WHERE)) {
        condition = parse_expression(parser);
      }

      expect(parser, TOKEN_RPAREN, "Expected ')' after generator expression.");
      return ast_create_gen_expr(expr, iterable, var_name, condition,
                                 lparen->line, lparen->column);
    }

    expect(parser, TOKEN_RPAREN, "Expected ')' after expression.");
    return expr;
  }

  /* Manifest - instance creation: manifest ClassName(args) */
  bool is_manifest = match(parser, TOKEN_MANIFEST);

  if (!is_manifest && check(parser, TOKEN_IDENTIFIER)) {
    Token *tok = current(parser);
    if (tok->length == 8 && strncmp(token_lexeme(tok), "manifest", 8) == 0) {
      advance(parser);
      is_manifest = true;
    }
  }

  if (is_manifest) {
    Token *class_name = expect(parser, TOKEN_IDENTIFIER,
                               "Expected class name after 'manifest'.");
    if (!class_name)
      return NULL;

    ASTNode *node = create_node(AST_MANIFEST, token->line, token->column);
    node->data.manifest.class_name = token_lexeme(class_name);
    ast_array_init(&node->data.manifest.args);

    expect(parser, TOKEN_LPAREN, "Expected '(' after class name.");

    if (!check(parser, TOKEN_RPAREN)) {
      do {
        ASTNode *arg = parse_expression(parser);
        if (arg) {
          ast_array_push(&node->data.manifest.args, arg);
        }
      } while (match(parser, TOKEN_COMMA));
    }

    expect(parser, TOKEN_RPAREN, "Expected ')' after arguments.");
    return node;
  }

  /* Spread operator */
  if (match(parser, TOKEN_ELLIPSIS)) {
    ASTNode *expr = parse_expression(parser);
    ASTNode *node = create_node(AST_SPREAD, token->line, token->column);
    node->data.spread.expr = expr;
    return node;
  }

  /* Self reference */
  if (match(parser, TOKEN_SELF)) {
    ASTNode *node = create_node(AST_SELF, token->line, token->column);
    return node;
  }

  /* Ascend - call parent protocol: ascend ProtocolName(args) */
  if (match(parser, TOKEN_ASCEND)) {
    Token *protocol_name = expect(parser, TOKEN_IDENTIFIER,
                                  "Expected protocol name after 'ascend'.");
    if (!protocol_name)
      return NULL;

    char *name = token_lexeme(protocol_name);
    ASTNode *node = ast_create_ascend(name, token->line, token->column);
    free(name);

    expect(parser, TOKEN_LPAREN, "Expected '(' after protocol name.");

    if (!check(parser, TOKEN_RPAREN)) {
      do {
        ASTNode *arg = parse_expression(parser);
        if (arg) {
          ast_array_push(&node->data.ascend.args, arg);
        }
      } while (match(parser, TOKEN_COMMA));
    }

    expect(parser, TOKEN_RPAREN, "Expected ')' after arguments.");
    return node;
  }

  error(parser, "Expected expression. The system awaits valid syntax.");
  return NULL;
}

static ASTNode *parse_postfix(Parser *parser) {
  ASTNode *left = parse_primary(parser);
  if (!left)
    return NULL;

  while (true) {
    /* Function call */
    if (match(parser, TOKEN_LPAREN)) {
      ASTNode *call = NULL;

      if (left->type == AST_IDENTIFIER) {
        /* Regular function call */
        char *name = strdup(left->data.identifier.name);
        call = create_node(AST_CALL, left->line, left->column);
        call->data.call.name = name;
        ast_array_init(&call->data.call.args);
        ast_destroy(left);
      } else if (left->type == AST_MEMBER) {
        /* Method call: obj.method(args) */
        call = create_node(AST_METHOD_CALL, left->line, left->column);
        call->data.method_call.object = left->data.member.object;
        call->data.method_call.method_name = strdup(left->data.member.member);
        ast_array_init(&call->data.method_call.args);

        /* Free the member access node, but not the object (moved to call) */
        free(left->data.member.member);
        free(left);
      } else {
        error(parser, "Can only call functions by name or methods.");
        return left;
      }

      left = call; /* Set left to the new call node so args can be added */

      ASTNodeArray *args;
      if (call->type == AST_CALL) {
        args = &call->data.call.args;
      } else {
        args = &call->data.method_call.args;
      }

      if (!check(parser, TOKEN_RPAREN)) {
        do {
          ASTNode *arg = parse_expression(parser);
          if (arg) {
            ast_array_push(args, arg);
          }
        } while (match(parser, TOKEN_COMMA));
      }

      expect(parser, TOKEN_RPAREN, "Expected ')' after arguments.");
      left = call;
    }
    /* Index or Slice access */
    else if (match(parser, TOKEN_LBRACKET)) {
      /* Check for slice syntax: [start:end:step] */
      ASTNode *start_idx = NULL;
      ASTNode *end_idx = NULL;
      ASTNode *step_idx = NULL;
      bool is_slice = false;

      /* Parse start (optional before first colon) */
      if (!check(parser, TOKEN_COLON) && !check(parser, TOKEN_RBRACKET)) {
        start_idx = parse_expression(parser);
      }

      /* Check for colon - indicates slice */
      if (match(parser, TOKEN_COLON)) {
        is_slice = true;

        /* Parse end (optional before second colon or bracket) */
        if (!check(parser, TOKEN_COLON) && !check(parser, TOKEN_RBRACKET)) {
          end_idx = parse_expression(parser);
        }

        /* Check for step */
        if (match(parser, TOKEN_COLON)) {
          if (!check(parser, TOKEN_RBRACKET)) {
            step_idx = parse_expression(parser);
          }
        }
      }

      expect(parser, TOKEN_RBRACKET, "Expected ']' after index/slice.");

      if (is_slice) {
        ASTNode *slice = create_node(AST_SLICE, left->line, left->column);
        slice->data.slice.object = left;
        slice->data.slice.start = start_idx;
        slice->data.slice.end = end_idx;
        slice->data.slice.step = step_idx;
        left = slice;
      } else {
        left = ast_create_index(left, start_idx, left->line, left->column);
      }
    }
    /* Member access */
    else if (match(parser, TOKEN_DOT)) {
      Token *name =
          expect(parser, TOKEN_IDENTIFIER, "Expected member name after '.'.");
      if (name) {
        char *member = token_lexeme(name);
        ASTNode *node = create_node(AST_MEMBER, left->line, left->column);
        node->data.member.object = left;
        node->data.member.member = member;
        left = node;
      }
    } else {
      break;
    }
  }

  return left;
}

static ASTNode *parse_unary(Parser *parser) {
  if (match(parser, TOKEN_MINUS)) {
    Token *op = previous(parser);
    ASTNode *operand = parse_unary(parser);
    return ast_create_unary(OP_NEG, operand, op->line, op->column);
  }

  if (match(parser, TOKEN_NOT)) {
    Token *op = previous(parser);
    ASTNode *operand = parse_unary(parser);
    return ast_create_unary(OP_NOT, operand, op->line, op->column);
  }

  /* Await expression */
  if (match(parser, TOKEN_AWAIT)) {
    Token *op = previous(parser);
    ASTNode *operand = parse_unary(parser);
    return ast_create_await(operand, op->line, op->column);
  }

  return parse_postfix(parser);
}

static ASTNode *parse_power(Parser *parser) {
  ASTNode *left = parse_unary(parser);

  if (match(parser, TOKEN_DOUBLE_STAR)) {
    Token *op = previous(parser);
    ASTNode *right = parse_power(parser); /* Right associative */
    left = ast_create_binary(OP_POW, left, right, op->line, op->column);
  }

  return left;
}

static ASTNode *parse_multiplicative(Parser *parser) {
  ASTNode *left = parse_power(parser);

  while (check(parser, TOKEN_STAR) || check(parser, TOKEN_SLASH) ||
         check(parser, TOKEN_DOUBLE_SLASH) || check(parser, TOKEN_PERCENT)) {
    Token *op = advance(parser);
    BinaryOp bin_op;
    switch (op->type) {
    case TOKEN_STAR:
      bin_op = OP_MUL;
      break;
    case TOKEN_SLASH:
      bin_op = OP_DIV;
      break;
    case TOKEN_DOUBLE_SLASH:
      bin_op = OP_INT_DIV;
      break;
    case TOKEN_PERCENT:
      bin_op = OP_MOD;
      break;
    default:
      bin_op = OP_MUL;
      break;
    }
    ASTNode *right = parse_power(parser);
    left = ast_create_binary(bin_op, left, right, op->line, op->column);
  }

  return left;
}

static ASTNode *parse_additive(Parser *parser) {
  ASTNode *left = parse_multiplicative(parser);

  while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
    Token *op = advance(parser);
    BinaryOp bin_op = (op->type == TOKEN_PLUS) ? OP_ADD : OP_SUB;
    ASTNode *right = parse_multiplicative(parser);
    left = ast_create_binary(bin_op, left, right, op->line, op->column);
  }

  return left;
}

static ASTNode *parse_comparison(Parser *parser) {
  ASTNode *left = parse_additive(parser);

  while (check(parser, TOKEN_EQUAL) || check(parser, TOKEN_NOT_EQUAL) ||
         check(parser, TOKEN_LESS) || check(parser, TOKEN_LESS_EQUAL) ||
         check(parser, TOKEN_GREATER) || check(parser, TOKEN_GREATER_EQUAL)) {
    Token *op = advance(parser);
    BinaryOp bin_op;
    switch (op->type) {
    case TOKEN_EQUAL:
      bin_op = OP_EQ;
      break;
    case TOKEN_NOT_EQUAL:
      bin_op = OP_NE;
      break;
    case TOKEN_LESS:
      bin_op = OP_LT;
      break;
    case TOKEN_LESS_EQUAL:
      bin_op = OP_LE;
      break;
    case TOKEN_GREATER:
      bin_op = OP_GT;
      break;
    case TOKEN_GREATER_EQUAL:
      bin_op = OP_GE;
      break;
    default:
      bin_op = OP_EQ;
      break;
    }
    ASTNode *right = parse_additive(parser);
    left = ast_create_binary(bin_op, left, right, op->line, op->column);
  }

  return left;
}

static ASTNode *parse_not(Parser *parser) {
  if (match(parser, TOKEN_NOT)) {
    Token *op = previous(parser);
    ASTNode *operand = parse_not(parser);
    return ast_create_unary(OP_NOT, operand, op->line, op->column);
  }
  return parse_comparison(parser);
}

static ASTNode *parse_and(Parser *parser) {
  ASTNode *left = parse_not(parser);

  while (match(parser, TOKEN_AND)) {
    Token *op = previous(parser);
    ASTNode *right = parse_not(parser);
    left = ast_create_binary(OP_AND, left, right, op->line, op->column);
  }

  return left;
}

static ASTNode *parse_or(Parser *parser) {
  ASTNode *left = parse_and(parser);

  while (match(parser, TOKEN_OR)) {
    Token *op = previous(parser);
    ASTNode *right = parse_and(parser);
    left = ast_create_binary(OP_OR, left, right, op->line, op->column);
  }

  return left;
}

/* Lambda: (params) => expr  or  () => expr  or  identifier => expr */
static ASTNode *parse_lambda(Parser *parser) {
  Token *start = previous(parser); /* The LPAREN we just matched */

  ASTNode *node = create_node(AST_LAMBDA, start->line, start->column);
  ast_param_array_init(&node->data.lambda.params);

  /* Parse parameters */
  if (!check(parser, TOKEN_RPAREN)) {
    do {
      bool is_rest = match(parser, TOKEN_ELLIPSIS);
      ASTNode *pattern = parse_primary(parser);
      if (pattern) {
        ast_param_array_push(&node->data.lambda.params, pattern, NULL, is_rest);
      }
      if (is_rest)
        break; /* Rest must be last */
    } while (match(parser, TOKEN_COMMA));
  }

  expect(parser, TOKEN_RPAREN, "Expected ')' after lambda parameters.");
  expect(parser, TOKEN_ARROW, "Expected '=>' after lambda parameters.");

  if (check(parser, TOKEN_COLON)) {
    /* Block body lambda: (args) => : body */
    node->data.lambda.body = create_node(AST_BLOCK, start->line, start->column);
    node->data.lambda.body->data.block.statements = parse_block(parser);
  } else {
    /* Single expression lambda: (args) => expr */
    node->data.lambda.body = parse_expression(parser);
  }
  return node;
}

/* Ternary: value foresee condition otherwise other_value */
static ASTNode *parse_ternary(Parser *parser, ASTNode *true_val) {
  Token *keyword = previous(parser); /* TOKEN_FORESEE */

  ASTNode *condition = parse_or(parser);

  expect(parser, TOKEN_OTHERWISE,
         "Expected 'otherwise' in ternary expression.");

  ASTNode *false_val = parse_or(parser);

  ASTNode *node = create_node(AST_TERNARY, keyword->line, keyword->column);
  node->data.ternary.condition = condition;
  node->data.ternary.true_value = true_val;
  node->data.ternary.false_value = false_val;

  return node;
}

static ASTNode *parse_expression(Parser *parser) {
  /* Check for lambda: (params) => expr */
  if (check(parser, TOKEN_LPAREN)) {
    /* Look ahead to see if this could be a lambda */
    size_t saved = parser->current;
    advance(parser); /* consume ( */

    /* Skip potential params */
    int depth = 1;
    while (depth > 0 && !at_end(parser)) {
      if (check(parser, TOKEN_LPAREN))
        depth++;
      else if (check(parser, TOKEN_RPAREN))
        depth--;
      if (depth > 0)
        advance(parser);
    }

    if (check(parser, TOKEN_RPAREN)) {
      advance(parser); /* consume ) */
      if (check(parser, TOKEN_ARROW)) {
        /* It's a lambda! Reset and parse it properly */
        parser->current = saved;
        advance(parser); /* consume ( */
        return parse_lambda(parser);
      }
    }

    /* Not a lambda, reset and parse normally */
    parser->current = saved;
  }

  ASTNode *expr = parse_or(parser);

  /* Check for ternary: expr foresee condition otherwise other */
  if (check(parser, TOKEN_FORESEE)) {
    Token *next = peek(parser, 1);
    /* Only treat as ternary if followed by expression, not block (colon) */
    if (next && next->type != TOKEN_COLON) {
      /* Actually, simpler check - if no newline between, it's ternary */
      advance(parser); /* consume foresee */
      return parse_ternary(parser, expr);
    }
  }

  return expr;
}

/* ============================================================================
 * Block Parsing
 * ============================================================================
 */

static ASTNodeArray parse_block(Parser *parser) {
  ASTNodeArray stmts;
  ast_array_init(&stmts);

  expect(parser, TOKEN_COLON, "Expected ':' to begin block.");
  skip_newlines(parser);

  /* Check if INDENT exists - if not, return empty block with error */
  if (!check(parser, TOKEN_INDENT)) {
    error(parser, "Expected indented block.");
    return stmts;
  }
  advance(parser); /* consume INDENT */

  while (!at_end(parser) && !parser_has_error(parser)) {
    skip_newlines(parser);
    if (check(parser, TOKEN_DEDENT))
      break;

    ASTNode *stmt = parse_statement(parser);
    if (stmt) {
      ast_array_push(&stmts, stmt);
    } else {
      /* If statement parsing failed, and we're not at a DEDENT, skip to avoid
       * loop */
      if (!check(parser, TOKEN_DEDENT) && !at_end(parser)) {
        advance(parser);
      } else {
        break;
      }
    }
    skip_newlines(parser);
  }

  match(parser, TOKEN_DEDENT);
  return stmts;
}

/* ============================================================================
 * Statement Parsing
 * ============================================================================
 */

static ASTNode *parse_designate(Parser *parser) {
  Token *keyword = previous(parser);
  Token *name_tok = expect(parser, TOKEN_IDENTIFIER,
                           "Expected variable name after 'designate'.");
  if (!name_tok)
    return NULL;

  expect(parser, TOKEN_ASSIGN, "Expected '=' in designation.");
  ASTNode *value = parse_expression(parser);
  match(parser, TOKEN_NEWLINE);

  char *name_str = token_lexeme(name_tok);
  ASTNode *target =
      ast_create_identifier(name_str, name_tok->line, name_tok->column);
  free(name_str);
  ASTNode *node =
      ast_create_designate(target, value, keyword->line, keyword->column);
  return node;
}

static ASTNode *parse_assignment(Parser *parser, ASTNode *lhs) {
  advance(parser); /* consume = or := */
  ASTNode *value = parse_expression(parser);
  match(parser, TOKEN_NEWLINE);

  return ast_create_assign(lhs, value, lhs->line, lhs->column);
}

static ASTNode *parse_foresee(Parser *parser) {
  Token *keyword = previous(parser);
  ASTNode *condition = parse_expression(parser);
  ASTNode *node = ast_create_foresee(condition, keyword->line, keyword->column);
  node->data.foresee.body = parse_block(parser);

  /* Parse alternates */
  while (true) {
    skip_newlines(parser);
    if (!check(parser, TOKEN_ALTERNATE))
      break;
    advance(parser);
    ASTNode *alt_cond = parse_expression(parser);
    ASTNodeArray alt_body = parse_block(parser);
    ast_alternate_array_push(&node->data.foresee.alternates, alt_cond,
                             alt_body);
  }

  /* Parse otherwise */
  skip_newlines(parser);
  if (match(parser, TOKEN_OTHERWISE)) {
    node->data.foresee.otherwise = parse_block(parser);
  }

  return node;
}

static ASTNode *parse_cycle(Parser *parser) {
  Token *keyword = previous(parser);

  if (match(parser, TOKEN_WHILE)) {
    ASTNode *condition = parse_expression(parser);
    ASTNode *node =
        ast_create_cycle_while(condition, keyword->line, keyword->column);
    node->data.cycle_while.body = parse_block(parser);
    return node;
  }

  if (match(parser, TOKEN_THROUGH)) {
    ASTNode *iterable = parse_expression(parser);
    expect(parser, TOKEN_AS, "Expected 'as' after iterable.");

    /* Pattern can be an identifier or a list for destructuring */
    ASTNode *pattern = parse_primary(parser);
    if (!pattern) {
      error(parser, "Expected variable pattern (identifier or list).");
      return NULL;
    }

    ASTNode *node = ast_create_cycle_through(iterable, pattern, keyword->line,
                                             keyword->column);
    node->data.cycle_through.body = parse_block(parser);
    return node;
  }

  if (match(parser, TOKEN_FROM)) {
    ASTNode *start = parse_expression(parser);
    expect(parser, TOKEN_TO, "Expected 'to' in range.");
    ASTNode *end = parse_expression(parser);

    ASTNode *pattern = NULL;
    if (match(parser, TOKEN_AS)) {
      pattern = parse_primary(parser);
    } else {
      pattern = ast_create_identifier("i", keyword->line, keyword->column);
    }

    ASTNode *node = ast_create_cycle_from_to(start, end, pattern, keyword->line,
                                             keyword->column);
    node->data.cycle_from_to.body = parse_block(parser);
    return node;
  }

  error(parser, "Expected 'while', 'through', or 'from' after 'cycle'.");
  return NULL;
}

static ASTNode *parse_protocol(Parser *parser) {
  Token *keyword = previous(parser);
  Token *name = expect(parser, TOKEN_IDENTIFIER, "Expected protocol name.");
  if (!name)
    return NULL;

  char *proto_name = token_lexeme(name);
  ASTNode *node;
  if (keyword->type == TOKEN_SEQUENCE) {
    node = ast_create_sequence(proto_name, keyword->line, keyword->column);
  } else {
    node = ast_create_protocol(proto_name, keyword->line, keyword->column);
  }
  free(proto_name);

  expect(parser, TOKEN_LPAREN, "Expected '(' after protocol name.");

  if (!check(parser, TOKEN_RPAREN)) {
    do {
      bool is_rest = match(parser, TOKEN_ELLIPSIS);
      ASTNode *pattern = parse_primary(parser);
      if (!pattern)
        break;

      ASTNode *default_val = NULL;

      if (!is_rest &&
          (match(parser, TOKEN_WALRUS) || match(parser, TOKEN_ASSIGN))) {
        default_val = parse_expression(parser);
      }

      ast_param_array_push(&node->data.protocol.params, pattern, default_val,
                           is_rest);
      if (is_rest)
        break; /* Rest must be last */
    } while (match(parser, TOKEN_COMMA));
  }

  expect(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
  node->data.protocol.body = parse_block(parser);

  return node;
}

static ASTNode *parse_yield(Parser *parser) {
  Token *keyword = previous(parser);
  ASTNode *value = NULL;

  if (!check(parser, TOKEN_NEWLINE) && !at_end(parser)) {
    value = parse_expression(parser);
  }

  match(parser, TOKEN_NEWLINE);
  return ast_create_yield(value, keyword->line, keyword->column);
}

static ASTNode *parse_scheme(Parser *parser) {
  Token *keyword = previous(parser);
  ASTNode *node = ast_create_scheme(keyword->line, keyword->column);
  node->data.scheme.body = parse_block(parser);

  skip_newlines(parser);
  expect(parser, TOKEN_EXECUTE, "Expected 'execute' to close the scheme.");
  match(parser, TOKEN_NEWLINE);

  return node;
}

static ASTNode *parse_preview(Parser *parser) {
  Token *keyword = previous(parser);
  ASTNode *expr = parse_expression(parser);
  match(parser, TOKEN_NEWLINE);
  return ast_create_preview(expr, keyword->line, keyword->column);
}

static ASTNode *parse_override(Parser *parser) {
  Token *keyword = previous(parser);
  Token *name = expect(parser, TOKEN_IDENTIFIER,
                       "Expected variable name after 'override'.");
  if (!name)
    return NULL;

  expect(parser, TOKEN_ASSIGN, "Expected '=' in override.");
  ASTNode *value = parse_expression(parser);
  match(parser, TOKEN_NEWLINE);

  char *var_name = token_lexeme(name);
  ASTNode *node =
      ast_create_override(var_name, value, keyword->line, keyword->column);
  free(var_name);
  return node;
}

static ASTNode *parse_absolute(Parser *parser) {
  Token *keyword = previous(parser);

  /* Capture expression as string for error messages */
  size_t start = parser->current;
  ASTNode *condition = parse_expression(parser);
  size_t end = parser->current;

  /* Build expression string */
  char expr_str[256] = "";
  size_t len = 0;
  for (size_t i = start; i < end && len < 250; i++) {
    char *lexeme = token_lexeme(&parser->tokens[i]);
    len += snprintf(expr_str + len, 256 - len, "%s ", lexeme);
    free(lexeme);
  }

  match(parser, TOKEN_NEWLINE);
  return ast_create_absolute(condition, expr_str, keyword->line,
                             keyword->column);
}

static ASTNode *parse_anomaly(Parser *parser) {
  Token *keyword = previous(parser);
  ASTNode *node = ast_create_anomaly(keyword->line, keyword->column);
  node->data.anomaly.body = parse_block(parser);
  return node;
}

/* Parse entity (class) definition */
static ASTNode *parse_entity(Parser *parser) {
  Token *keyword = previous(parser);

  Token *name =
      expect(parser, TOKEN_IDENTIFIER, "Expected entity name after 'entity'.");
  if (!name)
    return NULL;

  char *entity_name = token_lexeme(name);
  char *parent_name = NULL;

  /* Check for inheritance */
  if (match(parser, TOKEN_INHERITS)) {
    Token *parent = expect(parser, TOKEN_IDENTIFIER,
                           "Expected parent entity name after 'inherits'.");
    if (parent) {
      parent_name = token_lexeme(parent);
    }
  }

  /* Create entity node */
  ASTNode *node = create_node(AST_ENTITY, keyword->line, keyword->column);
  node->data.entity.name = entity_name;
  node->data.entity.parent = parent_name;
  ast_array_init(&node->data.entity.members);

  /* Parse body */
  node->data.entity.members = parse_block(parser);

  return node;
}

/* Parse incorporate (import) statement */
static ASTNode *parse_incorporate(Parser *parser) {
  Token *keyword = previous(parser);

  Token *path = expect(parser, TOKEN_STRING,
                       "Expected file path string after 'incorporate'.");
  if (!path)
    return NULL;

  match(parser, TOKEN_NEWLINE);

  ASTNode *node = create_node(AST_INCORPORATE, keyword->line, keyword->column);
  node->data.incorporate.path = strdup(path->value.string_value);

  return node;
}

/* Parse attempt/recover (try/catch) block */
static ASTNode *parse_attempt(Parser *parser) {
  Token *keyword = previous(parser);

  ASTNode *node = create_node(AST_ATTEMPT, keyword->line, keyword->column);
  ast_array_init(&node->data.attempt.try_body);
  ast_array_init(&node->data.attempt.recover_body);
  node->data.attempt.error_var = NULL;

  /* Parse try body */
  node->data.attempt.try_body = parse_block(parser);

  /* Check for recover block */
  skip_newlines(parser);
  if (match(parser, TOKEN_RECOVER)) {
    /* Optional error variable */
    if (match(parser, TOKEN_AS)) {
      Token *err_var = expect(parser, TOKEN_IDENTIFIER,
                              "Expected error variable name after 'as'.");
      if (err_var) {
        node->data.attempt.error_var = token_lexeme(err_var);
      }
    }

    /* Parse recover body */
    node->data.attempt.recover_body = parse_block(parser);
  }

  return node;
}

static ASTNode *parse_situation(Parser *parser) {
  Token *keyword = previous(parser);
  ASTNode *value = parse_expression(parser);
  expect(parser, TOKEN_COLON, "Expected ':' after situation value.");

  skip_newlines(parser);
  expect(parser, TOKEN_INDENT, "Expected indentation after situation.");

  ASTNode *node = create_node(AST_SITUATION, keyword->line, keyword->column);
  node->data.situation.value = value;
  ast_array_init(&node->data.situation.alignments);

  while (!check(parser, TOKEN_DEDENT) && !at_end(parser)) {
    skip_newlines(parser);
    if (check(parser, TOKEN_DEDENT))
      break;

    if (match(parser, TOKEN_ALIGNMENT)) {
      Token *align_tok = previous(parser);
      ASTNode *alignment =
          create_node(AST_ALIGNMENT, align_tok->line, align_tok->column);
      alignment->data.alignment.is_otherwise = false;
      ast_array_init(&alignment->data.alignment.values);
      ast_array_init(&alignment->data.alignment.body);

      do {
        ast_array_push(&alignment->data.alignment.values,
                       parse_expression(parser));
      } while (match(parser, TOKEN_COMMA));

      expect(parser, TOKEN_COLON, "Expected ':' after alignment values.");

      skip_newlines(parser);
      if (match(parser, TOKEN_INDENT)) {
        while (!check(parser, TOKEN_DEDENT) && !at_end(parser)) {
          ASTNode *stmt = parse_statement(parser);
          if (stmt)
            ast_array_push(&alignment->data.alignment.body, stmt);
          skip_newlines(parser);
        }
        expect(parser, TOKEN_DEDENT, "Expected dedent after alignment body.");
      } else {
        ASTNode *stmt = parse_statement(parser);
        if (stmt)
          ast_array_push(&alignment->data.alignment.body, stmt);
      }
      ast_array_push(&node->data.situation.alignments, alignment);
    } else if (match(parser, TOKEN_OTHERWISE)) {
      Token *otherwise_tok = previous(parser);
      ASTNode *alignment = create_node(AST_ALIGNMENT, otherwise_tok->line,
                                       otherwise_tok->column);
      alignment->data.alignment.is_otherwise = true;
      ast_array_init(&alignment->data.alignment.values);
      ast_array_init(&alignment->data.alignment.body);

      expect(parser, TOKEN_COLON, "Expected ':' after otherwise.");

      skip_newlines(parser);
      if (match(parser, TOKEN_INDENT)) {
        while (!check(parser, TOKEN_DEDENT) && !at_end(parser)) {
          ASTNode *stmt = parse_statement(parser);
          if (stmt)
            ast_array_push(&alignment->data.alignment.body, stmt);
          skip_newlines(parser);
        }
        expect(parser, TOKEN_DEDENT, "Expected dedent after otherwise body.");
      } else {
        ASTNode *stmt = parse_statement(parser);
        if (stmt)
          ast_array_push(&alignment->data.alignment.body, stmt);
      }
      ast_array_push(&node->data.situation.alignments, alignment);
    } else {
      break;
    }
  }

  expect(parser, TOKEN_DEDENT, "Expected dedent after situation statement.");
  return node;
}

static ASTNode *parse_statement(Parser *parser) {
  skip_newlines(parser);

  if (match(parser, TOKEN_DESIGNATE)) {
    return parse_designate(parser);
  }

  if (match(parser, TOKEN_FORESEE)) {
    return parse_foresee(parser);
  }

  if (match(parser, TOKEN_CYCLE)) {
    return parse_cycle(parser);
  }

  /* Async protocol/sequence */
  if (match(parser, TOKEN_ASYNC)) {
    if (match(parser, TOKEN_PROTOCOL) || match(parser, TOKEN_SEQUENCE)) {
      ASTNode *node = parse_protocol(parser);
      if (node) {
        node->data.protocol.is_async = true;
      }
      return node;
    }
    error(parser, "Expected 'protocol' or 'sequence' after 'async'.");
    return NULL;
  }

  if (match(parser, TOKEN_PROTOCOL) || match(parser, TOKEN_SEQUENCE)) {
    return parse_protocol(parser);
  }

  if (match(parser, TOKEN_YIELD)) {
    return parse_yield(parser);
  }

  if (match(parser, TOKEN_DELEGATE)) {
    Token *keyword = previous(parser);
    ASTNode *iterable = parse_expression(parser);
    return ast_create_delegate(iterable, keyword->line, keyword->column);
  }

  if (match(parser, TOKEN_SCHEME)) {
    return parse_scheme(parser);
  }

  if (match(parser, TOKEN_PREVIEW)) {
    return parse_preview(parser);
  }

  if (match(parser, TOKEN_OVERRIDE)) {
    return parse_override(parser);
  }

  if (match(parser, TOKEN_ABSOLUTE)) {
    return parse_absolute(parser);
  }

  if (match(parser, TOKEN_ANOMALY)) {
    return parse_anomaly(parser);
  }

  /* Entity (class) definition */
  if (match(parser, TOKEN_ENTITY)) {
    return parse_entity(parser);
  }

  /* Incorporate (import) statement */
  if (match(parser, TOKEN_INCORPORATE)) {
    return parse_incorporate(parser);
  }

  if (match(parser, TOKEN_BREAK)) {
    return ast_create_break(previous(parser)->line, previous(parser)->column);
  }

  if (match(parser, TOKEN_CONTINUE)) {
    return ast_create_continue(previous(parser)->line,
                               previous(parser)->column);
  }

  /* Attempt (try/catch) block */
  if (match(parser, TOKEN_ATTEMPT)) {
    return parse_attempt(parser);
  }

  if (match(parser, TOKEN_SITUATION)) {
    return parse_situation(parser);
  }

  /* Expression statement OR Assignment */
  skip_newlines(parser);
  if (check(parser, TOKEN_DEDENT) || check(parser, TOKEN_EOF)) {
    return NULL;
  }
  ASTNode *expr = parse_expression(parser);
  if (!expr)
    return NULL;

  if (check(parser, TOKEN_ASSIGN) || check(parser, TOKEN_WALRUS)) {
    return parse_assignment(parser, expr);
  }

  match(parser, TOKEN_NEWLINE);

  ASTNode *stmt = (ASTNode *)calloc(1, sizeof(ASTNode));
  stmt->type = AST_EXPR_STMT;
  stmt->line = expr->line;
  stmt->column = expr->column;
  stmt->data.expr_stmt.expr = expr;
  return stmt;
}

/* ============================================================================
 * Main Parse Function
 * ============================================================================
 */

ASTNode *parser_parse(Parser *parser) {
  ASTNode *program = ast_create_program(1, 1);

  skip_newlines(parser);

  while (!at_end(parser) && !parser->has_error) {
    ASTNode *stmt = parse_statement(parser);
    if (stmt) {
      ast_array_push(&program->data.program.statements, stmt);
    } else {
      if (check(parser, TOKEN_DEDENT)) {
        error(parser, "Unexpected indentation decrease (DEDENT) at top level.");
      }
      if (!at_end(parser) && !parser->has_error) {
        advance(parser);
      } else {
        break;
      }
    }
    skip_newlines(parser);

    if (parser->panic_mode) {
      /* Try to recover */
      parser->panic_mode = false;
      while (!at_end(parser) && !check(parser, TOKEN_NEWLINE) &&
             !check(parser, TOKEN_DEDENT)) {
        advance(parser);
      }
    }
  }

  return program;
}
