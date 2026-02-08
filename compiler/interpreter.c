/*
 * Keikaku Programming Language - Interpreter Implementation
 *
 * "The scenario unfolds precisely as calculated."
 */

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INTERP_DEBUG 0
#define DEBUG_PRINT(...)                                                       \
  do {                                                                         \
    if (INTERP_DEBUG) {                                                        \
      printf("DEBUG: " __VA_ARGS__);                                           \
      fflush(stdout);                                                          \
    }                                                                          \
  } while (0)

/* ============================================================================
 * Voice - Personality Messages
 * ============================================================================
 */

void voice_print_welcome(void) {
  printf("\n");
  printf("  ╔═══════════════════════════════════════════════════════════╗\n");
  printf("  ║                K E I K A K U  v1.0.0                      ║\n");
  printf("  ║         \"Everything proceeds according to plan.\"          ║\n");
  printf("  ╚═══════════════════════════════════════════════════════════╝\n");
  printf("\n");
  printf("  The scenario begins. Your actions have been anticipated.\n");
  printf("  Type 'conclude' to exit. The system observes.\n\n");
}

void voice_print_goodbye(void) {
  printf("\n  The scenario concludes. Your participation was... adequate.\n");
  printf("  Until the next iteration.\n\n");
}

void voice_print_prompt(void) {
  printf("keikaku> ");
  fflush(stdout);
}

void voice_print_result(Value *val) {
  if (val->type == VAL_NULL)
    return;
  char *str = value_to_string(val);
  printf("  → %s\n", str);
  free(str);
}

void voice_print_scheme_registered(void) {
  printf("  ◈ Scheme registered. Awaiting execution command.\n");
}

void voice_print_scheme_executed(void) {
  printf("  ◈ Scheme executed. Outcome aligned with expectations.\n");
}

void voice_print_preview(Value *val) {
  char *str = value_to_string(val);
  printf("  ◇ Preview: %s\n", str);
  printf("    Reality remains unaltered. As intended.\n");
  free(str);
}

void voice_print_override(const char *name, Value *val) {
  char *str = value_to_string(val);
  printf("  ◆ Override applied: %s := %s\n", name, str);
  printf("    The adjustment was permitted.\n");
  free(str);
}

void voice_print_absolute_failed(const char *expr) {
  printf("  ⚠ ABSOLUTE DEVIATION: Condition failed.\n");
  printf("    Expression: %s\n", expr);
  printf("    This was... unexpected. The scenario attempts to stabilize.\n");
  printf("    Your certainty was misplaced. Noted.\n");
}

void voice_print_anomaly_enter(void) {
  printf("  ◊ Anomaly block entered. Your deviation is... acknowledged.\n");
}

void voice_print_anomaly_exit(void) {
  printf("  ◊ Anomaly concluded. Normalcy resumes—as anticipated.\n");
}

void voice_print_error(const char *msg, int line) {
  printf("  ⚠ Structural anomaly at line %d.\n", line);
  printf("    %s\n", msg);
  printf("    Your intent was... misaligned. The scenario adjusts.\n");
}

void voice_print_runtime_error(const char *msg, int line) {
  printf("  ⚠ Scenario instability at line %d.\n", line);
  printf("    %s\n", msg);
  printf("    The plan adapts. Stability will be restored.\n");
}

/* Enhanced error with tracking */
void voice_print_runtime_error_tracked(const char *msg, int line,
                                       int repeat_count) {
  if (repeat_count <= 1) {
    /* First occurrence - vague message */
    printf("  ⚠ A deviation has occurred at line %d.\n", line);
    printf("    Error: %s\n", msg);
    printf("    This outcome was... anticipated.\n");
    printf("    The scenario adjusts accordingly.\n");
  } else if (repeat_count == 2) {
    /* Second occurrence - hint at the problem */
    printf("  ⚠ The same deviation persists at line %d.\n", line);
    printf("    Your approach requires... reconsideration.\n");
    printf("    Hint: %s\n", msg);
  } else {
    /* Third+ occurrence - full reveal with Soul Society reference */
    printf("  ⚠ TERMINAL DEVIATION at line %d.\n", line);
    printf("    Error: %s\n", msg);
    printf("\n");
    printf("    │  \"You will never reach the Zenith.\"                │\n");
    printf("    │                                                     │\n");
    printf("    │  Your repeated failures have been noted.            │\n");
    printf("    │  Perhaps programming was not part of your plan.     │\n");
    printf("    └─────────────────────────────────────────────────────┘\n");
  }
}

/* ============================================================================
 * Value Functions
 * ============================================================================
 */

Value value_null(void) {
  Value v;
  v.type = VAL_NULL;
  return v;
}

Value value_bool(bool val) {
  Value v;
  v.type = VAL_BOOL;
  v.data.bool_val = val;
  return v;
}

Value value_int(int64_t val) {
  Value v;
  v.type = VAL_INT;
  v.data.int_val = val;
  return v;
}

Value value_float(double val) {
  Value v;
  v.type = VAL_FLOAT;
  v.data.float_val = val;
  return v;
}

Value value_string(const char *val) {
  Value v;
  v.type = VAL_STRING;
  v.data.string_val = strdup(val);
  return v;
}

Value value_list_new(void) {
  Value v;
  v.type = VAL_LIST;
  v.data.list_val = (ValueList *)calloc(1, sizeof(ValueList));
  return v;
}

Value value_dict_new(void) {
  Value v;
  v.type = VAL_DICT;
  v.data.dict_val = (ValueDict *)calloc(1, sizeof(ValueDict));
  return v;
}

Value value_function(ASTNode *node, Environment *closure) {
  Value v;
  v.type = VAL_FUNCTION;
  v.data.func_val = (Function *)calloc(1, sizeof(Function));
  v.data.func_val->name = strdup(node->data.protocol.name);
  v.data.func_val->node = node;
  v.data.func_val->closure = closure;
  v.data.func_val->is_lambda = false;
  v.data.func_val->is_sequence = node->data.protocol.is_sequence;
  return v;
}

Value value_generator_new(Function *func, Environment *env, Value self_val) {
  Value v;
  v.type = VAL_GENERATOR;
  v.data.gen_val = (Generator *)calloc(1, sizeof(Generator));

  Value func_v;
  func_v.type = VAL_FUNCTION;
  func_v.data.func_val = func;
  v.data.gen_val->func_val = value_copy(&func_v);

  v.data.gen_val->env = env;
  v.data.gen_val->self_val = value_copy(&self_val);
  v.data.gen_val->status = GEN_SUSPENDED;
  v.data.gen_val->stack = NULL;
  v.data.gen_val->stack_count = 0;
  v.data.gen_val->stack_capacity = 0;
  v.data.gen_val->sent_value = value_null();
  v.data.gen_val->has_sent = false;
  v.data.gen_val->thrown_value = value_null();
  v.data.gen_val->has_thrown = false;
  return v;
}

Value value_builtin(BuiltinFn fn) {
  Value v;
  v.type = VAL_BUILTIN;
  v.data.builtin_val = fn;
  return v;
}

Value value_promise_new(void) {
  Value v;
  v.type = VAL_PROMISE;
  v.data.promise_val = (Promise *)calloc(1, sizeof(Promise));
  v.data.promise_val->state = PROMISE_PENDING;
  v.data.promise_val->result = value_null();
  v.data.promise_val->continuation = NULL;
  return v;
}

Value value_promise_resolved(Value result) {
  Value v;
  v.type = VAL_PROMISE;
  v.data.promise_val = (Promise *)calloc(1, sizeof(Promise));
  v.data.promise_val->state = PROMISE_RESOLVED;
  v.data.promise_val->result = result;
  v.data.promise_val->continuation = NULL;
  return v;
}

const char *value_type_name(ValueType type) {
  switch (type) {
  case VAL_NULL:
    return "void";
  case VAL_BOOL:
    return "bool";
  case VAL_INT:
    return "int";
  case VAL_FLOAT:
    return "float";
  case VAL_STRING:
    return "string";
  case VAL_LIST:
    return "list";
  case VAL_DICT:
    return "dict";
  case VAL_FUNCTION:
    return "protocol";
  case VAL_BUILTIN:
    return "builtin";
  case VAL_INSTANCE:
    return "instance";
  case VAL_CLASS:
    return "entity";
  case VAL_GENERATOR:
    return "sequence";
  case VAL_PROMISE:
    return "promise";
  default:
    return "unknown";
  }
}

char *value_to_string(Value *val) {
  char buffer[1024];

  switch (val->type) {
  case VAL_NULL:
    return strdup("void");
  case VAL_BOOL:
    return strdup(val->data.bool_val ? "true" : "false");
  case VAL_INT:
    snprintf(buffer, sizeof(buffer), "%lld", (long long)val->data.int_val);
    return strdup(buffer);
  case VAL_FLOAT:
    snprintf(buffer, sizeof(buffer), "%g", val->data.float_val);
    return strdup(buffer);
  case VAL_STRING:
    snprintf(buffer, sizeof(buffer), "\"%s\"", val->data.string_val);
    return strdup(buffer);
  case VAL_LIST: {
    char *result = strdup("[");
    for (size_t i = 0; i < val->data.list_val->count; i++) {
      if (i > 0) {
        char *temp = result;
        result = (char *)malloc(strlen(temp) + 3);
        sprintf(result, "%s, ", temp);
        free(temp);
      }
      char *item = value_to_string(&val->data.list_val->items[i]);
      char *temp = result;
      result = (char *)malloc(strlen(temp) + strlen(item) + 1);
      sprintf(result, "%s%s", temp, item);
      free(temp);
      free(item);
    }
    char *temp = result;
    result = (char *)malloc(strlen(temp) + 2);
    sprintf(result, "%s]", temp);
    free(temp);
    return result;
  }
  case VAL_DICT:
    return strdup("{...}");
  case VAL_FUNCTION:
    snprintf(buffer, sizeof(buffer), "<protocol %s>", val->data.func_val->name);
    return strdup(buffer);
  case VAL_BUILTIN:
    return strdup("<builtin>");
  case VAL_INSTANCE:
    snprintf(buffer, sizeof(buffer), "<manifestation of %s>",
             val->data.instance_val->class_def->name);
    return strdup(buffer);
  case VAL_CLASS:
    snprintf(buffer, sizeof(buffer), "<entity %s>", val->data.class_val->name);
    return strdup(buffer);
  case VAL_GENERATOR:
    snprintf(buffer, sizeof(buffer), "<sequence %s>",
             val->data.gen_val->func_val.data.func_val->name);
    return strdup(buffer);
  default:
    return strdup("<unknown>");
  }
}

bool value_is_truthy(Value *val) {
  switch (val->type) {
  case VAL_NULL:
    return false;
  case VAL_BOOL:
    return val->data.bool_val;
  case VAL_INT:
    return val->data.int_val != 0;
  case VAL_FLOAT:
    return val->data.float_val != 0.0;
  case VAL_STRING:
    return strlen(val->data.string_val) > 0;
  case VAL_LIST:
    return val->data.list_val->count > 0;
  default:
    return true;
  }
}

void value_free(Value *val) {
  switch (val->type) {
  case VAL_STRING:
    free(val->data.string_val);
    break;
  case VAL_LIST:
    for (size_t i = 0; i < val->data.list_val->count; i++) {
      value_free(&val->data.list_val->items[i]);
    }
    free(val->data.list_val->items);
    free(val->data.list_val);
    break;
  case VAL_DICT:
    for (size_t i = 0; i < val->data.dict_val->count; i++) {
      free(val->data.dict_val->entries[i].key);
      value_free(&val->data.dict_val->entries[i].value);
    }
    free(val->data.dict_val->entries);
    free(val->data.dict_val);
    break;
  case VAL_FUNCTION:
    free(val->data.func_val->name);
    free(val->data.func_val);
    break;
  case VAL_GENERATOR: {
    Generator *gen = val->data.gen_val;
    value_free(&gen->func_val);
    value_free(&gen->self_val);
    env_destroy(gen->env);
    for (size_t i = 0; i < gen->stack_count; i++) {
      if (gen->stack[i].type == GEN_FRAME_CYCLE_THROUGH) {
        value_free(&gen->stack[i].iterable);
      }
    }
    free(gen->stack);
    free(gen);
    break;
  }
  default:
    break;
  }
  val->type = VAL_NULL;
}

Value value_copy(Value *val) {
  Value copy;
  copy.type = val->type;

  switch (val->type) {
  case VAL_STRING:
    copy.data.string_val = strdup(val->data.string_val);
    break;
  case VAL_LIST: {
    copy.data.list_val = (ValueList *)calloc(1, sizeof(ValueList));
    for (size_t i = 0; i < val->data.list_val->count; i++) {
      value_list_push(&copy, value_copy(&val->data.list_val->items[i]));
    }
    break;
  }
  case VAL_FUNCTION: {
    /* Deep copy the function struct */
    copy.data.func_val = (Function *)malloc(sizeof(Function));
    copy.data.func_val->name =
        val->data.func_val->name ? strdup(val->data.func_val->name) : NULL;
    copy.data.func_val->node =
        val->data.func_val->node; /* AST node is shared */
    copy.data.func_val->closure =
        val->data.func_val->closure; /* Closure is shared */
    copy.data.func_val->is_lambda = val->data.func_val->is_lambda;
    copy.data.func_val->is_sequence = val->data.func_val->is_sequence;
    break;
  }
  case VAL_GENERATOR: {
    Generator *src = val->data.gen_val;
    copy.data.gen_val = (Generator *)calloc(1, sizeof(Generator));
    copy.data.gen_val->func_val = value_copy(&src->func_val);
    copy.data.gen_val->env = env_create(src->env->parent); // New local env
    // Copy entries from src->env to copy->env
    for (EnvEntry *e = src->env->entries; e != NULL; e = e->next) {
      env_define(copy.data.gen_val->env, e->name, value_copy(&e->value));
    }
    copy.data.gen_val->self_val = value_copy(&src->self_val);
    copy.data.gen_val->status = src->status;
    copy.data.gen_val->stack_count = src->stack_count;
    copy.data.gen_val->stack_capacity = src->stack_count;
    if (src->stack_count > 0) {
      copy.data.gen_val->stack =
          (GenFrame *)malloc(sizeof(GenFrame) * src->stack_count);
      for (size_t i = 0; i < src->stack_count; i++) {
        copy.data.gen_val->stack[i] = src->stack[i];
        if (src->stack[i].type == GEN_FRAME_CYCLE_THROUGH) {
          copy.data.gen_val->stack[i].iterable =
              value_copy(&src->stack[i].iterable);
        }
      }
    }
    break;
  }
  default:
    copy.data = val->data;
    break;
  }

  return copy;
}

bool value_equals(Value *a, Value *b) {
  if (a->type != b->type)
    return false;

  switch (a->type) {
  case VAL_NULL:
    return true;
  case VAL_BOOL:
    return a->data.bool_val == b->data.bool_val;
  case VAL_INT:
    return a->data.int_val == b->data.int_val;
  case VAL_FLOAT:
    return a->data.float_val == b->data.float_val;
  case VAL_STRING:
    return strcmp(a->data.string_val, b->data.string_val) == 0;
  case VAL_LIST:
    if (a->data.list_val->count != b->data.list_val->count)
      return false;
    for (size_t i = 0; i < a->data.list_val->count; i++) {
      if (!value_equals(&a->data.list_val->items[i],
                        &b->data.list_val->items[i]))
        return false;
    }
    return true;
  case VAL_DICT:
    return a->data.dict_val == b->data.dict_val; // For now
  case VAL_FUNCTION:
    return a->data.func_val == b->data.func_val;
  case VAL_BUILTIN:
    return a->data.builtin_val == b->data.builtin_val;
  case VAL_CLASS:
    return a->data.class_val == b->data.class_val;
  case VAL_INSTANCE:
    return a->data.instance_val == b->data.instance_val;
  default:
    return false;
  }
}

void value_list_push(Value *list, Value item) {
  ValueList *l = list->data.list_val;
  if (l->count >= l->capacity) {
    l->capacity = l->capacity == 0 ? 4 : l->capacity * 2;
    l->items = (Value *)realloc(l->items, sizeof(Value) * l->capacity);
  }
  l->items[l->count++] = item;
}

Value value_list_get(Value *list, int64_t index) {
  ValueList *l = list->data.list_val;
  if (index < 0 || (size_t)index >= l->count) {
    return value_null();
  }
  return value_copy(&l->items[index]);
}

/* ============================================================================
 * Environment Functions
 * ============================================================================
 */

Environment *env_create(Environment *parent) {
  Environment *env = (Environment *)calloc(1, sizeof(Environment));
  env->parent = parent;
  env->global = parent ? parent->global : env;
  return env;
}

void env_destroy(Environment *env) {
  EnvEntry *entry = env->entries;
  while (entry) {
    EnvEntry *next = entry->next;
    free(entry->name);
    value_free(&entry->value);
    free(entry);
    entry = next;
  }
  free(env);
}

void env_define(Environment *env, const char *name, Value value) {
  EnvEntry *entry = (EnvEntry *)malloc(sizeof(EnvEntry));
  entry->name = strdup(name);
  entry->value = value;
  entry->is_override = false;
  entry->next = env->entries;
  env->entries = entry;
}

static EnvEntry *env_find(Environment *env, const char *name) {
  for (EnvEntry *e = env->entries; e != NULL; e = e->next) {
    if (strcmp(e->name, name) == 0) {
      return e;
    }
  }
  return NULL;
}

void env_set(Environment *env, const char *name, Value value) {
  /* Check current scope */
  EnvEntry *entry = env_find(env, name);
  if (entry) {
    value_free(&entry->value);
    entry->value = value;
    return;
  }

  /* Check parent scopes */
  if (env->parent) {
    entry = env_find(env->parent, name);
    if (entry) {
      value_free(&entry->value);
      entry->value = value;
      return;
    }
  }

  /* Define new */
  env_define(env, name, value);
}

Value env_get(Environment *env, const char *name, bool *found) {
  /* Check current scope */
  EnvEntry *entry = env_find(env, name);
  if (entry) {
    *found = true;
    return value_copy(&entry->value);
  }

  /* Check parent scopes */
  if (env->parent) {
    return env_get(env->parent, name, found);
  }

  *found = false;
  return value_null();
}

void env_force_set(Environment *env, const char *name, Value value) {
  /* Force set at global level */
  Environment *global = env->global;
  EnvEntry *entry = env_find(global, name);
  if (entry) {
    value_free(&entry->value);
    entry->value = value;
    entry->is_override = true;
  } else {
    env_define(global, name, value);
  }
}

/* ============================================================================
 * Built-in Functions
 * ============================================================================
 */

static Value builtin_declare(int argc, Value *argv) {
  printf("  ");
  for (int i = 0; i < argc; i++) {
    if (i > 0)
      printf(" ");
    if (argv[i].type == VAL_STRING) {
      printf("%s", argv[i].data.string_val);
    } else {
      char *str = value_to_string(&argv[i]);
      printf("%s", str);
      free(str);
    }
  }
  printf("\n");
  return value_null();
}

static Value builtin_inquire(int argc, Value *argv) {
  if (argc > 0 && argv[0].type == VAL_STRING) {
    printf("  %s", argv[0].data.string_val);
    fflush(stdout);
  }

  char buffer[1024];
  if (fgets(buffer, sizeof(buffer), stdin)) {
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    return value_string(buffer);
  }
  return value_string("");
}

static Value builtin_measure(int argc, Value *argv) {
  if (argc < 1)
    return value_int(0);

  switch (argv[0].type) {
  case VAL_STRING:
    return value_int(strlen(argv[0].data.string_val));
  case VAL_LIST:
    return value_int(argv[0].data.list_val->count);
  case VAL_DICT:
    return value_int(argv[0].data.dict_val->count);
  default:
    return value_int(0);
  }
}

static Value builtin_span(int argc, Value *argv) {
  int64_t start = 0, end = 0, step = 1;

  if (argc == 1 && argv[0].type == VAL_INT) {
    end = argv[0].data.int_val;
  } else if (argc == 2 && argv[0].type == VAL_INT && argv[1].type == VAL_INT) {
    start = argv[0].data.int_val;
    end = argv[1].data.int_val;
  } else if (argc == 3 && argv[0].type == VAL_INT && argv[1].type == VAL_INT &&
             argv[2].type == VAL_INT) {
    start = argv[0].data.int_val;
    end = argv[1].data.int_val;
    step = argv[2].data.int_val;
  }

  Value list = value_list_new();
  for (int64_t i = start; step > 0 ? i < end : i > end; i += step) {
    value_list_push(&list, value_int(i));
  }
  return list;
}

static Value builtin_text(int argc, Value *argv) {
  if (argc < 1)
    return value_string("");
  char *str = value_to_string(&argv[0]);
  Value result = value_string(str);
  free(str);
  return result;
}

static Value builtin_number(int argc, Value *argv) {
  if (argc < 1)
    return value_int(0);

  switch (argv[0].type) {
  case VAL_INT:
    return value_copy(&argv[0]);
  case VAL_FLOAT:
    return value_int((int64_t)argv[0].data.float_val);
  case VAL_STRING:
    return value_int(atoll(argv[0].data.string_val));
  case VAL_BOOL:
    return value_int(argv[0].data.bool_val ? 1 : 0);
  default:
    return value_int(0);
  }
}

static Value builtin_decimal(int argc, Value *argv) {
  if (argc < 1)
    return value_float(0.0);

  switch (argv[0].type) {
  case VAL_INT:
    return value_float((double)argv[0].data.int_val);
  case VAL_FLOAT:
    return value_copy(&argv[0]);
  case VAL_STRING:
    return value_float(atof(argv[0].data.string_val));
  default:
    return value_float(0.0);
  }
}

static Value builtin_boolean(int argc, Value *argv) {
  if (argc < 1)
    return value_bool(false);
  return value_bool(value_is_truthy(&argv[0]));
}

static Value builtin_classify(int argc, Value *argv) {
  if (argc < 1)
    return value_string("void");
  return value_string(value_type_name(argv[0].type));
}

/* ============================================================================
 * File I/O Built-ins
 * ============================================================================
 */

/* inscribe(filename, content) - Write to file */
static Value builtin_inscribe(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_STRING) {
    return value_bool(false);
  }

  FILE *file = fopen(argv[0].data.string_val, "w");
  if (!file) {
    printf("  ⚠ Unable to inscribe to '%s'. Path inaccessible.\n",
           argv[0].data.string_val);
    return value_bool(false);
  }

  if (argv[1].type == VAL_STRING) {
    fprintf(file, "%s", argv[1].data.string_val);
  } else {
    char *content = value_to_string(&argv[1]);
    fprintf(file, "%s", content);
    free(content);
  }
  fclose(file);

  printf("  ◈ Data inscribed to '%s'. The record is preserved.\n",
         argv[0].data.string_val);
  return value_bool(true);
}

/* decipher(filename) - Read from file */
static Value builtin_decipher(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_STRING) {
    return value_null();
  }

  FILE *file = fopen(argv[0].data.string_val, "r");
  if (!file) {
    printf("  ⚠ Unable to decipher '%s'. File does not exist.\n",
           argv[0].data.string_val);
    return value_null();
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(size + 1);
  size_t read_size = fread(buffer, 1, size, file);
  buffer[read_size] = '\0';
  fclose(file);

  Value result = value_string(buffer);
  free(buffer);
  return result;
}

/* chronicle(filename, content) - Append to file */
static Value builtin_chronicle(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_STRING) {
    return value_bool(false);
  }

  FILE *file = fopen(argv[0].data.string_val, "a");
  if (!file) {
    return value_bool(false);
  }

  if (argv[1].type == VAL_STRING) {
    fprintf(file, "%s", argv[1].data.string_val);
  } else {
    char *content = value_to_string(&argv[1]);
    fprintf(file, "%s", content);
    free(content);
  }
  fclose(file);
  return value_bool(true);
}

/* exists(filename) - Check if file exists */
static Value builtin_exists(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_STRING) {
    return value_bool(false);
  }

  FILE *file = fopen(argv[0].data.string_val, "r");
  if (file) {
    fclose(file);
    return value_bool(true);
  }
  return value_bool(false);
}

/* ============================================================================
 * Math Built-ins
 * ============================================================================
 */

static Value builtin_abs_val(int argc, Value *argv) {
  if (argc < 1)
    return value_int(0);

  if (argv[0].type == VAL_INT) {
    int64_t v = argv[0].data.int_val;
    return value_int(v < 0 ? -v : v);
  } else if (argv[0].type == VAL_FLOAT) {
    double v = argv[0].data.float_val;
    return value_float(v < 0 ? -v : v);
  }
  return value_int(0);
}

static Value builtin_sqrt_val(int argc, Value *argv) {
  if (argc < 1)
    return value_float(0);
  double v = argv[0].type == VAL_FLOAT ? argv[0].data.float_val
                                       : (double)argv[0].data.int_val;
  return value_float(sqrt(v));
}

static Value builtin_min_val(int argc, Value *argv) {
  if (argc < 2)
    return value_null();
  double a = argv[0].type == VAL_FLOAT ? argv[0].data.float_val
                                       : (double)argv[0].data.int_val;
  double b = argv[1].type == VAL_FLOAT ? argv[1].data.float_val
                                       : (double)argv[1].data.int_val;
  bool use_float = (argv[0].type == VAL_FLOAT || argv[1].type == VAL_FLOAT);
  return use_float ? value_float(a < b ? a : b)
                   : value_int((int64_t)(a < b ? a : b));
}

static Value builtin_max_val(int argc, Value *argv) {
  if (argc < 2)
    return value_null();
  double a = argv[0].type == VAL_FLOAT ? argv[0].data.float_val
                                       : (double)argv[0].data.int_val;
  double b = argv[1].type == VAL_FLOAT ? argv[1].data.float_val
                                       : (double)argv[1].data.int_val;
  bool use_float = (argv[0].type == VAL_FLOAT || argv[1].type == VAL_FLOAT);
  return use_float ? value_float(a > b ? a : b)
                   : value_int((int64_t)(a > b ? a : b));
}

static Value builtin_random_val(int argc, Value *argv) {
  static bool seeded = false;
  if (!seeded) {
    srand((unsigned int)time(NULL));
    seeded = true;
  }

  if (argc >= 2 && argv[0].type == VAL_INT && argv[1].type == VAL_INT) {
    int64_t min = argv[0].data.int_val;
    int64_t max = argv[1].data.int_val;
    return value_int(min + rand() % (max - min + 1));
  } else if (argc >= 1 && argv[0].type == VAL_INT) {
    return value_int(rand() % argv[0].data.int_val);
  }
  return value_float((double)rand() / RAND_MAX);
}

/* ============================================================================
 * String Built-ins
 * ============================================================================
 */

static Value builtin_uppercase(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_STRING)
    return value_string("");

  char *str = strdup(argv[0].data.string_val);
  for (int i = 0; str[i]; i++) {
    str[i] = toupper((unsigned char)str[i]);
  }
  Value result = value_string(str);
  free(str);
  return result;
}

static Value builtin_lowercase(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_STRING)
    return value_string("");

  char *str = strdup(argv[0].data.string_val);
  for (int i = 0; str[i]; i++) {
    str[i] = tolower((unsigned char)str[i]);
  }
  Value result = value_string(str);
  free(str);
  return result;
}

static Value builtin_split(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_STRING || argv[1].type != VAL_STRING) {
    return value_list_new();
  }

  Value list = value_list_new();
  char *str = strdup(argv[0].data.string_val);
  const char *delim = argv[1].data.string_val;

  char *token = strtok(str, delim);
  while (token != NULL) {
    value_list_push(&list, value_string(token));
    token = strtok(NULL, delim);
  }

  free(str);
  return list;
}

static Value builtin_join(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_LIST || argv[1].type != VAL_STRING) {
    return value_string("");
  }

  ValueList *list = argv[0].data.list_val;
  const char *delim = argv[1].data.string_val;

  size_t total_len = 1;
  for (size_t i = 0; i < list->count; i++) {
    if (list->items[i].type == VAL_STRING) {
      total_len += strlen(list->items[i].data.string_val) + strlen(delim);
    }
  }

  char *result = (char *)malloc(total_len + list->count * 20);
  result[0] = '\0';

  for (size_t i = 0; i < list->count; i++) {
    if (i > 0)
      strcat(result, delim);
    if (list->items[i].type == VAL_STRING) {
      strcat(result, list->items[i].data.string_val);
    } else {
      char *s = value_to_string(&list->items[i]);
      strcat(result, s);
      free(s);
    }
  }

  Value v = value_string(result);
  free(result);
  return v;
}

static Value builtin_contains(int argc, Value *argv) {
  if (argc < 2)
    return value_bool(false);

  if (argv[0].type == VAL_STRING && argv[1].type == VAL_STRING) {
    return value_bool(
        strstr(argv[0].data.string_val, argv[1].data.string_val) != NULL);
  }

  if (argv[0].type == VAL_LIST) {
    ValueList *list = argv[0].data.list_val;
    for (size_t i = 0; i < list->count; i++) {
      if (list->items[i].type == argv[1].type) {
        if (argv[1].type == VAL_INT &&
            list->items[i].data.int_val == argv[1].data.int_val) {
          return value_bool(true);
        }
        if (argv[1].type == VAL_STRING &&
            strcmp(list->items[i].data.string_val, argv[1].data.string_val) ==
                0) {
          return value_bool(true);
        }
      }
    }
  }
  return value_bool(false);
}

/* ============================================================================
 * List Built-ins
 * ============================================================================
 */

static Value builtin_push(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_LIST) {
    return value_null();
  }
  value_list_push(&argv[0], value_copy(&argv[1]));
  return value_copy(&argv[0]);
}

static Value builtin_reverse(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_LIST) {
    return value_list_new();
  }

  Value result = value_list_new();
  ValueList *list = argv[0].data.list_val;

  for (int i = (int)list->count - 1; i >= 0; i--) {
    value_list_push(&result, value_copy(&list->items[i]));
  }
  return result;
}

/* ============================================================================
 * Utility Built-ins
 * ============================================================================
 */

static Value builtin_clock(int argc, Value *argv) {
  (void)argc;
  (void)argv;
  return value_int((int64_t)time(NULL));
}

static Value builtin_terminate(int argc, Value *argv) {
  int code = 0;
  if (argc >= 1 && argv[0].type == VAL_INT) {
    code = (int)argv[0].data.int_val;
  }
  printf("  The scenario terminates. Exit code: %d\n", code);
  exit(code);
  return value_null();
}

static Value builtin_timestamp(int argc, Value *argv) {
  (void)argc;
  (void)argv;
  return value_int((int64_t)time(NULL));
}

/* Helper for higher-order functions - call a function value */
static Interpreter *g_interp = NULL; /* Temporary global for callbacks */

static Value call_lambda(Value *func_val, Value *arg) {
  if (!g_interp || !func_val || func_val->type != VAL_FUNCTION) {
    return value_null();
  }

  Value args[1] = {*arg};
  return interpreter_call(g_interp, func_val->data.func_val, value_null(), 1,
                          args);
}

static Value builtin_transform(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_LIST || argv[1].type != VAL_FUNCTION) {
    return value_null();
  }

  ValueList *list = argv[0].data.list_val;
  Value result = value_list_new();

  for (size_t i = 0; i < list->count; i++) {
    Value mapped = call_lambda(&argv[1], &list->items[i]);
    value_list_push(&result, mapped);
  }

  return result;
}

static Value builtin_select(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_LIST || argv[1].type != VAL_FUNCTION) {
    return value_null();
  }

  ValueList *list = argv[0].data.list_val;
  Value result = value_list_new();

  for (size_t i = 0; i < list->count; i++) {
    Value keep = call_lambda(&argv[1], &list->items[i]);
    if (value_is_truthy(&keep)) {
      value_list_push(&result, value_copy(&list->items[i]));
    }
    value_free(&keep);
  }

  return result;
}

static Value builtin_fold(int argc, Value *argv) {
  if (argc < 3 || argv[0].type != VAL_LIST || argv[1].type != VAL_FUNCTION) {
    return value_null();
  }

  ValueList *list = argv[0].data.list_val;
  Value acc = value_copy(&argv[2]);

  for (size_t i = 0; i < list->count; i++) {
    Value args[2] = {acc, list->items[i]};
    Value new_acc = interpreter_call(g_interp, argv[1].data.func_val,
                                     value_null(), 2, args);
    value_free(&acc);
    acc = new_acc;
  }

  return acc;
}

/* Simple JSON encoder */
static void json_encode_value(Value *val, char *buf, size_t size);

static void json_encode_value(Value *val, char *buf, size_t size) {
  size_t len = strlen(buf);
  char *p = buf + len;
  size_t remaining = size - len;

  switch (val->type) {
  case VAL_NULL:
    snprintf(p, remaining, "null");
    break;
  case VAL_INT:
    snprintf(p, remaining, "%lld", (long long)val->data.int_val);
    break;
  case VAL_FLOAT:
    snprintf(p, remaining, "%g", val->data.float_val);
    break;
  case VAL_BOOL:
    snprintf(p, remaining, "%s", val->data.bool_val ? "true" : "false");
    break;
  case VAL_STRING:
    snprintf(p, remaining, "\"%s\"", val->data.string_val);
    break;
  case VAL_LIST: {
    strncat(buf, "[", remaining - 1);
    for (size_t i = 0; i < val->data.list_val->count; i++) {
      if (i > 0)
        strncat(buf, ",", size - strlen(buf) - 1);
      json_encode_value(&val->data.list_val->items[i], buf, size);
    }
    strncat(buf, "]", size - strlen(buf) - 1);
    break;
  }
  default:
    snprintf(p, remaining, "null");
  }
}

static Value builtin_encode_json(int argc, Value *argv) {
  if (argc < 1)
    return value_null();

  char buf[4096] = {0};
  json_encode_value(&argv[0], buf, sizeof(buf));
  return value_string(buf);
}

static Value builtin_decode_json(int argc, Value *argv) {
  /* Simplified JSON decoder - only handles primitives */
  if (argc < 1 || argv[0].type != VAL_STRING)
    return value_null();

  const char *s = argv[0].data.string_val;

  /* Skip whitespace */
  while (*s == ' ' || *s == '\t' || *s == '\n')
    s++;

  if (strcmp(s, "null") == 0)
    return value_null();
  if (strcmp(s, "true") == 0)
    return value_bool(true);
  if (strcmp(s, "false") == 0)
    return value_bool(false);

  /* Try number */
  char *end;
  long long int_val = strtoll(s, &end, 10);
  if (end != s && (*end == '\0' || *end == '\n')) {
    return value_int(int_val);
  }

  double float_val = strtod(s, &end);
  if (end != s && (*end == '\0' || *end == '\n')) {
    return value_float(float_val);
  }

  /* Try string */
  if (*s == '"') {
    s++;
    const char *end_quote = strchr(s, '"');
    if (end_quote) {
      size_t len = end_quote - s;
      char *str = (char *)malloc(len + 1);
      strncpy(str, s, len);
      str[len] = '\0';
      return value_string(str);
    }
  }

  return value_null();
}

/* Generator control - proceed (next) and transmit (send) */
static Value builtin_proceed(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_GENERATOR) {
    return value_null();
  }
  return interpreter_gen_next(g_interp, argv[0]);
}

static Value builtin_transmit(int argc, Value *argv) {
  if (argc < 2 || argv[0].type != VAL_GENERATOR) {
    return value_null();
  }

  Generator *gen = argv[0].data.gen_val;
  gen->sent_value = value_copy(&argv[1]);
  gen->has_sent = true;

  Value result = interpreter_gen_next(g_interp, argv[0]);
  return result;
}

static Value builtin_receive(int argc, Value *argv) {
  (void)argc;
  (void)argv;
  /* Get the sent value from current generator context */
  if (g_interp->current_gen && g_interp->current_gen->has_sent) {
    Value result = g_interp->current_gen->sent_value;
    g_interp->current_gen->sent_value = value_null();
    g_interp->current_gen->has_sent = false;
    return result;
  }
  return value_null();
}

static Value builtin_disrupt(int argc, Value *argv) {
  /* disrupt(gen, error) - throw an exception into a generator */
  if (argc < 2 || argv[0].type != VAL_GENERATOR) {
    return value_null();
  }

  Generator *gen = argv[0].data.gen_val;
  gen->thrown_value = value_copy(&argv[1]);
  gen->has_thrown = true;

  /* Resume the generator - it will see the exception */
  Value result = interpreter_gen_next(g_interp, argv[0]);
  return result;
}

/* Async builtins */
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

static Value builtin_sleep(int argc, Value *argv) {
  if (argc < 1 || argv[0].type != VAL_INT) {
    return value_null();
  }
  int64_t ms = argv[0].data.int_val;
  SLEEP_MS(ms);
  return value_null();
}

static Value builtin_resolve(int argc, Value *argv) {
  if (argc < 1) {
    return value_promise_resolved(value_null());
  }
  return value_promise_resolved(value_copy(&argv[0]));
}

static Value builtin_defer(int argc, Value *argv) {
  /* defer(ms, protocol, ...args) - call protocol after ms delay */
  if (argc < 2 || argv[0].type != VAL_INT) {
    return value_null();
  }

  int64_t ms = argv[0].data.int_val;
  SLEEP_MS(ms);

  /* Call the protocol with remaining args */
  if (argv[1].type == VAL_FUNCTION || argv[1].type == VAL_BUILTIN) {
    int call_argc = argc - 2;
    Value *call_argv = (call_argc > 0) ? &argv[2] : NULL;

    if (argv[1].type == VAL_BUILTIN) {
      return argv[1].data.builtin_val(call_argc, call_argv);
    } else {
      return interpreter_call(g_interp, argv[1].data.func_val, value_null(),
                              call_argc, call_argv);
    }
  }

  return value_null();
}

/* ============================================================================
 * Interpreter
 * ============================================================================
 */

Interpreter *interpreter_create(void) {
  DEBUG_PRINT("interpreter_create\n");
  Interpreter *interp = (Interpreter *)calloc(1, sizeof(Interpreter));
  interp->global_env = env_create(NULL);
  interp->current_env = interp->global_env;
  interp->return_value = value_null();
  interp->has_return = false;
  interp->has_error = false;
  interp->last_error[0] = '\0';
  interp->error_repeat_count = 0;

  /* Set global interpreter for higher-order functions */
  g_interp = interp;

  /* Register builtins */
  env_define(interp->global_env, "declare", value_builtin(builtin_declare));
  env_define(interp->global_env, "announce", value_builtin(builtin_declare));
  env_define(interp->global_env, "inquire", value_builtin(builtin_inquire));
  env_define(interp->global_env, "measure", value_builtin(builtin_measure));
  env_define(interp->global_env, "span", value_builtin(builtin_span));
  env_define(interp->global_env, "text", value_builtin(builtin_text));
  env_define(interp->global_env, "number", value_builtin(builtin_number));
  env_define(interp->global_env, "decimal", value_builtin(builtin_decimal));
  env_define(interp->global_env, "boolean", value_builtin(builtin_boolean));
  env_define(interp->global_env, "classify", value_builtin(builtin_classify));

  /* File I/O */
  env_define(interp->global_env, "inscribe", value_builtin(builtin_inscribe));
  env_define(interp->global_env, "decipher", value_builtin(builtin_decipher));
  env_define(interp->global_env, "chronicle", value_builtin(builtin_chronicle));
  env_define(interp->global_env, "exists", value_builtin(builtin_exists));

  /* Math */
  env_define(interp->global_env, "abs", value_builtin(builtin_abs_val));
  env_define(interp->global_env, "sqrt", value_builtin(builtin_sqrt_val));
  env_define(interp->global_env, "min", value_builtin(builtin_min_val));
  env_define(interp->global_env, "max", value_builtin(builtin_max_val));
  env_define(interp->global_env, "random", value_builtin(builtin_random_val));

  /* String */
  env_define(interp->global_env, "uppercase", value_builtin(builtin_uppercase));
  env_define(interp->global_env, "lowercase", value_builtin(builtin_lowercase));
  env_define(interp->global_env, "split", value_builtin(builtin_split));
  env_define(interp->global_env, "join", value_builtin(builtin_join));
  env_define(interp->global_env, "contains", value_builtin(builtin_contains));

  /* List */
  env_define(interp->global_env, "push", value_builtin(builtin_push));
  env_define(interp->global_env, "reverse", value_builtin(builtin_reverse));

  /* Utility */
  env_define(interp->global_env, "clock", value_builtin(builtin_clock));
  env_define(interp->global_env, "terminate", value_builtin(builtin_terminate));

  /* Higher-order functions - transform (map), select (filter), fold (reduce) */
  env_define(interp->global_env, "transform", value_builtin(builtin_transform));
  env_define(interp->global_env, "select", value_builtin(builtin_select));
  env_define(interp->global_env, "fold", value_builtin(builtin_fold));

  /* JSON */
  env_define(interp->global_env, "encode_json",
             value_builtin(builtin_encode_json));
  env_define(interp->global_env, "decode_json",
             value_builtin(builtin_decode_json));

  /* Timestamp */
  env_define(interp->global_env, "timestamp", value_builtin(builtin_timestamp));

  /* Generator control */
  env_define(interp->global_env, "proceed", value_builtin(builtin_proceed));
  env_define(interp->global_env, "transmit", value_builtin(builtin_transmit));
  env_define(interp->global_env, "receive", value_builtin(builtin_receive));
  env_define(interp->global_env, "disrupt", value_builtin(builtin_disrupt));

  /* Async */
  env_define(interp->global_env, "sleep", value_builtin(builtin_sleep));
  env_define(interp->global_env, "resolve", value_builtin(builtin_resolve));
  env_define(interp->global_env, "defer", value_builtin(builtin_defer));

  return interp;
}

void interpreter_destroy(Interpreter *interp) {
  if (interp) {
    env_destroy(interp->global_env);
    free(interp);
  }
}

static void runtime_error(Interpreter *interp, const char *msg, int line) {
  interp->has_error = true;
  snprintf(interp->error_buffer, sizeof(interp->error_buffer), "%s", msg);

  /* Track repeated errors */
  if (strcmp(interp->last_error, msg) == 0) {
    interp->error_repeat_count++;
  } else {
    strncpy(interp->last_error, msg, sizeof(interp->last_error) - 1);
    interp->last_error[sizeof(interp->last_error) - 1] = '\0';
    interp->error_repeat_count = 1;
  }

  voice_print_runtime_error_tracked(msg, line, interp->error_repeat_count);
}

bool interpreter_has_error(const Interpreter *interp) {
  return interp->has_error;
}

const char *interpreter_get_error(const Interpreter *interp) {
  return interp->error_buffer;
}

/* ============================================================================
 * Expression Evaluation
 * ============================================================================
 */

static Value eval_expr(Interpreter *interp, ASTNode *node);
static Value eval_stmt(Interpreter *interp, ASTNode *node);
static void exec_block(Interpreter *interp, ASTNodeArray *stmts);

static void gen_push_frame(Generator *gen, GenFrame frame) {
  DEBUG_PRINT("gen_push_frame: gen=%p (%s), type=%d, count=%zu\n", (void *)gen,
              gen->func_val.data.func_val->name, frame.type, gen->stack_count);
  if (gen->stack_count >= gen->stack_capacity) {
    gen->stack_capacity =
        gen->stack_capacity == 0 ? 4 : gen->stack_capacity * 2;
    gen->stack =
        (GenFrame *)realloc(gen->stack, sizeof(GenFrame) * gen->stack_capacity);
  }
  gen->stack[gen->stack_count++] = frame;
}

static Value eval_expr(Interpreter *interp, ASTNode *node);
static Value eval_stmt(Interpreter *interp, ASTNode *node);
static void exec_block(Interpreter *interp, ASTNodeArray *stmts);

static Value eval_binary(Interpreter *interp, ASTNode *node) {
  Value left = eval_expr(interp, node->data.binary.left);

  /* Short-circuit for and/or */
  if (node->data.binary.op == OP_AND) {
    if (!value_is_truthy(&left))
      return value_bool(false);
    Value right = eval_expr(interp, node->data.binary.right);
    return value_bool(value_is_truthy(&right));
  }
  if (node->data.binary.op == OP_OR) {
    if (value_is_truthy(&left))
      return value_bool(true);
    Value right = eval_expr(interp, node->data.binary.right);
    return value_bool(value_is_truthy(&right));
  }

  Value right = eval_expr(interp, node->data.binary.right);

  /* String concatenation */
  if (node->data.binary.op == OP_ADD &&
      (left.type == VAL_STRING || right.type == VAL_STRING)) {
    char *left_str = value_to_string(&left);
    char *right_str = value_to_string(&right);

    /* Remove quotes if present */
    if (left.type == VAL_STRING) {
      memmove(left_str, left_str + 1, strlen(left_str) - 2);
      left_str[strlen(left_str) - 2] = '\0';
    }
    if (right.type == VAL_STRING) {
      memmove(right_str, right_str + 1, strlen(right_str) - 2);
      right_str[strlen(right_str) - 2] = '\0';
    }

    char *result = (char *)malloc(strlen(left_str) + strlen(right_str) + 1);
    sprintf(result, "%s%s", left_str, right_str);

    Value v = value_string(result);
    free(left_str);
    free(right_str);
    free(result);
    value_free(&left);
    value_free(&right);
    return v;
  }

  /* String multiplication */
  if (node->data.binary.op == OP_MUL && left.type == VAL_STRING &&
      right.type == VAL_INT) {
    int64_t times = right.data.int_val;
    size_t len = strlen(left.data.string_val);
    char *result = (char *)malloc(len * times + 1);
    result[0] = '\0';
    for (int64_t i = 0; i < times; i++) {
      strcat(result, left.data.string_val);
    }
    Value v = value_string(result);
    free(result);
    value_free(&left);
    value_free(&right);
    return v;
  }

  /* Numeric operations */
  bool use_float = (left.type == VAL_FLOAT || right.type == VAL_FLOAT);
  double a =
      left.type == VAL_FLOAT ? left.data.float_val : (double)left.data.int_val;
  double b = right.type == VAL_FLOAT ? right.data.float_val
                                     : (double)right.data.int_val;

  value_free(&left);
  value_free(&right);

  switch (node->data.binary.op) {
  case OP_ADD:
    return use_float ? value_float(a + b) : value_int((int64_t)(a + b));
  case OP_SUB:
    return use_float ? value_float(a - b) : value_int((int64_t)(a - b));
  case OP_MUL:
    return use_float ? value_float(a * b) : value_int((int64_t)(a * b));
  case OP_DIV:
    if (b == 0) {
      runtime_error(interp, "Division by zero. Even infinity has its limits.",
                    node->line);
      return value_null();
    }
    return value_float(a / b);
  case OP_INT_DIV:
    if (b == 0) {
      runtime_error(interp, "Division by zero. Even infinity has its limits.",
                    node->line);
      return value_null();
    }
    return value_int((int64_t)(a / b));
  case OP_MOD:
    return value_int((int64_t)a % (int64_t)b);
  case OP_POW:
    return value_float(pow(a, b));
  case OP_EQ:
    return value_bool(a == b);
  case OP_NE:
    return value_bool(a != b);
  case OP_LT:
    return value_bool(a < b);
  case OP_LE:
    return value_bool(a <= b);
  case OP_GT:
    return value_bool(a > b);
  case OP_GE:
    return value_bool(a >= b);
  default:
    return value_null();
  }
}

static Value eval_call(Interpreter *interp, ASTNode *node) {
  bool found;
  Value func = env_get(interp->current_env, node->data.call.name, &found);

  if (!found) {
    char msg[256];
    snprintf(msg, sizeof(msg),
             "'%s' is unknown. Perhaps you intended to define it first.",
             node->data.call.name);
    runtime_error(interp, msg, node->line);
    return value_null();
  }

  /* Evaluate arguments */
  size_t capacity = node->data.call.args.count;
  int argc = 0;
  Value *argv = (Value *)malloc(sizeof(Value) * (capacity > 0 ? capacity : 1));

  for (size_t i = 0; i < node->data.call.args.count; i++) {
    ASTNode *arg_node = node->data.call.args.nodes[i];
    if (arg_node->type == AST_SPREAD) {
      Value spread_val = eval_expr(interp, arg_node->data.spread.expr);
      if (spread_val.type == VAL_LIST) {
        size_t spread_cnt = spread_val.data.list_val->count;
        argv = (Value *)realloc(argv, sizeof(Value) * (argc + spread_cnt));
        for (size_t j = 0; j < spread_cnt; j++) {
          argv[argc++] = value_copy(&spread_val.data.list_val->items[j]);
        }
      }
      value_free(&spread_val);
    } else {
      argv[argc++] = eval_expr(interp, arg_node);
    }
  }

  Value result;

  if (func.type == VAL_BUILTIN) {
    result = func.data.builtin_val(argc, argv);
  } else if (func.type == VAL_FUNCTION) {
    result =
        interpreter_call(interp, func.data.func_val, value_null(), argc, argv);
  } else {
    char msg[256];
    snprintf(msg, sizeof(msg), "'%s' is not callable.", node->data.call.name);
    runtime_error(interp, msg, node->line);
    result = value_null();
  }

  for (int i = 0; i < argc; i++) {
    value_free(&argv[i]);
  }
  free(argv);
  value_free(&func);

  return result;
}

static Value eval_expr(Interpreter *interp, ASTNode *node) {
  if (!node)
    return value_null();

  switch (node->type) {
  case AST_INTEGER:
    return value_int(node->data.int_value);

  case AST_FLOAT:
    return value_float(node->data.float_value);

  case AST_STRING:
    return value_string(node->data.string_value);

  case AST_BOOL:
    return value_bool(node->data.bool_value);

  case AST_IDENTIFIER: {
    bool found;
    Value val =
        env_get(interp->current_env, node->data.identifier.name, &found);
    if (!found) {
      char msg[256];
      snprintf(msg, sizeof(msg),
               "'%s' is unknown. Perhaps you intended to designate it first.",
               node->data.identifier.name);
      runtime_error(interp, msg, node->line);
      return value_null();
    }
    return val;
  }

  case AST_BINARY_OP:
    return eval_binary(interp, node);

  case AST_UNARY_OP: {
    Value operand = eval_expr(interp, node->data.unary.operand);
    if (node->data.unary.op == OP_NEG) {
      if (operand.type == VAL_INT) {
        return value_int(-operand.data.int_val);
      } else if (operand.type == VAL_FLOAT) {
        return value_float(-operand.data.float_val);
      }
    } else if (node->data.unary.op == OP_NOT) {
      return value_bool(!value_is_truthy(&operand));
    }
    value_free(&operand);
    return value_null();
  }

  case AST_CALL:
    return eval_call(interp, node);

  case AST_LIST: {
    Value list = value_list_new();
    for (size_t i = 0; i < node->data.list.elements.count; i++) {
      ASTNode *elem_node = node->data.list.elements.nodes[i];
      if (elem_node->type == AST_SPREAD) {
        Value spread_val = eval_expr(interp, elem_node->data.spread.expr);
        if (spread_val.type == VAL_LIST) {
          for (size_t j = 0; j < spread_val.data.list_val->count; j++) {
            value_list_push(&list,
                            value_copy(&spread_val.data.list_val->items[j]));
          }
        }
        value_free(&spread_val);
      } else {
        Value elem = eval_expr(interp, elem_node);
        value_list_push(&list, elem);
      }
    }
    return list;
  }

  case AST_LIST_COMP: {
    Value iterable = eval_expr(interp, node->data.list_comp.iterable);
    if (iterable.type != VAL_LIST) {
      runtime_error(interp, "Iteration target must be a list.", node->line);
      value_free(&iterable);
      return value_null();
    }

    Value result = value_list_new();
    ValueList *input = iterable.data.list_val;

    for (size_t i = 0; i < input->count; i++) {
      Environment *item_env = env_create(interp->current_env);
      Environment *old_env = interp->current_env;
      interp->current_env = item_env;

      env_define(item_env, node->data.list_comp.var_name,
                 value_copy(&input->items[i]));

      bool include = true;
      if (node->data.list_comp.condition) {
        Value cond = eval_expr(interp, node->data.list_comp.condition);
        include = value_is_truthy(&cond);
        value_free(&cond);
      }

      if (include) {
        Value val = eval_expr(interp, node->data.list_comp.expr);
        value_list_push(&result, val);
      }

      interp->current_env = old_env;
      env_destroy(item_env);
    }

    value_free(&iterable);
    return result;
  }

  case AST_GEN_EXPR: {
    /* Generator expression creates an anonymous generator
     * For now, we implement it as a lazy wrapper around the iterable
     * The generator yields each item that passes the filter, transformed
     */
    Value iterable = eval_expr(interp, node->data.gen_expr.iterable);

    if (iterable.type == VAL_LIST) {
      /* Convert list to a generator that yields transformed values */
      /* For simplicity, we create a list of all values and return as
       * generator-like */
      Value result = value_list_new();
      ValueList *input = iterable.data.list_val;

      for (size_t i = 0; i < input->count; i++) {
        Environment *item_env = env_create(interp->current_env);
        Environment *old_env = interp->current_env;
        interp->current_env = item_env;

        env_define(item_env, node->data.gen_expr.var_name,
                   value_copy(&input->items[i]));

        bool include = true;
        if (node->data.gen_expr.condition) {
          Value cond = eval_expr(interp, node->data.gen_expr.condition);
          include = value_is_truthy(&cond);
          value_free(&cond);
        }

        if (include) {
          Value val = eval_expr(interp, node->data.gen_expr.expr);
          value_list_push(&result, val);
        }

        interp->current_env = old_env;
        env_destroy(item_env);
      }

      value_free(&iterable);
      /* Return as a generator-like iterable */
      return result;
    } else if (iterable.type == VAL_GENERATOR) {
      /* Pull from generator and transform */
      Value result = value_list_new();

      while (true) {
        Value next_val = interpreter_gen_next(interp, iterable);
        if (next_val.type == VAL_NULL &&
            iterable.data.gen_val->status == GEN_DONE) {
          value_free(&next_val);
          break;
        }

        Environment *item_env = env_create(interp->current_env);
        Environment *old_env = interp->current_env;
        interp->current_env = item_env;

        env_define(item_env, node->data.gen_expr.var_name, next_val);

        bool include = true;
        if (node->data.gen_expr.condition) {
          Value cond = eval_expr(interp, node->data.gen_expr.condition);
          include = value_is_truthy(&cond);
          value_free(&cond);
        }

        if (include) {
          Value val = eval_expr(interp, node->data.gen_expr.expr);
          value_list_push(&result, val);
        }

        interp->current_env = old_env;
        env_destroy(item_env);
      }

      value_free(&iterable);
      return result;
    }

    runtime_error(interp, "Generator expression requires an iterable.",
                  node->line);
    value_free(&iterable);
    return value_null();
  }

  case AST_INDEX: {
    Value obj = eval_expr(interp, node->data.index.object);
    Value idx = eval_expr(interp, node->data.index.index);

    if (obj.type == VAL_LIST && idx.type == VAL_INT) {
      Value result = value_list_get(&obj, idx.data.int_val);
      value_free(&obj);
      value_free(&idx);
      return result;
    }

    value_free(&obj);
    value_free(&idx);
    return value_null();
  }

  case AST_MEMBER: {
    Value obj = eval_expr(interp, node->data.member.object);
    if (obj.type == VAL_INSTANCE) {
      KeikakuInstance *inst = obj.data.instance_val;

      /* Private Member check */
      if (node->data.member.member[0] == '_') {
        bool found_self = false;
        Value self_val = env_get(interp->current_env, "self", &found_self);
        if (!found_self || self_val.data.instance_val != inst) {
          runtime_error(interp, "Access to private member inhibited.",
                        node->line);
          value_free(&obj);
          return value_null();
        }
      }

      bool found = false;
      Value val = env_get(inst->fields, node->data.member.member, &found);
      if (found) {
        Value res = value_copy(&val);
        value_free(&obj);
        return res;
      }

      /* Look in class methods */
      Value method =
          env_get(inst->class_def->methods, node->data.member.member, &found);
      if (found) {
        Value res = value_copy(&method);
        value_free(&obj);
        return res;
      }

      char msg[256];
      snprintf(msg, sizeof(msg), "Member '%s' not found on instance of '%s'.",
               node->data.member.member, inst->class_def->name);
      runtime_error(interp, msg, node->line);
    } else {
      runtime_error(interp, "Only instances have members.", node->line);
    }
    value_free(&obj);
    return value_null();
  }

  case AST_METHOD_CALL: {
    Value obj = eval_expr(interp, node->data.method_call.object);
    /* TODO: Support other types (string methods etc) */

    if (obj.type != VAL_INSTANCE) {
      char msg[256];
      snprintf(msg, sizeof(msg),
               "Method calls only supported on class instances.");
      runtime_error(interp, msg, node->line);
      value_free(&obj);
      return value_null();
    }

    KeikakuInstance *inst = obj.data.instance_val;
    KeikakuClass *cls = inst->class_def;

    bool found = false;
    Value method =
        env_get(cls->methods, node->data.method_call.method_name, &found);

    if (!found || method.type != VAL_FUNCTION) {
      char msg[256];
      snprintf(msg, sizeof(msg), "Method '%s' not found.",
               node->data.method_call.method_name);
      runtime_error(interp, msg, node->line);
      value_free(&obj);
      return value_null();
    }

    /* Evaluate args */
    size_t capacity = node->data.method_call.args.count;
    int argc = 0;
    Value *argv =
        (Value *)malloc(sizeof(Value) * (capacity > 0 ? capacity : 1));

    for (size_t i = 0; i < node->data.method_call.args.count; i++) {
      ASTNode *arg_node = node->data.method_call.args.nodes[i];
      if (arg_node->type == AST_SPREAD) {
        Value spread_val = eval_expr(interp, arg_node->data.spread.expr);
        if (spread_val.type == VAL_LIST) {
          size_t spread_cnt = spread_val.data.list_val->count;
          argv = (Value *)realloc(argv, sizeof(Value) * (argc + spread_cnt));
          for (size_t j = 0; j < spread_cnt; j++) {
            argv[argc++] = value_copy(&spread_val.data.list_val->items[j]);
          }
        }
        value_free(&spread_val);
      } else {
        argv[argc++] = eval_expr(interp, arg_node);
      }
    }

    Value result =
        interpreter_call(interp, method.data.func_val, obj, argc, argv);

    for (int i = 0; i < argc; i++) {
      value_free(&argv[i]);
    }
    free(argv);
    value_free(&obj);

    return result;
  }

  case AST_ASCEND: {
    /* 1. Get current 'self' */
    bool found_self = false;
    Value self = env_get(interp->current_env, "self", &found_self);
    if (!found_self || self.type != VAL_INSTANCE) {
      runtime_error(interp,
                    "'ascend' can only be used inside an instance protocol.",
                    node->line);
      return value_null();
    }

    KeikakuInstance *inst = self.data.instance_val;
    KeikakuClass *cls = inst->class_def;

    /* 2. Find parent class */
    if (!cls->parent) {
      runtime_error(interp, "This entity does not ascend to any parent.",
                    node->line);
      return value_null();
    }

    KeikakuClass *parent_cls = cls->parent;

    /* 3. Find protocol in parent */
    bool found_method = false;
    Value method =
        env_get(parent_cls->methods, node->data.ascend.name, &found_method);

    if (!found_method || method.type != VAL_FUNCTION) {
      char msg[256];
      snprintf(msg, sizeof(msg), "Parent protocol '%s' not found.",
               node->data.ascend.name);
      runtime_error(interp, msg, node->line);
      return value_null();
    }

    /* 4. Evaluate args */
    size_t capacity = node->data.ascend.args.count;
    int argc = 0;
    Value *argv =
        (Value *)malloc(sizeof(Value) * (capacity > 0 ? capacity : 1));

    for (size_t i = 0; i < node->data.ascend.args.count; i++) {
      ASTNode *arg_node = node->data.ascend.args.nodes[i];
      if (arg_node->type == AST_SPREAD) {
        Value spread_val = eval_expr(interp, arg_node->data.spread.expr);
        if (spread_val.type == VAL_LIST) {
          size_t spread_cnt = spread_val.data.list_val->count;
          argv = (Value *)realloc(argv, sizeof(Value) * (argc + spread_cnt));
          for (size_t j = 0; j < spread_cnt; j++) {
            argv[argc++] = value_copy(&spread_val.data.list_val->items[j]);
          }
        }
        value_free(&spread_val);
      } else {
        argv[argc++] = eval_expr(interp, arg_node);
      }
    }

    /* 5. Call parent method with current 'self' */
    Value result =
        interpreter_call(interp, method.data.func_val, self, argc, argv);

    /* Cleanup */
    for (int i = 0; i < argc; i++) {
      value_free(&argv[i]);
    }
    free(argv);

    return result;
  }

  case AST_TERNARY: {
    Value cond = eval_expr(interp, node->data.ternary.condition);
    bool is_true = value_is_truthy(&cond);
    value_free(&cond);

    if (is_true) {
      return eval_expr(interp, node->data.ternary.true_value);
    } else {
      return eval_expr(interp, node->data.ternary.false_value);
    }
  }

  case AST_LAMBDA: {
    /* Create a function value from the lambda */
    Function *fn = (Function *)calloc(1, sizeof(Function));
    fn->node = node;
    fn->closure = interp->current_env;
    fn->is_lambda = true;

    Value val;
    val.type = VAL_FUNCTION;
    val.data.func_val = fn;
    return val;
  }

  case AST_AWAIT: {
    /* Await expression - evaluate the expression and wait for result */
    Value awaited = eval_expr(interp, node->data.await.expr);

    /* If it's a promise, wait for it to resolve */
    if (awaited.type == VAL_PROMISE) {
      Promise *promise = awaited.data.promise_val;
      if (promise->state == PROMISE_RESOLVED) {
        Value result = value_copy(&promise->result);
        value_free(&awaited);
        return result;
      } else if (promise->state == PROMISE_REJECTED) {
        runtime_error(interp, "Promise rejected", node->line);
        value_free(&awaited);
        return value_null();
      }
      /* PROMISE_PENDING - would need event loop to handle */
      /* For now, return the promise */
      return awaited;
    }

    /* If it's a generator (async function), get next value */
    if (awaited.type == VAL_GENERATOR) {
      Value result = interpreter_gen_next(interp, awaited);
      value_free(&awaited);
      return result;
    }

    /* For non-promise/non-generator values, just return them */
    return awaited;
  }

  case AST_SLICE: {
    Value obj = eval_expr(interp, node->data.slice.object);

    if (obj.type != VAL_LIST && obj.type != VAL_STRING) {
      runtime_error(interp, "Slice requires list or string", node->line);
      value_free(&obj);
      return value_null();
    }

    /* Get list/string length */
    int64_t len = 0;
    if (obj.type == VAL_LIST) {
      len = obj.data.list_val->count;
    } else {
      len = strlen(obj.data.string_val);
    }

    /* Evaluate slice bounds */
    int64_t start = 0, end = len, step = 1;

    if (node->data.slice.start) {
      Value v = eval_expr(interp, node->data.slice.start);
      if (v.type == VAL_INT)
        start = v.data.int_val;
      value_free(&v);
    }

    if (node->data.slice.end) {
      Value v = eval_expr(interp, node->data.slice.end);
      if (v.type == VAL_INT)
        end = v.data.int_val;
      value_free(&v);
    }

    if (node->data.slice.step) {
      Value v = eval_expr(interp, node->data.slice.step);
      if (v.type == VAL_INT)
        step = v.data.int_val;
      value_free(&v);
    }

    /* Handle negative indices */
    if (start < 0)
      start = len + start;
    if (end < 0)
      end = len + end;

    /* Clamp to bounds */
    if (start < 0)
      start = 0;
    if (end > len)
      end = len;
    if (start > len)
      start = len;

    if (step == 0) {
      runtime_error(interp, "Slice step cannot be zero", node->line);
      value_free(&obj);
      return value_null();
    }

    /* Perform slice */
    if (obj.type == VAL_LIST) {
      Value result;
      result.type = VAL_LIST;
      result.data.list_val = (ValueList *)calloc(1, sizeof(ValueList));
      ValueList *res_list = result.data.list_val;

      if (step > 0) {
        for (int64_t i = start; i < end; i += step) {
          /* Inline push */
          if (res_list->count >= res_list->capacity) {
            res_list->capacity =
                res_list->capacity == 0 ? 4 : res_list->capacity * 2;
            res_list->items = (Value *)realloc(
                res_list->items, sizeof(Value) * res_list->capacity);
          }
          res_list->items[res_list->count++] =
              value_copy(&obj.data.list_val->items[i]);
        }
      } else {
        for (int64_t i = (end < start ? start - 1 : start); i > end;
             i += step) {
          if (i >= 0 && i < len) {
            if (res_list->count >= res_list->capacity) {
              res_list->capacity =
                  res_list->capacity == 0 ? 4 : res_list->capacity * 2;
              res_list->items = (Value *)realloc(
                  res_list->items, sizeof(Value) * res_list->capacity);
            }
            res_list->items[res_list->count++] =
                value_copy(&obj.data.list_val->items[i]);
          }
        }
      }

      value_free(&obj);
      return result;
    } else {
      /* String slice */
      size_t result_len = 0;
      for (int64_t i = start; i < end && i < len; i += step) {
        result_len++;
      }

      char *result = (char *)malloc(result_len + 1);
      size_t idx = 0;
      for (int64_t i = start; i < end && i < len; i += step) {
        result[idx++] = obj.data.string_val[i];
      }
      result[idx] = '\0';

      value_free(&obj);
      return value_string(result);
    }
  }

  case AST_MANIFEST: {
    /* Create a new instance of a class */
    const char *class_name = node->data.manifest.class_name;

    /* Look up the class */
    bool found = false;
    Value class_val = env_get(interp->global_env, class_name, &found);

    if (!found || class_val.type != VAL_CLASS) {
      char err[256];
      snprintf(err, sizeof(err), "Entity '%s' is not defined", class_name);
      runtime_error(interp, err, node->line);
      return value_null();
    }

    KeikakuClass *cls = class_val.data.class_val;

    /* Create instance */
    KeikakuInstance *instance =
        (KeikakuInstance *)calloc(1, sizeof(KeikakuInstance));
    instance->class_def = cls;
    instance->fields = env_create(NULL);

    /* Call constructor if exists (method named 'construct') */
    found = false;
    Value construct = env_get(cls->methods, "construct", &found);
    if (found && construct.type == VAL_FUNCTION) {
      /* Evaluate arguments */
      /* Evaluate arguments */
      size_t capacity = node->data.manifest.args.count;
      int argc = 0;
      Value *argv =
          (Value *)malloc(sizeof(Value) * (capacity > 0 ? capacity : 1));

      for (size_t i = 0; i < node->data.manifest.args.count; i++) {
        ASTNode *arg_node = node->data.manifest.args.nodes[i];
        if (arg_node->type == AST_SPREAD) {
          Value spread_val = eval_expr(interp, arg_node->data.spread.expr);
          if (spread_val.type == VAL_LIST) {
            size_t spread_cnt = spread_val.data.list_val->count;
            argv = (Value *)realloc(argv, sizeof(Value) * (argc + spread_cnt));
            for (size_t j = 0; j < spread_cnt; j++) {
              argv[argc++] = value_copy(&spread_val.data.list_val->items[j]);
            }
          }
          value_free(&spread_val);
        } else {
          argv[argc++] = eval_expr(interp, arg_node);
        }
      }

      Value self_val;
      self_val.type = VAL_INSTANCE;
      self_val.data.instance_val = instance;

      Value result = interpreter_call(interp, construct.data.func_val, self_val,
                                      argc, argv);
      value_free(&result);

      for (int i = 0; i < argc; i++) {
        value_free(&argv[i]);
      }
      free(argv);
    }

    Value result;
    result.type = VAL_INSTANCE;
    result.data.instance_val = instance;
    return result;
  }

  case AST_SELF: {
    /* Return current instance (self) from environment */
    bool found = false;
    Value self = env_get(interp->current_env, "self", &found);
    if (!found) {
      runtime_error(interp, "'self' can only be used inside a method",
                    node->line);
      return value_null();
    }
    return value_copy(&self);
  }

  default:
    return value_null();
  }
}

/* ============================================================================
 * Statement Execution
 * ============================================================================
 */

static void assign_to_target(Interpreter *interp, ASTNode *target, Value val,
                             bool is_designate) {
  if (target->type == AST_IDENTIFIER) {
    if (is_designate) {
      env_define(interp->current_env, target->data.identifier.name,
                 value_copy(&val));
    } else {
      env_set(interp->current_env, target->data.identifier.name,
              value_copy(&val));
    }
  } else if (target->type == AST_LIST) {
    /* Destructuring: [a, b] = [1, 2] */
    if (val.type != VAL_LIST) {
      runtime_error(interp, "Unable to destructure non-list value.",
                    target->line);
      return;
    }
    ValueList *list = val.data.list_val;
    for (size_t i = 0; i < target->data.list.elements.count; i++) {
      if (i < list->count) {
        assign_to_target(interp, target->data.list.elements.nodes[i],
                         list->items[i], is_designate);
      } else {
        Value null_val = value_null();
        assign_to_target(interp, target->data.list.elements.nodes[i], null_val,
                         is_designate);
      }
    }
  } else if (target->type == AST_MEMBER) {
    Value obj = eval_expr(interp, target->data.member.object);
    if (obj.type == VAL_INSTANCE) {
      KeikakuInstance *inst = obj.data.instance_val;

      /* Private Member check */
      if (target->data.member.member[0] == '_') {
        bool found_self = false;
        Value self_val = env_get(interp->current_env, "self", &found_self);
        if (!found_self || self_val.data.instance_val != inst) {
          runtime_error(interp, "Modification of private member inhibited.",
                        target->line);
          value_free(&obj);
          return;
        }
      }

      bool found = false;
      env_get(inst->fields, target->data.member.member, &found);
      if (found) {
        env_set(inst->fields, target->data.member.member, value_copy(&val));
      } else {
        /* First time initialization of field */
        env_define(inst->fields, target->data.member.member, value_copy(&val));
      }
    } else {
      runtime_error(interp, "Only instances have properties.", target->line);
    }
    value_free(&obj);
  } else if (target->type == AST_INDEX) {
    Value obj = eval_expr(interp, target->data.index.object);
    Value idx = eval_expr(interp, target->data.index.index);
    if (obj.type == VAL_LIST && idx.type == VAL_INT) {
      int64_t i = idx.data.int_val;
      if (i >= 0 && (size_t)i < obj.data.list_val->count) {
        value_free(&obj.data.list_val->items[i]);
        obj.data.list_val->items[i] = value_copy(&val);
      } else {
        runtime_error(interp, "List index out of bounds.", target->line);
      }
    } else {
      runtime_error(interp, "Invalid index access.", target->line);
    }
    value_free(&obj);
    value_free(&idx);
  } else {
    runtime_error(interp, "Invalid assignment target.", target->line);
  }
}

static void exec_block(Interpreter *interp, ASTNodeArray *stmts) {
  size_t start_idx = 0;

  if (interp->is_resuming && interp->resume_count > 0) {
    GenFrame *frame = &interp->resume_stack[interp->resume_count - 1];
    if (frame->type == GEN_FRAME_BLOCK && frame->node == (void *)stmts) {
      DEBUG_PRINT("exec_block: resuming at index %zu (stmts=%p)\n",
                  frame->index, (void *)stmts);
      start_idx = frame->index;
      interp->resume_count--;
      if (interp->resume_count == 0)
        interp->is_resuming = false;
    } else {
      DEBUG_PRINT("exec_block: is_resuming, but frame type %d/node %p doesn't "
                  "match BLOCK/%p\n",
                  frame->type, frame->node, (void *)stmts);
      /* Do NOT clear is_resuming here, the frame might belong to a loop inside
       */
    }
  }

  DEBUG_PRINT("exec_block: loop start count=%zu\n", stmts->count);
  for (size_t i = start_idx; i < stmts->count; i++) {
    size_t old_stack_count =
        interp->current_gen ? interp->current_gen->stack_count : 0;
    DEBUG_PRINT("exec_block (level %p): stmt %zu/%zu type %s\n", (void *)stmts,
                i, stmts->count, ast_node_type_name(stmts->nodes[i]->type));
    eval_stmt(interp, stmts->nodes[i]);
    if (interp->has_return || interp->has_error || interp->has_break ||
        interp->has_continue) {
      if (interp->has_return && interp->current_gen && !interp->has_error) {
        bool child_suspended =
            (interp->current_gen->stack_count > old_stack_count);
        GenFrame f;
        memset(&f, 0, sizeof(GenFrame));
        f.type = GEN_FRAME_BLOCK;
        f.index = child_suspended ? i : i + 1;
        f.node = (void *)stmts;
        gen_push_frame(interp->current_gen, f);
        DEBUG_PRINT("exec_block: suspended at %zu, child_susp: %d\n", i,
                    child_suspended);
      }
      break;
    }
  }
}

static Value eval_stmt(Interpreter *interp, ASTNode *node) {
  if (!node || interp->has_return)
    return value_null();

  switch (node->type) {
  case AST_DESIGNATE:
  case AST_ASSIGN: {
    Value val = eval_expr(interp, node->data.assign.value);
    assign_to_target(interp, node->data.assign.target, val,
                     node->type == AST_DESIGNATE);
    value_free(&val);
    return value_null();
  }

  case AST_EXPR_STMT:
    return eval_expr(interp, node->data.expr_stmt.expr);

  case AST_FORESEE: {
    Value cond = eval_expr(interp, node->data.foresee.condition);
    bool taken = value_is_truthy(&cond);
    value_free(&cond);

    if (taken) {
      exec_block(interp, &node->data.foresee.body);
    } else {
      /* Check alternates */
      bool alt_taken = false;
      for (size_t i = 0; i < node->data.foresee.alternates.count; i++) {
        Value alt_cond =
            eval_expr(interp, node->data.foresee.alternates.alts[i].condition);
        if (value_is_truthy(&alt_cond)) {
          value_free(&alt_cond);
          exec_block(interp, &node->data.foresee.alternates.alts[i].body);
          alt_taken = true;
          break;
        }
        value_free(&alt_cond);
      }

      if (!alt_taken && node->data.foresee.otherwise.count > 0) {
        exec_block(interp, &node->data.foresee.otherwise);
      }
    }
    return value_null();
  }

  case AST_CYCLE_WHILE: {
    while (true) {
      bool resuming_this_iteration = false;
      if (interp->is_resuming && interp->resume_count > 0) {
        GenFrame *frame = &interp->resume_stack[interp->resume_count - 1];
        if (frame->type == GEN_FRAME_CYCLE_WHILE && frame->node == node) {
          DEBUG_PRINT("AST_CYCLE_WHILE: resuming into body\n");
          resuming_this_iteration = true;
          interp->resume_count--;
          if (interp->resume_count == 0)
            interp->is_resuming = false;
        }
      }

      if (!resuming_this_iteration) {
        Value cond = eval_expr(interp, node->data.cycle_while.condition);
        if (!value_is_truthy(&cond)) {
          value_free(&cond);
          break;
        }
        value_free(&cond);
      }

      exec_block(interp, &node->data.cycle_while.body);

      if (interp->has_continue) {
        interp->has_continue = false;
        continue;
      }

      if (interp->has_break) {
        interp->has_break = false;
        break;
      }

      if (interp->has_return || interp->has_error) {
        if (interp->has_return && interp->current_gen && !interp->has_error) {
          GenFrame f;
          memset(&f, 0, sizeof(GenFrame));
          f.type = GEN_FRAME_CYCLE_WHILE;
          f.node = node;
          gen_push_frame(interp->current_gen, f);
          DEBUG_PRINT("AST_CYCLE_WHILE: suspended\n");
        }
        break;
      }
    }
    return value_null();
  }

  case AST_CYCLE_THROUGH: {
    Value iterable;
    size_t start_idx = 0;
    bool initially_resuming_gen =
        false; /* Track if we're resuming a generator loop */

    if (interp->is_resuming && interp->resume_count > 0) {
      GenFrame *frame = &interp->resume_stack[interp->resume_count - 1];
      if (frame->type == GEN_FRAME_CYCLE_THROUGH && frame->node == node) {
        /* Correct: Use a COPIED value of iterable to keep it alive */
        iterable = value_copy(&frame->iterable);
        start_idx = frame->index;
        interp->resume_count--;

        /* For generators, track if there are more frames (we're resuming into
         * body) */
        if (frame->iterable.type == VAL_GENERATOR && interp->resume_count > 0) {
          initially_resuming_gen = true;
        }

        if (interp->resume_count == 0)
          interp->is_resuming = false;
      } else {
        iterable = eval_expr(interp, node->data.cycle_through.iterable);
      }
    } else {
      iterable = eval_expr(interp, node->data.cycle_through.iterable);
    }

    if (iterable.type != VAL_LIST && iterable.type != VAL_GENERATOR) {
      runtime_error(interp, "Can only cycle through a list or sequence.",
                    node->line);
      value_free(&iterable);
      return value_null();
    }

    if (iterable.type == VAL_LIST) {
      ValueList *list = iterable.data.list_val;
      for (size_t i = start_idx; i < list->count; i++) {
        assign_to_target(interp, node->data.cycle_through.var_pattern,
                         list->items[i], true);
        exec_block(interp, &node->data.cycle_through.body);
        if (interp->has_return || interp->has_error) {
          if (interp->has_return && interp->current_gen && !interp->has_error) {
            GenFrame f;
            memset(&f, 0, sizeof(GenFrame));
            f.type = GEN_FRAME_CYCLE_THROUGH;
            f.index = i;
            f.iterable = value_copy(&iterable);
            f.node = node;
            gen_push_frame(interp->current_gen, f);
          }
          break;
        }
      }
    } else {
      /* VAL_GENERATOR */
      bool first_iteration = true;
      while (true) {
        bool resuming_into_body = false;

        /* Use the flag set from initial frame check */
        if (first_iteration && initially_resuming_gen) {
          resuming_into_body = true;
          DEBUG_PRINT("AST_CYCLE_THROUGH: resuming into generator body\n");
        }
        first_iteration = false;

        Value next_val;
        if (resuming_into_body) {
          next_val = value_null(); /* Dummy, environment has the value */
        } else {
          next_val = interpreter_gen_next(interp, iterable);
          if (next_val.type == VAL_NULL &&
              iterable.data.gen_val->status == GEN_DONE) {
            value_free(&next_val);
            break;
          }
          assign_to_target(interp, node->data.cycle_through.var_pattern,
                           next_val, true);
        }

        exec_block(interp, &node->data.cycle_through.body);
        value_free(&next_val);

        if (interp->has_continue) {
          interp->has_continue = false;
          continue;
        }

        if (interp->has_break) {
          interp->has_break = false;
          break;
        }

        if (interp->has_return || interp->has_error) {
          if (interp->has_return && interp->current_gen && !interp->has_error) {
            GenFrame f;
            memset(&f, 0, sizeof(GenFrame));
            f.type = GEN_FRAME_CYCLE_THROUGH;
            f.index = 0; /* Index not used for generator iteration */
            f.iterable = value_copy(&iterable);
            f.node = node;
            gen_push_frame(interp->current_gen, f);
          }
          break;
        }
      }
    }

    value_free(&iterable);
    return value_null();
  }

  case AST_CYCLE_FROM_TO: {
    int64_t current_i = 0;
    int64_t end_val = 0;

    if (interp->is_resuming && interp->resume_count > 0) {
      GenFrame *frame = &interp->resume_stack[interp->resume_count - 1];
      if (frame->type == GEN_FRAME_CYCLE_FROM_TO && frame->node == node) {
        current_i = frame->current;
        end_val = frame->end;
        interp->resume_count--;
        if (interp->resume_count == 0)
          interp->is_resuming = false;
      } else {
        Value start = eval_expr(interp, node->data.cycle_from_to.start);
        Value end = eval_expr(interp, node->data.cycle_from_to.end);
        current_i = start.type == VAL_INT ? start.data.int_val : 0;
        end_val = end.type == VAL_INT ? end.data.int_val : 0;
        value_free(&start);
        value_free(&end);
      }
    } else {
      Value start = eval_expr(interp, node->data.cycle_from_to.start);
      Value end = eval_expr(interp, node->data.cycle_from_to.end);
      current_i = start.type == VAL_INT ? start.data.int_val : 0;
      end_val = end.type == VAL_INT ? end.data.int_val : 0;
      value_free(&start);
      value_free(&end);
    }

    for (int64_t i = current_i; i < end_val; i++) {
      Value val = value_int(i);
      assign_to_target(interp, node->data.cycle_from_to.var_pattern, val, true);
      exec_block(interp, &node->data.cycle_from_to.body);

      if (interp->has_continue) {
        interp->has_continue = false;
        continue;
      }

      if (interp->has_break) {
        interp->has_break = false;
        break;
      }

      if (interp->has_return || interp->has_error) {
        if (interp->has_return && interp->current_gen && !interp->has_error) {
          GenFrame f;
          memset(&f, 0, sizeof(GenFrame));
          f.type = GEN_FRAME_CYCLE_FROM_TO;
          f.current = i;
          f.end = end_val;
          f.node = node;
          gen_push_frame(interp->current_gen, f);
        }
        break;
      }
    }
    return value_null();
  }

  case AST_PROTOCOL: {
    Value func = value_function(node, interp->current_env);
    env_define(interp->current_env, node->data.protocol.name, func);
    return value_null();
  }

  case AST_YIELD: {
    if (node->data.yield.value) {
      interp->return_value = eval_expr(interp, node->data.yield.value);
    } else {
      interp->return_value = value_null();
    }
    interp->has_return = true;
    return value_null();
  }

  case AST_BREAK: {
    interp->has_break = true;
    return value_null();
  }

  case AST_CONTINUE: {
    interp->has_continue = true;
    return value_null();
  }

  case AST_DELEGATE: {
    /* yield from / delegate - iterate through iterable and yield each value */
    Value iterable;
    size_t start_idx = 0;

    /* Check if we're resuming a delegate */
    if (interp->is_resuming && interp->resume_count > 0) {
      GenFrame *frame = &interp->resume_stack[interp->resume_count - 1];
      if (frame->type == GEN_FRAME_DELEGATE && frame->node == node) {
        iterable = value_copy(&frame->iterable);
        start_idx = frame->index;
        interp->resume_count--;
        if (interp->resume_count == 0)
          interp->is_resuming = false;
      } else {
        iterable = eval_expr(interp, node->data.delegate.iterable);
      }
    } else {
      iterable = eval_expr(interp, node->data.delegate.iterable);
    }

    if (iterable.type == VAL_LIST) {
      /* Delegate to a list - yield each item */
      ValueList *list = iterable.data.list_val;
      for (size_t i = start_idx; i < list->count; i++) {
        interp->return_value = value_copy(&list->items[i]);
        interp->has_return = true;

        if (interp->current_gen) {
          /* Save position for resumption */
          GenFrame f;
          memset(&f, 0, sizeof(GenFrame));
          f.type = GEN_FRAME_DELEGATE;
          f.index = i + 1; /* Next index to process */
          f.iterable = value_copy(&iterable);
          f.node = node;
          gen_push_frame(interp->current_gen, f);
        }
        value_free(&iterable);
        return value_null();
      }
    } else if (iterable.type == VAL_GENERATOR) {
      /* Delegate to another generator - pull and yield each value */
      while (true) {
        Value next_val = interpreter_gen_next(interp, iterable);
        if (next_val.type == VAL_NULL &&
            iterable.data.gen_val->status == GEN_DONE) {
          value_free(&next_val);
          break;
        }
        /* Yield this value */
        interp->return_value = next_val;
        interp->has_return = true;

        if (interp->current_gen) {
          /* Save the delegated generator for resumption */
          GenFrame f;
          memset(&f, 0, sizeof(GenFrame));
          f.type = GEN_FRAME_DELEGATE;
          f.index = 0; /* Not used for generators */
          f.iterable = value_copy(&iterable);
          f.node = node;
          gen_push_frame(interp->current_gen, f);
        }
        value_free(&iterable);
        return value_null();
      }
    } else {
      runtime_error(interp, "Can only delegate to a list or sequence.",
                    node->line);
    }

    value_free(&iterable);
    return value_null();
  }

  case AST_SCHEME: {
    voice_print_scheme_registered();
    exec_block(interp, &node->data.scheme.body);
    voice_print_scheme_executed();
    return value_null();
  }

  case AST_PREVIEW: {
    Value val = eval_expr(interp, node->data.preview.expr);
    voice_print_preview(&val);
    value_free(&val);
    return value_null();
  }

  case AST_OVERRIDE: {
    Value val = eval_expr(interp, node->data.override.value);
    env_force_set(interp->current_env, node->data.override.name, val);
    voice_print_override(node->data.override.name, &val);
    return value_null();
  }

  case AST_SITUATION: {
    Value val = eval_expr(interp, node->data.situation.value);
    bool matched = false;

    for (size_t i = 0; i < node->data.situation.alignments.count; i++) {
      ASTNode *alignment = node->data.situation.alignments.nodes[i];
      if (alignment->data.alignment.is_otherwise) {
        /* Otherwise handled at end if no match */
        continue;
      }

      for (size_t j = 0; j < alignment->data.alignment.values.count; j++) {
        Value case_val =
            eval_expr(interp, alignment->data.alignment.values.nodes[j]);
        if (value_equals(&val, &case_val)) {
          value_free(&case_val);
          exec_block(interp, &alignment->data.alignment.body);
          matched = true;
          break;
        }
        value_free(&case_val);
      }
      if (matched)
        break;
    }

    if (!matched) {
      /* Search for otherwise case */
      for (size_t i = 0; i < node->data.situation.alignments.count; i++) {
        ASTNode *alignment = node->data.situation.alignments.nodes[i];
        if (alignment->data.alignment.is_otherwise) {
          exec_block(interp, &alignment->data.alignment.body);
          break;
        }
      }
    }

    value_free(&val);
    return value_null();
  }

  case AST_ABSOLUTE: {
    Value cond = eval_expr(interp, node->data.absolute.condition);
    if (!value_is_truthy(&cond)) {
      voice_print_absolute_failed(node->data.absolute.expr_str
                                      ? node->data.absolute.expr_str
                                      : "condition");
    }
    value_free(&cond);
    return value_null();
  }

  case AST_ANOMALY: {
    voice_print_anomaly_enter();
    interp->anomaly_mode = true;
    exec_block(interp, &node->data.anomaly.body);
    interp->anomaly_mode = false;
    voice_print_anomaly_exit();
    return value_null();
  }

  case AST_ENTITY: {
    /* Create a class definition */
    KeikakuClass *cls = (KeikakuClass *)calloc(1, sizeof(KeikakuClass));
    cls->name = strdup(node->data.entity.name);
    cls->parent = NULL;
    cls->definition = node;
    cls->methods = env_create(interp->current_env);

    /* Look up parent class if specified */
    if (node->data.entity.parent) {
      bool found = false;
      Value parent_val =
          env_get(interp->global_env, node->data.entity.parent, &found);
      if (found && parent_val.type == VAL_CLASS) {
        cls->parent = parent_val.data.class_val;
        /* Inherit methods */
        cls->methods = env_create(cls->parent->methods);
      }
    }

    /* Process class body - extract methods */
    Environment *old_env = interp->current_env;
    interp->current_env = cls->methods;

    for (size_t i = 0; i < node->data.entity.members.count; i++) {
      ASTNode *member = node->data.entity.members.nodes[i];
      if (member->type == AST_PROTOCOL) {
        /* Define method in class */
        Function *method = (Function *)malloc(sizeof(Function));
        method->name = strdup(member->data.protocol.name);
        method->node = member;
        method->closure = cls->methods;
        method->is_lambda = false;

        Value method_val;
        method_val.type = VAL_FUNCTION;
        method_val.data.func_val = method;
        env_define(cls->methods, method->name, method_val);
      }
    }

    interp->current_env = old_env;

    /* Register class */
    Value class_val;
    class_val.type = VAL_CLASS;
    class_val.data.class_val = cls;
    env_define(interp->global_env, cls->name, class_val);

    printf("  ◈ Entity '%s' has been defined. The blueprint awaits "
           "manifestation.\n",
           cls->name);
    return value_null();
  }

  case AST_INCORPORATE: {
    /* Import another file */
    const char *path = node->data.incorporate.path;

    FILE *file = fopen(path, "r");
    if (!file) {
      printf("  ⚠ Unable to incorporate '%s'. File not found.\n", path);
      runtime_error(interp, "Incorporate failed: file not found", node->line);
      return value_null();
    }

    /* Read file */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *source = (char *)malloc(size + 1);
    size_t read_size = fread(source, 1, size, file);
    source[read_size] = '\0';
    fclose(file);

    printf("  ◈ Incorporating '%s'. External knowledge absorbed.\n", path);

    /* Parse and execute the imported file */
    Lexer *lexer = lexer_create(source, path);
    size_t token_count;
    Token *tokens = lexer_tokenize_all(lexer, &token_count);

    if (!lexer_has_error(lexer)) {
      Parser *parser = parser_create(tokens, token_count, source, path);
      ASTNode *ast = parser_parse(parser);

      if (!parser_has_error(parser)) {
        exec_block(interp, &ast->data.program.statements);
      }

      /* NOTE: We do NOT destroy ast or free source here!
       * Functions defined in the imported file reference the AST nodes.
       * This is a memory leak, but necessary for correct behavior.
       * A proper solution would be to deep-copy function nodes. */

      parser_destroy(parser);
    }

    lexer_free_tokens(tokens, token_count);
    lexer_destroy(lexer);
    /* Don't free source - functions may reference strings from it */

    return value_null();
  }

  case AST_ATTEMPT: {
    /* Save error state */
    bool old_error = interp->has_error;
    char old_error_buf[1024];
    strncpy(old_error_buf, interp->error_buffer, sizeof(old_error_buf));
    interp->has_error = false;

    /* Execute try block */
    Value result = value_null();
    for (size_t i = 0; i < node->data.attempt.try_body.count; i++) {
      result = eval_stmt(interp, node->data.attempt.try_body.nodes[i]);
      if (interp->has_error || interp->has_return)
        break;
    }

    /* If error occurred, execute recover block */
    if (interp->has_error && node->data.attempt.recover_body.count > 0) {
      char *error_msg = strdup(interp->error_buffer);
      interp->has_error = false;
      interp->error_buffer[0] = '\0';

      printf("  ◇ Deviation intercepted. Recovery protocol engaged.\n");

      /* Bind error variable if specified */
      if (node->data.attempt.error_var) {
        env_define(interp->current_env, node->data.attempt.error_var,
                   value_string(error_msg));
      }

      /* Execute recover block */
      for (size_t i = 0; i < node->data.attempt.recover_body.count; i++) {
        result = eval_stmt(interp, node->data.attempt.recover_body.nodes[i]);
        if (interp->has_return)
          break;
      }

      free(error_msg);
    } else if (!interp->has_error) {
      /* Restore previous error state if no new error */
      interp->has_error = old_error;
      strncpy(interp->error_buffer, old_error_buf,
              sizeof(interp->error_buffer));
    }

    return result;
  }

  case AST_PROGRAM: {
    Value last = value_null();
    for (size_t i = 0; i < node->data.program.statements.count; i++) {
      value_free(&last);
      last = eval_stmt(interp, node->data.program.statements.nodes[i]);
    }
    return last;
  }

  default:
    return value_null();
  }
}

Value interpreter_call(Interpreter *interp, Function *func, Value self_val,
                       int argc, Value *argv) {
  Environment *call_env = env_create(func->closure);
  Environment *old_env = interp->current_env;
  interp->current_env = call_env;

  /* Bind self if provided */
  if (self_val.type != VAL_NULL) {
    env_define(call_env, "self", value_copy(&self_val));
  }

  /* Handle lambda vs regular function */
  if (func->is_lambda) {
    /* Lambda: params and body are in lambda struct */
    ASTParamArray *params = &func->node->data.lambda.params;
    for (size_t i = 0; i < params->count; i++) {
      if (params->params[i].is_rest) {
        Value list = value_list_new();
        for (int j = i; j < argc; j++) {
          value_list_push(&list, value_copy(&argv[j]));
        }
        assign_to_target(interp, params->params[i].pattern, list, true);
        value_free(&list);
        break;
      }

      if (i < (size_t)argc) {
        assign_to_target(interp, params->params[i].pattern, argv[i], true);
      } else {
        Value null_val = value_null();
        assign_to_target(interp, params->params[i].pattern, null_val, true);
      }
    }

    /* Lambda body - could be a single expression or a block */
    Value result;
    if (func->node->data.lambda.body->type == AST_BLOCK) {
      exec_block(interp, &func->node->data.lambda.body->data.block.statements);
      result = value_null();
      if (interp->has_return) {
        result = interp->return_value;
        interp->return_value = value_null();
        interp->has_return = false;
      }
    } else {
      result = eval_expr(interp, func->node->data.lambda.body);
    }

    interp->current_env = old_env;
    env_destroy(call_env);
    return result;
  }

  /* Regular protocol function */
  /* Bind parameters */
  ASTParamArray *params = &func->node->data.protocol.params;
  for (size_t i = 0; i < params->count; i++) {
    if (params->params[i].is_rest) {
      Value list = value_list_new();
      for (int j = i; j < argc; j++) {
        value_list_push(&list, value_copy(&argv[j]));
      }
      assign_to_target(interp, params->params[i].pattern, list, true);
      value_free(&list);
      break;
    }

    if (i < (size_t)argc) {
      assign_to_target(interp, params->params[i].pattern, argv[i], true);
    } else if (params->params[i].default_value) {
      Value def = eval_expr(interp, params->params[i].default_value);
      assign_to_target(interp, params->params[i].pattern, def, true);
      value_free(&def);
    } else {
      Value null_val = value_null();
      assign_to_target(interp, params->params[i].pattern, null_val, true);
    }
  }

  if (func->is_sequence) {
    interp->current_env = old_env;
    return value_generator_new(func, call_env, self_val);
  }

  /* Execute body */
  exec_block(interp, &func->node->data.protocol.body);

  Value result = value_null();
  if (interp->has_return) {
    result = interp->return_value;
    interp->return_value = value_null();
    interp->has_return = false;
  }

  interp->current_env = old_env;
  env_destroy(call_env);

  return result;
}

Value interpreter_gen_next(Interpreter *interp, Value gen_val) {
  Generator *gen = gen_val.data.gen_val;
  Function *func = gen->func_val.data.func_val;
  DEBUG_PRINT("interpreter_gen_next: starting for %s (status %d, stack %zu)\n",
              func->name, gen->status, gen->stack_count);
  if (gen->status == GEN_DONE)
    return value_null();

  Environment *old_env = interp->current_env;
  Generator *old_gen = interp->current_gen;
  bool old_resuming = interp->is_resuming;
  GenFrame *old_resume_stack = interp->resume_stack;
  size_t old_resume_count = interp->resume_count;

  /* Move saved stack to interpreter for resumption */
  interp->resume_stack = gen->stack;
  interp->resume_count = gen->stack_count;
  size_t original_resume_count = interp->resume_count; /* Save for cleanup */
  interp->is_resuming = (interp->resume_count > 0);
  interp->current_gen = gen;
  interp->current_env = gen->env;

  /* Reset generator stack for new suspension record */
  gen->stack = NULL;
  gen->stack_count = 0;
  gen->stack_capacity = 0;

  DEBUG_PRINT("interpreter_gen_next: calling exec_block (resuming: %d)\n",
              interp->is_resuming);
  exec_block(interp, &func->node->data.protocol.body);
  DEBUG_PRINT("interpreter_gen_next: return from exec_block (has_return: %d, "
              "new stack: %zu)\n",
              interp->has_return, gen->stack_count);

  Value result = value_null();
  if (interp->has_return) {
    result = interp->return_value;
    interp->return_value = value_null();
    interp->has_return = false;
    gen->status = GEN_SUSPENDED;
  } else {
    gen->status = GEN_DONE;
  }

  /* Clean up resume stack - consumed frames still need their internal values
   * freed */
  for (size_t i = 0; i < original_resume_count; i++) {
    if (interp->resume_stack[i].type == GEN_FRAME_CYCLE_THROUGH) {
      value_free(&interp->resume_stack[i].iterable);
    }
  }
  free(interp->resume_stack);

  interp->current_env = old_env;
  interp->current_gen = old_gen;
  interp->is_resuming = old_resuming;
  interp->resume_stack = old_resume_stack;
  interp->resume_count = old_resume_count;

  return result;
}

Value interpreter_execute(Interpreter *interp, ASTNode *ast) {
  interp->has_error = false;
  interp->has_return = false;
  return eval_stmt(interp, ast);
}
