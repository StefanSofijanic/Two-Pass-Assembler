
#include "Assembler.h"

#include "lexer.h"
#include "parser.h"
#include "Helpers.h"
using namespace std;
#include <iostream>
#include <istream>
#include <fstream>


void Assembler::FirstPass(){

}

void Assembler::SecondPass(){

}

 
int main(int argc, char *argv[]){
    try{
        if (argc<2)
            __throw_runtime_error("Invalid command arguments, expected [-o output_file] input_file");
        string secondParam(argv[1]);
        if (argc == 4 && secondParam=="-o" || argc == 2) {

            //FILE *lexer_file = fopen("test123.txt", "r");
            FILE *lexer_file = fopen(argc==4?argv[3]:argv[1], "r");
            yyin = lexer_file;

            ofstream outf(argc==4?argv[2]:"log.o");
            outf.clear();
            if (argv[1]=="-o"){
                outf.open(argv[2]);
            }

            int ret = yyparse();//1st pass

            if (ret)
                return 1;

            print_prog();

            rearrange_symbol_table();
            print_symbol_table(outf);

            second_run(outf);

            outf.close();
            delete_prog();
        }
        else {
            __throw_runtime_error("Invalid command arguments, expected [-o output_file] input_file");
        }

        return 0;
    }
	catch (exception& e)
	{
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}
}