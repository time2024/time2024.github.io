/* parser.y - JavaScript Grammar using Bison */

/* CRITICAL: 这部分会被包含到parser.tab.h中，确保ASTNode类型在头文件中可用 */
%code requires {
    #include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h" 

extern int yylex();
extern void yyerror(const char *s);
extern int yylineno;
extern int yycolno;
extern char *yytext;

ASTNode *root = NULL;
int parse_error_count = 0;
%}

%union {
    char *str;
    ASTNode *node;
}

/* Keywords */
%token ASYNC AWAIT BREAK CASE CATCH CLASS CONST CONTINUE DEBUGGER DEFAULT
%token DELETE DO ELSE EXPORT EXTENDS FINALLY FOR FUNCTION IF IMPORT IN
%token INSTANCEOF LET NEW RETURN SUPER SWITCH THIS THROW TRY TYPEOF VAR
%token VOID WHILE WITH YIELD

/* Literals */
%token <str> IDENTIFIER NUMERIC_LITERAL STRING_LITERAL BIGINT_LITERAL
%token <str> TEMPLATE_LITERAL REGEX_LITERAL NULL_LITERAL
%token TRUE FALSE

/* Operators */
%token INCREMENT DECREMENT
%token LEFT_SHIFT RIGHT_SHIFT UNSIGNED_RIGHT_SHIFT
%token LESS_EQUAL GREATER_EQUAL
%token EQUAL NOT_EQUAL STRICT_EQUAL STRICT_NOT_EQUAL
%token LOGICAL_AND LOGICAL_OR
%token PLUS_ASSIGN MINUS_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN
%token EXPONENT_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN
%token UNSIGNED_RIGHT_SHIFT_ASSIGN AND_ASSIGN OR_ASSIGN XOR_ASSIGN
%token LOGICAL_AND_ASSIGN LOGICAL_OR_ASSIGN NULLISH_ASSIGN
%token ARROW ELLIPSIS OPTIONAL_CHAIN NULLISH_COALESCING EXPONENT
%token POSTINC POSTDEC

/* Non-terminals */
%type <node> Program StatementList Statement
%type <node> VariableStatement VariableDeclarationList VariableDeclaration
%type <node> EmptyStatement ExpressionStatement IfStatement
%type <node> IterationStatement ContinueStatement BreakStatement
%type <node> ReturnStatement ThrowStatement
%type <node> BlockStatement Block StatementListItem
%type <node> FunctionDeclaration FormalParameters FormalParameterList
%type <node> Expression AssignmentExpression ConditionalExpression
%type <node> LogicalORExpression LogicalANDExpression BitwiseORExpression
%type <node> BitwiseXORExpression BitwiseANDExpression EqualityExpression
%type <node> RelationalExpression ShiftExpression AdditiveExpression
%type <node> MultiplicativeExpression ExponentiationExpression
%type <node> UnaryExpression UpdateExpression LeftHandSideExpression
%type <node> CallExpression MemberExpression NewExpression
%type <node> PrimaryExpression Literal Arguments ArgumentList
%type <node> ArrayLiteral ElementList ObjectLiteral PropertyDefinitionList PropertyDefinition

/* Operator precedence (lowest to highest) */
%right '=' PLUS_ASSIGN MINUS_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN
%right EXPONENT_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN
%right UNSIGNED_RIGHT_SHIFT_ASSIGN AND_ASSIGN OR_ASSIGN XOR_ASSIGN
%right LOGICAL_AND_ASSIGN LOGICAL_OR_ASSIGN NULLISH_ASSIGN
%right '?' ':'
%left LOGICAL_OR
%left LOGICAL_AND
%left '|'
%left '^'
%left '&'
%left EQUAL NOT_EQUAL STRICT_EQUAL STRICT_NOT_EQUAL
%left '<' '>' LESS_EQUAL GREATER_EQUAL INSTANCEOF IN
%left LEFT_SHIFT RIGHT_SHIFT UNSIGNED_RIGHT_SHIFT
%left '+' '-'
%left '*' '/' '%'
%right EXPONENT
%right '!' '~' TYPEOF VOID DELETE
%right INCREMENT DECREMENT
%left '.' '[' '('

%start Program

%%

/* ===== Program Structure ===== */

Program:
    StatementList {
        root = create_program($1);
        $$ = root;
    }
    | /* empty */ {
        root = create_program(create_statement_list());
        $$ = root;
    }
    ;

StatementList:
    StatementListItem {
        $$ = create_statement_list();
        $$ = append_statement($$, $1);
    }
    | StatementList StatementListItem {
        $$ = append_statement($1, $2);
    }
    ;

StatementListItem:
    Statement { $$ = $1; }
    | FunctionDeclaration { $$ = $1; }
    ;

/* ===== Statements ===== */

Statement:
    BlockStatement { $$ = $1; }
    | VariableStatement { $$ = $1; }
    | EmptyStatement { $$ = $1; }
    | ExpressionStatement { $$ = $1; }
    | IfStatement { $$ = $1; }
    | IterationStatement { $$ = $1; }
    | ContinueStatement { $$ = $1; }
    | BreakStatement { $$ = $1; }
    | ReturnStatement { $$ = $1; }
    | ThrowStatement { $$ = $1; }
    ;

BlockStatement:
    Block { $$ = $1; }
    ;

Block:
    '{' '}' {
        $$ = create_block_statement(create_statement_list());
    }
    | '{' StatementList '}' {
        $$ = create_block_statement($2);
    }
    ;

/* Variable Declarations */

VariableStatement:
    VAR VariableDeclarationList ';' {
        $$ = $2;
    }
    | VAR VariableDeclarationList {
        /* ASI: semicolon automatically inserted */
        $$ = $2;
    }
    | LET VariableDeclarationList ';' {
        $$ = $2;
    }
    | LET VariableDeclarationList {
        $$ = $2;
    }
    | CONST VariableDeclarationList ';' {
        $$ = $2;
    }
    | CONST VariableDeclarationList {
        $$ = $2;
    }
    ;

VariableDeclarationList:
    VariableDeclaration {
        $$ = create_statement_list();
        $$ = append_statement($$, $1);
    }
    | VariableDeclarationList ',' VariableDeclaration {
        $$ = append_statement($1, $3);
    }
    ;

VariableDeclaration:
    IDENTIFIER {
        $$ = create_var_declaration($1, NULL);
        free($1);
    }
    | IDENTIFIER '=' AssignmentExpression {
        $$ = create_var_declaration($1, $3);
        free($1);
    }
    ;

/* Empty Statement */

EmptyStatement:
    ';' {
        $$ = create_empty_statement();
    }
    ;

/* Expression Statement */

ExpressionStatement:
    Expression ';' {
        $$ = create_expression_statement($1);
    }
    | Expression {
        /* ASI: semicolon automatically inserted */
        $$ = create_expression_statement($1);
    }
    ;

/* If Statement */

IfStatement:
    IF '(' Expression ')' Statement {
        $$ = create_if_statement($3, $5, NULL);
    }
    | IF '(' Expression ')' Statement ELSE Statement {
        $$ = create_if_statement($3, $5, $7);
    }
    ;

/* Iteration Statements */

IterationStatement:
    WHILE '(' Expression ')' Statement {
        $$ = create_while_statement($3, $5);
    }
    | DO Statement WHILE '(' Expression ')' ';' {
        $$ = create_while_statement($5, $2);
    }
    | DO Statement WHILE '(' Expression ')' {
        /* ASI after do-while */
        $$ = create_while_statement($5, $2);
    }
    | FOR '(' ';' ';' ')' Statement {
        $$ = create_for_statement(NULL, NULL, NULL, $6);
    }
    | FOR '(' ';' ';' Expression ')' Statement {
        $$ = create_for_statement(NULL, NULL, $5, $7);
    }
    | FOR '(' ';' Expression ';' ')' Statement {
        $$ = create_for_statement(NULL, $4, NULL, $7);
    }
    | FOR '(' ';' Expression ';' Expression ')' Statement {
        $$ = create_for_statement(NULL, $4, $6, $8);
    }
    | FOR '(' Expression ';' ';' ')' Statement {
        $$ = create_for_statement($3, NULL, NULL, $7);
    }
    | FOR '(' Expression ';' ';' Expression ')' Statement {
        $$ = create_for_statement($3, NULL, $6, $8);
    }
    | FOR '(' Expression ';' Expression ';' ')' Statement {
        $$ = create_for_statement($3, $5, NULL, $8);
    }
    | FOR '(' Expression ';' Expression ';' Expression ')' Statement {
        $$ = create_for_statement($3, $5, $7, $9);
    }
    | FOR '(' VAR VariableDeclaration ';' ';' ')' Statement {
        $$ = create_for_statement($4, NULL, NULL, $8);
    }
    | FOR '(' VAR VariableDeclaration ';' ';' Expression ')' Statement {
        $$ = create_for_statement($4, NULL, $7, $9);
    }
    | FOR '(' VAR VariableDeclaration ';' Expression ';' ')' Statement {
        $$ = create_for_statement($4, $6, NULL, $9);
    }
    | FOR '(' VAR VariableDeclaration ';' Expression ';' Expression ')' Statement {
        $$ = create_for_statement($4, $6, $8, $10);
    }
    ;

/* Continue Statement */

ContinueStatement:
    CONTINUE ';' {
        $$ = create_continue_statement(NULL);
    }
    | CONTINUE {
        /* ASI: restricted production */
        $$ = create_continue_statement(NULL);
    }
    | CONTINUE IDENTIFIER ';' {
        $$ = create_continue_statement(create_identifier($2));
        free($2);
    }
    | CONTINUE IDENTIFIER {
        $$ = create_continue_statement(create_identifier($2));
        free($2);
    }
    ;

/* Break Statement */

BreakStatement:
    BREAK ';' {
        $$ = create_break_statement(NULL);
    }
    | BREAK {
        /* ASI: restricted production */
        $$ = create_break_statement(NULL);
    }
    | BREAK IDENTIFIER ';' {
        $$ = create_break_statement(create_identifier($2));
        free($2);
    }
    | BREAK IDENTIFIER {
        $$ = create_break_statement(create_identifier($2));
        free($2);
    }
    ;

/* Return Statement */

ReturnStatement:
    RETURN ';' {
        $$ = create_return_statement(NULL);
    }
    | RETURN {
        /* ASI: restricted production - return with no expression */
        $$ = create_return_statement(NULL);
    }
    | RETURN Expression ';' {
        $$ = create_return_statement($2);
    }
    | RETURN Expression {
        /* Note: This should NOT trigger ASI if expression is on same line */
        /* The lexer handles this via seen_newline flag */
        $$ = create_return_statement($2);
    }
    ;

/* Throw Statement */

ThrowStatement:
    THROW Expression ';' {
        $$ = create_throw_statement($2);
    }
    | THROW Expression {
        /* ASI: restricted production */
        $$ = create_throw_statement($2);
    }
    ;

/* ===== Function Declaration ===== */

FunctionDeclaration:
    FUNCTION IDENTIFIER '(' ')' Block {
        $$ = create_function_declaration($2, NULL, $5);
        free($2);
    }
    | FUNCTION IDENTIFIER '(' FormalParameters ')' Block {
        $$ = create_function_declaration($2, $4, $6);
        free($2);
    }
    | ASYNC FUNCTION IDENTIFIER '(' ')' Block {
        $$ = create_async_function_declaration($3, NULL, $6);
        free($3);
    }
    | ASYNC FUNCTION IDENTIFIER '(' FormalParameters ')' Block {
        $$ = create_async_function_declaration($3, $5, $7);
        free($3);
    }
    ;

FormalParameters:
    FormalParameterList { $$ = $1; }
    ;

FormalParameterList:
    IDENTIFIER {
        $$ = create_statement_list();
        $$ = append_statement($$, create_identifier($1));
        free($1);
    }
    | FormalParameterList ',' IDENTIFIER {
        $$ = append_statement($1, create_identifier($3));
        free($3);
    }
    ;

/* ===== Expressions ===== */

Expression:
    AssignmentExpression { $$ = $1; }
    | Expression ',' AssignmentExpression {
        $$ = create_comma_expression($1, $3);
    }
    ;

AssignmentExpression:
    ConditionalExpression { $$ = $1; }
    | LeftHandSideExpression '=' AssignmentExpression {
        $$ = create_assignment_expression("=", $1, $3);
    }
    | LeftHandSideExpression PLUS_ASSIGN AssignmentExpression {
        $$ = create_assignment_expression("+=", $1, $3);
    }
    | LeftHandSideExpression MINUS_ASSIGN AssignmentExpression {
        $$ = create_assignment_expression("-=", $1, $3);
    }
    | LeftHandSideExpression MULTIPLY_ASSIGN AssignmentExpression {
        $$ = create_assignment_expression("*=", $1, $3);
    }
    | LeftHandSideExpression DIVIDE_ASSIGN AssignmentExpression {
        $$ = create_assignment_expression("/=", $1, $3);
    }
    | LeftHandSideExpression MODULO_ASSIGN AssignmentExpression {
        $$ = create_assignment_expression("%=", $1, $3);
    }
    ;

ConditionalExpression:
    LogicalORExpression { $$ = $1; }
    | LogicalORExpression '?' AssignmentExpression ':' AssignmentExpression {
        $$ = create_conditional_expression($1, $3, $5);
    }
    ;

LogicalORExpression:
    LogicalANDExpression { $$ = $1; }
    | LogicalORExpression LOGICAL_OR LogicalANDExpression {
        $$ = create_binary_expression("||", $1, $3);
    }
    ;

LogicalANDExpression:
    BitwiseORExpression { $$ = $1; }
    | LogicalANDExpression LOGICAL_AND BitwiseORExpression {
        $$ = create_binary_expression("&&", $1, $3);
    }
    ;

BitwiseORExpression:
    BitwiseXORExpression { $$ = $1; }
    | BitwiseORExpression '|' BitwiseXORExpression {
        $$ = create_binary_expression("|", $1, $3);
    }
    ;

BitwiseXORExpression:
    BitwiseANDExpression { $$ = $1; }
    | BitwiseXORExpression '^' BitwiseANDExpression {
        $$ = create_binary_expression("^", $1, $3);
    }
    ;

BitwiseANDExpression:
    EqualityExpression { $$ = $1; }
    | BitwiseANDExpression '&' EqualityExpression {
        $$ = create_binary_expression("&", $1, $3);
    }
    ;

EqualityExpression:
    RelationalExpression { $$ = $1; }
    | EqualityExpression EQUAL RelationalExpression {
        $$ = create_binary_expression("==", $1, $3);
    }
    | EqualityExpression NOT_EQUAL RelationalExpression {
        $$ = create_binary_expression("!=", $1, $3);
    }
    | EqualityExpression STRICT_EQUAL RelationalExpression {
        $$ = create_binary_expression("===", $1, $3);
    }
    | EqualityExpression STRICT_NOT_EQUAL RelationalExpression {
        $$ = create_binary_expression("!==", $1, $3);
    }
    ;

RelationalExpression:
    ShiftExpression { $$ = $1; }
    | RelationalExpression '<' ShiftExpression {
        $$ = create_binary_expression("<", $1, $3);
    }
    | RelationalExpression '>' ShiftExpression {
        $$ = create_binary_expression(">", $1, $3);
    }
    | RelationalExpression LESS_EQUAL ShiftExpression {
        $$ = create_binary_expression("<=", $1, $3);
    }
    | RelationalExpression GREATER_EQUAL ShiftExpression {
        $$ = create_binary_expression(">=", $1, $3);
    }
    | RelationalExpression INSTANCEOF ShiftExpression {
        $$ = create_binary_expression("instanceof", $1, $3);
    }
    | RelationalExpression IN ShiftExpression {
        $$ = create_binary_expression("in", $1, $3);
    }
    ;

ShiftExpression:
    AdditiveExpression { $$ = $1; }
    | ShiftExpression LEFT_SHIFT AdditiveExpression {
        $$ = create_binary_expression("<<", $1, $3);
    }
    | ShiftExpression RIGHT_SHIFT AdditiveExpression {
        $$ = create_binary_expression(">>", $1, $3);
    }
    | ShiftExpression UNSIGNED_RIGHT_SHIFT AdditiveExpression {
        $$ = create_binary_expression(">>>", $1, $3);
    }
    ;

AdditiveExpression:
    MultiplicativeExpression { $$ = $1; }
    | AdditiveExpression '+' MultiplicativeExpression {
        $$ = create_binary_expression("+", $1, $3);
    }
    | AdditiveExpression '-' MultiplicativeExpression {
        $$ = create_binary_expression("-", $1, $3);
    }
    ;

MultiplicativeExpression:
    ExponentiationExpression { $$ = $1; }
    | MultiplicativeExpression '*' ExponentiationExpression {
        $$ = create_binary_expression("*", $1, $3);
    }
    | MultiplicativeExpression '/' ExponentiationExpression {
        $$ = create_binary_expression("/", $1, $3);
    }
    | MultiplicativeExpression '%' ExponentiationExpression {
        $$ = create_binary_expression("%", $1, $3);
    }
    ;

ExponentiationExpression:
    UnaryExpression { $$ = $1; }
    | UpdateExpression EXPONENT ExponentiationExpression {
        $$ = create_binary_expression("**", $1, $3);
    }
    ;

UnaryExpression:
    UpdateExpression { $$ = $1; }
    | DELETE UnaryExpression {
        $$ = create_unary_expression("delete", $2);
    }
    | VOID UnaryExpression {
        $$ = create_unary_expression("void", $2);
    }
    | TYPEOF UnaryExpression {
        $$ = create_unary_expression("typeof", $2);
    }
    | '+' UnaryExpression {
        $$ = create_unary_expression("+", $2);
    }
    | '-' UnaryExpression {
        $$ = create_unary_expression("-", $2);
    }
    | '~' UnaryExpression {
        $$ = create_unary_expression("~", $2);
    }
    | '!' UnaryExpression {
        $$ = create_unary_expression("!", $2);
    }
    ;

UpdateExpression:
    LeftHandSideExpression { $$ = $1; }
    | LeftHandSideExpression INCREMENT {
        /* Postfix increment */
        $$ = create_update_expression("++", $1, 0);
    }
    | LeftHandSideExpression DECREMENT {
        /* Postfix decrement */
        $$ = create_update_expression("--", $1, 0);
    }
    | INCREMENT UnaryExpression {
        /* Prefix increment */
        $$ = create_update_expression("++", $2, 1);
    }
    | DECREMENT UnaryExpression {
        /* Prefix decrement */
        $$ = create_update_expression("--", $2, 1);
    }
    ;

LeftHandSideExpression:
    NewExpression { $$ = $1; }
    | CallExpression { $$ = $1; }
    ;

CallExpression:
    MemberExpression Arguments {
        $$ = create_call_expression($1, $2);
    }
    | CallExpression Arguments {
        $$ = create_call_expression($1, $2);
    }
    | CallExpression '[' Expression ']' {
        $$ = create_member_expression($1, $3, 1);
    }
    | CallExpression '.' IDENTIFIER {
        $$ = create_member_expression($1, create_identifier($3), 0);
        free($3);
    }
    ;

NewExpression:
    MemberExpression { $$ = $1; }
    | NEW NewExpression {
        $$ = create_new_expression($2);
    }
    ;

MemberExpression:
    PrimaryExpression { $$ = $1; }
    | MemberExpression '[' Expression ']' {
        $$ = create_member_expression($1, $3, 1);
    }
    | MemberExpression '.' IDENTIFIER {
        $$ = create_member_expression($1, create_identifier($3), 0);
        free($3);
    }
    | NEW MemberExpression Arguments {
        $$ = create_call_expression(create_new_expression($2), $3);
    }
    ;

PrimaryExpression:
    THIS {
        $$ = create_this_expression();
    }
    | IDENTIFIER {
        $$ = create_identifier($1);
        free($1);
    }
    | Literal { $$ = $1; }
    | ArrayLiteral { $$ = $1; }
    | ObjectLiteral { $$ = $1; }
    | '(' Expression ')' {
        $$ = $2;
    }
    ;

/* ===== Literals ===== */

Literal:
    NULL_LITERAL {
        $$ = create_null_literal();
    }
    | TRUE {
        $$ = create_boolean_literal(1);
    }
    | FALSE {
        $$ = create_boolean_literal(0);
    }
    | NUMERIC_LITERAL {
        $$ = create_number_literal($1);
        free($1);
    }
    | STRING_LITERAL {
        $$ = create_string_literal($1);
        free($1);
    }
    | BIGINT_LITERAL {
        $$ = create_number_literal($1);
        free($1);
    }
    ;

/* Array Literal */

ArrayLiteral:
    '[' ']' {
        $$ = create_array_expression(create_statement_list());
    }
    | '[' ElementList ']' {
        $$ = create_array_expression($2);
    }
    | '[' ElementList ',' ']' {
        $$ = create_array_expression($2);
    }
    ;

ElementList:
    AssignmentExpression {
        $$ = create_statement_list();
        $$ = append_statement($$, $1);
    }
    | ElementList ',' AssignmentExpression {
        $$ = append_statement($1, $3);
    }
    ;

/* Object Literal */

ObjectLiteral:
    '{' '}' {
        $$ = create_object_expression(create_statement_list());
    }
    | '{' PropertyDefinitionList '}' {
        $$ = create_object_expression($2);
    }
    | '{' PropertyDefinitionList ',' '}' {
        $$ = create_object_expression($2);
    }
    ;

PropertyDefinitionList:
    PropertyDefinition {
        $$ = create_statement_list();
        $$ = append_statement($$, $1);
    }
    | PropertyDefinitionList ',' PropertyDefinition {
        $$ = append_statement($1, $3);
    }
    ;

PropertyDefinition:
    IDENTIFIER ':' AssignmentExpression {
        ASTNode *key = create_identifier($1);
        $$ = create_binary_expression(":", key, $3);
        free($1);
    }
    | STRING_LITERAL ':' AssignmentExpression {
        ASTNode *key = create_string_literal($1);
        $$ = create_binary_expression(":", key, $3);
        free($1);
    }
    | NUMERIC_LITERAL ':' AssignmentExpression {
        ASTNode *key = create_number_literal($1);
        $$ = create_binary_expression(":", key, $3);
        free($1);
    }
    ;

/* Arguments */

Arguments:
    '(' ')' {
        $$ = create_statement_list();
    }
    | '(' ArgumentList ')' {
        $$ = $2;
    }
    ;

ArgumentList:
    AssignmentExpression {
        $$ = create_statement_list();
        $$ = append_statement($$, $1);
    }
    | ArgumentList ',' AssignmentExpression {
        $$ = append_statement($1, $3);
    }
    ;

%%