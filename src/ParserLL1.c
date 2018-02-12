#include "ParserLL1.h"


/////////////////////
// Data Structures //
/////////////////////

typedef struct ParserLL1{

}ParserLL1;


////////////////////////////////
// Constructors & Destructors //
////////////////////////////////

ParserLL1 *ParserLL1_new(int *variable_symbols, int len_variable_symbols, int *terminal_symbols, int terminal_symbols, int start_symbol){
	
}

void ParserLL1_destroy(ParserLL1 *psr_ptr){
	
}


//////////////////////
// Production rules //
//////////////////////

void ParserLL1_add_rule(ParserLL1 *psr_ptr, int variable_symbol, int *expansion_symbols, int len_expansion_symbols){
	
}

Parser_InitializeResult_type ParserLL1_initialize_rules(ParserLL1 *psr_ptr){
	
}


/////////
// Run //
/////////

Parser_StepResult_type ParserLL1_step(ParserLL1 *psr_ptr, int symbol){
	
}
