#include <stdlib.h>
#include <string.h>

#include "ParserLL1.h"
#include "LinkedList.h"


/////////////////////
// Data Structures //
/////////////////////

typedef struct ParserLL1{

	// Symbols

	int *variable_symbols;
	int len_variable_symbols;

	int *terminal_symbols;
	int len_terminal_symbols;

	int start_symbol;


	// Rules

	LinkedList *rules;


	// State

	LinkedList *stack;

}ParserLL1;

typedef struct Rule{
	int variable_symbol;
	int *expansion_symbols;
	int len_expansion_symbols;
}Rule;


/////////////////////////////////
// Private Function Prototypes //
/////////////////////////////////

static Rule *Rule_new(int variable_symbol, int *expansion_symbols, int len_expansion_symbols);

static void Rule_destroy(Rule *rul_ptr);


////////////////////////////////
// Constructors & Destructors //
////////////////////////////////

ParserLL1 *ParserLL1_new(int *variable_symbols, int len_variable_symbols, int *terminal_symbols, int len_terminal_symbols, int start_symbol){

	// Allocate
	ParserLL1 *psr_ptr = malloc( sizeof(ParserLL1) );

	// Copy parameters
	psr_ptr->variable_symbols = variable_symbols;
	psr_ptr->len_variable_symbols = len_variable_symbols;
	psr_ptr->terminal_symbols = terminal_symbols;
	psr_ptr->len_terminal_symbols = len_terminal_symbols;
	psr_ptr->start_symbol = start_symbol;

	// Initialize rule list
	psr_ptr->rules = LinkedList_new();

	// Initialize stack
	psr_ptr->stack = LinkedList_new();
}

void ParserLL1_destroy(ParserLL1 *psr_ptr){
	// Free rules
	LinkedListIterator *itr_ptr = LinkedListIterator_new(psr_ptr->rules);
	LinkedListIterator_move_to_first(itr_ptr);
	Rule *rul_ptr;
	while(1){
		rul_ptr = LinkedListIterator_get_item(itr_ptr);
		if(rul_ptr == NULL)	break;
		Rule_destroy(rul_ptr);
		LinkedListIterator_move_to_next(itr_ptr);
	}
	LinkedListIterator_destroy(itr_ptr);
	LinkedList_destroy(psr_ptr->rules);

	// Free stack
	free(psr_ptr->stack);

	// Free parser
	free(psr_ptr);
}

static Rule *Rule_new(int variable_symbol, int *expansion_symbols, int len_expansion_symbols){
	Rule *rul_ptr = malloc( sizeof(Rule) );
	
	rul_ptr->variable_symbol = variable_symbol;
	rul_ptr->expansion_symbols = malloc( sizeof(int) * len_expansion_symbols );
	memcpy( rul_ptr->expansion_symbols, expansion_symbols, sizeof(int) * len_expansion_symbols );
	rul_ptr->len_expansion_symbols = len_expansion_symbols;

	return rul_ptr;
}

static void Rule_destroy(Rule *rul_ptr){
	free(rul_ptr->expansion_symbols);
	free(rul_ptr);
}


//////////////////////
// Production rules //
//////////////////////

void ParserLL1_add_rule(ParserLL1 *psr_ptr, int variable_symbol, int *expansion_symbols, int len_expansion_symbols){
	Rule *rul_ptr = Rule_new(variable_symbol, expansion_symbols, len_expansion_symbols);
	LinkedList_pushback(psr_ptr->rules, rul_ptr);
}

Parser_InitializeResult_type ParserLL1_initialize_rules(ParserLL1 *psr_ptr){
	
}


/////////
// Run //
/////////

Parser_StepResult_type ParserLL1_step(ParserLL1 *psr_ptr, int symbol){
	
}
