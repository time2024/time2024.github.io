/* lexer.re - JavaScript Lexer using re2c with Unicode support */

#define _POSIX_C_SOURCE 200809L  /* 启用strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "ast.h"

/* Global variables for tracking position and ASI */
int yylineno = 1;
int yycolno = 1;
int seen_newline = 0;      /* 追踪是否遇到换行 */
int last_token = 0;        /* 上一个token */
int paren_depth = 0;       /* 括号嵌套深度 */
int in_for_header = 0;     /* 是否在for循环头部 */

char *yytext = NULL;
size_t yyleng = 0;

/* Input buffer management */
static const char *input_cursor = NULL;
static const char *input_marker = NULL;
static const char *input_limit = NULL;
static const char *input_token = NULL;

void init_lexer(const char *input) {
    input_cursor = input;
    input_marker = input;
    input_limit = input + strlen(input);
    input_token = input;
    yylineno = 1;
    yycolno = 1;
    seen_newline = 0;
    last_token = 0;
    paren_depth = 0;
    in_for_header = 0;
}

/* Check if ASI should be applied based on restricted productions */
int should_insert_semicolon_after(int token) {
    return (token == RETURN || token == BREAK || 
            token == CONTINUE || token == THROW ||
            token == POSTINC || token == POSTDEC);
}

/* Check if current context allows ASI */
int can_insert_semicolon(int next_token) {
    /* Rule 1: Offending token after newline */
    if (seen_newline) {
        /* Rule 3: Restricted productions */
        if (should_insert_semicolon_after(last_token)) {
            return 1;
        }
        /* Rule 4: Before closing brace */
        if (next_token == '}') {
            return 1;
        }
        /* Exception: Never in for-loop headers */
        if (in_for_header) {
            return 0;
        }
    }
    /* Rule 2: EOF */
    if (next_token == 0) {
        return 1;
    }
    return 0;
}

/* Save token text */
void save_token() {
    yyleng = input_cursor - input_token;
    if (yytext) free(yytext);
    yytext = (char*)malloc(yyleng + 1);
    memcpy(yytext, input_token, yyleng);
    yytext[yyleng] = '\0';
}

int yylex() {
    int token;
    
    /* Check for ASI insertion point */
    if (seen_newline && should_insert_semicolon_after(last_token)) {
        seen_newline = 0;
        last_token = ';';
        return ';';
    }

    input_token = input_cursor;
    
    /*!re2c
    re2c:define:YYCTYPE = "unsigned char";
    re2c:define:YYCURSOR = input_cursor;
    re2c:define:YYMARKER = input_marker;
    re2c:define:YYLIMIT = input_limit;
    re2c:yyfill:enable = 0;
    re2c:encoding:utf8 = 1;
    
    // Whitespace (not including line terminators)
    WhiteSpace = [ \t\v\f\xa0\u1680\u2000-\u200a\u202f\u205f\u3000\ufeff];
    
    // Line terminators
    LineTerminator = [\n\r\u2028\u2029];
    LineTerminatorSequence = "\r\n" | LineTerminator;
    
    // Comments
    SingleLineComment = "//" [^\n\r\u2028\u2029]*;
    MultiLineComment = "/*" ([^*] | "*" [^/])* "*/";
    
    // Unicode identifier parts (simplified)
    UnicodeIDStart = [a-zA-Z_$] | [\u00aa-\uffff];
    UnicodeIDContinue = [a-zA-Z0-9_$] | [\u00aa-\uffff];
    Identifier = UnicodeIDStart UnicodeIDContinue*;
    
    // Numeric literals
    DecimalDigit = [0-9];
    NonZeroDigit = [1-9];
    DecimalIntegerLiteral = "0" | NonZeroDigit DecimalDigit*;
    ExponentPart = [eE] [+-]? DecimalDigit+;
    DecimalLiteral = DecimalIntegerLiteral "." DecimalDigit* ExponentPart?
                   | "." DecimalDigit+ ExponentPart?
                   | DecimalIntegerLiteral ExponentPart?;
    HexDigit = [0-9a-fA-F];
    HexIntegerLiteral = "0" [xX] HexDigit+;
    BinaryDigit = [01];
    BinaryIntegerLiteral = "0" [bB] BinaryDigit+;
    OctalDigit = [0-7];
    OctalIntegerLiteral = "0" [oO] OctalDigit+;
    BigIntLiteral = (DecimalIntegerLiteral | HexIntegerLiteral | BinaryIntegerLiteral | OctalIntegerLiteral) "n";
    
    // String literals
    DoubleStringChar = [^"\\\n\r] | "\\" .;
    SingleStringChar = [^'\\\n\r] | "\\" .;
    StringLiteral = '"' DoubleStringChar* '"' | "'" SingleStringChar* "'";
    
    // Template literals (basic support)
    TemplateLiteral = "`" ([^`\\] | "\\" .)* "`";
    
    // Regular expression literal (simplified)
    RegularExpressionLiteral = "/" ([^\\/\n\r] | "\\" [^\n\r])+ "/" [gimsuvy]*;
    
    // Keywords
    "async"         { save_token(); last_token = ASYNC; return ASYNC; }
    "await"         { save_token(); last_token = AWAIT; return AWAIT; }
    "break"         { save_token(); last_token = BREAK; return BREAK; }
    "case"          { save_token(); last_token = CASE; return CASE; }
    "catch"         { save_token(); last_token = CATCH; return CATCH; }
    "class"         { save_token(); last_token = CLASS; return CLASS; }
    "const"         { save_token(); last_token = CONST; return CONST; }
    "continue"      { save_token(); last_token = CONTINUE; return CONTINUE; }
    "debugger"      { save_token(); last_token = DEBUGGER; return DEBUGGER; }
    "default"       { save_token(); last_token = DEFAULT; return DEFAULT; }
    "delete"        { save_token(); last_token = DELETE; return DELETE; }
    "do"            { save_token(); last_token = DO; return DO; }
    "else"          { save_token(); last_token = ELSE; return ELSE; }
    "export"        { save_token(); last_token = EXPORT; return EXPORT; }
    "extends"       { save_token(); last_token = EXTENDS; return EXTENDS; }
    "finally"       { save_token(); last_token = FINALLY; return FINALLY; }
    "for"           { save_token(); in_for_header = 1; last_token = FOR; return FOR; }
    "function"      { save_token(); last_token = FUNCTION; return FUNCTION; }
    "if"            { save_token(); last_token = IF; return IF; }
    "import"        { save_token(); last_token = IMPORT; return IMPORT; }
    "in"            { save_token(); last_token = IN; return IN; }
    "instanceof"    { save_token(); last_token = INSTANCEOF; return INSTANCEOF; }
    "let"           { save_token(); last_token = LET; return LET; }
    "new"           { save_token(); last_token = NEW; return NEW; }
    "return"        { save_token(); last_token = RETURN; return RETURN; }
    "super"         { save_token(); last_token = SUPER; return SUPER; }
    "switch"        { save_token(); last_token = SWITCH; return SWITCH; }
    "this"          { save_token(); last_token = THIS; return THIS; }
    "throw"         { save_token(); last_token = THROW; return THROW; }
    "try"           { save_token(); last_token = TRY; return TRY; }
    "typeof"        { save_token(); last_token = TYPEOF; return TYPEOF; }
    "var"           { save_token(); last_token = VAR; return VAR; }
    "void"          { save_token(); last_token = VOID; return VOID; }
    "while"         { save_token(); last_token = WHILE; return WHILE; }
    "with"          { save_token(); last_token = WITH; return WITH; }
    "yield"         { save_token(); last_token = YIELD; return YIELD; }
    
    // Null, Boolean literals
    "null"          { save_token(); yylval.str = strdup(yytext); last_token = NULL_LITERAL; return NULL_LITERAL; }
    "true"          { save_token(); yylval.str = strdup(yytext); last_token = TRUE; return TRUE; }
    "false"         { save_token(); yylval.str = strdup(yytext); last_token = FALSE; return FALSE; }
    
    // Punctuators
    "{"             { save_token(); last_token = '{'; return '{'; }
    "}"             { save_token(); seen_newline = 0; last_token = '}'; return '}'; }
    "("             { save_token(); paren_depth++; last_token = '('; return '('; }
    ")"             { save_token(); paren_depth--; if (paren_depth == 0 && in_for_header) in_for_header = 0; last_token = ')'; return ')'; }
    "["             { save_token(); last_token = '['; return '['; }
    "]"             { save_token(); last_token = ']'; return ']'; }
    ";"             { save_token(); seen_newline = 0; last_token = ';'; return ';'; }
    ","             { save_token(); last_token = ','; return ','; }
    "."             { save_token(); last_token = '.'; return '.'; }
    "..."           { save_token(); last_token = ELLIPSIS; return ELLIPSIS; }
    "?."            { save_token(); last_token = OPTIONAL_CHAIN; return OPTIONAL_CHAIN; }
    "??"            { save_token(); last_token = NULLISH_COALESCING; return NULLISH_COALESCING; }
    
    // Operators
    "+"             { save_token(); last_token = '+'; return '+'; }
    "-"             { save_token(); last_token = '-'; return '-'; }
    "*"             { save_token(); last_token = '*'; return '*'; }
    "/"             { save_token(); last_token = '/'; return '/'; }
    "%"             { save_token(); last_token = '%'; return '%'; }
    "**"            { save_token(); last_token = EXPONENT; return EXPONENT; }
    
    "++"            { save_token(); last_token = INCREMENT; return INCREMENT; }
    "--"            { save_token(); last_token = DECREMENT; return DECREMENT; }
    
    "<<"            { save_token(); last_token = LEFT_SHIFT; return LEFT_SHIFT; }
    ">>"            { save_token(); last_token = RIGHT_SHIFT; return RIGHT_SHIFT; }
    ">>>"           { save_token(); last_token = UNSIGNED_RIGHT_SHIFT; return UNSIGNED_RIGHT_SHIFT; }
    
    "<"             { save_token(); last_token = '<'; return '<'; }
    ">"             { save_token(); last_token = '>'; return '>'; }
    "<="            { save_token(); last_token = LESS_EQUAL; return LESS_EQUAL; }
    ">="            { save_token(); last_token = GREATER_EQUAL; return GREATER_EQUAL; }
    
    "=="            { save_token(); last_token = EQUAL; return EQUAL; }
    "!="            { save_token(); last_token = NOT_EQUAL; return NOT_EQUAL; }
    "==="           { save_token(); last_token = STRICT_EQUAL; return STRICT_EQUAL; }
    "!=="           { save_token(); last_token = STRICT_NOT_EQUAL; return STRICT_NOT_EQUAL; }
    
    "&"             { save_token(); last_token = '&'; return '&'; }
    "|"             { save_token(); last_token = '|'; return '|'; }
    "^"             { save_token(); last_token = '^'; return '^'; }
    "~"             { save_token(); last_token = '~'; return '~'; }
    "!"             { save_token(); last_token = '!'; return '!'; }
    
    "&&"            { save_token(); last_token = LOGICAL_AND; return LOGICAL_AND; }
    "||"            { save_token(); last_token = LOGICAL_OR; return LOGICAL_OR; }
    
    "?"             { save_token(); last_token = '?'; return '?'; }
    ":"             { save_token(); last_token = ':'; return ':'; }
    
    "="             { save_token(); last_token = '='; return '='; }
    "+="            { save_token(); last_token = PLUS_ASSIGN; return PLUS_ASSIGN; }
    "-="            { save_token(); last_token = MINUS_ASSIGN; return MINUS_ASSIGN; }
    "*="            { save_token(); last_token = MULTIPLY_ASSIGN; return MULTIPLY_ASSIGN; }
    "/="            { save_token(); last_token = DIVIDE_ASSIGN; return DIVIDE_ASSIGN; }
    "%="            { save_token(); last_token = MODULO_ASSIGN; return MODULO_ASSIGN; }
    "**="           { save_token(); last_token = EXPONENT_ASSIGN; return EXPONENT_ASSIGN; }
    "<<="           { save_token(); last_token = LEFT_SHIFT_ASSIGN; return LEFT_SHIFT_ASSIGN; }
    ">>="           { save_token(); last_token = RIGHT_SHIFT_ASSIGN; return RIGHT_SHIFT_ASSIGN; }
    ">>>="          { save_token(); last_token = UNSIGNED_RIGHT_SHIFT_ASSIGN; return UNSIGNED_RIGHT_SHIFT_ASSIGN; }
    "&="            { save_token(); last_token = AND_ASSIGN; return AND_ASSIGN; }
    "|="            { save_token(); last_token = OR_ASSIGN; return OR_ASSIGN; }
    "^="            { save_token(); last_token = XOR_ASSIGN; return XOR_ASSIGN; }
    "&&="           { save_token(); last_token = LOGICAL_AND_ASSIGN; return LOGICAL_AND_ASSIGN; }
    "||="           { save_token(); last_token = LOGICAL_OR_ASSIGN; return LOGICAL_OR_ASSIGN; }
    "??="           { save_token(); last_token = NULLISH_ASSIGN; return NULLISH_ASSIGN; }
    
    "=>"            { save_token(); last_token = ARROW; return ARROW; }
    
    // Numeric literals
    BigIntLiteral        { save_token(); yylval.str = strdup(yytext); last_token = BIGINT_LITERAL; return BIGINT_LITERAL; }
    HexIntegerLiteral    { save_token(); yylval.str = strdup(yytext); last_token = NUMERIC_LITERAL; return NUMERIC_LITERAL; }
    BinaryIntegerLiteral { save_token(); yylval.str = strdup(yytext); last_token = NUMERIC_LITERAL; return NUMERIC_LITERAL; }
    OctalIntegerLiteral  { save_token(); yylval.str = strdup(yytext); last_token = NUMERIC_LITERAL; return NUMERIC_LITERAL; }
    DecimalLiteral       { save_token(); yylval.str = strdup(yytext); last_token = NUMERIC_LITERAL; return NUMERIC_LITERAL; }
    
    // String literals
    StringLiteral        { save_token(); yylval.str = strdup(yytext); last_token = STRING_LITERAL; return STRING_LITERAL; }
    TemplateLiteral      { save_token(); yylval.str = strdup(yytext); last_token = TEMPLATE_LITERAL; return TEMPLATE_LITERAL; }
    
    // Regular expression
    RegularExpressionLiteral { save_token(); yylval.str = strdup(yytext); last_token = REGEX_LITERAL; return REGEX_LITERAL; }
    
    // Identifier
    Identifier           { save_token(); yylval.str = strdup(yytext); last_token = IDENTIFIER; return IDENTIFIER; }
    
    // Line terminators - trigger ASI check
    LineTerminatorSequence {
        yylineno++;
        yycolno = 1;
        seen_newline = 1;
        goto yylex_start;  /* Continue to next token */
    }
    
    // Whitespace - ignore
    WhiteSpace+ {
        yycolno += (input_cursor - input_token);
        goto yylex_start;
    }
    
    // Comments - ignore
    SingleLineComment {
        goto yylex_start;
    }
    
    MultiLineComment {
        const char *p;
        for (p = input_token; p < input_cursor; p++) {
            if (*p == '\n' || *p == '\r') {
                yylineno++;
                seen_newline = 1;
            }
        }
        goto yylex_start;
    }
    
    // End of input
    [\x00] {
        /* Check for ASI at EOF */
        if (can_insert_semicolon(0) && last_token != ';' && last_token != '}') {
            last_token = ';';
            return ';';
        }
        return 0;
    }
    
    // Invalid character
    * {
        fprintf(stderr, "Error: Invalid character '%c' at line %d, column %d\n", 
                *input_token, yylineno, yycolno);
        exit(1);
    }
    */

yylex_start:
    goto *&&yylex_start_impl;
yylex_start_impl:
    return yylex();
}

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d, column %d: %s\n", yylineno, yycolno, s);
    if (yytext) {
        fprintf(stderr, "Near token: '%s'\n", yytext);
    }
}