/*
 * Keikaku Programming Language - Main Entry Point
 *
 * "All proceeds according to plan."
 *
 * Copyright (c) 2026
 */

#include "ast.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEIKAKU_VERSION "1.0.0"

/* ============================================================================
 * File Reading
 * ============================================================================
 */

static char *read_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "  ⚠ Unable to locate file '%s'.\n", path);
    fprintf(stderr,
            "    The designated path was not found. Check your parameters.\n");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(size + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  size_t read = fread(buffer, 1, size, file);
  buffer[read] = '\0';

  fclose(file);
  return buffer;
}

/* ============================================================================
 * Run Source Code
 * ============================================================================
 */

/* Personality messages for REPL results */
static const char *result_messages[] = {"Result aligned with expectations.",
                                        "Outcome as anticipated.",
                                        "The calculation proceeds as planned.",
                                        "As foreseen.",
                                        "Precisely as calculated.",
                                        NULL};

static const char *get_random_message(void) {
  static int index = 0;
  const char *msg = result_messages[index];
  index = (index + 1) % 5;
  return msg;
}

static int run_source_repl(Interpreter *interp, const char *source,
                           const char *filename, bool show_result) {
  /* Lexing */
  Lexer *lexer = lexer_create(source, filename);
  if (!lexer) {
    fprintf(stderr,
            "  ⚠ Memory allocation failed. The scenario cannot proceed.\n");
    return 1;
  }

  size_t token_count;
  Token *tokens = lexer_tokenize_all(lexer, &token_count);

  if (lexer_has_error(lexer)) {
    fprintf(stderr, "%s\n", lexer_get_error(lexer));
    lexer_free_tokens(tokens, token_count);
    lexer_destroy(lexer);
    return 1;
  }

  /* Parsing */
  Parser *parser = parser_create(tokens, token_count, source, filename);
  if (!parser) {
    lexer_free_tokens(tokens, token_count);
    lexer_destroy(lexer);
    return 1;
  }

  ASTNode *ast = parser_parse(parser);

  if (parser_has_error(parser)) {
    fprintf(stderr, "%s\n", parser_get_error(parser));
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_free_tokens(tokens, token_count);
    lexer_destroy(lexer);
    return 1;
  }

  /* Execution */
  Value result = interpreter_execute(interp, ast);

  int exit_code = interpreter_has_error(interp) ? 1 : 0;

  /* Print result in REPL mode */
  if (show_result && !exit_code && result.type != VAL_NULL) {
    char *str = value_to_string(&result);
    printf("  %s\n", str);
    printf("  %s\n", get_random_message());
    free(str);
  }

  /* Cleanup */
  value_free(&result);
  ast_destroy(ast);
  parser_destroy(parser);
  lexer_free_tokens(tokens, token_count);
  lexer_destroy(lexer);

  return exit_code;
}

static int run_source(Interpreter *interp, const char *source,
                      const char *filename) {
  return run_source_repl(interp, source, filename, false);
}

/* ============================================================================
 * REPL
 * ============================================================================
 */

static void run_repl(void) {
  voice_print_welcome();

  Interpreter *interp = interpreter_create();
  if (!interp) {
    fprintf(stderr, "  ⚠ Failed to initialize interpreter.\n");
    return;
  }

  char line[4096];
  char buffer[65536];

  while (1) {
    voice_print_prompt();

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    /* Remove trailing newline */
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
      len--;
    }

    /* Check for exit command */
    if (strcmp(line, "conclude") == 0) {
      voice_print_goodbye();
      break;
    }

    /* Check for multi-line input */
    if (len > 0 && line[len - 1] == ':') {
      strcpy(buffer, line);
      strcat(buffer, "\n");

      /* Read indented lines */
      while (1) {
        printf("... ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
          break;

        /* Empty line ends input */
        if (line[0] == '\n' || (line[0] == '\r' && line[1] == '\n')) {
          break;
        }

        strcat(buffer, line);
      }

      run_source_repl(interp, buffer, "<repl>", true);
    } else if (len > 0) {
      run_source_repl(interp, line, "<repl>", true);
    }
  }

  interpreter_destroy(interp);
}

/* ============================================================================
 * Run File
 * ============================================================================
 */

static int run_file(const char *path) {
  char *source = read_file(path);
  if (!source) {
    return 1;
  }

  Interpreter *interp = interpreter_create();
  if (!interp) {
    free(source);
    return 1;
  }

  int result = run_source(interp, source, path);

  interpreter_destroy(interp);
  free(source);

  return result;
}

/* ============================================================================
 * Print Usage
 * ============================================================================
 */

static void print_usage(const char *prog) {
  printf("\n");
  printf("  K E I K A K U  v%s\n", KEIKAKU_VERSION);
  printf("  \"Everything proceeds according to plan.\"\n\n");
  printf("  Usage:\n");
  printf("    %s              Start interactive REPL\n", prog);
  printf("    %s <file.kei>   Execute a Keikaku script\n", prog);
  printf("    %s --help       Display this message\n", prog);
  printf("    %s --version    Display version information\n\n", prog);
  printf("  The system awaits your input.\n\n");
}

static void print_version(void) {
  printf("\n");
  printf("  Keikaku Programming Language v%s\n", KEIKAKU_VERSION);
  printf("  \"All proceeds according to keikaku.\"\n");
  printf("  (keikaku means plan)\n\n");
}

/* ============================================================================
 * Main
 * ============================================================================
 */

int main(int argc, char *argv[]) {
  if (argc == 1) {
    run_repl();
    return 0;
  }

  if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      print_usage(argv[0]);
      return 0;
    }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
      print_version();
      return 0;
    }

    return run_file(argv[1]);
  }

  print_usage(argv[0]);
  return 1;
}
