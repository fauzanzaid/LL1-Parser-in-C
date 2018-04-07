#ifndef INCLUDE_GUARD_FC41E67B8AC9429A8C4C6898EFB5E4FE
#define INCLUDE_GUARD_FC41E67B8AC9429A8C4C6898EFB5E4FE

#include "Token.h"
#include "ParseTree.h"

///////////
// Types //
///////////

typedef enum{
	PARSER_STEP_RESULT_MORE_INPUT,
	PARSER_STEP_RESULT_SUCCESS,
	PARSER_STEP_RESULT_FAIL = -1,
	PARSER_STEP_RESULT_UNKNOWN_INPUT = -2,
	PARSER_STEP_RESULT_HALTED = -3,
} Parser_StepResult_type;


/////////////////////
// Data Structures //
/////////////////////

typedef struct ParserLL1 ParserLL1;


////////////////////////////////
// Constructors & Destructors //
////////////////////////////////

/**
 * Allocates and initializes a ParserLL1 struct and returns a pointer to it.
 * Each symbol must be uniquely identified by an integer. Arrays must be kept
 * allocated by the user for the lifetime of the ParserLL1 struct
 * @param  variable_symbols     Array of non terminal or variably symbols
 * @param  len_variable_symbols Length of array
 * @param  terminal_symbols     Array of terminal symbols
 * @param  len_terminal_symbols Length of array
 * @param  start_symbol         The start symbol
 * @param  token_to_symbol      A user defined function that takes a terminal
 * token as parameter and outputs the corresponding terminal symbol
 * @param  symbol_to_string     A user defined function that takes a symbol
 * identifier as input and returns a pointer to user allocated null terminated
 * string which can be printed. The string must be kept allocated until the
 * lifetime of the ParserLL1 struct
 * @return                      Pointer to ParserLL1 struct
 */
ParserLL1 *ParserLL1_new(int *variable_symbols, int len_variable_symbols, int *terminal_symbols, int len_terminal_symbols, int start_symbol, int empty_symbol, int end_symbol, int *forget_terminal_symbols, int len_forget_terminal_symbols, int (*token_to_symbol)(Token *), char *(*symbol_to_string)(int), void (*token_to_value)(Token *, char *, int));

/**
 * Deallocates all internally allocated memory to the struct
 * @param psr_ptr Pointer to ParserLL1 struct
 */
void ParserLL1_destroy(ParserLL1 *psr_ptr);


//////////////////////
// Production rules //
//////////////////////

/**
 * Adds a production rule to the grammar. Rule must conform to LL1 grammar.
 * Arrays values are copied by this function, passed arrays need not be kept
 * allocated by the user after calling this function
 * @param psr_ptr               Pointer to ParserLL1 struct
 * @param rule_num              Rule number. Non leaf parse tree nodes will have
 * this attribute to specify which rule was used to expand that node
 * @param variable_symbol       The LHS symbol
 * @param expansion_symbols     Array of RHS symbols in order. Can be NULL for
 * empty string
 * @param len_expansion_symbols Length of array. Can be zero
 */
void ParserLL1_add_rule(ParserLL1 *psr_ptr, int rule_num, int variable_symbol, int *expansion_symbols, int len_expansion_symbols);

/**
 * Calculates the first and follow sets for each symbol and initializes the
 * parse table
 * @param psr_ptr Pointer to ParserLL1 struct
 */
void ParserLL1_initialize_rules(ParserLL1 *psr_ptr);


/////////
// Run //
/////////

/**
 * Attempts to process @p symbol if possible
 * @param  psr_ptr  Pointer to ParserLL1 struct
 * @param  tkn_ptr  Input token to process
 * @return          Status
 * @retval PARSER_STEP_RESULT_MORE_INPUT Input more tokens
 * @retval PARSER_STEP_RESULT_SUCCESS    Parsing complete, parse tree completely
 * constructed
 * @retval PARSER_STEP_RESULT_FAIL       Parsing failed
 */
Parser_StepResult_type ParserLL1_step(ParserLL1 *psr_ptr, Token *tkn_ptr);

/**
 * Returns a pointer to the internally constructed parse tree, if it has been
 * completely constructed. Otherwise returns NULL. The tree must be freed by the
 * user if this function is called
 * @param  psr_ptr Pointer to ParserLL1 struct
 * @return         Pointer to ParseTree struct
 */
ParseTree *ParserLL1_get_parse_tree(ParserLL1 *psr_ptr);


////////////
// Errors //
////////////

/**
 * Prints information about each encountered error so far
 * @param psr_ptr Pointer to ParserLL1 struct
 */
void ParserLL1_print_errors(ParserLL1 *psr_ptr);

/**
 * Set if parsing errors should be printed immediately on stdout as soon as they
 * are detected
 * @param psr_ptr Pointer to ParserLL1 struct
 * @param val     0 to not print, non zero to print
 */
void ParserLL1_set_immediate_print_error(ParserLL1 *psr_ptr, int val);

#endif
