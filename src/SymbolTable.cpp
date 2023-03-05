#include "SymbolTable.h"
#include "Line.h"
#include <iomanip>
#include <fstream>

int SymbolEntry::sOrdinal = 0;


SymbolEntry::SymbolEntry(string symbolName, Section* section, int value, bool isSection, Visibility visibility){
	mName = symbolName;
	mSection = section;
	mValue = value;
	mIsSection = isSection;
	mVisibility = visibility;
}

ofstream& operator<<(ofstream& os, const SymbolEntry& entry){
	os<<setw(20)<<entry.mName<<setw(20)<<entry.mSection->orderSymTable<<setw(20)<<std::hex<<entry.mValue<<setw(20)<<(string)((entry.mVisibility==local)?"local":"global")<<setw(20)<<entry.mOrdinal<<endl;
	return os;
}

shared_ptr<Section> SymbolTable::absoluteSection = make_shared<Section>("", "absolute");
shared_ptr<Section> SymbolTable::undefSection = make_shared<Section>("", "undefined");

SymbolTable::SymbolTable(){
    addEntry("undefined", undefSection.get(), 0, true);
    addEntry("absolute", absoluteSection.get(), 0, true);
}

bool SymbolTable::isEqu(string symbol){
	SymbolEntry* s = findEntry(symbol);
	if (s->mSection == absoluteSection.get()) return true;
	return false;
}
SymbolEntry* SymbolTable::findEntry(string symbolName) {
	for (auto& entry : mTable) {
		if (entry.mName == symbolName)
			return &entry;
	}
	return nullptr;
}

void SymbolTable::addEntry(string symbolName, Section* section, int value, bool isSection, Visibility visibility){
	SymbolEntry s(symbolName, section, value, isSection, visibility);
	mTable.push_back(s);
}

void SymbolTable::sortSections(){
	for (int i=0; i<mTable.size(); i++) {
		auto& entry1 = mTable[i];
		if (entry1.mIsSection){
			entry1.mSection->orderSymTable = i;
			continue;
		} 
		for (int j=i+1; j<mTable.size(); j++){
			auto& entry2 = mTable[j];
			if (entry2.mIsSection){
				swap(entry1, entry2);
			}
		}
	}
	for (int i=0; i<mTable.size(); i++){
		mTable[i].mOrdinal = i;
	}
}

void SymbolTable::updateGlobals(){
	for (auto& e:mTable){
		if (e.mVisibility == extH)
			e.mVisibility = global;
	}
}

ofstream& operator<<(ofstream& os, const SymbolTable& table){
	os<<setw(20)<<"Name"<<setw(20)<<"Section"<<setw(20)<<"Value"<<setw(20)<<"Visibility"<<setw(20)<<"Number"<<endl;
	
	for (auto& entry : table.mTable) {
		os<<entry;
	}
	return os;
}

RelocationEntry::RelocationEntry(int offset, RelocationType relType, int value){
	mValue = value;
	mRelocationType = relType;
	mOffset = offset;
}

ofstream& operator<<(ofstream& os, const RelocationEntry& entry){
	os<<setw(20)<<entry.mOffset<<setw(20)<<(entry.mRelocationType == R_16? "R_16":"R_16_PC")<<setw(20)<<std::hex<<entry.mValue<<endl;
	return os;
}

void RelocationTable::addEntry(int offset, RelocationType relType, int value){
	RelocationEntry entry(offset, relType, value);
	mTable.push_back(entry);
}

ofstream& operator<<(ofstream& os, const RelocationTable& table){
	os<<setw(20)<<"Offset"<<setw(20)<<"Type"<<setw(20)<<"Value"<<endl;
	
	for (auto& entry : table.mTable) {
		os<<entry;
	}
	return os;
}