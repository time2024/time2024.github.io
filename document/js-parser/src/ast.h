/* ast.h - Abstract Syntax Tree Node Definitions */

#ifndef AST_H
#define AST_H

/* Forward declaration */
struct ASTNode;
typedef struct ASTNode ASTNode;

/* AST Node Types */
typedef enum {
    NODE_PROGRAM,
    NODE_STATEMENT_LIST,
    NODE_EXPRESSION_STMT,
    NODE_VAR_DECLARATION,
    NODE_LET_DECLARATION,
    NODE_CONST_DECLARATION,
    NODE_FUNCTION_DECLARATION,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_BREAK_STATEMENT,
    NODE_CONTINUE_STATEMENT,
    NODE_THROW_STATEMENT,
    NODE_BLOCK_STATEMENT,
    NODE_EMPTY_STATEMENT,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_UPDATE_EXPR,
    NODE_ASSIGNMENT_EXPR,
    NODE_CONDITIONAL_EXPR,
    NODE_CALL_EXPR,
    NODE_MEMBER_EXPR,
    NODE_NEW_EXPR,
    NODE_COMMA_EXPR,
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_THIS,
    NODE_ARRAY_EXPR,
    NODE_OBJECT_EXPR
} NodeType;

/* Program Node */
typedef struct Program {
    ASTNode *body;  /* StatementList */
} Program;

/* Statement List */
typedef struct StatementList {
    ASTNode **statements;
    int count;
    int capacity;
} StatementList;

/* Variable Declaration */
typedef struct VarDeclaration {
    char *kind;  /* "var", "let", "const" */
    char *name;
    ASTNode *init;  /* initializer expression */
} VarDeclaration;

/* Function Declaration */
typedef struct FunctionDeclaration {
    char *name;
    ASTNode *params;  /* parameter list */
    ASTNode *body;    /* block statement */
    int is_async;
    int is_generator;
} FunctionDeclaration;

/* If Statement */
typedef struct IfStatement {
    ASTNode *test;
    ASTNode *consequent;
    ASTNode *alternate;
} IfStatement;

/* While Statement */
typedef struct WhileStatement {
    ASTNode *test;
    ASTNode *body;
} WhileStatement;

/* For Statement */
typedef struct ForStatement {
    ASTNode *init;
    ASTNode *test;
    ASTNode *update;
    ASTNode *body;
} ForStatement;

/* Return Statement */
typedef struct ReturnStatement {
    ASTNode *argument;
} ReturnStatement;

/* Binary Expression */
typedef struct BinaryExpression {
    char *operator;
    ASTNode *left;
    ASTNode *right;
} BinaryExpression;

/* Unary Expression */
typedef struct UnaryExpression {
    char *operator;
    ASTNode *argument;
    int prefix;
} UnaryExpression;

/* Assignment Expression */
typedef struct AssignmentExpression {
    char *operator;
    ASTNode *left;
    ASTNode *right;
} AssignmentExpression;

/* Conditional Expression */
typedef struct ConditionalExpression {
    ASTNode *test;
    ASTNode *consequent;
    ASTNode *alternate;
} ConditionalExpression;

/* Call Expression */
typedef struct CallExpression {
    ASTNode *callee;
    ASTNode *arguments;
} CallExpression;

/* Member Expression */
typedef struct MemberExpression {
    ASTNode *object;
    ASTNode *property;
    int computed;  /* 0 for dot, 1 for bracket */
} MemberExpression;

/* Identifier */
typedef struct Identifier {
    char *name;
} Identifier;

/* Literal */
typedef struct Literal {
    enum { LIT_NULL, LIT_BOOLEAN, LIT_NUMBER, LIT_STRING, LIT_BIGINT } kind;
    union {
        int boolean;
        char *string;
    } value;
} Literal;

/* Base AST Node - 修复：统一使用void*避免类型冲突 */
struct ASTNode {
    NodeType type;
    int line;
    int column;
    union {
        Program *program;
        StatementList *stmt_list;
        VarDeclaration *var_decl;
        FunctionDeclaration *func_decl;
        IfStatement *if_stmt;
        WhileStatement *while_stmt;
        ForStatement *for_stmt;
        ReturnStatement *return_stmt;
        BinaryExpression *binary_expr;
        UnaryExpression *unary_expr;
        AssignmentExpression *assign_expr;
        ConditionalExpression *cond_expr;
        CallExpression *call_expr;
        MemberExpression *member_expr;
        Identifier *identifier;
        Literal *literal;
        ASTNode *child;  /* 通用子节点指针 */
        void *ptr;       /* 通用指针 */
    } data;
};

/* Function Prototypes */
ASTNode* create_program(ASTNode *body);
ASTNode* create_statement_list(void);
ASTNode* append_statement(ASTNode *list, ASTNode *stmt);
ASTNode* create_expression_statement(ASTNode *expr);
ASTNode* create_var_declaration(char *name, ASTNode *init);
ASTNode* create_let_declaration(char *name, ASTNode *init);
ASTNode* create_const_declaration(char *name, ASTNode *init);
ASTNode* create_function_declaration(char *name, ASTNode *params, ASTNode *body);
ASTNode* create_async_function_declaration(char *name, ASTNode *params, ASTNode *body);
ASTNode* create_if_statement(ASTNode *test, ASTNode *consequent, ASTNode *alternate);
ASTNode* create_while_statement(ASTNode *test, ASTNode *body);
ASTNode* create_for_statement(ASTNode *init, ASTNode *test, ASTNode *update, ASTNode *body);
ASTNode* create_for_statement_with_var(char *var, ASTNode *init, ASTNode *test, ASTNode *update, ASTNode *body);
ASTNode* create_return_statement(ASTNode *argument);
ASTNode* create_break_statement(ASTNode *label);
ASTNode* create_continue_statement(ASTNode *label);
ASTNode* create_throw_statement(ASTNode *argument);
ASTNode* create_block_statement(ASTNode *body);
ASTNode* create_empty_statement(void);
ASTNode* create_binary_expression(char *op, ASTNode *left, ASTNode *right);
ASTNode* create_unary_expression(char *op, ASTNode *arg);
ASTNode* create_update_expression(char *op, ASTNode *arg, int prefix);
ASTNode* create_assignment_expression(char *op, ASTNode *left, ASTNode *right);
ASTNode* create_conditional_expression(ASTNode *test, ASTNode *cons, ASTNode *alt);
ASTNode* create_call_expression(ASTNode *callee, ASTNode *args);
ASTNode* create_member_expression(ASTNode *obj, ASTNode *prop, int computed);
ASTNode* create_new_expression(ASTNode *callee);
ASTNode* create_comma_expression(ASTNode *left, ASTNode *right);
ASTNode* create_identifier(char *name);
ASTNode* create_null_literal(void);
ASTNode* create_boolean_literal(int value);
ASTNode* create_number_literal(char *value);
ASTNode* create_string_literal(char *value);
ASTNode* create_this_expression(void);
ASTNode* create_array_expression(ASTNode *elements);
ASTNode* create_object_expression(ASTNode *properties);

void print_ast(ASTNode *node, int indent);
void free_ast(ASTNode *node);

#endif /* AST_H */
