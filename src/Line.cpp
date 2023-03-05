#include "Line.h"
#include "SymbolTable.h"
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include <sstream>

using namespace  std;
int getReg(string r){
    static unordered_map<string, int> registers{{"r0", 0},
                                        {"r1", 1},
                                        {"r2", 2},
                                        {"r3", 3},
                                        {"r4", 4},
                                        {"r5", 5},
                                        {"r6", 6},
                                        {"r7", 7},
                                        {"sp", 6},
                                        {"pc", 7},
                                        {"psw", 8}};
    return registers[r];
}

string toBinOpCode(long value, int bits4){
    std::stringstream stream;
    stream << setfill ('0') << setw(bits4*2) << std::hex << value;
    return stream.str();
}

int toBin(int value){
    int lowB = value & 0xFF;
    int highB = (value >> 8) & 0xFF;
    int newNum = lowB << 8 | highB;
    return newNum;
}
string toBinString(int value){
    std::stringstream stream;
    stream << setw(16) << std::hex << toBin(value);
    return stream.str();
}
void toLower(string& s){
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

int getPushUA(){
    return (1<<4) | 2;
}
int getPopUA(){
    return (4<<4) | 2;
}


ostream& operator<<(ostream& os, const DirectivePart& part){
    return part.format(os);
}
void DirectivePart::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, int lineNo, int num) {}

ostream& LiteralPart::format(ostream& os) const {
    os<<mValue;
    return os;
}

LiteralPart::LiteralPart(int val){
    mValue = val;
}

ostream& SymbolPart::format(ostream& os) const {
    os<<mSymbol;
    return os;
}

SymbolPart::SymbolPart(string symb, int lineNo):mLineNo(lineNo){
    mSymbol = symb;
}

void SymbolPart::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, int lineNo, int num) {
    if (symbolTable.isEqu(mSymbol)) return;

    SymbolEntry* s = symbolTable.findEntry(mSymbol);
    // da li apsolutna sekcija i dalje moze da se tretira kao global
    if (s->mVisibility == global)
        relTable.addEntry(lineNo+2*num, R_16, s->mOrdinal);
    else { //local
        SymbolEntry* sec = symbolTable.findEntry(s->mSection->mName);
        relTable.addEntry(lineNo+2*num, R_16, sec->mOrdinal);///redni br sekcije
    }
}

ostream& operator<<(ostream& os, const Operand& operand){
    return operand.format(os);
}

Operand::Operand(int mode){
    mMode =  static_cast<AdressMode>(mode);
}

void Operand::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection, int lineNo) {}

Literal::Literal(int mode, int num):Operand(mode){
    mNum = num;
}

ostream& Literal::format(ostream& os) const {
    if (mMode == DATA1) os<<'$';
    if (mMode == JMP4) os<<'%';
    os<<mNum;
    return os;
}

int Literal::getOpCode(SymbolTable &table, Section* curSection, int lineNo) { 
    int adrMode = 0;
    if (mMode == DATA1 || mMode == JMP1) adrMode =  0;
    else if (mMode == DATA3 || mMode == JMP4) adrMode = 4;
    return adrMode<< 16 | toBin(mNum);
    }

Symbol::Symbol(int mode, string symbol):Operand(mode){
    mSymbol = symbol;
}
ostream& Symbol::format(ostream& os) const{
    if (mMode == DATA2) os<<'$';
    if (mMode == DATA5) os<<'%';
    if (mMode == JMP3) os<<'%';
    if (mMode == JMP5) os<<'*';
    os<<mSymbol;
    return os;
}
int Symbol::getOpCode(SymbolTable &table, Section* curSection, int lineNo) { 
    int adrMode = 0;
    if (mMode == DATA2 || mMode == JMP2) adrMode =  0;
    else if (mMode == JMP3) adrMode =  0x7 << 8 | 5;
    else if (mMode == DATA4 || mMode == JMP5) adrMode = 4;
    else if (mMode == DATA5) adrMode =  0x7 << 8 | 3;
    adrMode <<= 16;
    SymbolEntry* s = table.findEntry(mSymbol);
    if (isPCRel()){
        int offset = -2;
        if (s->mSection == curSection) offset = s->mValue - lineNo - 5;
        else if (s->mVisibility == global) offset = -2; 
        else offset = s->mValue - 2;
        return adrMode | toBin(offset); 
    }
    if (table.isEqu(mSymbol)) return adrMode | toBin(s->mValue); 
    if (s->mVisibility == global) return adrMode;
    return adrMode | toBin(s->mValue); 
}

void Symbol::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection, int lineNo){
    SymbolEntry* s = symbolTable.findEntry(mSymbol);
    if (isPCRel()){
        if (s->mSection == curSection) return;

        if (s->mVisibility == global)
            relTable.addEntry(lineNo+3, R_16_PC, s->mOrdinal);
        else {
            SymbolEntry* sec = symbolTable.findEntry(s->mSection->mName);
            relTable.addEntry(lineNo+3, R_16_PC, sec->mOrdinal);
        }
    }
    else {
        if (symbolTable.isEqu(mSymbol)) return;

        if (s->mVisibility == global)
            relTable.addEntry(lineNo+3, R_16, s->mOrdinal);
        else {
            SymbolEntry* sec = symbolTable.findEntry(s->mSection->mName);
            relTable.addEntry(lineNo+3, R_16, sec->mOrdinal);
        }
    }
}
Reg::Reg(int mode, string reg):Operand(mode){
    toLower(mReg = reg);
}
ostream& Reg::format(ostream& os) const{
    if (mMode == DATA7) os<<'[';
    if (mMode == JMP6) os<<'*';
    if (mMode == JMP7) os<<"*[";
    os<<mReg;
    if (mMode == DATA7) os<<']';
    if (mMode == JMP7) os<<']';
    return os;
}
int Reg::getOpCode(SymbolTable &table, Section* curSection, int lineNo) { 
    int reg = getReg(mReg) << 8;
    if (mMode == DATA6 || mMode == JMP6)
        reg |= 1;
    else 
        reg |= 2;
    return reg;
}

LiteralReg::LiteralReg(int mode, string reg, int literal):Reg(mode, reg){
    mNum = literal;
}
ostream& LiteralReg::format(ostream& os) const{
    if (mMode == JMP8) 
        os<<'*';
    os<<'['<<mReg<<" + "<<mNum<<']';
    return os;
}

int LiteralReg::getOpCode(SymbolTable &table, Section* curSection, int lineNo) {
    int adrMode = getReg(mReg)<< 8;
    adrMode |=  3;
    return adrMode << 16 | toBin(mNum);
}

SymbolReg::SymbolReg(int mode, string reg, string symbol):Reg(mode, reg){
    mSymbol = symbol;
}
ostream& SymbolReg::format(ostream& os) const{
    if (mMode == JMP9) 
        os<<'*';
    os<<'['<<mReg<<" + "<<mSymbol<<']';
    return os;
}

int SymbolReg::getOpCode(SymbolTable &table, Section* curSection, int lineNo) {
    //uvek apsolutno
    int adrMode = getReg(mReg) << 8;
    adrMode |=  3;
    adrMode <<= 16;
    SymbolEntry* s = table.findEntry(mSymbol);
    if (table.isEqu(mSymbol)) return adrMode | toBin(s->mValue);
    if (s->mVisibility == global) return adrMode | 0; 

    return adrMode | toBin(s->mValue);
}

void SymbolReg::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection, int lineNo){
    if (symbolTable.isEqu(mSymbol)) return;

    SymbolEntry* s = symbolTable.findEntry(mSymbol);
    // ABS
    if (s->mVisibility == global)
        relTable.addEntry(lineNo+3, R_16, s->mOrdinal);
    else { //global
        SymbolEntry* sec = symbolTable.findEntry(s->mSection->mName);
        relTable.addEntry(lineNo+3, R_16, sec->mOrdinal);
    }
    
}

Line::Line(string label, int pc){
    mLabel = label;
    mPc = pc;
}
void Line::print(SymbolTable& table){
    if (mLabel.size()>0)
        cout<<mLabel<<":"<<endl;
}

int Line::getLength() { return 0; }

int Line::missingSymbol(SymbolTable& table){
    return -1;
}

void Line::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) {}

LabelLine::LabelLine(string label, int pc):Line(label, pc) {}

string LabelLine::getOpCode(SymbolTable &table, Section* curSection) { return ""; }

Section::Section(string label, string name, int pc):Line(label, pc){
    mName = name;
}

void Section::print(SymbolTable& table){
    Line::print(table);
    cout<<".section "<<mName;
}

string Section::getOpCode(SymbolTable &table, Section* curSection) {
    return "";
}

Equ::Equ(string label, string symbol, int value, int pc):Line(label, pc){
    mSymbol = symbol;
    mValue = value;
}

void Equ::print(SymbolTable& table) {
    Line::print(table);
    cout<<".equ "<<mSymbol<<", "<<mValue;
}
string Equ::getOpCode(SymbolTable &table, Section* curSection) {
    return "";
}

Skip::Skip(string label, int value, int pc):Line(label, pc) {
    mValue = value;
}

void Skip::print(SymbolTable& table){
    Line::print(table);
    cout<<".skip "<<hex<<mValue;
}

int Skip::getLength() { return mValue; }

string Skip::getOpCode(SymbolTable &table, Section* curSection) {
    return toBinOpCode(0, mValue);
}

Declaration::Declaration(string label, string decl, vector<DirectivePart*>& symbList, int pc):Line(label, pc) {
    toLower(mDecl = decl);
    mSymbolList = symbList;
}

Declaration::~Declaration(){
    for (auto* part:mSymbolList) delete part;
}

void Declaration::print(SymbolTable& table){
    Line::print(table);
    cout<<mDecl<<" ";
    for (auto& symb:mSymbolList) {
        cout<<*symb;
        if (symb!=mSymbolList[mSymbolList.size()-1]) cout<<", ";
    }
}

string Declaration::getOpCode(SymbolTable &table, Section* curSection) {
    return "";
}

WordList::WordList(string label, vector<DirectivePart*>& symbList, int pc, int lineNo):Line(label, pc){
    mSymbolList = symbList;
    mLineNo = lineNo;
}

WordList::~WordList(){
    for (auto* part:mSymbolList) {
        delete part;
    }
}

void WordList::print(SymbolTable& table){
    Line::print(table);
    cout<<".word ";
    for (auto* part:mSymbolList) {
        if (auto* s = dynamic_cast<SymbolPart*>(part))
            cout<<(s->mSymbol);
        else if (auto* l=dynamic_cast<LiteralPart*>(part))
            cout<<l->mValue;
        if (part!=mSymbolList[mSymbolList.size()-1]) cout<<", ";
    }
}

int WordList::getLength() { return mSymbolList.size() * 2; }

int WordList::missingSymbol(SymbolTable& table){
    for (auto& wordPart : mSymbolList){
        if (auto* symbol = dynamic_cast<SymbolPart*>(wordPart)){
            if (table.findEntry(symbol->mSymbol)==nullptr)
                return mLineNo;
        }
    }
    return -1;
}
string WordList::getOpCode(SymbolTable &table, Section* curSection) {
    string ret = "";
    for (auto& wordPart : mSymbolList){
        if (auto* symbol = dynamic_cast<SymbolPart*>(wordPart)){
            if (auto* s = table.findEntry(symbol->mSymbol)){
                int offset = 0;
                if (s->mVisibility == local || table.isEqu(symbol->mSymbol))
                    offset = s->mValue;
                ret += toBinOpCode(toBin(offset), 2);
            }
            else ret+=toBinOpCode(0, 2*8);
        }
        if (auto* literal = dynamic_cast<LiteralPart*>(wordPart)){
            ret += toBinOpCode(toBin(literal->mValue), 2);
        }
    }
    return ret;
}

void WordList::updateRelTable(RelocationTable& relTable, SymbolTable& symbolTable, Section* curSection) {
    int i=0;
    for (auto& wordPart : mSymbolList){
        if (auto* symbol = dynamic_cast<SymbolPart*>(wordPart)){
            symbol -> updateRelTable(relTable, symbolTable, mPc, i);
        }
        i++;
    }
}

Instruction::Instruction(string label, string op, int pc):Line(label, pc){
    toLower(mOperation = op);
}
void Instruction::print(SymbolTable& table){
    Line::print(table);
    cout<<mOperation<<" ";
}

MulopInstruction::MulopInstruction(string label, string op, string rd, string rs, int pc):Instruction(label, op, pc) {
    mRegD = rd; mRegS = rs;
}
void MulopInstruction::print(SymbolTable& table){
    Instruction::print(table);
    cout<<mRegD<<", "<<mRegS;
}

int MulopInstruction::getLength() { return 1 + 1; }

string MulopInstruction::getOpCode(SymbolTable &table, Section* curSection) {
    int code = 0;
    if (mOperation == "add"){
            code = 7<<4 | 0;
    }
    else if (mOperation == "sub"){
            code = 7<<4 | 1;
    }
    else if (mOperation == "mul"){
            code = 7<<4 | 2;
    }
    else if (mOperation == "div"){
            code = 7<<4 | 3;
    }
    else if (mOperation == "cmp"){
            code = 7<<4 | 4;
    }
    else if (mOperation == "xchg"){
            code  = 6<<4 | 0;
    }
    else if (mOperation == "and"){
            code  = 8<<4 | 1;
    }
    else if (mOperation == "or"){
            code  = 8<<4 | 2;
    }
    else if (mOperation == "xor"){
            code  = 8<<4 | 3;
    }
    else if (mOperation == "test"){
            code  = 8<<4 | 4;
    }
    else if (mOperation == "shl"){
            code  = 9<<4 | 0;
    }
    else if (mOperation == "shr"){
            code  = 9<<4 | 1;
    }
    code = (code<<4 | getReg(mRegD)) << 4 | getReg(mRegS);
    return toBinOpCode(code, 2);
}

LdrInstruction::LdrInstruction(string label, string rd, Operand* operand, int pc, int lineNo):Instruction(label, "ldr", pc){
    mRegD = rd; mOperand = operand;
    mLineNo = lineNo;
}
LdrInstruction::~LdrInstruction(){
    delete mOperand;
}
void LdrInstruction::print(SymbolTable& table){
    Instruction::print(table);
    cout<<mRegD<<", "<<*mOperand;
}

int LdrInstruction::getLength() { return 1 + 1 + 1 + (additionalBytes()?2:0); }

int LdrInstruction::missingSymbol(SymbolTable& table){
    auto s = mOperand->getSymbol();
    if (s.size()!=0 && table.findEntry(s)==nullptr)
        return mLineNo;
    
    return -1;
}

string LdrInstruction::getOpCode(SymbolTable &table, Section* curSection) {
    long code = 10<<4;//1. bajt
    int bytesNeeded = 2 + (additionalBytes()?2:0);//jos 2 ili 4
    code = (code << 4 | getReg(mRegD)) << (bytesNeeded*8 - 4) | mOperand->getOpCode(table, curSection, mPc);
    return toBinOpCode(code, (1+bytesNeeded));
}

void LdrInstruction::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) {
    mOperand -> updateRelTable(relTable, symbolTable, curSection, mPc);
}

StrInstruction::StrInstruction(string label, string rs, Operand *operand, int pc, int lineNo):Instruction(label, "str", pc){
    mRegS = rs; mOperand = operand;
    mLineNo = lineNo;
}
StrInstruction::~StrInstruction(){
    delete mOperand;
}

void StrInstruction::print(SymbolTable& table){
    Instruction::print(table);
    cout<<mRegS<<", "<<*mOperand;
}

int StrInstruction::getLength() { return 1 + 1 + 1 + (additionalBytes()?2:0); }

int StrInstruction::missingSymbol(SymbolTable& table){
    auto s = mOperand->getSymbol();
    if (s.size()!=0 && table.findEntry(s)==nullptr)
        return mLineNo;
    
    return -1;
}

string StrInstruction::getOpCode(SymbolTable &table, Section* curSection) {
    long code = 11<<4;//1. bajt
    int bytesNeeded = 2 + (additionalBytes()?2:0);//jos 2 ili 4
    code = (code << 4 | getReg(mRegS)) << (bytesNeeded*8 - 4) | mOperand->getOpCode(table, curSection, mPc);
    return toBinOpCode(code, (1+bytesNeeded));
}

void StrInstruction::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) {
    mOperand -> updateRelTable(relTable, symbolTable, curSection, mPc);
}

DestOpInstruction::DestOpInstruction(string label, string op, string rd, int pc):Instruction(label, op, pc){
    mRegD = rd;
}
void DestOpInstruction::print(SymbolTable& table){
    Instruction::print(table);
    cout<<mRegD;
}

int DestOpInstruction::getLength() { return 1 + 1 + ((mOperation == "push" || mOperation == "pop")?1:0); }

string DestOpInstruction::getOpCode(SymbolTable &table, Section* curSection) {
    int code = 0;
    int bytesNeeded = 0;
    if (mOperation == "int"){
        code = 1<<4 | 0;
        code = (code << 4 | getReg(mRegD)) << 4 | 15;
        bytesNeeded = 2;
    }
    else if (mOperation == "push"){
        code = 11<<4 | 0;
        code = ((code << 4 | getReg(mRegD)) << 4 | 6) << 8 | getPushUA();//update i adress mode, 3B
        bytesNeeded = 3;
    }
    else if (mOperation == "pop"){
        code = 10<<4 | 0;
        code = ((code << 4 | getReg(mRegD)) << 4 | 6) << 8 | getPopUA();//update i adress mode, 3B
        bytesNeeded = 3;
    }
    else if (mOperation == "not"){
        code = 8<<4 | 0;
        code = (code << 4 | getReg(mRegD)) << 4 | 0;
        bytesNeeded = 2;
    }
    return toBinOpCode(code, bytesNeeded);
}

JmpOpInstruction::JmpOpInstruction(string label, string op, Operand* operand, int pc, int lineNo):Instruction(label, op, pc){
    mOperand = operand;
    mLineNo = lineNo;
}
JmpOpInstruction::~JmpOpInstruction(){
    delete mOperand;
}
void JmpOpInstruction::print(SymbolTable& table){
    Instruction::print(table);
    cout<<*mOperand;
}

int JmpOpInstruction::getLength() { return 1 + 1 + 1 + (additionalBytes()?2:0); }
int JmpOpInstruction::missingSymbol(SymbolTable& table){
    auto s = mOperand->getSymbol();
    if (s.size()!=0 && table.findEntry(s)==nullptr)
        return mLineNo;
    
    return -1;
}

string JmpOpInstruction::getOpCode(SymbolTable &table, Section* curSection) {
    long code = 0;
    if (mOperation == "jmp"){
        code = 5<<4 | 0;
    }
    else if (mOperation == "jeq"){
        code = 5<<4 | 1;
    }
    else if (mOperation == "jne"){
        code = 5<<4 | 2;
    }
    else if (mOperation == "jgt"){
        code = 5<<4 | 3;
    }
    else if (mOperation == "call"){
        code = 3<<4 | 0;
    }
    int bytesNeeded = 2 + (additionalBytes()?2:0);
    code = (code << 4 | 15) << (bytesNeeded*8 - 4) | mOperand->getOpCode(table, curSection, mPc);
    return toBinOpCode(code, (bytesNeeded+1));
}

void JmpOpInstruction::updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) {
    mOperand -> updateRelTable(relTable, symbolTable, curSection, mPc);
}

NoOperandInstruction::NoOperandInstruction(string label, string op, int pc):Instruction(label, op, pc){
}
void NoOperandInstruction::print(SymbolTable& table){
    Instruction::print(table);
}

int NoOperandInstruction::getLength() { return 1; }
string NoOperandInstruction::getOpCode(SymbolTable &table, Section* curSection) {
    int code = 0;
    if (mOperation == "ret"){
        code = 0x40;
    }
    else if (mOperation == "iret"){
        code = 0x20;
    }
    else if (mOperation == "halt"){
        code = 0;
    }

    return toBinOpCode(code, 1);
}
