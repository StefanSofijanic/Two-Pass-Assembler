#pragma once
#include "Line.h"
#include <vector>
using namespace std;

vector<Line*>& get_prog();
void print_prog();

void add_end();
void add_start();
void add_pc();

void add_literal_to_symbollist(int literal);
void add_symbol_to_symbollist(string symbol, int lineNo);

void add_section(string sec, int lineNo);
void add_equ(string symb, int literal, int lineNo);
void add_skip(int literal);
void add_word(int lineNo);
void add_declaration(string decl, int lineNo);

void add_symbol(string symbol, int adrModel);
void add_literal(int literal, int adrModel);
void add_reg(string reg, int adrModel);
void add_regliteral(string reg, int literal, int adrModel);
void add_regsymbol(string reg, string symbol, int adrModel);

void add_label(string _label, int lineNo);
void add_mulop(string op, string rd, string rs);
void add_ldr(string rd, int lineNo);
void add_str(string rs, int lineNo);
void add_destop(string op, string rd);
void add_jmpop(string op, int lineNo);
void add_nooperand(string op);

void rearrange_symbol_table();
ofstream& print_symbol_table(ofstream& os);

void second_run(ofstream& os);

void close_line();
void delete_prog();