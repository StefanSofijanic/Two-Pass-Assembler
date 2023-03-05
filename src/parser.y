%{
#include <stdio.h>
#include "Helpers.h"
    int yylex();
    int yyerror(const char *s);
    extern int yylineno;
%}

%error-verbose
/* These declare our output file names. */
%output "parser.cpp"
%defines "parser.h"

%token END
%token <name> DECLARATION
%token SECTIONDECL
%token <name> REG
%token WORD
%token SKIP
%token EQU
%token <number> HEX
%token <number> NUM
%token <name> OPERANDJMP
%token <name> NOOPERAND
%token <name> DESTOP
%token LDR
%token STR
%token <name> MULOP
%token <name> SYMBOL
%token <name> LABEL
%token OTHER
%token NEWLINE

%type <name> string
%type <name> lbl
%type <number> literal

%union{
    const char *name;
    int number;
}

%%

prog:
    { add_start();} prog1
;

prog1: block | NEWLINE block ;

block: 
    stmts | 
;

stmts:
    line | stmts line
;

line:
    stmt
    | lbl stmt
;

lbl: LABEL { add_label($1, yylineno); } optNewLine;
optNewLine: NEWLINE | ;

stmt: subStmt { add_pc(); } NEWLINE { close_line(); }
    | END { add_end(); };

subStmt:
    declaration
    | WORD wordList { add_word(yylineno); }
    | SKIP literal { add_skip($2); }
    | EQU SYMBOL ',' literal { add_equ($2, $4, yylineno); }
    | MULOP REG ',' REG { add_mulop($1, $2, $4);}
    | LDR REG ',' operanddata { add_ldr($2, yylineno);}
    | STR REG ',' destoperand { add_str($2, yylineno);}
    | DESTOP REG { add_destop($1, $2);}
    | OPERANDJMP operandjmp { add_jmpop($1, yylineno);}
    | NOOPERAND { add_nooperand($1);}
    | SECTIONDECL SYMBOL { add_section($2, yylineno);}
;
wordList: wordPart | wordList ',' wordPart
;

wordPart: SYMBOL { add_symbol_to_symbollist($1, yylineno); }  | literal { add_literal_to_symbollist($1); };
declaration:
    DECLARATION symbolList { add_declaration($1, yylineno); }
;

symbolList:
    SYMBOL { add_symbol_to_symbollist($1, yylineno); } | symbolList ',' SYMBOL  { add_symbol_to_symbollist($3, yylineno); }
;

operanddata:
    operandliteral | destoperand
;

destoperand:
    '$' SYMBOL { add_symbol($2, 1); }
    | literal { add_literal($1, 2); }
    | SYMBOL  { add_symbol($1, 3); }
    | '%' SYMBOL  { add_symbol($2, 4); }
    | REG  { add_reg($1, 5); }
    | '[' REG ']' { add_reg($2, 6); }
    | '[' REG  '+' literal ']' { add_regliteral($2, $4, 7); }
    | '[' REG  '+' SYMBOL ']' { add_regsymbol($2, $4, 8); }
;

operandliteral:
    '$'literal { add_literal($2, 0); }
;

literal:
    HEX | NUM
;

operandjmp:
    literal                         { add_literal($1, 9); }
    | SYMBOL                        { add_symbol($1, 10); }
    | '%' SYMBOL                    { add_symbol($2, 11); }
    | '*' literal                { add_literal($2, 12); }
    | '*' SYMBOL                { add_symbol($2, 13); }
    | '*' REG                    { add_reg($2, 14); }
    | '*' '[' REG ']'               { add_reg($3, 15); }
    | '*' '[' REG '+' literal ']'   { add_regliteral($3, $5, 16); }
    | '*' '[' REG '+' SYMBOL ']'    { add_regsymbol($3, $5, 17); }
;

%%

int yyerror(const char *s){
    printf("Syntax error on line %d for %s\n",yylineno, s);
    return 0;
}
