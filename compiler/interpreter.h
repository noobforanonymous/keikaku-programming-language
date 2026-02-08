/*
 * Keikaku Programming Language - Interpreter Header
 *
 * "Your code shall execute. As intended."
 */

#ifndef KEIKAKU_INTERPRETER_H
#define KEIKAKU_INTERPRETER_H

#include "ast.h"
#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 * Value Types
 * ============================================================================
 */

typedef enum {
  VAL_NULL,
  VAL_BOOL,
  VAL_INT,
  VAL_FLOAT,
  VAL_STRING,
  VAL_LIST,
  VAL_DICT,
  VAL_FUNCTION,
  VAL_BUILTIN,
  VAL_INSTANCE,  /* Class instance */
  VAL_CLASS,     /* Class definition */
  VAL_GENERATOR, /* Generator instance */
  VAL_PROMISE    /* Promise for async operations */
} ValueType;

/* Forward declarations */
struct Value;
struct Environment;
struct ValueList;
struct ValueDict;
struct Function;

/* Builtin function - forward declare Value* signature */
typedef struct Value (*BuiltinFn)(int argc, struct Value *argv);

/* Value structure - define first */
typedef struct Value {
  ValueType type;
  union {
    bool bool_val;
    int64_t int_val;
    double float_val;
    char *string_val;
    struct ValueList *list_val;
    struct ValueDict *dict_val;
    struct Function *func_val;
    BuiltinFn builtin_val;
    struct KeikakuClass *class_val;
    struct KeikakuInstance *instance_val;
    struct Generator *gen_val;
    struct Promise *promise_val;
  } data;
} Value;

/* List structure */
typedef struct ValueList {
  Value *items;
  size_t count;
  size_t capacity;
} ValueList;

/* Dict entry */
typedef struct DictEntry {
  char *key;
  Value value;
} DictEntry;

typedef struct ValueDict {
  DictEntry *entries;
  size_t count;
  size_t capacity;
} ValueDict;

/* Function structure */
typedef struct Function {
  char *name;
  ASTNode *node; /* Protocol node */
  struct Environment *closure;
  bool is_lambda;   /* True if this is a lambda function */
  bool is_sequence; /* True if this is a sequence (generator) */
} Function;

/* Class structure */
typedef struct KeikakuClass {
  char *name;
  struct KeikakuClass *parent; /* Parent class for inheritance */
  struct Environment *methods; /* Method definitions */
  ASTNode *definition;         /* Original AST node */
} KeikakuClass;

/* Instance structure */
typedef struct KeikakuInstance {
  KeikakuClass *class_def;    /* Reference to class */
  struct Environment *fields; /* Instance fields */
} KeikakuInstance;

/* ============================================================================
 * Environment
 * ============================================================================
 */

typedef struct EnvEntry {
  char *name;
  Value value;
  bool is_override;
  struct EnvEntry *next;
} EnvEntry;

typedef struct Environment {
  EnvEntry *entries;
  struct Environment *parent;
  struct Environment *global; /* For override */
} Environment;

/* ============================================================================
 * Generators
 * ============================================================================
 */

typedef enum { GEN_STOPPED, GEN_RUNNING, GEN_SUSPENDED, GEN_DONE } GenStatus;

typedef enum {
  GEN_FRAME_BLOCK,
  GEN_FRAME_CYCLE_THROUGH,
  GEN_FRAME_CYCLE_FROM_TO,
  GEN_FRAME_CYCLE_WHILE,
  GEN_FRAME_DELEGATE
} GenFrameType;

typedef struct GenFrame {
  GenFrameType type;
  size_t index;    /* Current instruction index in block */
  Value iterable;  /* For THROUGH */
  int64_t current; /* For FROM_TO */
  int64_t end;     /* For FROM_TO */
  ASTNode *node;   /* Reference to the AST node (Block or Cycle) */
} GenFrame;

typedef struct Generator {
  Value func_val;
  struct Environment *env;
  Value self_val;
  GenStatus status;

  GenFrame *stack;
  size_t stack_count;
  size_t stack_capacity;

  /* For send/transmit - value sent to generator */
  Value sent_value;
  bool has_sent;

  /* For disrupt/exception - value thrown into generator */
  Value thrown_value;
  bool has_thrown;
} Generator;

/* ============================================================================
 * Promise (for async/await)
 * ============================================================================
 */

typedef enum {
  PROMISE_PENDING,
  PROMISE_RESOLVED,
  PROMISE_REJECTED
} PromiseState;

typedef struct Promise {
  PromiseState state;
  Value result;            /* Resolved value or rejection reason */
  Generator *continuation; /* Generator to resume when resolved */
} Promise;

/* ============================================================================
 * Interpreter Structure
 * ============================================================================
 */

typedef struct Interpreter {
  Environment *global_env;
  Environment *current_env;

  /* Return value from yield */
  Value return_value;
  bool has_return;
  bool has_break;
  bool has_continue;

  /* Error handling */
  bool has_error;
  char error_buffer[1024];

  /* Error tracking for repeated errors */
  char last_error[256];
  int error_repeat_count;

  /* Preview mode */
  bool preview_mode;

  /* Anomaly mode */
  bool anomaly_mode;

  /* Generator state */
  Generator *current_gen;
  bool is_resuming;
  GenFrame *resume_stack;
  size_t resume_count;
} Interpreter;

/* ============================================================================
 * Value Functions
 * ============================================================================
 */

Value value_null(void);
Value value_bool(bool val);
Value value_int(int64_t val);
Value value_float(double val);
Value value_string(const char *val);
Value value_list_new(void);
Value value_dict_new(void);
Value value_function(ASTNode *node, Environment *closure);
Value value_generator_new(Function *func, Environment *env, Value self_val);
Value value_builtin(BuiltinFn fn);

void value_free(Value *val);
Value value_copy(Value *val);
char *value_to_string(Value *val);
const char *value_type_name(ValueType type);
bool value_is_truthy(Value *val);
bool value_equals(Value *a, Value *b);

/* List operations */
void value_list_push(Value *list, Value item);
Value value_list_get(Value *list, int64_t index);

/* Dict operations */
void value_dict_set(Value *dict, const char *key, Value val);
Value value_dict_get(Value *dict, const char *key);

/* ============================================================================
 * Environment Functions
 * ============================================================================
 */

Environment *env_create(Environment *parent);
void env_destroy(Environment *env);
void env_define(Environment *env, const char *name, Value value);
void env_set(Environment *env, const char *name, Value value);
Value env_get(Environment *env, const char *name, bool *found);
void env_force_set(Environment *env, const char *name, Value value);

/* ============================================================================
 * Interpreter Functions
 * ============================================================================
 */

Interpreter *interpreter_create(void);
void interpreter_destroy(Interpreter *interp);
void interpreter_reset(Interpreter *interp);

/* Execution */
Value interpreter_execute(Interpreter *interp, ASTNode *ast);
Value interpreter_call(Interpreter *interp, Function *func, Value self_val,
                       int argc, Value *argv);
Value interpreter_gen_next(Interpreter *interp, Value gen_val);

/* Error handling */
bool interpreter_has_error(const Interpreter *interp);
const char *interpreter_get_error(const Interpreter *interp);

/* ============================================================================
 * Voice Messages (Personality)
 * ============================================================================
 */

void voice_print_welcome(void);
void voice_print_goodbye(void);
void voice_print_prompt(void);
void voice_print_result(Value *val);
void voice_print_scheme_registered(void);
void voice_print_scheme_executed(void);
void voice_print_preview(Value *val);
void voice_print_override(const char *name, Value *val);
void voice_print_absolute_failed(const char *expr);
void voice_print_anomaly_enter(void);
void voice_print_anomaly_exit(void);
void voice_print_error(const char *msg, int line);
void voice_print_runtime_error(const char *msg, int line);

#endif /* KEIKAKU_INTERPRETER_H */
