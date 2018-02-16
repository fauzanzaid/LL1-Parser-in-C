#ifndef INCLUDE_GUARD_FC41E67B8AC9429A8C4C6898EFB5E4FE
#define INCLUDE_GUARD_FC41E67B8AC9429A8C4C6898EFB5E4FE


///////////
// Types //
///////////

typedef enum{
	PARSER_INITIALIZE_RESULT_SUCCESS,
	PARSER_INITIALIZE_RESULT_FAIL_FIRST = -1,
	PARSER_INITIALIZE_RESULT_FAIL_FOLLOW = -2
} Parser_InitializeResult_type;

typedef enum{
	PARSER_STEP_RESULT_SUCCESS,
	PARSER_STEP_RESULT_FAIL = -1,
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
 * @return                      Pointer to ParserLL1 struct
 */
ParserLL1 *ParserLL1_new(int *variable_symbols, int len_variable_symbols, int *terminal_symbols, int len_terminal_symbols, int start_symbol, int empty_symbol, int end_symbol);

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
 * @param variable_symbol       The LHS symbol
 * @param expansion_symbols     Array of RHS symbols in order. Can be NULL for
 * empty string
 * @param len_expansion_symbols Length of array. Can be zero
 */
void ParserLL1_add_rule(ParserLL1 *psr_ptr, int variable_symbol, int *expansion_symbols, int len_expansion_symbols);

/**
 * Calculates the first and follow sets for each symbol and initializes the
 * parse table
 * @param psr_ptr Pointer to ParserLL1 struct
 * @return        Status
 * @retval PARSER_INITIALIZE_RESULT_SUCCESS     Initialization successful
 * @retval PARSER_INITIALIZE_RESULT_FAIL_FIRST  Failed to calculate first sets
 * @retval PARSER_INITIALIZE_RESULT_FAIL_FOLLOW Failed to calculate follow sets 
 */
Parser_InitializeResult_type ParserLL1_initialize_rules(ParserLL1 *psr_ptr);


/////////
// Run //
/////////

/**
 * Attempts to process @p symbol if possible
 * @param  psr_ptr Pointer to ParserLL1 struct
 * @param  symbol  Input symbol to process
 * @return         Status
 * @retval PARSER_STEP_RESULT_SUCCESS Processing successful
 * @retval PARSER_STEP_RESULT_FAIL Processing failed
 */
Parser_StepResult_type ParserLL1_step(ParserLL1 *psr_ptr, int symbol);


#endif
