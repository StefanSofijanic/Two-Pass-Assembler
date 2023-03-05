#pragma once
#include <vector>
#include <string>
#include <memory>

using namespace std;
class Section;

enum Visibility { global, local, extH };
class SymbolEntry {
public:
	static int sOrdinal;

	string mName;
	Section* mSection;
	int mValue;
	Visibility mVisibility;
	bool mIsSection;
	int mOrdinal;


	SymbolEntry(string symbolName, Section* section, int value, bool isSection, Visibility visibility);
	bool isSection();

	friend ofstream& operator<<(ofstream& os, const SymbolEntry& entry);
};

class SymbolTable {
	vector <SymbolEntry> mTable;

public:
	SymbolTable();
	SymbolEntry* findEntry(string symbolName);
	void addEntry(string symbolName, Section* section, int value, bool isSection, Visibility visibility = local);
	void sortSections();
	void updateGlobals();

	friend ofstream& operator<<(ofstream& os, const SymbolTable& entry);
	bool isEqu(string symbol);

	
	static shared_ptr<Section> absoluteSection;
	static shared_ptr<Section> undefSection;
};
enum RelocationType { R_16, R_16_PC };

class RelocationEntry{
public:
	int mOffset;
	RelocationType mRelocationType;
	int mValue;

	RelocationEntry(int offset, RelocationType relType, int value);
	friend ofstream& operator<<(ofstream& os, const RelocationEntry& entry);
};

class RelocationTable {
	vector<RelocationEntry> mTable;
public:

	void addEntry(int offset, RelocationType relType, int value);
	friend ofstream& operator<<(ofstream& os, const RelocationTable& table);
	void clear() { mTable.clear(); }
};