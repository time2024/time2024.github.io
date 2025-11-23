/* main.c - Main driver program for JavaScript parser */

#define _POSIX_C_SOURCE 200809L  /* 启用strdup等POSIX函数 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* External declarations from lexer and parser */
extern int yyparse();
extern void init_lexer(const char *input);
extern ASTNode *root;
extern int parse_error_count;
extern int yylineno;

/* Read entire file into memory */
char* read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    /* Get file size */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    /* Allocate buffer and read */
    char *buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

/* Main function */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <javascript_file>\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -v, --verbose    Print detailed AST\n");
        fprintf(stderr, "  -h, --help       Show this help message\n");
        return 1;
    }
    
    int verbose = 0;
    const char *filename = NULL;
    
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            fprintf(stderr, "JavaScript Parser - re2c + bison implementation\n\n");
            fprintf(stderr, "Usage: %s [options] <javascript_file>\n\n", argv[0]);
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "  -v, --verbose    Print detailed AST\n");
            fprintf(stderr, "  -h, --help       Show this help message\n");
            return 0;
        } else {
            filename = argv[i];
        }
    }
    
    if (!filename) {
        fprintf(stderr, "Error: No input file specified\n");
        return 1;
    }
    
    /* Read input file */
    char *input = read_file(filename);
    if (!input) {
        return 1;
    }
    
    /* Initialize lexer */
    init_lexer(input);
    
    /* Parse the input */
    printf("Parsing '%s'...\n", filename);
    int result = yyparse();
    
    if (result == 0 && parse_error_count == 0 && root != NULL) {
        printf("✓ Parsing successful!\n");
        printf("  Total lines: %d\n", yylineno);
        
        if (verbose) {
            printf("\n=== Abstract Syntax Tree ===\n");
            print_ast(root, 0);
        }
        
        /* Cleanup */
        free_ast(root);
        free(input);
        return 0;
    } else {
        printf("✗ Parsing failed with %d error(s)\n", parse_error_count > 0 ? parse_error_count : 1);
        
        /* Cleanup */
        if (root) {
            free_ast(root);
        }
        free(input);
        return 1;
    }
}