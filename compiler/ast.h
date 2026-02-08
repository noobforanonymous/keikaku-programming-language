/*
 * Keikaku Programming Language - AST Header
 *
 * "The structure of your intentions has been mapped."
 */

#ifndef KEIKAKU_AST_H
#define KEIKAKU_AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ============================================================================
 * AST Node Types
 * ============================================================================
 */

typedef enum {
  /* Literals */
  AST_INTEGER,
  AST_FLOAT,
  AST_STRING,
  AST_BOOL,
  AST_LIST,
  AST_DICT,

  /* Expressions */
  AST_IDENTIFIER,
  AST_BINARY_OP,
  AST_UNARY_OP,
  AST_CALL,
  AST_INDEX,
  AST_MEMBER,

  /* Statements */
  AST_DESIGNATE,
  AST_ASSIGN,
  AST_EXPR_STMT,
  AST_BLOCK,

  /* Control Flow */
  AST_FORESEE,
  AST_CYCLE_WHILE,
  AST_CYCLE_THROUGH,
  AST_CYCLE_FROM_TO,

  /* Functions */
  AST_PROTOCOL,
  AST_YIELD,
  AST_DELEGATE, /* yield from / delegate */
  AST_PARAM,
  AST_BREAK,
  AST_CONTINUE,

  /* Special Constructs */
  AST_SCHEME,
  AST_PREVIEW,
  AST_OVERRIDE,
  AST_ABSOLUTE,
  AST_ANOMALY,

  /* OOP - Classes */
  AST_ENTITY,      /* class definition */
  AST_MANIFEST,    /* new instance creation */
  AST_SELF,        /* self reference */
  AST_METHOD_CALL, /* method call on object */
  AST_ASCEND,      /* call parent protocol */

  /* Imports */
  AST_INCORPORATE, /* import statement */

  /* Error Handling */
  AST_ATTEMPT, /* try block */

  /* Functional */
  AST_LAMBDA,    /* lambda expression */
  AST_TERNARY,   /* ternary expression */
  AST_LIST_COMP, /* list comprehension */
  AST_SLICE,     /* list slicing */
  AST_SITUATION, /* situation (match) statement */
  AST_ALIGNMENT, /* Case in a situation */
  AST_SPREAD,    /* Spread operator (...) */

  /* Generator Expressions */
  AST_GEN_EXPR, /* (expr for var in iterable) */

  /* Async */
  AST_AWAIT, /* await expression */

  /* Program */
  AST_PROGRAM,

  AST_NODE_COUNT
} ASTNodeType;

/* ============================================================================
 * Binary Operators
 * ============================================================================
 */

typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_INT_DIV,
  OP_MOD,
  OP_POW,
  OP_EQ,
  OP_NE,
  OP_LT,
  OP_LE,
  OP_GT,
  OP_GE,
  OP_AND,
  OP_OR
} BinaryOp;

/* ============================================================================
 * Unary Operators
 * ============================================================================
 */

typedef enum { OP_NEG, OP_NOT } UnaryOp;

/* ============================================================================
 * AST Node Structure
 * ============================================================================
 */

typedef struct ASTNode ASTNode;

/* Array of nodes */
typedef struct {
  ASTNode **nodes;
  size_t count;
  size_t capacity;
} ASTNodeArray;

/* Key-value pair for dictionaries */
typedef struct {
  ASTNode *key;
  ASTNode *value;
} ASTKeyValue;

typedef struct {
  ASTKeyValue *pairs;
  size_t count;
  size_t capacity;
} ASTKeyValueArray;

/* Parameter */
typedef struct {
  ASTNode *pattern;
  ASTNode *default_value; /* NULL if no default */
  bool is_rest;
} ASTParam;

typedef struct {
  ASTParam *params;
  size_t count;
  size_t capacity;
} ASTParamArray;

/* Alternate branch (for foresee) */
typedef struct {
  ASTNode *condition;
  ASTNodeArray body;
} ASTAlternate;

typedef struct {
  ASTAlternate *alts;
  size_t count;
  size_t capacity;
} ASTAlternateArray;

/* AST Node */
struct ASTNode {
  ASTNodeType type;
  int line;
  int column;

  union {
    /* Literals */
    int64_t int_value;
    double float_value;
    char *string_value;
    bool bool_value;

    /* List */
    struct {
      ASTNodeArray elements;
    } list;

    /* Dict */
    struct {
      ASTKeyValueArray pairs;
    } dict;

    /* Identifier */
    struct {
      char *name;
    } identifier;

    /* Binary Op */
    struct {
      BinaryOp op;
      ASTNode *left;
      ASTNode *right;
    } binary;

    /* Unary Op */
    struct {
      UnaryOp op;
      ASTNode *operand;
    } unary;

    /* Function Call */
    struct {
      char *name;
      ASTNodeArray args;
    } call;

    /* Index Access */
    struct {
      ASTNode *object;
      ASTNode *index;
    } index;

    /* Member Access */
    struct {
      ASTNode *object;
      char *member;
    } member;

    /* Designate / Assign */
    struct {
      ASTNode *target;
      ASTNode *value;
    } assign;

    /* Expression Statement */
    struct {
      ASTNode *expr;
    } expr_stmt;

    /* Block */
    struct {
      ASTNodeArray statements;
    } block;

    /* Foresee (if/elif/else) */
    struct {
      ASTNode *condition;
      ASTNodeArray body;
      ASTAlternateArray alternates;
      ASTNodeArray otherwise;
    } foresee;

    /* Cycle While */
    struct {
      ASTNode *condition;
      ASTNodeArray body;
    } cycle_while;

    /* Cycle Through */
    struct {
      ASTNode *iterable;
      ASTNode *var_pattern;
      ASTNodeArray body;
    } cycle_through;

    /* Cycle From To */
    struct {
      ASTNode *start;
      ASTNode *end;
      ASTNode *step;
      ASTNode *var_pattern;
      ASTNodeArray body;
    } cycle_from_to;

    /* Protocol (function definition) */
    struct {
      char *name;
      ASTParamArray params;
      ASTNodeArray body;
      bool is_sequence;
      bool is_async;
    } protocol;

    /* Yield (return) */
    struct {
      ASTNode *value; /* NULL for void return */
    } yield;

    /* Delegate (yield from) */
    struct {
      ASTNode *iterable; /* Generator or list to delegate to */
    } delegate;

    /* Scheme */
    struct {
      ASTNodeArray body;
    } scheme;

    /* Preview */
    struct {
      ASTNode *expr;
    } preview;

    /* Override */
    struct {
      char *name;
      ASTNode *value;
    } override;

    /* Absolute */
    struct {
      ASTNode *condition;
      char *expr_str; /* Original expression for error messages */
    } absolute;

    /* Anomaly */
    struct {
      ASTNodeArray body;
    } anomaly;

    /* Entity (class) */
    struct {
      char *name;
      char *parent;         /* Inherits from (NULL if none) */
      ASTNodeArray members; /* Methods and properties */
    } entity;

    /* Manifest (new instance) */
    struct {
      char *class_name;
      ASTNodeArray args; /* Constructor arguments */
    } manifest;

    /* Method Call */
    struct {
      ASTNode *object; /* Object instance */
      char *method_name;
      ASTNodeArray args;
    } method_call;

    /* Ascend (super call) */
    struct {
      char *name; /* Protocol name */
      ASTNodeArray args;
    } ascend;

    /* Incorporate (import) */
    struct {
      char *path; /* File path to import */
    } incorporate;

    /* Attempt (try/catch) */
    struct {
      ASTNodeArray try_body;
      char *error_var; /* Variable name for caught error */
      ASTNodeArray recover_body;
    } attempt;

    /* Lambda Expression */
    struct {
      ASTParamArray params;
      ASTNode *body; /* Single expression */
    } lambda;

    /* Ternary Expression */
    struct {
      ASTNode *condition;
      ASTNode *true_value;
      ASTNode *false_value;
    } ternary;

    /* List Comprehension */
    struct {
      ASTNode *expr;      /* Expression to evaluate */
      ASTNode *iterable;  /* What to iterate */
      char *var_name;     /* Loop variable */
      ASTNode *condition; /* Optional filter (NULL if none) */
    } list_comp;

    /* Slice */
    struct {
      ASTNode *object; /* List/string being sliced */
      ASTNode *start;  /* Start index (NULL = 0) */
      ASTNode *end;    /* End index (NULL = len) */
      ASTNode *step;   /* Step (NULL = 1) */
    } slice;

    /* Situation (Match) */
    struct {
      ASTNode *value;
      ASTNodeArray alignments; /* Cases */
    } situation;

    /* Alignment (Case) */
    struct {
      bool is_otherwise;
      ASTNodeArray values; /* Support multiple values: alignment 1, 2: */
      ASTNodeArray body;
    } alignment;

    /* Spread */
    struct {
      ASTNode *expr;
    } spread;

    /* Generator Expression */
    struct {
      ASTNode *expr;      /* Expression to yield */
      ASTNode *iterable;  /* Source iterable */
      char *var_name;     /* Loop variable */
      ASTNode *condition; /* Optional filter (NULL if none) */
    } gen_expr;

    /* Await Expression */
    struct {
      ASTNode *expr; /* Promise/async expression to await */
    } await;

    /* Program */
    struct {
      ASTNodeArray statements;
    } program;

  } data;
};

/* ============================================================================
 * AST Functions
 * ============================================================================
 */

/* Node creation */
ASTNode *ast_create_int(int64_t value, int line, int col);
ASTNode *ast_create_float(double value, int line, int col);
ASTNode *ast_create_string(const char *value, int line, int col);
ASTNode *ast_create_bool(bool value, int line, int col);
ASTNode *ast_create_identifier(const char *name, int line, int col);
ASTNode *ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, int line,
                           int col);
ASTNode *ast_create_unary(UnaryOp op, ASTNode *operand, int line, int col);
ASTNode *ast_create_call(const char *name, int line, int col);
ASTNode *ast_create_index(ASTNode *object, ASTNode *index, int line, int col);
ASTNode *ast_create_list(int line, int col);
ASTNode *ast_create_dict(int line, int col);
ASTNode *ast_create_designate(ASTNode *target, ASTNode *value, int line,
                              int col);
ASTNode *ast_create_assign(ASTNode *target, ASTNode *value, int line, int col);
ASTNode *ast_create_foresee(ASTNode *condition, int line, int col);
ASTNode *ast_create_cycle_while(ASTNode *condition, int line, int col);
ASTNode *ast_create_cycle_through(ASTNode *iterable, ASTNode *var_pattern,
                                  int line, int col);
ASTNode *ast_create_cycle_from_to(ASTNode *start, ASTNode *end,
                                  ASTNode *var_pattern, int line, int col);
ASTNode *ast_create_protocol(const char *name, int line, int col);
ASTNode *ast_create_sequence(const char *name, int line, int col);
ASTNode *ast_create_yield(ASTNode *value, int line, int col);
ASTNode *ast_create_break(int line, int col);
ASTNode *ast_create_continue(int line, int col);
ASTNode *ast_create_delegate(ASTNode *iterable, int line, int col);
ASTNode *ast_create_scheme(int line, int col);
ASTNode *ast_create_preview(ASTNode *expr, int line, int col);
ASTNode *ast_create_override(const char *name, ASTNode *value, int line,
                             int col);
ASTNode *ast_create_absolute(ASTNode *condition, const char *expr_str, int line,
                             int col);
ASTNode *ast_create_anomaly(int line, int col);
ASTNode *ast_create_program(int line, int col);
ASTNode *ast_create_block(int line, int col);
ASTNode *ast_create_gen_expr(ASTNode *expr, ASTNode *iterable,
                             const char *var_name, ASTNode *condition, int line,
                             int col);
ASTNode *ast_create_await(ASTNode *expr, int line, int col);

/* Node destruction */
void ast_destroy(ASTNode *node);
ASTNode *ast_create_ascend(const char *name, int line, int col);

void ast_destroy_array(ASTNodeArray *arr);

/* Array operations */
void ast_array_init(ASTNodeArray *arr);
void ast_array_push(ASTNodeArray *arr, ASTNode *node);

void ast_param_array_init(ASTParamArray *arr);
void ast_param_array_push(ASTParamArray *arr, ASTNode *pattern,
                          ASTNode *default_val, bool is_rest);

void ast_alternate_array_init(ASTAlternateArray *arr);
void ast_alternate_array_push(ASTAlternateArray *arr, ASTNode *cond,
                              ASTNodeArray body);

void ast_kv_array_init(ASTKeyValueArray *arr);
void ast_kv_array_push(ASTKeyValueArray *arr, ASTNode *key, ASTNode *value);

/* Debug */
void ast_print(ASTNode *node, int indent);
const char *ast_node_type_name(ASTNodeType type);
const char *ast_binary_op_name(BinaryOp op);
const char *ast_unary_op_name(UnaryOp op);

#endif /* KEIKAKU_AST_H */
