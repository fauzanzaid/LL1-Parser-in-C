#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ParserLL1.h"
#include "LinkedList.h"
#include "HashTable.h"
#include "BitSet.h"


///////////////
// Constants //
///////////////

static const int SYMBOL_CLASS_VARIABLE = 0;
static const int SYMBOL_CLASS_TERMINAL = 1;


/////////////////////
// Data Structures //
/////////////////////

typedef struct ParserLL1{

	// Symbols

	int *variable_symbols;
	int len_variable_symbols;
	// Minimum and maximum for BitSet
	int variable_symbols_min, variable_symbols_max;

	int *terminal_symbols;
	int len_terminal_symbols;
	// Minimum and maximum for BitSet
	int terminal_symbols_min, terminal_symbols_max;

	int start_symbol;
	int empty_symbol;

	HashTable *symbol_class_table;


	// Rules

	LinkedList *rules;

	BitSet *nullable_set;


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

static int hash_function(void *key);

static int key_compare(void *key1, void *key2);

static int calculate_nullable_set(ParserLL1 *psr_ptr);


////////////////////////////////
// Constructors & Destructors //
////////////////////////////////

ParserLL1 *ParserLL1_new(int *variable_symbols, int len_variable_symbols, int *terminal_symbols, int len_terminal_symbols, int start_symbol, int empty_symbol){

	// Allocate
	ParserLL1 *psr_ptr = malloc( sizeof(ParserLL1) );


	// Copy parameters
	psr_ptr->variable_symbols = variable_symbols;
	psr_ptr->len_variable_symbols = len_variable_symbols;
	psr_ptr->terminal_symbols = terminal_symbols;
	psr_ptr->len_terminal_symbols = len_terminal_symbols;
	psr_ptr->start_symbol = start_symbol;
	psr_ptr->empty_symbol = empty_symbol;


	// Initialize symbol class table, calc minimum and maximum
	psr_ptr->variable_symbols_min = INT_MAX;
	psr_ptr->variable_symbols_max = INT_MIN;
	psr_ptr->terminal_symbols_min = INT_MAX;
	psr_ptr->terminal_symbols_max = INT_MIN;
	psr_ptr->symbol_class_table = HashTable_new(len_variable_symbols + len_terminal_symbols, hash_function, key_compare);
	
	for (int i = 0; i < len_variable_symbols; ++i){
		HashTable_add(psr_ptr->symbol_class_table, (void*)&variable_symbols[i], (void*)&SYMBOL_CLASS_VARIABLE);
		
		if(variable_symbols[i] < psr_ptr->variable_symbols_min)
			psr_ptr->variable_symbols_min = variable_symbols[i];
		if(variable_symbols[i] > psr_ptr->variable_symbols_max)
			psr_ptr->variable_symbols_max = variable_symbols[i];
	}

	for (int i = 0; i < len_terminal_symbols; ++i){
		HashTable_add(psr_ptr->symbol_class_table, (void*)&terminal_symbols[i], (void*)&SYMBOL_CLASS_TERMINAL);

		if(terminal_symbols[i] < psr_ptr->terminal_symbols_min)
			psr_ptr->terminal_symbols_min = terminal_symbols[i];
		if(terminal_symbols[i] > psr_ptr->terminal_symbols_max)
			psr_ptr->terminal_symbols_max = terminal_symbols[i];
	}


	// Initialize rule list
	psr_ptr->rules = LinkedList_new();

	// Initialize stack
	psr_ptr->stack = LinkedList_new();
}

void ParserLL1_destroy(ParserLL1 *psr_ptr){
	// Free symbol class table
	HashTable_destroy(psr_ptr->symbol_class_table);

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


//////////
// Hash //
//////////

static int hash_function(void *key){
	return (*(int *)(key));
}

static int key_compare(void *key1, void *key2){
	return *(int *)(key1) - *(int *)(key2);
}
