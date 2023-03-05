#pragma once
#include <string>
#include <iostream>
#include <vector>
using namespace std;

class SymbolEntry;
class SymbolTable;
class RelocationTable;
class Section;

class DirectivePart{
public:
    virtual ostream& format(ostream& os) const = 0 ;
    friend ostream& operator<<(ostream& os, const DirectivePart& part);
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, int lineNo, int num);
};
class LiteralPart:public DirectivePart{
public:
    int mValue;
    LiteralPart(int val);
    virtual ostream& format(ostream& os) const;
};
class SymbolPart : public DirectivePart{
public:
    int mLineNo;
    string mSymbol;

    SymbolPart(string symb, int lineNo);
    virtual ostream& format(ostream& os) const;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, int lineNo, int num) override;
};

enum AdressMode { DATA1 = 0, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8, DATA9, JMP1, JMP2, JMP3, JMP4, JMP5, JMP6,JMP7, JMP8, JMP9 };

class Operand {
public:
    AdressMode mMode;

    Operand(int mode);
    virtual ostream& format(ostream& os) const = 0;
    friend ostream& operator<<(ostream& os, const Operand& operand);
    virtual string getSymbol() = 0;
    virtual int getOpCode(SymbolTable &table, Section* curSection, int lineNo) = 0;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable& symbolTable, Section* curSection, int lineNo);
};

class Literal : public Operand {
public:
    int mNum;

    Literal(int mode, int num);
    virtual ostream& format(ostream& os) const override;
    string getSymbol() override { return ""; }
    int getOpCode(SymbolTable &table, Section* curSection, int lineNo) override;
};

class Symbol : public Operand {
public:
    string mSymbol;

    Symbol(int mode, string symbol);
    virtual ostream& format(ostream& os) const override;
    string getSymbol() override { return mSymbol; }
    int getOpCode(SymbolTable &table, Section* curSection, int lineNo) override;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection, int lineNo) override;
    bool isPCRel() { return mMode == DATA5 || mMode == JMP3; }
};

class Reg : public Operand {
public:
    string mReg;

    Reg(int mode, string reg);
    virtual ostream& format(ostream& os) const override;
    string getSymbol() override { return ""; }
    int getOpCode(SymbolTable &table, Section* curSection, int lineNo) override;
};

class LiteralReg : public Reg {
public:
    int mNum;

    LiteralReg(int mode, string reg, int literal);
    virtual ostream& format(ostream& os) const override;
    string getSymbol() override { return ""; }
    int getOpCode(SymbolTable &table, Section* curSection, int lineNo) override;
};

class SymbolReg : public Reg {
public:
    string mSymbol;

    SymbolReg(int mode, string reg, string symbol);
    virtual ostream& format(ostream& os) const override;
    string getSymbol() override { return mSymbol; }
    int getOpCode(SymbolTable &table, Section* curSection, int lineNo) override;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection, int lineNo) override;
};


class Line {
public:
    string mLabel;
    int mPc;

    Line(string label, int pc);
    virtual void print(SymbolTable& table);
    virtual int getLength();
    virtual int missingSymbol(SymbolTable& table);
    virtual string getOpCode(SymbolTable &table, Section* curSection) = 0;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection);
};

class LabelLine : public Line {
public:

    LabelLine(string label, int pc);
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class Section : public Line {
public:
    string mName;
    int orderSymTable;

    Section(string label, string name, int pc = 0);
    void print(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class Equ : public Line {
public:
    string mSymbol;
    int mValue;

    Equ(string label, string symbol, int value, int pc);
    void print(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class Skip : public Line {
public:
    int mValue;

    Skip(string label, int value, int pc);
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class Declaration : public Line {
public:
    string mDecl;
    vector<DirectivePart*> mSymbolList;

    Declaration(string label, string decl, vector<DirectivePart*>& symbList, int pc);
    virtual ~Declaration();
    void print(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class WordList : public Line {
public:
    vector<DirectivePart*> mSymbolList;
    int mLineNo;

    WordList(string label, vector<DirectivePart*>& symbList, int pc, int lineNo);
    virtual ~WordList();
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    int missingSymbol(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) override;
};

class Instruction : public Line {
public:
    string mOperation;

    Instruction(string label, string op, int pc);
    void print(SymbolTable& table) override;

    static inline bool containsNumber(AdressMode am){ return (am != DATA6 && am != DATA7 && am != JMP6 && am != JMP7); }
};

class MulopInstruction : public Instruction {
public:
    string mRegS;
    string mRegD;

    MulopInstruction(string label, string op, string rd, string rs, int pc);
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class LdrInstruction : public Instruction {
public:
    string mRegD;
    Operand *mOperand;
    int mLineNo;

    LdrInstruction(string label, string rd, Operand* operand, int pc, int lineNo);
    virtual ~LdrInstruction();
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    inline bool additionalBytes() { return containsNumber(mOperand->mMode); }
    int missingSymbol(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) override;
};

class StrInstruction : public Instruction {
public:
    string mRegS;
    Operand *mOperand;
    int mLineNo;

    StrInstruction(string label, string rs, Operand *operand, int pc, int lineNo);
    virtual ~StrInstruction();
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    inline bool additionalBytes() { return containsNumber(mOperand->mMode); }
    int missingSymbol(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) override;
};

class DestOpInstruction : public Instruction {
public:
    string mRegD;

    DestOpInstruction(string label, string op, string rd, int pc);
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};

class JmpOpInstruction : public Instruction {
public:
    Operand* mOperand;
    int mLineNo;

    JmpOpInstruction(string label, string op, Operand* operand, int pc, int lineNo);
    virtual ~JmpOpInstruction();
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    inline bool additionalBytes() { return containsNumber(mOperand->mMode); }
    int missingSymbol(SymbolTable& table) override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
    virtual void updateRelTable(RelocationTable& relTable, SymbolTable&  symbolTable, Section* curSection) override;
};

class NoOperandInstruction : public Instruction {
public:
    NoOperandInstruction(string label, string op, int pc);
    void print(SymbolTable& table) override;
    virtual int getLength() override;
    virtual string getOpCode(SymbolTable &table, Section* curSection) override;
};