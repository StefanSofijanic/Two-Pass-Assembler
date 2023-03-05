#include "Helpers.h"
#include "SymbolTable.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <algorithm>

string label = "";
Operand* operand = nullptr;
vector<DirectivePart*> symbolList;
vector<pair<DirectivePart*, Visibility>> declarationList;

bool endOfFile = false;
int pc = 0;
vector<Line*> prog;

SymbolTable table;
Section* section = table.undefSection.get();

void throw_runtime_error(const char* error, int lineNo){
        for (auto& declaration : declarationList){
            delete declaration.first;
            declaration.first = nullptr;
        }
        declarationList.clear();
        for (auto& symbol:symbolList){
            delete symbol;
        }
        symbolList.clear();

        delete_prog();
        string errOutput = "Error on line " + to_string(lineNo);
        errOutput += ": ";
        errOutput += error;
        __throw_runtime_error(errOutput.c_str());
}
string formatBytes(string toFormat){
    string ret = "";
    int i=0;
    for (auto& c:toFormat){
        ret+=c;
        i++;
        if (i%2==0) ret+=" ";
    }
    return ret;
}
vector<Line*>& get_prog(){
    return prog;
}

void print_prog(){
    for (auto& line:prog){
        line->print(table);
        cout<<endl;
    }
}

void add_pc(){
    pc+=prog[prog.size()-1]->getLength();
}

void add_end(){
    if (label!="") {
        auto* line = new LabelLine(label, pc);
        prog.push_back(line);
    }
    endOfFile = true;
    pc = 0;
    section = table.undefSection.get();
}
void add_start(){}

void add_literal_to_symbollist(int literal){
    symbolList.push_back(new LiteralPart(literal));
}

void add_symbol_to_symbollist(string symbol, int lineNo){
    symbolList.push_back(new SymbolPart(symbol, lineNo));
}
bool checkExternal(string symb){
    for (auto& d:declarationList){
        auto* s = static_cast<SymbolPart*>(d.first);
        if (s->mSymbol==symb && d.second == extH){
            return true;
        }
    }
    return false;
}

void add_section(string sec, int lineNo){
    if (checkExternal(sec))
        throw_runtime_error("Symbol already defined external", lineNo);
    Line* line = new Section(label, sec);
    section = static_cast<Section*>(line);
    prog.push_back(line);
    pc = 0;
    //sekcija ne treba da bude oznacena ni sa global, ni sa ekstern, niti da se preklapa sa drugim simbolima(sekcijama, labelama, equ simbolima) 
    //jer posle ako neko referencira taj simbol, nece znati koji da referencira
    //potencijalno labela i simbol sa istim nazivom bi mogli da pokazuju na istu adresu, ali bi i dalje postojao problem sa referenciranjem(da li ciljamo labelu ili simbol za ref tabelu) 
    if (!table.findEntry(sec)){
        table.addEntry(sec, section, 0, true);
    }
    else {
        throw_runtime_error("Invalid redefinition of symbol", lineNo);
    }
}

void add_equ(string symb, int literal, int lineNo){
    if (checkExternal(symb))
        throw_runtime_error("Symbol already defined external", lineNo);
    Line* line = new Equ(label, symb, literal, pc);
    prog.push_back(line);
    //1. prolaz dodaje samo labele, sekcije i equ simbole
    SymbolEntry* e = table.findEntry(symb);
    if (e){
        throw_runtime_error("Invalid redefinition of symbol", lineNo);
    }
    else {
        table.addEntry(symb, table.absoluteSection.get(), literal, false);
    }
}

void add_skip(int literal){
    Line* line = new Skip(label, literal, pc);
    prog.push_back(line);
}

void add_word(int lineNo){
    int lineNumber = (static_cast<SymbolPart*>(symbolList[symbolList.size()-1]))->mLineNo;
    //cout<<((static_cast<SymbolPart*>(symbolList[symbolList.size()-1]))->mLineNo)<<endl;
    //cout<<lineNo<<endl; ----- iz nekog razloga salje sledeci line number
    Line* line = new WordList(label, symbolList, pc, lineNumber);
    prog.push_back(line);
}

void add_declaration(string decl, int lineNo){

    std::transform(decl.begin(), decl.end(), decl.begin(), ::tolower);
    Line* line = new Declaration(label, decl, symbolList, pc);
    prog.push_back(line);

    for (auto* symbol : symbolList){
        declarationList.push_back(make_pair(symbol, decl == ".global" ? global : extH));
    }
    //lista se koristi na kraju 1. iteracije za azuriranje tabele simbola na global
}

void add_symbol(string symbol, int adrModel){
    operand = new Symbol(adrModel, symbol);
}
void add_literal(int literal, int adrModel){
    operand = new Literal(adrModel, literal);
}
void add_reg(string reg, int adrModel){
    operand = new Reg(adrModel, reg);
}
void add_regliteral(string reg, int literal, int adrModel){
    operand = new LiteralReg(adrModel, reg, literal);
}
void add_regsymbol(string reg, string symbol, int adrModel){
    operand = new SymbolReg(adrModel, reg, symbol);
}

void add_mulop(string op, string rd, string rs){
    Line* instr = new MulopInstruction(label, op, rd, rs, pc);
    prog.push_back(instr);
}

void add_ldr(string rd, int lineNo){
    Line* instr = new LdrInstruction(label, rd, operand, pc, lineNo);
    prog.push_back(instr);
}
void add_str(string rs, int lineNo){
    Line* instr = new StrInstruction(label, rs, operand, pc, lineNo);
    prog.push_back(instr);
}
void add_destop(string op, string rd){
    Line* instr = new DestOpInstruction(label, op, rd, pc);
    prog.push_back(instr);
}
void add_jmpop(string op, int lineNo){
    Line* instr = new JmpOpInstruction(label, op, operand, pc, lineNo);
    prog.push_back(instr);
}
void add_nooperand(string op){
    Line* instr = new NoOperandInstruction(label, op, pc);
    prog.push_back(instr);
    
}

void add_label(string _label, int lineNo){
    if (checkExternal(_label.substr(0, _label.size()-1)))
        throw_runtime_error("Symbol already defined external", lineNo);
    label = _label.substr(0, _label.size()-1);

    //1. prolaz dodaje samo labele, sekcije i equ simbole
    SymbolEntry* e = table.findEntry(label);
    if (e){
        throw_runtime_error("Invalid redefinition of symbol", lineNo);
    }
    else {
        table.addEntry(label, section, pc, false);
    }
}

void close_line(){
    label = "";
    operand = nullptr;
    symbolList.clear();
}

void rearrange_symbol_table(){
    //update globals
    //add extern symbols
    for (auto& declaration : declarationList){
        SymbolPart* symbol = static_cast<SymbolPart*>(declaration.first);
        SymbolEntry* e = table.findEntry(symbol->mSymbol);
        if (declaration.second == global){
            if (!e)
                throw_runtime_error("Symbol declared global, but not defined in file", symbol->mLineNo);
            else if (e->mVisibility == extH)
                throw_runtime_error("Symbol already declared as extern", symbol->mLineNo);
            else
                e->mVisibility = global;
        }
        else {
            if (!e)
                table.addEntry(symbol->mSymbol, table.undefSection.get(), 0, false, extH);
            else if (e->mVisibility == global || e->mVisibility == local)
                throw_runtime_error("Symbol already defined", symbol->mLineNo);
        }
    }
    table.sortSections();
    table.updateGlobals();
}

ofstream& print_symbol_table(ofstream& os){
    os<<table;
    return os;
}


void second_run(ofstream& os){

    unordered_map<Section*, pair<string, RelocationTable>> sectionData;
    Section* curSection = table.undefSection.get();
    string opCode = "";
    RelocationTable relTable;

    for (auto* line:prog){
        int lineNo = line->missingSymbol(table);
        if (lineNo != -1){
            throw_runtime_error("Missing symbol", lineNo);
        }
        if (auto* v = dynamic_cast<Section*>(line)){
            sectionData[curSection]= make_pair(opCode, relTable);
            curSection = v;
            opCode = "";
            relTable.clear();
        }
        opCode += formatBytes(line->getOpCode(table, curSection));
        line->updateRelTable(relTable, table, curSection);
    }
    sectionData[curSection]= make_pair(opCode, relTable);
    os<<string(100, '#')<<endl;
    //print opCode
    for (auto& it:sectionData){
        os<<"#"<<it.first->mName<<endl;
        os<<it.second.first<<endl;
    }
    os<<string(100, '#')<<endl;
    //relTable
    for (auto& it:sectionData){
        os<<"#rel "<<it.first->mName<<endl;
        os<<it.second.second<<endl;
    }
}

void delete_prog(){
    for (auto& line:prog){
        delete line;
    }
}