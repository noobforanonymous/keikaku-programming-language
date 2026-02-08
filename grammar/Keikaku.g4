/*
 * Keikaku Programming Language Grammar
 * 
 * "Your syntax has been formalized."
 * 
 * This is the ANTLR4 grammar definition for Keikaku.
 * Note: The main interpreter uses a hand-written recursive descent parser,
 * but this grammar serves as the formal specification.
 */

grammar Keikaku;

// ===== PARSER RULES =====

program
    : NEWLINE* (statement NEWLINE*)* EOF
    ;

statement
    : designateStmt
    | assignStmt
    | foreseeStmt
    | cycleStmt
    | protocolDef
    | yieldStmt
    | schemeStmt
    | previewStmt
    | overrideStmt
    | absoluteStmt
    | anomalyStmt
    | expressionStmt
    ;

// Variable declaration
designateStmt
    : DESIGNATE IDENTIFIER '=' expression
    | IDENTIFIER WALRUS expression
    ;

// Assignment
assignStmt
    : IDENTIFIER '=' expression
    ;

// Conditional
foreseeStmt
    : FORESEE expression ':' block 
      (ALTERNATE expression ':' block)*
      (OTHERWISE ':' block)?
    ;

// Loops
cycleStmt
    : CYCLE WHILE expression ':' block                           # cycleWhile
    | CYCLE THROUGH expression AS IDENTIFIER ':' block           # cycleThrough
    | CYCLE FROM expression TO expression (AS IDENTIFIER)? ':' block  # cycleFromTo
    ;

// Function definition
protocolDef
    : PROTOCOL IDENTIFIER '(' paramList? ')' ':' block
    ;

paramList
    : param (',' param)*
    ;

param
    : IDENTIFIER (('=' | WALRUS) expression)?
    ;

// Return
yieldStmt
    : YIELD expression?
    ;

// Special constructs
schemeStmt
    : SCHEME ':' block EXECUTE
    ;

previewStmt
    : PREVIEW expression
    ;

overrideStmt
    : OVERRIDE IDENTIFIER '=' expression
    ;

absoluteStmt
    : ABSOLUTE expression
    ;

anomalyStmt
    : ANOMALY ':' block
    ;

// Expression statement
expressionStmt
    : expression
    ;

// Block (indentation-based)
block
    : INDENT statement+ DEDENT
    ;

// ===== EXPRESSIONS =====

expression
    : orExpr
    ;

orExpr
    : andExpr (OR andExpr)*
    ;

andExpr
    : notExpr (AND notExpr)*
    ;

notExpr
    : NOT notExpr
    | comparisonExpr
    ;

comparisonExpr
    : addExpr (('==' | '!=' | '<' | '<=' | '>' | '>=') addExpr)*
    ;

addExpr
    : mulExpr (('+' | '-') mulExpr)*
    ;

mulExpr
    : powerExpr (('*' | '/' | '//' | '%') powerExpr)*
    ;

powerExpr
    : unaryExpr ('**' powerExpr)?
    ;

unaryExpr
    : '-' unaryExpr
    | postfixExpr
    ;

postfixExpr
    : primaryExpr (
        '(' argumentList? ')'     // Function call
        | '[' expression ']'      // Index
        | '.' IDENTIFIER          // Member access
      )*
    ;

primaryExpr
    : INTEGER
    | FLOAT
    | STRING
    | TRUE
    | FALSE
    | IDENTIFIER
    | listLiteral
    | dictLiteral
    | '(' expression ')'
    ;

listLiteral
    : '[' (expression (',' expression)*)? ']'
    ;

dictLiteral
    : '{' (dictEntry (',' dictEntry)*)? '}'
    ;

dictEntry
    : expression ':' expression
    ;

argumentList
    : expression (',' expression)*
    ;

// ===== LEXER RULES =====

// Keywords
DESIGNATE   : 'designate' ;
FORESEE     : 'foresee' ;
ALTERNATE   : 'alternate' ;
OTHERWISE   : 'otherwise' ;
CYCLE       : 'cycle' ;
WHILE       : 'while' ;
THROUGH     : 'through' ;
FROM        : 'from' ;
TO          : 'to' ;
AS          : 'as' ;
PROTOCOL    : 'protocol' ;
YIELD       : 'yield' ;
AND         : 'and' ;
OR          : 'or' ;
NOT         : 'not' ;
TRUE        : 'true' ;
FALSE       : 'false' ;
SCHEME      : 'scheme' ;
EXECUTE     : 'execute' ;
PREVIEW     : 'preview' ;
OVERRIDE    : 'override' ;
ABSOLUTE    : 'absolute' ;
ANOMALY     : 'anomaly' ;

// Literals
INTEGER     : [0-9]+ ;
FLOAT       : [0-9]+ '.' [0-9]+ ([eE] [+-]? [0-9]+)? ;
STRING      : '"' (~["\\\r\n] | '\\' .)* '"' 
            | '\'' (~['\\\r\n] | '\\' .)* '\'' ;

// Operators
WALRUS      : ':=' ;

// Identifiers
IDENTIFIER  : [a-zA-Z_] [a-zA-Z0-9_]* ;

// Whitespace handling (for indentation-sensitive parsing)
NEWLINE     : ('\r'? '\n' | '\r') ;
INDENT      : '<INDENT>' ;  // Synthetic token
DEDENT      : '<DEDENT>' ;  // Synthetic token

// Comments
COMMENT     : '#' ~[\r\n]* -> skip ;

// Inline whitespace
WS          : [ \t]+ -> skip ;
