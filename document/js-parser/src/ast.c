/* ast.c - Complete AST Implementation for JavaScript Parser */

#define _POSIX_C_SOURCE 200809L  /* 启用strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* External variables from lexer */
extern int yylineno;
extern int yycolno;

/* ============================================================================
   HELPER FUNCTIONS
   ============================================================================ */

/* Helper to create base node */
static ASTNode* create_node(NodeType type) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: Out of memory when creating AST node\n");
        exit(1);
    }
    node->type = type;
    node->line = yylineno;
    node->column = yycolno;
    memset(&node->data, 0, sizeof(node->data));
    return node;
}

/* ============================================================================
   PROGRAM AND STATEMENT LIST
   ============================================================================ */

ASTNode* create_program(ASTNode *body) {
    ASTNode *node = create_node(NODE_PROGRAM);
    Program *prog = (Program*)malloc(sizeof(Program));
    if (!prog) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    prog->body = body;
    node->data.program = prog;
    return node;
}

ASTNode* create_statement_list(void) {
    ASTNode *node = create_node(NODE_STATEMENT_LIST);
    StatementList *list = (StatementList*)malloc(sizeof(StatementList));
    if (!list) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    list->capacity = 16;
    list->count = 0;
    list->statements = (ASTNode**)malloc(sizeof(ASTNode*) * list->capacity);
    if (!list->statements) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    node->data.stmt_list = list;
    return node;
}

ASTNode* append_statement(ASTNode *list, ASTNode *stmt) {
    if (!list) return stmt;
    if (!stmt) return list;
    
    if (list->type != NODE_STATEMENT_LIST) {
        fprintf(stderr, "Error: Trying to append to non-list node\n");
        return list;
    }
    
    StatementList *sl = list->data.stmt_list;
    if (sl->count >= sl->capacity) {
        sl->capacity *= 2;
        sl->statements = (ASTNode**)realloc(sl->statements, 
                                            sizeof(ASTNode*) * sl->capacity);
        if (!sl->statements) {
            fprintf(stderr, "Error: Out of memory\n");
            exit(1);
        }
    }
    sl->statements[sl->count++] = stmt;
    return list;
}

/* ============================================================================
   STATEMENT NODES
   ============================================================================ */

ASTNode* create_expression_statement(ASTNode *expr) {
    ASTNode *node = create_node(NODE_EXPRESSION_STMT);
    node->data.child = expr;
    return node;
}

ASTNode* create_var_declaration(char *name, ASTNode *init) {
    ASTNode *node = create_node(NODE_VAR_DECLARATION);
    VarDeclaration *decl = (VarDeclaration*)malloc(sizeof(VarDeclaration));
    if (!decl) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    decl->kind = "var";
    decl->name = strdup(name);
    decl->init = init;
    node->data.var_decl = decl;
    return node;
}

ASTNode* create_let_declaration(char *name, ASTNode *init) {
    ASTNode *node = create_node(NODE_LET_DECLARATION);
    VarDeclaration *decl = (VarDeclaration*)malloc(sizeof(VarDeclaration));
    if (!decl) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    decl->kind = "let";
    decl->name = strdup(name);
    decl->init = init;
    node->data.var_decl = decl;
    return node;
}

ASTNode* create_const_declaration(char *name, ASTNode *init) {
    ASTNode *node = create_node(NODE_CONST_DECLARATION);
    VarDeclaration *decl = (VarDeclaration*)malloc(sizeof(VarDeclaration));
    if (!decl) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    decl->kind = "const";
    decl->name = strdup(name);
    decl->init = init;
    node->data.var_decl = decl;
    return node;
}

ASTNode* create_function_declaration(char *name, ASTNode *params, ASTNode *body) {
    ASTNode *node = create_node(NODE_FUNCTION_DECLARATION);
    FunctionDeclaration *func = (FunctionDeclaration*)malloc(sizeof(FunctionDeclaration));
    if (!func) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    func->name = strdup(name);
    func->params = params;
    func->body = body;
    func->is_async = 0;
    func->is_generator = 0;
    node->data.func_decl = func;
    return node;
}

ASTNode* create_async_function_declaration(char *name, ASTNode *params, ASTNode *body) {
    ASTNode *node = create_function_declaration(name, params, body);
    FunctionDeclaration *func = node->data.func_decl;
    func->is_async = 1;
    return node;
}

ASTNode* create_if_statement(ASTNode *test, ASTNode *consequent, ASTNode *alternate) {
    ASTNode *node = create_node(NODE_IF_STATEMENT);
    IfStatement *stmt = (IfStatement*)malloc(sizeof(IfStatement));
    if (!stmt) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    stmt->test = test;
    stmt->consequent = consequent;
    stmt->alternate = alternate;
    node->data.if_stmt = stmt;
    return node;
}

ASTNode* create_while_statement(ASTNode *test, ASTNode *body) {
    ASTNode *node = create_node(NODE_WHILE_STATEMENT);
    WhileStatement *stmt = (WhileStatement*)malloc(sizeof(WhileStatement));
    if (!stmt) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    stmt->test = test;
    stmt->body = body;
    node->data.while_stmt = stmt;
    return node;
}

ASTNode* create_for_statement(ASTNode *init, ASTNode *test, ASTNode *update, ASTNode *body) {
    ASTNode *node = create_node(NODE_FOR_STATEMENT);
    ForStatement *stmt = (ForStatement*)malloc(sizeof(ForStatement));
    if (!stmt) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    stmt->init = init;
    stmt->test = test;
    stmt->update = update;
    stmt->body = body;
    node->data.for_stmt = stmt;
    return node;
}

ASTNode* create_for_statement_with_var(char *var, ASTNode *init, ASTNode *test, 
                                       ASTNode *update, ASTNode *body) {
    ASTNode *var_decl = create_var_declaration(var, init);
    return create_for_statement(var_decl, test, update, body);
}

ASTNode* create_return_statement(ASTNode *argument) {
    ASTNode *node = create_node(NODE_RETURN_STATEMENT);
    ReturnStatement *stmt = (ReturnStatement*)malloc(sizeof(ReturnStatement));
    if (!stmt) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    stmt->argument = argument;
    node->data.return_stmt = stmt;
    return node;
}

ASTNode* create_break_statement(ASTNode *label) {
    ASTNode *node = create_node(NODE_BREAK_STATEMENT);
    node->data.child = label;
    return node;
}

ASTNode* create_continue_statement(ASTNode *label) {
    ASTNode *node = create_node(NODE_CONTINUE_STATEMENT);
    node->data.child = label;
    return node;
}

ASTNode* create_throw_statement(ASTNode *argument) {
    ASTNode *node = create_node(NODE_THROW_STATEMENT);
    node->data.child = argument;
    return node;
}

ASTNode* create_block_statement(ASTNode *body) {
    ASTNode *node = create_node(NODE_BLOCK_STATEMENT);
    node->data.child = body;
    return node;
}

ASTNode* create_empty_statement(void) {
    return create_node(NODE_EMPTY_STATEMENT);
}

/* ============================================================================
   EXPRESSION NODES
   ============================================================================ */

ASTNode* create_binary_expression(char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(NODE_BINARY_EXPR);
    BinaryExpression *expr = (BinaryExpression*)malloc(sizeof(BinaryExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->operator = strdup(op);
    expr->left = left;
    expr->right = right;
    node->data.binary_expr = expr;
    return node;
}

ASTNode* create_unary_expression(char *op, ASTNode *arg) {
    ASTNode *node = create_node(NODE_UNARY_EXPR);
    UnaryExpression *expr = (UnaryExpression*)malloc(sizeof(UnaryExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->operator = strdup(op);
    expr->argument = arg;
    expr->prefix = 1;
    node->data.unary_expr = expr;
    return node;
}

ASTNode* create_update_expression(char *op, ASTNode *arg, int prefix) {
    ASTNode *node = create_node(NODE_UPDATE_EXPR);
    UnaryExpression *expr = (UnaryExpression*)malloc(sizeof(UnaryExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->operator = strdup(op);
    expr->argument = arg;
    expr->prefix = prefix;
    node->data.unary_expr = expr;
    return node;
}

ASTNode* create_assignment_expression(char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(NODE_ASSIGNMENT_EXPR);
    AssignmentExpression *expr = (AssignmentExpression*)malloc(sizeof(AssignmentExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->operator = strdup(op);
    expr->left = left;
    expr->right = right;
    node->data.assign_expr = expr;
    return node;
}

ASTNode* create_conditional_expression(ASTNode *test, ASTNode *cons, ASTNode *alt) {
    ASTNode *node = create_node(NODE_CONDITIONAL_EXPR);
    ConditionalExpression *expr = (ConditionalExpression*)malloc(sizeof(ConditionalExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->test = test;
    expr->consequent = cons;
    expr->alternate = alt;
    node->data.cond_expr = expr;
    return node;
}

ASTNode* create_call_expression(ASTNode *callee, ASTNode *args) {
    ASTNode *node = create_node(NODE_CALL_EXPR);
    CallExpression *expr = (CallExpression*)malloc(sizeof(CallExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->callee = callee;
    expr->arguments = args;
    node->data.call_expr = expr;
    return node;
}

ASTNode* create_member_expression(ASTNode *obj, ASTNode *prop, int computed) {
    ASTNode *node = create_node(NODE_MEMBER_EXPR);
    MemberExpression *expr = (MemberExpression*)malloc(sizeof(MemberExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->object = obj;
    expr->property = prop;
    expr->computed = computed;
    node->data.member_expr = expr;
    return node;
}

ASTNode* create_new_expression(ASTNode *callee) {
    ASTNode *node = create_node(NODE_NEW_EXPR);
    node->data.child = callee;
    return node;
}

ASTNode* create_comma_expression(ASTNode *left, ASTNode *right) {
    ASTNode *node = create_node(NODE_COMMA_EXPR);
    BinaryExpression *expr = (BinaryExpression*)malloc(sizeof(BinaryExpression));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    expr->operator = strdup(",");
    expr->left = left;
    expr->right = right;
    node->data.binary_expr = expr;
    return node;
}

/* ============================================================================
   LITERAL AND IDENTIFIER NODES
   ============================================================================ */

ASTNode* create_identifier(char *name) {
    ASTNode *node = create_node(NODE_IDENTIFIER);
    Identifier *id = (Identifier*)malloc(sizeof(Identifier));
    if (!id) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    id->name = strdup(name);
    node->data.identifier = id;
    return node;
}

ASTNode* create_null_literal(void) {
    ASTNode *node = create_node(NODE_LITERAL);
    Literal *lit = (Literal*)malloc(sizeof(Literal));
    if (!lit) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    lit->kind = LIT_NULL;
    memset(&lit->value, 0, sizeof(lit->value));
    node->data.literal = lit;
    return node;
}

ASTNode* create_boolean_literal(int value) {
    ASTNode *node = create_node(NODE_LITERAL);
    Literal *lit = (Literal*)malloc(sizeof(Literal));
    if (!lit) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    lit->kind = LIT_BOOLEAN;
    lit->value.boolean = value;
    node->data.literal = lit;
    return node;
}

ASTNode* create_number_literal(char *value) {
    ASTNode *node = create_node(NODE_LITERAL);
    Literal *lit = (Literal*)malloc(sizeof(Literal));
    if (!lit) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    lit->kind = LIT_NUMBER;
    lit->value.string = strdup(value);
    node->data.literal = lit;
    return node;
}

ASTNode* create_string_literal(char *value) {
    ASTNode *node = create_node(NODE_LITERAL);
    Literal *lit = (Literal*)malloc(sizeof(Literal));
    if (!lit) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    lit->kind = LIT_STRING;
    lit->value.string = strdup(value);
    node->data.literal = lit;
    return node;
}

ASTNode* create_this_expression(void) {
    return create_node(NODE_THIS);
}

ASTNode* create_array_expression(ASTNode *elements) {
    ASTNode *node = create_node(NODE_ARRAY_EXPR);
    node->data.child = elements;
    return node;
}

ASTNode* create_object_expression(ASTNode *properties) {
    ASTNode *node = create_node(NODE_OBJECT_EXPR);
    node->data.child = properties;
    return node;
}

/* ============================================================================
   AST PRINTING
   ============================================================================ */

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static const char* node_type_name(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "Program";
        case NODE_STATEMENT_LIST: return "StatementList";
        case NODE_EXPRESSION_STMT: return "ExpressionStatement";
        case NODE_VAR_DECLARATION: return "VariableDeclaration(var)";
        case NODE_LET_DECLARATION: return "VariableDeclaration(let)";
        case NODE_CONST_DECLARATION: return "VariableDeclaration(const)";
        case NODE_FUNCTION_DECLARATION: return "FunctionDeclaration";
        case NODE_IF_STATEMENT: return "IfStatement";
        case NODE_WHILE_STATEMENT: return "WhileStatement";
        case NODE_FOR_STATEMENT: return "ForStatement";
        case NODE_RETURN_STATEMENT: return "ReturnStatement";
        case NODE_BREAK_STATEMENT: return "BreakStatement";
        case NODE_CONTINUE_STATEMENT: return "ContinueStatement";
        case NODE_THROW_STATEMENT: return "ThrowStatement";
        case NODE_BLOCK_STATEMENT: return "BlockStatement";
        case NODE_EMPTY_STATEMENT: return "EmptyStatement";
        case NODE_BINARY_EXPR: return "BinaryExpression";
        case NODE_UNARY_EXPR: return "UnaryExpression";
        case NODE_UPDATE_EXPR: return "UpdateExpression";
        case NODE_ASSIGNMENT_EXPR: return "AssignmentExpression";
        case NODE_CONDITIONAL_EXPR: return "ConditionalExpression";
        case NODE_CALL_EXPR: return "CallExpression";
        case NODE_MEMBER_EXPR: return "MemberExpression";
        case NODE_NEW_EXPR: return "NewExpression";
        case NODE_COMMA_EXPR: return "CommaExpression";
        case NODE_IDENTIFIER: return "Identifier";
        case NODE_LITERAL: return "Literal";
        case NODE_THIS: return "ThisExpression";
        case NODE_ARRAY_EXPR: return "ArrayExpression";
        case NODE_OBJECT_EXPR: return "ObjectExpression";
        default: return "Unknown";
    }
}

void print_ast(ASTNode *node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }
    
    print_indent(indent);
    printf("%s", node_type_name(node->type));
    
    switch (node->type) {
        case NODE_PROGRAM: {
            Program *prog = node->data.program;
            printf("\n");
            print_ast(prog->body, indent + 1);
            break;
        }
        
        case NODE_STATEMENT_LIST: {
            StatementList *list = node->data.stmt_list;
            printf(" (%d statements)\n", list->count);
            for (int i = 0; i < list->count; i++) {
                print_ast(list->statements[i], indent + 1);
            }
            break;
        }
        
        case NODE_VAR_DECLARATION:
        case NODE_LET_DECLARATION:
        case NODE_CONST_DECLARATION: {
            VarDeclaration *decl = node->data.var_decl;
            printf(" name=%s\n", decl->name);
            if (decl->init) {
                print_indent(indent + 1);
                printf("Initializer:\n");
                print_ast(decl->init, indent + 2);
            }
            break;
        }
        
        case NODE_FUNCTION_DECLARATION: {
            FunctionDeclaration *func = node->data.func_decl;
            printf(" name=%s%s%s\n", 
                   func->name, 
                   func->is_async ? " (async)" : "",
                   func->is_generator ? " (generator)" : "");
            if (func->params) {
                print_indent(indent + 1);
                printf("Parameters:\n");
                print_ast(func->params, indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(func->body, indent + 2);
            break;
        }
        
        case NODE_IF_STATEMENT: {
            IfStatement *stmt = node->data.if_stmt;
            printf("\n");
            print_indent(indent + 1);
            printf("Test:\n");
            print_ast(stmt->test, indent + 2);
            print_indent(indent + 1);
            printf("Consequent:\n");
            print_ast(stmt->consequent, indent + 2);
            if (stmt->alternate) {
                print_indent(indent + 1);
                printf("Alternate:\n");
                print_ast(stmt->alternate, indent + 2);
            }
            break;
        }
        
        case NODE_WHILE_STATEMENT: {
            WhileStatement *stmt = node->data.while_stmt;
            printf("\n");
            print_indent(indent + 1);
            printf("Test:\n");
            print_ast(stmt->test, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(stmt->body, indent + 2);
            break;
        }
        
        case NODE_FOR_STATEMENT: {
            ForStatement *stmt = node->data.for_stmt;
            printf("\n");
            if (stmt->init) {
                print_indent(indent + 1);
                printf("Init:\n");
                print_ast(stmt->init, indent + 2);
            }
            if (stmt->test) {
                print_indent(indent + 1);
                printf("Test:\n");
                print_ast(stmt->test, indent + 2);
            }
            if (stmt->update) {
                print_indent(indent + 1);
                printf("Update:\n");
                print_ast(stmt->update, indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(stmt->body, indent + 2);
            break;
        }
        
        case NODE_RETURN_STATEMENT: {
            ReturnStatement *stmt = node->data.return_stmt;
            printf("\n");
            if (stmt->argument) {
                print_indent(indent + 1);
                printf("Argument:\n");
                print_ast(stmt->argument, indent + 2);
            } else {
                print_indent(indent + 1);
                printf("(no argument)\n");
            }
            break;
        }
        
        case NODE_BREAK_STATEMENT:
        case NODE_CONTINUE_STATEMENT: {
            printf("\n");
            if (node->data.child) {
                print_indent(indent + 1);
                printf("Label:\n");
                print_ast(node->data.child, indent + 2);
            }
            break;
        }
        
        case NODE_THROW_STATEMENT:
        case NODE_EXPRESSION_STMT: {
            printf("\n");
            print_ast(node->data.child, indent + 1);
            break;
        }
        
        case NODE_BLOCK_STATEMENT: {
            printf("\n");
            print_ast(node->data.child, indent + 1);
            break;
        }
        
        case NODE_BINARY_EXPR:
        case NODE_COMMA_EXPR: {
            BinaryExpression *expr = node->data.binary_expr;
            printf(" op='%s'\n", expr->operator);
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(expr->left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(expr->right, indent + 2);
            break;
        }
        
        case NODE_ASSIGNMENT_EXPR: {
            AssignmentExpression *expr = node->data.assign_expr;
            printf(" op='%s'\n", expr->operator);
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(expr->left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(expr->right, indent + 2);
            break;
        }
        
        case NODE_UNARY_EXPR:
        case NODE_UPDATE_EXPR: {
            UnaryExpression *expr = node->data.unary_expr;
            printf(" op='%s' %s\n", 
                   expr->operator, 
                   expr->prefix ? "(prefix)" : "(postfix)");
            print_indent(indent + 1);
            printf("Argument:\n");
            print_ast(expr->argument, indent + 2);
            break;
        }
        
        case NODE_CONDITIONAL_EXPR: {
            ConditionalExpression *expr = node->data.cond_expr;
            printf("\n");
            print_indent(indent + 1);
            printf("Test:\n");
            print_ast(expr->test, indent + 2);
            print_indent(indent + 1);
            printf("Consequent:\n");
            print_ast(expr->consequent, indent + 2);
            print_indent(indent + 1);
            printf("Alternate:\n");
            print_ast(expr->alternate, indent + 2);
            break;
        }
        
        case NODE_CALL_EXPR: {
            CallExpression *expr = node->data.call_expr;
            printf("\n");
            print_indent(indent + 1);
            printf("Callee:\n");
            print_ast(expr->callee, indent + 2);
            print_indent(indent + 1);
            printf("Arguments:\n");
            print_ast(expr->arguments, indent + 2);
            break;
        }
        
        case NODE_MEMBER_EXPR: {
            MemberExpression *expr = node->data.member_expr;
            printf(" %s\n", expr->computed ? "(computed)" : "(dot)");
            print_indent(indent + 1);
            printf("Object:\n");
            print_ast(expr->object, indent + 2);
            print_indent(indent + 1);
            printf("Property:\n");
            print_ast(expr->property, indent + 2);
            break;
        }
        
        case NODE_NEW_EXPR: {
            printf("\n");
            print_indent(indent + 1);
            printf("Callee:\n");
            print_ast(node->data.child, indent + 2);
            break;
        }
        
        case NODE_IDENTIFIER: {
            Identifier *id = node->data.identifier;
            printf(" '%s'\n", id->name);
            break;
        }
        
        case NODE_LITERAL: {
            Literal *lit = node->data.literal;
            switch (lit->kind) {
                case LIT_NULL:
                    printf(" null\n");
                    break;
                case LIT_BOOLEAN:
                    printf(" %s\n", lit->value.boolean ? "true" : "false");
                    break;
                case LIT_NUMBER:
                    printf(" (number) %s\n", lit->value.string);
                    break;
                case LIT_STRING:
                    printf(" (string) %s\n", lit->value.string);
                    break;
                case LIT_BIGINT:
                    printf(" (bigint) %s\n", lit->value.string);
                    break;
            }
            break;
        }
        
        case NODE_THIS: {
            printf("\n");
            break;
        }
        
        case NODE_ARRAY_EXPR: {
            printf("\n");
            if (node->data.child) {
                print_indent(indent + 1);
                printf("Elements:\n");
                print_ast(node->data.child, indent + 2);
            } else {
                print_indent(indent + 1);
                printf("(empty array)\n");
            }
            break;
        }
        
        case NODE_OBJECT_EXPR: {
            printf("\n");
            if (node->data.child) {
                print_indent(indent + 1);
                printf("Properties:\n");
                print_ast(node->data.child, indent + 2);
            } else {
                print_indent(indent + 1);
                printf("(empty object)\n");
            }
            break;
        }
        
        case NODE_EMPTY_STATEMENT: {
            printf("\n");
            break;
        }
        
        default:
            printf(" (unhandled node type: %d)\n", node->type);
            break;
    }
}

/* ============================================================================
   MEMORY MANAGEMENT
   ============================================================================ */

void free_ast(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM: {
            Program *prog = node->data.program;
            if (prog) {
                free_ast(prog->body);
                free(prog);
            }
            break;
        }
        
        case NODE_STATEMENT_LIST: {
            StatementList *list = node->data.stmt_list;
            if (list) {
                for (int i = 0; i < list->count; i++) {
                    free_ast(list->statements[i]);
                }
                free(list->statements);
                free(list);
            }
            break;
        }
        
        case NODE_VAR_DECLARATION:
        case NODE_LET_DECLARATION:
        case NODE_CONST_DECLARATION: {
            VarDeclaration *decl = node->data.var_decl;
            if (decl) {
                free(decl->name);
                free_ast(decl->init);
                free(decl);
            }
            break;
        }
        
        case NODE_FUNCTION_DECLARATION: {
            FunctionDeclaration *func = node->data.func_decl;
            if (func) {
                free(func->name);
                free_ast(func->params);
                free_ast(func->body);
                free(func);
            }
            break;
        }
        
        case NODE_IF_STATEMENT: {
            IfStatement *stmt = node->data.if_stmt;
            if (stmt) {
                free_ast(stmt->test);
                free_ast(stmt->consequent);
                free_ast(stmt->alternate);
                free(stmt);
            }
            break;
        }
        
        case NODE_WHILE_STATEMENT: {
            WhileStatement *stmt = node->data.while_stmt;
            if (stmt) {
                free_ast(stmt->test);
                free_ast(stmt->body);
                free(stmt);
            }
            break;
        }
        
        case NODE_FOR_STATEMENT: {
            ForStatement *stmt = node->data.for_stmt;
            if (stmt) {
                free_ast(stmt->init);
                free_ast(stmt->test);
                free_ast(stmt->update);
                free_ast(stmt->body);
                free(stmt);
            }
            break;
        }
        
        case NODE_RETURN_STATEMENT: {
            ReturnStatement *stmt = node->data.return_stmt;
            if (stmt) {
                free_ast(stmt->argument);
                free(stmt);
            }
            break;
        }
        
        case NODE_BREAK_STATEMENT:
        case NODE_CONTINUE_STATEMENT:
        case NODE_THROW_STATEMENT:
        case NODE_EXPRESSION_STMT:
        case NODE_NEW_EXPR:
        case NODE_ARRAY_EXPR:
        case NODE_OBJECT_EXPR:
        case NODE_BLOCK_STATEMENT: {
            free_ast(node->data.child);
            break;
        }
        
        case NODE_BINARY_EXPR:
        case NODE_COMMA_EXPR: {
            BinaryExpression *expr = node->data.binary_expr;
            if (expr) {
                free(expr->operator);
                free_ast(expr->left);
                free_ast(expr->right);
                free(expr);
            }
            break;
        }
        
        case NODE_ASSIGNMENT_EXPR: {
            AssignmentExpression *expr = node->data.assign_expr;
            if (expr) {
                free(expr->operator);
                free_ast(expr->left);
                free_ast(expr->right);
                free(expr);
            }
            break;
        }
        
        case NODE_UNARY_EXPR:
        case NODE_UPDATE_EXPR: {
            UnaryExpression *expr = node->data.unary_expr;
            if (expr) {
                free(expr->operator);
                free_ast(expr->argument);
                free(expr);
            }
            break;
        }
        
        case NODE_CONDITIONAL_EXPR: {
            ConditionalExpression *expr = node->data.cond_expr;
            if (expr) {
                free_ast(expr->test);
                free_ast(expr->consequent);
                free_ast(expr->alternate);
                free(expr);
            }
            break;
        }
        
        case NODE_CALL_EXPR: {
            CallExpression *expr = node->data.call_expr;
            if (expr) {
                free_ast(expr->callee);
                free_ast(expr->arguments);
                free(expr);
            }
            break;
        }
        
        case NODE_MEMBER_EXPR: {
            MemberExpression *expr = node->data.member_expr;
            if (expr) {
                free_ast(expr->object);
                free_ast(expr->property);
                free(expr);
            }
            break;
        }
        
        case NODE_IDENTIFIER: {
            Identifier *id = node->data.identifier;
            if (id) {
                free(id->name);
                free(id);
            }
            break;
        }
        
        case NODE_LITERAL: {
            Literal *lit = node->data.literal;
            if (lit) {
                if (lit->kind == LIT_STRING || lit->kind == LIT_NUMBER || lit->kind == LIT_BIGINT) {
                    if (lit->value.string) {
                        free(lit->value.string);
                    }
                }
                free(lit);
            }
            break;
        }
        
        case NODE_EMPTY_STATEMENT:
        case NODE_THIS:
            /* No additional cleanup needed */
            break;
        
        default:
            break;
    }
    
    free(node);
}
