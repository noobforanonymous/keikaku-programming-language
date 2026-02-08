/*
 * Keikaku Programming Language - AST Implementation
 *
 * "Your syntax tree has been constructed. Every branch was anticipated."
 */

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Node Type Names
 * ============================================================================
 */

static const char *node_type_names[] = {
    "INTEGER",   "FLOAT",       "STRING",      "BOOL",          "LIST",
    "DICT",      "IDENTIFIER",  "BINARY_OP",   "UNARY_OP",      "CALL",
    "INDEX",     "MEMBER",      "DESIGNATE",   "ASSIGN",        "EXPR_STMT",
    "BLOCK",     "FORESEE",     "CYCLE_WHILE", "CYCLE_THROUGH", "CYCLE_FROM_TO",
    "PROTOCOL",  "YIELD",       "DELEGATE",    "PARAM",         "BREAK",
    "CONTINUE",  "SCHEME",      "PREVIEW",     "OVERRIDE",      "ABSOLUTE",
    "ANOMALY",   "ENTITY",      "MANIFEST",    "SELF",          "METHOD_CALL",
    "ASCEND",    "INCORPORATE", "ATTEMPT",     "LAMBDA",        "TERNARY",
    "LIST_COMP", "SLICE",       "SITUATION",   "ALIGNMENT",     "SPREAD",
    "GEN_EXPR",  "AWAIT",       "PROGRAM"};

const char *ast_node_type_name(ASTNodeType type) {
  if (type >= 0 && type < AST_NODE_COUNT) {
    return node_type_names[type];
  }
  return "UNKNOWN";
}

static const char *binary_op_names[] = {"+",  "-",  "*",  "/",   "//",
                                        "%",  "**", "==", "!=",  "<",
                                        "<=", ">",  ">=", "and", "or"};

const char *ast_binary_op_name(BinaryOp op) { return binary_op_names[op]; }

static const char *unary_op_names[] = {"-", "not"};

const char *ast_unary_op_name(UnaryOp op) { return unary_op_names[op]; }

/* ============================================================================
 * Array Operations
 * ============================================================================
 */

void ast_array_init(ASTNodeArray *arr) {
  arr->nodes = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void ast_array_push(ASTNodeArray *arr, ASTNode *node) {
  if (arr->count >= arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->nodes =
        (ASTNode **)realloc(arr->nodes, sizeof(ASTNode *) * arr->capacity);
  }
  arr->nodes[arr->count++] = node;
}

void ast_param_array_init(ASTParamArray *arr) {
  arr->params = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void ast_param_array_push(ASTParamArray *arr, ASTNode *pattern,
                          ASTNode *default_val, bool is_rest) {
  if (arr->count >= arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->params =
        (ASTParam *)realloc(arr->params, sizeof(ASTParam) * arr->capacity);
  }
  arr->params[arr->count].pattern = pattern;
  arr->params[arr->count].default_value = default_val;
  arr->params[arr->count].is_rest = is_rest;
  arr->count++;
}

void ast_alternate_array_init(ASTAlternateArray *arr) {
  arr->alts = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void ast_alternate_array_push(ASTAlternateArray *arr, ASTNode *cond,
                              ASTNodeArray body) {
  if (arr->count >= arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->alts = (ASTAlternate *)realloc(arr->alts,
                                        sizeof(ASTAlternate) * arr->capacity);
  }
  arr->alts[arr->count].condition = cond;
  arr->alts[arr->count].body = body;
  arr->count++;
}

void ast_kv_array_init(ASTKeyValueArray *arr) {
  arr->pairs = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void ast_kv_array_push(ASTKeyValueArray *arr, ASTNode *key, ASTNode *value) {
  if (arr->count >= arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->pairs =
        (ASTKeyValue *)realloc(arr->pairs, sizeof(ASTKeyValue) * arr->capacity);
  }
  arr->pairs[arr->count].key = key;
  arr->pairs[arr->count].value = value;
  arr->count++;
}

/* ============================================================================
 * Node Creation
 * ============================================================================
 */

static ASTNode *create_node(ASTNodeType type, int line, int col) {
  ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
  node->type = type;
  node->line = line;
  node->column = col;
  return node;
}

ASTNode *ast_create_int(int64_t value, int line, int col) {
  ASTNode *node = create_node(AST_INTEGER, line, col);
  node->data.int_value = value;
  return node;
}

ASTNode *ast_create_float(double value, int line, int col) {
  ASTNode *node = create_node(AST_FLOAT, line, col);
  node->data.float_value = value;
  return node;
}

ASTNode *ast_create_string(const char *value, int line, int col) {
  ASTNode *node = create_node(AST_STRING, line, col);
  node->data.string_value = strdup(value);
  return node;
}

ASTNode *ast_create_bool(bool value, int line, int col) {
  ASTNode *node = create_node(AST_BOOL, line, col);
  node->data.bool_value = value;
  return node;
}

ASTNode *ast_create_identifier(const char *name, int line, int col) {
  ASTNode *node = create_node(AST_IDENTIFIER, line, col);
  node->data.identifier.name = strdup(name);
  return node;
}

ASTNode *ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, int line,
                           int col) {
  ASTNode *node = create_node(AST_BINARY_OP, line, col);
  node->data.binary.op = op;
  node->data.binary.left = left;
  node->data.binary.right = right;
  return node;
}

ASTNode *ast_create_unary(UnaryOp op, ASTNode *operand, int line, int col) {
  ASTNode *node = create_node(AST_UNARY_OP, line, col);
  node->data.unary.op = op;
  node->data.unary.operand = operand;
  return node;
}

ASTNode *ast_create_call(const char *name, int line, int col) {
  ASTNode *node = create_node(AST_CALL, line, col);
  node->data.call.name = strdup(name);
  ast_array_init(&node->data.call.args);
  return node;
}

ASTNode *ast_create_index(ASTNode *object, ASTNode *index, int line, int col) {
  ASTNode *node = create_node(AST_INDEX, line, col);
  node->data.index.object = object;
  node->data.index.index = index;
  return node;
}

ASTNode *ast_create_list(int line, int col) {
  ASTNode *node = create_node(AST_LIST, line, col);
  ast_array_init(&node->data.list.elements);
  return node;
}

ASTNode *ast_create_dict(int line, int col) {
  ASTNode *node = create_node(AST_DICT, line, col);
  ast_kv_array_init(&node->data.dict.pairs);
  return node;
}

ASTNode *ast_create_designate(ASTNode *target, ASTNode *value, int line,
                              int col) {
  ASTNode *node = create_node(AST_DESIGNATE, line, col);
  node->data.assign.target = target;
  node->data.assign.value = value;
  return node;
}

ASTNode *ast_create_assign(ASTNode *target, ASTNode *value, int line, int col) {
  ASTNode *node = create_node(AST_ASSIGN, line, col);
  node->data.assign.target = target;
  node->data.assign.value = value;
  return node;
}

ASTNode *ast_create_foresee(ASTNode *condition, int line, int col) {
  ASTNode *node = create_node(AST_FORESEE, line, col);
  node->data.foresee.condition = condition;
  ast_array_init(&node->data.foresee.body);
  ast_alternate_array_init(&node->data.foresee.alternates);
  ast_array_init(&node->data.foresee.otherwise);
  return node;
}

ASTNode *ast_create_cycle_while(ASTNode *condition, int line, int col) {
  ASTNode *node = create_node(AST_CYCLE_WHILE, line, col);
  node->data.cycle_while.condition = condition;
  ast_array_init(&node->data.cycle_while.body);
  return node;
}

ASTNode *ast_create_cycle_through(ASTNode *iterable, ASTNode *var_pattern,
                                  int line, int col) {
  ASTNode *node = create_node(AST_CYCLE_THROUGH, line, col);
  node->data.cycle_through.iterable = iterable;
  node->data.cycle_through.var_pattern = var_pattern;
  ast_array_init(&node->data.cycle_through.body);
  return node;
}

ASTNode *ast_create_cycle_from_to(ASTNode *start, ASTNode *end,
                                  ASTNode *var_pattern, int line, int col) {
  ASTNode *node = create_node(AST_CYCLE_FROM_TO, line, col);
  node->data.cycle_from_to.start = start;
  node->data.cycle_from_to.end = end;
  node->data.cycle_from_to.step = NULL;
  node->data.cycle_from_to.var_pattern = var_pattern;
  ast_array_init(&node->data.cycle_from_to.body);
  return node;
}

ASTNode *ast_create_break(int line, int col) {
  return create_node(AST_BREAK, line, col);
}

ASTNode *ast_create_continue(int line, int col) {
  return create_node(AST_CONTINUE, line, col);
}

ASTNode *ast_create_protocol(const char *name, int line, int col) {
  ASTNode *node = create_node(AST_PROTOCOL, line, col);
  node->data.protocol.name = strdup(name);
  node->data.protocol.is_sequence = false;
  ast_param_array_init(&node->data.protocol.params);
  ast_array_init(&node->data.protocol.body);
  return node;
}

ASTNode *ast_create_sequence(const char *name, int line, int col) {
  ASTNode *node = create_node(AST_PROTOCOL, line, col);
  node->data.protocol.name = strdup(name);
  node->data.protocol.is_sequence = true;
  ast_param_array_init(&node->data.protocol.params);
  ast_array_init(&node->data.protocol.body);
  return node;
}

ASTNode *ast_create_yield(ASTNode *value, int line, int col) {
  ASTNode *node = create_node(AST_YIELD, line, col);
  node->data.yield.value = value;
  return node;
}

ASTNode *ast_create_delegate(ASTNode *iterable, int line, int col) {
  ASTNode *node = create_node(AST_DELEGATE, line, col);
  node->data.delegate.iterable = iterable;
  return node;
}

ASTNode *ast_create_scheme(int line, int col) {
  ASTNode *node = create_node(AST_SCHEME, line, col);
  ast_array_init(&node->data.scheme.body);
  return node;
}

ASTNode *ast_create_preview(ASTNode *expr, int line, int col) {
  ASTNode *node = create_node(AST_PREVIEW, line, col);
  node->data.preview.expr = expr;
  return node;
}

ASTNode *ast_create_override(const char *name, ASTNode *value, int line,
                             int col) {
  ASTNode *node = create_node(AST_OVERRIDE, line, col);
  node->data.override.name = strdup(name);
  node->data.override.value = value;
  return node;
}

ASTNode *ast_create_absolute(ASTNode *condition, const char *expr_str, int line,
                             int col) {
  ASTNode *node = create_node(AST_ABSOLUTE, line, col);
  node->data.absolute.condition = condition;
  node->data.absolute.expr_str = expr_str ? strdup(expr_str) : NULL;
  return node;
}

ASTNode *ast_create_anomaly(int line, int col) {
  ASTNode *node = create_node(AST_ANOMALY, line, col);
  ast_array_init(&node->data.anomaly.body);
  return node;
}

ASTNode *ast_create_program(int line, int col) {
  ASTNode *node = create_node(AST_PROGRAM, line, col);
  ast_array_init(&node->data.program.statements);
  return node;
}

ASTNode *ast_create_block(int line, int col) {
  ASTNode *node = create_node(AST_BLOCK, line, col);
  ast_array_init(&node->data.block.statements);
  return node;
}

ASTNode *ast_create_gen_expr(ASTNode *expr, ASTNode *iterable,
                             const char *var_name, ASTNode *condition, int line,
                             int col) {
  ASTNode *node = create_node(AST_GEN_EXPR, line, col);
  node->data.gen_expr.expr = expr;
  node->data.gen_expr.iterable = iterable;
  node->data.gen_expr.var_name = var_name ? strdup(var_name) : NULL;
  node->data.gen_expr.condition = condition;
  return node;
}

ASTNode *ast_create_await(ASTNode *expr, int line, int col) {
  ASTNode *node = create_node(AST_AWAIT, line, col);
  node->data.await.expr = expr;
  return node;
}

ASTNode *ast_create_ascend(const char *name, int line, int col) {
  ASTNode *node = create_node(AST_ASCEND, line, col);
  node->data.ascend.name = strdup(name);
  ast_array_init(&node->data.ascend.args);
  return node;
}

/* ============================================================================
 * Node Destruction
 * ============================================================================
 */

void ast_destroy_array(ASTNodeArray *arr) {
  for (size_t i = 0; i < arr->count; i++) {
    ast_destroy(arr->nodes[i]);
  }
  free(arr->nodes);
  arr->nodes = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void ast_destroy(ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case AST_STRING:
    free(node->data.string_value);
    break;

  case AST_IDENTIFIER:
    free(node->data.identifier.name);
    break;

  case AST_BINARY_OP:
    ast_destroy(node->data.binary.left);
    ast_destroy(node->data.binary.right);
    break;

  case AST_UNARY_OP:
    ast_destroy(node->data.unary.operand);
    break;

  case AST_CALL:
    free(node->data.call.name);
    ast_destroy_array(&node->data.call.args);
    break;

  case AST_INDEX:
    ast_destroy(node->data.index.object);
    ast_destroy(node->data.index.index);
    break;

  case AST_LIST:
    ast_destroy_array(&node->data.list.elements);
    break;

  case AST_DICT:
    for (size_t i = 0; i < node->data.dict.pairs.count; i++) {
      ast_destroy(node->data.dict.pairs.pairs[i].key);
      ast_destroy(node->data.dict.pairs.pairs[i].value);
    }
    free(node->data.dict.pairs.pairs);
    break;

  case AST_DESIGNATE:
  case AST_ASSIGN:
    ast_destroy(node->data.assign.target);
    ast_destroy(node->data.assign.value);
    break;

  case AST_FORESEE:
    ast_destroy(node->data.foresee.condition);
    ast_destroy_array(&node->data.foresee.body);
    for (size_t i = 0; i < node->data.foresee.alternates.count; i++) {
      ast_destroy(node->data.foresee.alternates.alts[i].condition);
      ast_destroy_array(&node->data.foresee.alternates.alts[i].body);
    }
    free(node->data.foresee.alternates.alts);
    ast_destroy_array(&node->data.foresee.otherwise);
    break;

  case AST_CYCLE_WHILE:
    ast_destroy(node->data.cycle_while.condition);
    ast_destroy_array(&node->data.cycle_while.body);
    break;

  case AST_CYCLE_THROUGH:
    ast_destroy(node->data.cycle_through.iterable);
    ast_destroy(node->data.cycle_through.var_pattern);
    ast_destroy_array(&node->data.cycle_through.body);
    break;

  case AST_CYCLE_FROM_TO:
    ast_destroy(node->data.cycle_from_to.start);
    ast_destroy(node->data.cycle_from_to.end);
    ast_destroy(node->data.cycle_from_to.step);
    ast_destroy(node->data.cycle_from_to.var_pattern);
    ast_destroy_array(&node->data.cycle_from_to.body);
    break;

  case AST_PROTOCOL:
    free(node->data.protocol.name);
    for (size_t i = 0; i < node->data.protocol.params.count; i++) {
      ast_destroy(node->data.protocol.params.params[i].pattern);
      ast_destroy(node->data.protocol.params.params[i].default_value);
    }
    free(node->data.protocol.params.params);
    ast_destroy_array(&node->data.protocol.body);
    break;

  case AST_YIELD:
    ast_destroy(node->data.yield.value);
    break;

  case AST_SCHEME:
    ast_destroy_array(&node->data.scheme.body);
    break;

  case AST_PREVIEW:
    ast_destroy(node->data.preview.expr);
    break;

  case AST_OVERRIDE:
    free(node->data.override.name);
    ast_destroy(node->data.override.value);
    break;

  case AST_ABSOLUTE:
    ast_destroy(node->data.absolute.condition);
    free(node->data.absolute.expr_str);
    break;

  case AST_ANOMALY:
    ast_destroy_array(&node->data.anomaly.body);
    break;

  case AST_ENTITY:
    if (node->data.entity.name)
      free(node->data.entity.name);
    if (node->data.entity.parent)
      free(node->data.entity.parent);
    ast_destroy_array(&node->data.entity.members);
    break;

  case AST_MANIFEST:
    if (node->data.manifest.class_name)
      free(node->data.manifest.class_name);
    ast_destroy_array(&node->data.manifest.args);
    break;

  case AST_METHOD_CALL:
    ast_destroy(node->data.method_call.object);
    if (node->data.method_call.method_name)
      free(node->data.method_call.method_name);
    ast_destroy_array(&node->data.method_call.args);
    break;

  case AST_ASCEND:
    if (node->data.ascend.name)
      free(node->data.ascend.name);
    ast_destroy_array(&node->data.ascend.args);
    break;

  case AST_INCORPORATE:
    if (node->data.incorporate.path)
      free(node->data.incorporate.path);
    break;

  case AST_ATTEMPT:
    ast_destroy_array(&node->data.attempt.try_body);
    if (node->data.attempt.error_var)
      free(node->data.attempt.error_var);
    ast_destroy_array(&node->data.attempt.recover_body);
    break;

  case AST_LAMBDA:
    for (size_t i = 0; i < node->data.lambda.params.count; i++) {
      ast_destroy(node->data.lambda.params.params[i].pattern);
      ast_destroy(node->data.lambda.params.params[i].default_value);
    }
    free(node->data.lambda.params.params);
    ast_destroy(node->data.lambda.body);
    break;

  case AST_TERNARY:
    ast_destroy(node->data.ternary.condition);
    ast_destroy(node->data.ternary.true_value);
    ast_destroy(node->data.ternary.false_value);
    break;

  case AST_LIST_COMP:
    ast_destroy(node->data.list_comp.expr);
    ast_destroy(node->data.list_comp.iterable);
    if (node->data.list_comp.var_name)
      free(node->data.list_comp.var_name);
    ast_destroy(node->data.list_comp.condition);
    break;

  case AST_SLICE:
    ast_destroy(node->data.slice.object);
    ast_destroy(node->data.slice.start);
    ast_destroy(node->data.slice.end);
    ast_destroy(node->data.slice.step);
    break;

  case AST_PROGRAM:
    ast_destroy_array(&node->data.program.statements);
    break;

  case AST_BLOCK:
    ast_destroy_array(&node->data.block.statements);
    break;

  case AST_SITUATION:
    ast_destroy(node->data.situation.value);
    ast_destroy_array(&node->data.situation.alignments);
    break;

  case AST_ALIGNMENT:
    ast_destroy_array(&node->data.alignment.values);
    ast_destroy_array(&node->data.alignment.body);
    break;

  case AST_SPREAD:
    ast_destroy(node->data.spread.expr);
    break;

  default:
    break;
  }

  free(node);
}

/* ============================================================================
 * Debug Printing
 * ============================================================================
 */

static void print_indent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");
  }
}

void ast_print(ASTNode *node, int indent) {
  if (!node) {
    print_indent(indent);
    printf("(null)\n");
    return;
  }

  print_indent(indent);
  printf("%s", ast_node_type_name(node->type));

  switch (node->type) {
  case AST_INTEGER:
    printf(": %lld\n", (long long)node->data.int_value);
    break;

  case AST_FLOAT:
    printf(": %f\n", node->data.float_value);
    break;

  case AST_STRING:
    printf(": \"%s\"\n", node->data.string_value);
    break;

  case AST_BOOL:
    printf(": %s\n", node->data.bool_value ? "true" : "false");
    break;

  case AST_IDENTIFIER:
    printf(": %s\n", node->data.identifier.name);
    break;

  case AST_BINARY_OP:
    printf(" (%s)\n", ast_binary_op_name(node->data.binary.op));
    ast_print(node->data.binary.left, indent + 1);
    ast_print(node->data.binary.right, indent + 1);
    break;

  case AST_UNARY_OP:
    printf(" (%s)\n", ast_unary_op_name(node->data.unary.op));
    ast_print(node->data.unary.operand, indent + 1);
    break;

  case AST_CALL:
    printf(": %s\n", node->data.call.name);
    for (size_t i = 0; i < node->data.call.args.count; i++) {
      ast_print(node->data.call.args.nodes[i], indent + 1);
    }
    break;

  case AST_DESIGNATE:
  case AST_ASSIGN:
    printf("\n");
    print_indent(indent + 1);
    printf("target: ");
    ast_print(node->data.assign.target, indent + 1);
    print_indent(indent + 1);
    printf("value:\n");
    ast_print(node->data.assign.value, indent + 2);
    break;

  case AST_ASCEND:
    printf(": %s\n", node->data.ascend.name);
    for (size_t i = 0; i < node->data.ascend.args.count; i++) {
      ast_print(node->data.ascend.args.nodes[i], indent + 1);
    }
    break;

  case AST_PROTOCOL:
    printf(": %s\n", node->data.protocol.name);
    print_indent(indent + 1);
    printf("params: %zu\n", node->data.protocol.params.count);
    print_indent(indent + 1);
    printf("body:\n");
    for (size_t i = 0; i < node->data.protocol.body.count; i++) {
      ast_print(node->data.protocol.body.nodes[i], indent + 2);
    }
    break;

  case AST_PROGRAM:
    printf("\n");
    for (size_t i = 0; i < node->data.program.statements.count; i++) {
      ast_print(node->data.program.statements.nodes[i], indent + 1);
    }
    break;

  case AST_SITUATION:
    printf("\n");
    ast_print(node->data.situation.value, indent + 1);
    for (size_t i = 0; i < node->data.situation.alignments.count; i++) {
      ast_print(node->data.situation.alignments.nodes[i], indent + 1);
    }
    break;

  case AST_ALIGNMENT:
    if (node->data.alignment.is_otherwise) {
      printf(" (otherwise)\n");
    } else {
      printf(" (cases)\n");
      for (size_t i = 0; i < node->data.alignment.values.count; i++) {
        ast_print(node->data.alignment.values.nodes[i], indent + 2);
      }
    }
    print_indent(indent + 1);
    printf("body:\n");
    for (size_t i = 0; i < node->data.alignment.body.count; i++) {
      ast_print(node->data.alignment.body.nodes[i], indent + 2);
    }
    break;

  case AST_SPREAD:
    printf("\n");
    ast_print(node->data.spread.expr, indent + 1);
    break;

  default:
    printf("\n");
    break;
  }
}
