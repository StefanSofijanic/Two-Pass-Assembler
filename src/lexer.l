%{
#include "parser.h"
    void showError();
%}

%option outfile="lexer.cpp" header-file="lexer.h"

/* the int yywrap(void) is used internally by the lexer for the case
 *  where you want to parse multiple files in sequence with the same
 * parser state. Usually, you'd supply your own implementation that
 * would set up the next input stream and let the lexer know whether
 * to keep going. In our case, we disable it; this is effectively
 * the same as a dummy implementation like #define yywrap() 0.
 */
%option noyywrap
%option yylineno


letter              [a-zA-Z]
digit               [0-9]
REG                 [Rr][0-7]|[Pp][cC]|[Pp][sS][Ww]|[Ss][Pp]
IDENT               [A-Za-z_][A-Za-z0-9_]*
LABEL               [A-Za-z_][A-Za-z0-9_]*:
ws                  [ \r\t]+

HEX                 0[Xx][0-9A-Fa-f]+
NUM                 {digit}+
COM                 (\n*[ \r\t]*#[^\n\r]*\n*)+
END                 \.end(.|\n)*

alpha               ([a-zA-z])+
multinl             [\n \r\r]+

%%
{ws}        {                                           }
{COM}       {return (NEWLINE);}
{multinl}         {return (NEWLINE);}
(?i:{END})         {return(END);                               }
(?i:"."global)      {yylval.name = ".global"; return(DECLARATION);}
(?i:"."extern)      {yylval.name = ".extern"; return(DECLARATION);}
(?i:"."section)     {return (SECTIONDECL);}
(?i:"."word)     {return (WORD);}
(?i:"."skip)     {return (SKIP);}
(?i:"."equ)     {return (EQU);}
{REG}   {yylval.name = strdup(yytext); return(REG);}
{HEX}   {yylval.number = (int)strtol(yytext, NULL, 16); return(HEX);}
{NUM}   {yylval.number = (int)strtol(yytext, NULL, 10); return(NUM);}

(?i:halt)         {yylval.name = "halt"; return(NOOPERAND); }
(?i:ret)         {yylval.name = "ret"; return(NOOPERAND); }
(?i:iret)         {yylval.name = "iret"; return(NOOPERAND); }

(?i:jmp)         {yylval.name = "jmp"; return(OPERANDJMP); }
(?i:jeq)         {yylval.name = "jeq"; return(OPERANDJMP); }
(?i:jne)         {yylval.name = "jne"; return(OPERANDJMP); }
(?i:jgt)         {yylval.name = "jgt"; return(OPERANDJMP); }
(?i:call)        {yylval.name = "call"; return(OPERANDJMP); }

(?i:push)         {yylval.name = "push"; return(DESTOP); }
(?i:pop)         {yylval.name = "pop"; return(DESTOP); }
(?i:not)         {yylval.name = "not"; return(DESTOP); }
(?i:int)         {yylval.name = "int"; return(DESTOP); }

(?i:ldr)         {yylval.name = "ldr"; return(LDR); }
(?i:str)         {yylval.name = "str"; return(STR); }

(?i:xchng)         {yylval.name = "xchng"; return(MULOP); }
(?i:add)           {yylval.name = "add"; return(MULOP); }
(?i:sub)           {yylval.name = "sub"; return(MULOP); }
(?i:mul)           {yylval.name = "mul"; return(MULOP);   }
(?i:div)           {yylval.name = "div"; return(MULOP);   }
(?i:cmp)           {yylval.name = "cmp"; return(MULOP);   }
(?i:and)           {yylval.name = "and"; return(MULOP);   }
(?i:or)            {yylval.name = "or"; return(MULOP);   }
(?i:xor)           {yylval.name = "xor"; return(MULOP);   }
(?i:test)          {yylval.name = "test"; return(MULOP);   }
(?i:shl)           {yylval.name = "shl"; return(MULOP);   }
(?i:shr)           {yylval.name = "shr"; return(MULOP);   }



"["         {return *yytext;    }
"]"         {return *yytext;    }
"+"         {return *yytext;    }
"$"        {return *yytext;                        }
"<"         {return *yytext;                            }
">"         {return *yytext;                            }
"%"         {return *yytext;                            }
"*"         {return *yytext;                    }
":"         {return *yytext;                                }
"."         {return *yytext;                                }
","         {return *yytext;                                }

{LABEL}   {yylval.name = strdup(yytext); return(LABEL);}
{IDENT}   {yylval.name = strdup(yytext); return(SYMBOL);}
.           {showError(); return (OTHER);}

%%
void showError(){
    printf("\n Other input \n");
}