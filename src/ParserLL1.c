#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ParserLL1.h"
#include "ParseTree.h"
#include "Token.h"
#include "LinkedList.h"
#include "HashTable.h"
#include "BitSet.h"

#include <stdio.h>

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

	// Minimum and maximum of all symbols
	int symbols_min, symbols_max;

	int start_symbol;
	int empty_symbol;
	int end_symbol;

	// Set for terminals, clear for variables
	BitSet *symbol_class_set;


	// Rules

	HashTable *rule_table;

	// First set table does not capture epsilon reachability 
	BitSet *nullable_set;
	HashTable *first_table;
	HashTable *follow_table;
	HashTable *parse_table;

	// Parse tree

	int (*token_to_symbol)(Token *);
	LinkedList *stack;
	ParseTree_Node *tree;

}ParserLL1;

typedef struct Rule Rule;

typedef struct Rule{
	int variable_symbol;
	int *expansion_symbols;
	int len_expansion_symbols;

	// Next rule with the same variable symbol
	Rule *next;
}Rule;


/////////////////////////////////
// Private Function Prototypes //
/////////////////////////////////

static Rule *Rule_new(int variable_symbol, int *expansion_symbols, int len_expansion_symbols);

static void Rule_destroy(Rule *rul_ptr);

static int hash_function(void *key);

static int key_compare(void *key1, void *key2);

static void calculate_first_table(ParserLL1 *psr_ptr);

static void calculate_follow_table(ParserLL1 *psr_ptr);

static void populate_parse_table(ParserLL1 *psr_ptr);


////////////////////////////////
// Constructors & Destructors //
////////////////////////////////

ParserLL1 *ParserLL1_new(int *variable_symbols, int len_variable_symbols, int *terminal_symbols, int len_terminal_symbols, int start_symbol, int empty_symbol, int end_symbol, int (*token_to_symbol)(Token *)){

	// Allocate
	ParserLL1 *psr_ptr = malloc( sizeof(ParserLL1) );


	// Copy parameters
	psr_ptr->variable_symbols = variable_symbols;
	psr_ptr->len_variable_symbols = len_variable_symbols;
	psr_ptr->terminal_symbols = terminal_symbols;
	psr_ptr->len_terminal_symbols = len_terminal_symbols;
	psr_ptr->start_symbol = start_symbol;
	psr_ptr->empty_symbol = empty_symbol;
	psr_ptr->end_symbol = end_symbol;

	psr_ptr->token_to_symbol = token_to_symbol;


	// Calc minimum and maximum
	psr_ptr->variable_symbols_min = INT_MAX;
	psr_ptr->variable_symbols_max = INT_MIN;
	psr_ptr->terminal_symbols_min = INT_MAX;
	psr_ptr->terminal_symbols_max = INT_MIN;

	for (int i = 0; i < len_variable_symbols; ++i){
		if(variable_symbols[i] < psr_ptr->variable_symbols_min)
			psr_ptr->variable_symbols_min = variable_symbols[i];
		if(variable_symbols[i] > psr_ptr->variable_symbols_max)
			psr_ptr->variable_symbols_max = variable_symbols[i];
	}

	for (int i = 0; i < len_terminal_symbols; ++i){
		if(terminal_symbols[i] < psr_ptr->terminal_symbols_min)
			psr_ptr->terminal_symbols_min = terminal_symbols[i];
		if(terminal_symbols[i] > psr_ptr->terminal_symbols_max)
			psr_ptr->terminal_symbols_max = terminal_symbols[i];
	}

	if(psr_ptr->terminal_symbols_min < psr_ptr->variable_symbols_min)
		psr_ptr->symbols_min = psr_ptr->terminal_symbols_min;
	else
		psr_ptr->symbols_min = psr_ptr->variable_symbols_min;

	if(psr_ptr->terminal_symbols_max > psr_ptr->variable_symbols_max)
		psr_ptr->symbols_max = psr_ptr->terminal_symbols_max;
	else
		psr_ptr->symbols_max = psr_ptr->variable_symbols_max;

	// Create and Initialize symbol class set
	psr_ptr->symbol_class_set = BitSet_new(psr_ptr->symbols_min, psr_ptr->symbols_max);
	for (int i = 0; i < len_terminal_symbols; ++i)
		BitSet_set_bit(psr_ptr->symbol_class_set, psr_ptr->terminal_symbols[i]);

	// Create rule table
	psr_ptr->rule_table = HashTable_new(len_variable_symbols, hash_function, key_compare);

	// Create nullable set
	psr_ptr->nullable_set = BitSet_new(psr_ptr->variable_symbols_min, psr_ptr->variable_symbols_max);

	// Create first and follow set table
	psr_ptr->first_table = HashTable_new(len_variable_symbols, hash_function, key_compare);
	psr_ptr->follow_table = HashTable_new(len_variable_symbols, hash_function, key_compare);

	// Create first and follow set for each table entry
	for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
		HashTable_add(psr_ptr->first_table, &(psr_ptr->variable_symbols[i]), (void*) BitSet_new(psr_ptr->terminal_symbols_min, psr_ptr->terminal_symbols_max) );
		HashTable_add(psr_ptr->follow_table, &(psr_ptr->variable_symbols[i]), (void*) BitSet_new(psr_ptr->terminal_symbols_min, psr_ptr->terminal_symbols_max) );
	}

	// Create parse table
	psr_ptr->parse_table = HashTable_new(len_variable_symbols, hash_function, key_compare);

	// Create row entry table for each variable symbol
	for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
		HashTable_add(psr_ptr->parse_table, &(psr_ptr->variable_symbols[i]), HashTable_new(psr_ptr->len_terminal_symbols, hash_function, key_compare));
	}

	// Create stack
	psr_ptr->stack = LinkedList_new();

	// Create Parse Tree. This is not freed when parser is destroyed, must be freed by the caller
	// Add end symbol and starting symbol to tree and stack
	psr_ptr->tree = ParseTree_Node_new(psr_ptr->start_symbol, NULL);
	LinkedList_push(psr_ptr->stack, ParseTree_Node_new(psr_ptr->end_symbol, NULL) );
	LinkedList_push(psr_ptr->stack, psr_ptr->tree);

	return psr_ptr;
}

void ParserLL1_destroy(ParserLL1 *psr_ptr){
	// Free symbol class set
	BitSet_destroy(psr_ptr->symbol_class_set);

	// Free rule_table and rules
	for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
		Rule *rul_ptr = HashTable_get( psr_ptr->rule_table, (void*) &(psr_ptr->variable_symbols[i]) );
		while(rul_ptr != NULL){
			Rule *next_rul_ptr = rul_ptr->next;
			Rule_destroy(rul_ptr);
			rul_ptr = next_rul_ptr;
		}
	}
	HashTable_destroy(psr_ptr->rule_table);

	// Free nullable set
	BitSet_destroy(psr_ptr->nullable_set);

	// Free first and follow set for each table entry
	for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
		BitSet_destroy( (BitSet*) HashTable_get(psr_ptr->first_table, (void*) &(psr_ptr->variable_symbols[i]) ) );
		BitSet_destroy( (BitSet*) HashTable_get(psr_ptr->follow_table, (void*) &(psr_ptr->variable_symbols[i]) ) );
	}

	// Free first and follow set table
	HashTable_destroy(psr_ptr->first_table);
	HashTable_destroy(psr_ptr->follow_table);


	// Free row entry table for each variable symbol
	for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
		HashTable_destroy( HashTable_get( psr_ptr->parse_table, &(psr_ptr->variable_symbols[i]) ) );
	}

	// Free parse table
	HashTable_destroy(psr_ptr->parse_table);

	// Free stack
	LinkedList_destroy(psr_ptr->stack);

	// Free parse tree
	ParseTree_Node_destroy(psr_ptr->tree);

	// Free parser
	free(psr_ptr);
}

static Rule *Rule_new(int variable_symbol, int *expansion_symbols, int len_expansion_symbols){
	Rule *rul_ptr = malloc( sizeof(Rule) );
	
	rul_ptr->variable_symbol = variable_symbol;
	rul_ptr->expansion_symbols = malloc( sizeof(int) * len_expansion_symbols );
	memcpy( rul_ptr->expansion_symbols, expansion_symbols, sizeof(int) * len_expansion_symbols );
	rul_ptr->len_expansion_symbols = len_expansion_symbols;
	rul_ptr->next = NULL;

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
	Rule *new_rul_ptr = Rule_new(variable_symbol, expansion_symbols, len_expansion_symbols);
	Rule *prev_rul_ptr = HashTable_get(psr_ptr->rule_table, (void*) &(new_rul_ptr->variable_symbol) );

	if(prev_rul_ptr == NULL){
		// Need to add rule, no rule with same lhs exists. Use the key which
		// exists within the rule, as it will exist outside this function's
		// scope
		new_rul_ptr->next = NULL;
		HashTable_add(psr_ptr->rule_table, (void*) &(new_rul_ptr->variable_symbol), (void*) new_rul_ptr);
	}

	else{
		// Need to add rule to already existing rule list
		new_rul_ptr->next = prev_rul_ptr;
		HashTable_set(psr_ptr->rule_table, (void*) &(new_rul_ptr->variable_symbol), (void*) new_rul_ptr);
	}
}

static void calculate_first_table(ParserLL1 *psr_ptr){

	// Add empty symbol to nullable set
	BitSet_set_bit(psr_ptr->nullable_set, psr_ptr->empty_symbol);

	int flag_change = 1;
	while(flag_change){
		flag_change = 0;

		for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
			// For each variable symbol
			int variable_symbol = psr_ptr->variable_symbols[i];
			// Will be set to 1 if variable symbol is nullable, else 0
			int flag_nullable;

			// No rules expand empty symbol
			if(variable_symbol == psr_ptr->empty_symbol)
				continue;


			BitSet *var_first_set_ptr = HashTable_get(psr_ptr->first_table, (void*) &variable_symbol);
			Rule *rul_ptr = HashTable_get(psr_ptr->rule_table, (void*) &variable_symbol );

			while(rul_ptr != NULL){
				// For each expansion of the variable symbol

				for (int j = 0; j < rul_ptr->len_expansion_symbols; ++j){
					// For each symbol in expansion
					int expansion_symbol = rul_ptr->expansion_symbols[j];

					if( BitSet_get_bit(psr_ptr->symbol_class_set, expansion_symbol) == 1 ){
						// Symbol is terminal

						if( BitSet_get_bit(var_first_set_ptr, expansion_symbol) == 0 ){
							// Not in first set, add
							BitSet_set_bit(var_first_set_ptr, expansion_symbol);
							flag_change = 1;
						}

						// This rule cannot be nullable, as it has a terminal symbol
						flag_nullable = 0;

						// No need to look at further symbols
						break;
					}

					else{
						// Symbol is not a terminal

						// Get first set of expansion symbol
						BitSet *exp_first_set_ptr = HashTable_get(psr_ptr->first_table, (void*) &expansion_symbol);
						// Temporary set to find changes
						BitSet *tmp_set_ptr = BitSet_clone(exp_first_set_ptr);
						BitSet_subtract(tmp_set_ptr, var_first_set_ptr);

						if(BitSet_get_any(tmp_set_ptr) == 1){
							// New terminals exist to be added to first set
							BitSet_or(var_first_set_ptr, exp_first_set_ptr);
							flag_change = 1;
						}

						BitSet_destroy(tmp_set_ptr);

						if( BitSet_get_bit(psr_ptr->nullable_set, expansion_symbol) == 0 ){
							// expansion symbol not nullable
							flag_nullable = 0;
							// No need to look further
							break;
						}
						else{
							// Chance that this rule may be nullable. Check further 
							flag_nullable = 1;
							continue;
						}
					}

				}

				if(flag_nullable == 1){
					// The variable symbol is nullable

					if( BitSet_get_bit(psr_ptr->nullable_set, variable_symbol) == 0 ){
						// Not yet added to nullable set
						BitSet_set_bit(psr_ptr->nullable_set, variable_symbol);
						flag_change = 1;
					}
				}

				rul_ptr = rul_ptr->next;
			}
		}
	}
}

static void calculate_follow_table(ParserLL1 *psr_ptr){
	// TODO add end of input to follow set of start symbol
	BitSet *start_follow_set_ptr = HashTable_get(psr_ptr->follow_table, (void*) &(psr_ptr->start_symbol) );
	BitSet_set_bit(start_follow_set_ptr, psr_ptr->end_symbol);

	int flag_change = 1;
	while(flag_change){
		flag_change = 0;

		for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
			// For each variable symbol
			int variable_symbol = psr_ptr->variable_symbols[i];

			// Get follow set of lhs
			BitSet *var_follow_set_ptr = HashTable_get(psr_ptr->follow_table, (void*) &variable_symbol);


			Rule *rul_ptr = HashTable_get(psr_ptr->rule_table, (void*) &variable_symbol );

			while(rul_ptr != NULL){
				// For each expansion of the variable symbol

				// Flag to check if follow of lhs shoulf be included in the
				// expansion symbol. Initialized to 1, as the last expansion symbol, if
				// it is a non terminal, which will be iterated first,
				// necessarily contains follow of lhs.
				int flag_nullable = 1;

				// Iterate in reverse, quicker in case of nullable expansion
				// symbols
				for (int j = rul_ptr->len_expansion_symbols - 1; j >= 0; --j){
					// For each symbol in expansion
					int expansion_symbol = rul_ptr->expansion_symbols[j];

					// No follow set of empty symbol
					if(expansion_symbol == psr_ptr->empty_symbol)
						continue;

					if( BitSet_get_bit(psr_ptr->symbol_class_set, expansion_symbol) == 0 ){
						// Symbol is not terminal

						// Get follow set of expansion symbol
						BitSet *exp_follow_set_ptr = HashTable_get(psr_ptr->follow_table, (void*) &expansion_symbol);

						if(flag_nullable == 1){
							// Add follow of lhs

							// Temporary set to find changes
							BitSet *tmp_set_ptr = BitSet_clone(var_follow_set_ptr);
							BitSet_subtract(tmp_set_ptr, exp_follow_set_ptr);


							if(BitSet_get_any(tmp_set_ptr) == 1){
								// New terminals exist to be added to follow set

								BitSet_or(exp_follow_set_ptr, var_follow_set_ptr);
								flag_change = 1;
							}

							BitSet_destroy(tmp_set_ptr);
						}

						if( BitSet_get_bit(psr_ptr->nullable_set, expansion_symbol) == 0 ){
							// expansion is no longer nullable
							flag_nullable = 0;
						}


						if(j < rul_ptr->len_expansion_symbols - 1){
							// Symbol is not the last in rule, can add first set of next symbol
							int next_expansion_symbol = rul_ptr->expansion_symbols[j+1];

							if( BitSet_get_bit(psr_ptr->symbol_class_set, next_expansion_symbol) == 1 ){
								// Add the terminal symbol

								if( BitSet_get_bit(exp_follow_set_ptr, next_expansion_symbol) == 0 ){
									// Not yet set
									BitSet_set_bit(exp_follow_set_ptr, next_expansion_symbol);
									flag_change = 1;
								}
							}

							else{
								// Add first set of following symbol

								BitSet *next_exp_first_set_ptr = HashTable_get(psr_ptr->first_table, (void*) &next_expansion_symbol);

								// Temporary set to find changes
								BitSet *tmp_set_ptr = BitSet_clone(next_exp_first_set_ptr);
								BitSet_subtract(tmp_set_ptr, exp_follow_set_ptr);

								if(BitSet_get_any(tmp_set_ptr) == 1){
									// New terminals exist to be added to follow set
									BitSet_or(exp_follow_set_ptr, next_exp_first_set_ptr);
									flag_change = 1;
								}

								BitSet_destroy(tmp_set_ptr);
							}
						}
					}

					else{
						// Terminal symbol, no follow set

						// Set this to 0 as it is no longer possibe to have a
						// null expansion
						flag_nullable = 0;
					}

				}

				rul_ptr = rul_ptr->next;
			}
		}
	}
}

static void populate_parse_table(ParserLL1 *psr_ptr){
	for (int i = 0; i < psr_ptr->len_variable_symbols; ++i){
		// For each variable

		int variable_symbol = psr_ptr->variable_symbols[i];
		int *variable_symbol_ptr = &(psr_ptr->variable_symbols[i]);
		HashTable* var_row_tbl_ptr = HashTable_get(psr_ptr->parse_table, &variable_symbol);

		Rule *rul_ptr = HashTable_get(psr_ptr->rule_table, (void*) &variable_symbol );

		while(rul_ptr != NULL){
			// For each expansion of the variable symbol

			int expansion_symbol = rul_ptr->expansion_symbols[0];
			// Need pointer for hashtable key
			int *expansion_symbol_ptr = &(rul_ptr->expansion_symbols[0]);

			if( BitSet_get_bit(psr_ptr->symbol_class_set, expansion_symbol) == 1 ){
				// Symbol is terminal. First set is itself
				HashTable_add(var_row_tbl_ptr, expansion_symbol_ptr, rul_ptr);
				// printf("populate_parse_table : \t\t\t\t\t[%d,%d]\n", variable_symbol, expansion_symbol);
			}

			else{
				// Symbol is not terminal. Need to add rule for each symbol in
				// first set

				BitSet *exp_first_set_ptr = HashTable_get(psr_ptr->first_table, expansion_symbol_ptr);

				for (int j = 0; j < psr_ptr->len_terminal_symbols; ++j){
					if( BitSet_get_bit(exp_first_set_ptr, psr_ptr->terminal_symbols[j]) == 1 ){
						HashTable_add(var_row_tbl_ptr, &(psr_ptr->terminal_symbols[j]), rul_ptr);
						// printf("populate_parse_table : \t\t\t\t\t[%d,%d]\n", variable_symbol, psr_ptr->terminal_symbols[j]);
					}
				}

				// Check if rule is nullable
				int flag_nullable = 1;
				for (int j = 0; j < rul_ptr->len_expansion_symbols; ++j){
					if( BitSet_get_bit(psr_ptr->nullable_set, rul_ptr->expansion_symbols[j]) == 0 ){
						// Not nullable
						flag_nullable = 0;
						break;
					}
				}

				// Add if rule is nullable
				if( flag_nullable == 1 ){
					// Need to add rule for each symbol in follow set as the
					// rule is nullable

					BitSet *exp_follow_set_ptr = HashTable_get(psr_ptr->follow_table, variable_symbol_ptr);

					for (int j = 0; j < psr_ptr->len_terminal_symbols; ++j){
						if( BitSet_get_bit(exp_follow_set_ptr, psr_ptr->terminal_symbols[j]) == 1 ){
							HashTable_add(var_row_tbl_ptr, &(psr_ptr->terminal_symbols[j]), rul_ptr);
							// printf("populate_parse_table : \t\t\t\t\t[%d,%d]\n", variable_symbol, psr_ptr->terminal_symbols[j]);
						}
					}
				}
			}

			rul_ptr = rul_ptr->next;
		}
	}
}

void ParserLL1_initialize_rules(ParserLL1 *psr_ptr){
	calculate_first_table(psr_ptr);
	calculate_follow_table(psr_ptr);
	populate_parse_table(psr_ptr);
}


/////////
// Run //
/////////

Parser_StepResult_type ParserLL1_step(ParserLL1 *psr_ptr, Token *tkn_ptr){
	int lookahead_symbol = psr_ptr->token_to_symbol(tkn_ptr);

	// Check if symbol is valid terminal
	if(lookahead_symbol < psr_ptr->terminal_symbols_min || lookahead_symbol > psr_ptr->terminal_symbols_max){

		// Free token, as not added to parse tree, will be lost
		// otherwise
		Token_destroy(tkn_ptr);

		return PARSER_STEP_RESULT_UNKNOWN_INPUT;
	}

	while(1){
		// Loop until top of the stack is a terminal

		ParseTree_Node *top_node_ptr = LinkedList_peek(psr_ptr->stack);
		int top_symbol = top_node_ptr->symbol;

		// printf("top=%d\t", top_symbol);

		if( BitSet_get_bit(psr_ptr->symbol_class_set, top_symbol) == 1 ){
			// Top of the stack is a terminal

			if(lookahead_symbol == top_symbol){
				// Match
				// printf("Match\n");
				top_node_ptr->tkn_ptr = tkn_ptr;

				if(top_symbol == psr_ptr->end_symbol){
					// End of stack reached, parsing over

					// Need to free end node, as it does not belong to tree
					ParseTree_Node_destroy( LinkedList_pop(psr_ptr->stack) );
					return PARSER_STEP_RESULT_SUCCESS;
				}

				else{
					// Stack not empty, require more input

					// No need to free popped node, already exists in tree
					LinkedList_pop(psr_ptr->stack);
					return PARSER_STEP_RESULT_MORE_INPUT;
				}
			}

			else{
				// Parsing error, lookahead does not match top of stack
				// Free token, as not added to parse tree, will be lost
				// otherwise
				Token_destroy(tkn_ptr);

				return PARSER_STEP_RESULT_FAIL;
			}
		}

		else{
			// Top of stack is non termainl, need to expand

			// Get the row corresponding to top symbol
			HashTable *var_row_tbl_ptr = HashTable_get(psr_ptr->parse_table, (void *) &top_symbol);

			// Get the rule correspond to lookahead from row
			Rule *rul_ptr = HashTable_get(var_row_tbl_ptr, (void *) &lookahead_symbol);

			if(rul_ptr != NULL){
				// Rule exists, expand rule

				// No need to free popped node, already exists in tree
				LinkedList_pop(psr_ptr->stack);

				// Traverse rule list in reverse
				for (int i = rul_ptr->len_expansion_symbols - 1; i >= 0; --i){
					int expansion_symbol = rul_ptr->expansion_symbols[i];

					if( expansion_symbol == psr_ptr->empty_symbol ){
						// No need to push empty symbol onto stack
						continue;
					}

					else{
						// Add a new node to tree
						ParseTree_Node *child_node_ptr = ParseTree_Node_add_child_left_end(top_node_ptr, expansion_symbol, NULL);
						// Also push node onto stack
						LinkedList_push(psr_ptr->stack, child_node_ptr);
					}
				}
			}

			else{
				// Parsing error

				// Free token, as not added to parse tree, will be lost
				// otherwise
				Token_destroy(tkn_ptr);

				return PARSER_STEP_RESULT_FAIL;
			}
		}
	}
}

ParseTree *ParserLL1_get_parse_tree(ParserLL1 *psr_ptr){
	return psr_ptr->tree;
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
