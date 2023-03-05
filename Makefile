SRCS = Assembler.o SymbolTable.o Helpers.o Line.o parser.cpp lexer.cpp 
CC = g++
CXXFLAGS=-g -std=c++11

test: all
	cat test.s | ./Assembler test.s
	
all: clean $(SRCS)
	$(CC) $(SRCS) -o Assembler

Assembler.o: parser.cpp lexer.cpp SymbolTable.h Helpers.cpp SymbolTable.cpp Assembler.h Assembler.cpp
	$(CC) -c Assembler.cpp

	
%.o: %.cpp
	$(CC) -c -MMD $<

lexer.cpp: lexer.l Helpers.cpp
	flex lexer.l

parser.cpp: parser.y lexer.l Helpers.cpp
	bison -v -y parser.y

clean:
	rm -rf *.o lexer.cpp lexer.h parser.cpp parser.h Assembler
