/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"
#include "glk/jumps.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o The tokenizer doesn't differentiate between "&", "&&", and "and", but "&"
 *   is context sensitive -- either a logical and for numerics, or concaten-
 *   ation for strings.  As a result, "&&" and "and" also work as string
 *   concatenators when used in string expressions.  It may not be to spec,
 *   but we'll call this a "feature".
 */

/* Assorted definitions and constants. */
enum { MAX_NESTING_DEPTH = 32 };
static const sc_char NUL = '\0';
static const sc_char PERCENT = '%';
static const sc_char SINGLE_QUOTE = '\'';
static const sc_char DOUBLE_QUOTE = '"';


/*
 * Tokens.  Single character tokens are represented by their ascii value
 * (0-255), others by values above 255.  -1 represents a null token.  Because
 * '&' and '+' are context sensitive, the pseudo-token TOK_CONCATENATE
 * serves to indicate string concatenation -- it's never returned by the
 * tokenizer.
 */
enum {
	TOK_NONE = -1,
	TOK_ADD = '+', TOK_SUBTRACT = '-', TOK_MULTIPLY = '*', TOK_DIVIDE = '/',
	TOK_AND = '&', TOK_OR = '|',
	TOK_LPAREN = '(', TOK_RPAREN = ')', TOK_COMMA = ',', TOK_POWER = '^',
	TOK_EQUAL = '=', TOK_GREATER = '>', TOK_LESS = '<',

	TOK_IDENT = 256,
	TOK_INTEGER, TOK_STRING, TOK_VARIABLE, TOK_UMINUS, TOK_UPLUS,
	TOK_MOD, TOK_NOT_EQUAL, TOK_GREATER_EQ, TOK_LESS_EQ, TOK_IF,
	TOK_MIN, TOK_MAX, TOK_EITHER, TOK_RANDOM, TOK_INSTR, TOK_LEN, TOK_VAL,
	TOK_ABS, TOK_UPPER, TOK_LOWER, TOK_PROPER, TOK_RIGHT, TOK_LEFT, TOK_MID,
	TOK_STR, TOK_CONCATENATE,
	TOK_EOS
};

/*
 * Small tables tying multicharacter tokens strings to tokens.  At present,
 * the string lengths for names are not used.
 */
struct sc_expr_multichar_t {
	const sc_char *const name;
	const sc_int length;
	const sc_int token;
};

static const sc_expr_multichar_t FUNCTION_TOKENS[] = {
	{"either", 6, TOK_EITHER},
	{"proper", 6, TOK_PROPER}, {"pcase", 5, TOK_PROPER}, {"instr", 5, TOK_INSTR},
	{"upper", 5, TOK_UPPER}, {"ucase", 5, TOK_UPPER},
	{"lower", 5, TOK_LOWER}, {"lcase", 5, TOK_LOWER},
	{"right", 5, TOK_RIGHT}, {"left", 4, TOK_LEFT},
	{"rand", 4, TOK_RANDOM}, {"max", 3, TOK_MAX}, {"min", 3, TOK_MIN},
	{"mod", 3, TOK_MOD}, {"abs", 3, TOK_ABS}, {"len", 3, TOK_LEN},
	{"val", 3, TOK_VAL}, {"and", 3, TOK_AND}, {"mid", 3, TOK_MID},
	{"str", 3, TOK_STR}, {"or", 2, TOK_OR}, {"if", 2, TOK_IF},
	{NULL, 0, TOK_NONE}
};
static const sc_expr_multichar_t OPERATOR_TOKENS[] = {
	{"&&", 2, TOK_AND}, {"||", 2, TOK_OR},
	{"==", 2, TOK_EQUAL}, {"!=", 2, TOK_NOT_EQUAL},
	{"<>", 2, TOK_NOT_EQUAL}, {">=", 2, TOK_GREATER_EQ}, {"<=", 2, TOK_LESS_EQ},
	{NULL, 0, TOK_NONE}
};


/*
 * expr_multichar_search()
 *
 * Multicharacter token table search, returns the matching token, or
 * TOK_NONE if no match.
 */
static sc_int expr_multichar_search(const sc_char *name, const sc_expr_multichar_t *table) {
	const sc_expr_multichar_t *entry;

	/* Scan the table for a case-independent full string match. */
	for (entry = table; entry->name; entry++) {
		if (sc_strcasecmp(name, entry->name) == 0)
			break;
	}

	/* Return the token matched, or TOK_NONE. */
	return entry->name ? entry->token : (sc_int)TOK_NONE;
}


/* Tokenizer variables. */
static const sc_char *expr_expression = NULL;
static sc_int expr_index = 0;
static sc_vartype_t expr_token_value;
static sc_char *expr_temporary = NULL;
static sc_int expr_current_token = TOK_NONE;

/*
 * expr_tokenize_start()
 * expr_tokenize_end()
 *
 * Start and wrap up expression string tokenization.
 */
static void expr_tokenize_start(const sc_char *expression) {
	static sc_bool initialized = FALSE;

	/* On first call only, verify the string lengths in the tables. */
	if (!initialized) {
		const sc_expr_multichar_t *entry;

		/* Compare table lengths with string lengths. */
		for (entry = FUNCTION_TOKENS; entry->name; entry++) {
			if (entry->length != (sc_int) strlen(entry->name)) {
				sc_fatal("expr_tokenize_start:"
				         " token string length is wrong for \"%s\"\n",
				         entry->name);
			}
		}

		for (entry = OPERATOR_TOKENS; entry->name; entry++) {
			if (entry->length != (sc_int) strlen(entry->name)) {
				sc_fatal("expr_tokenize_start:"
				         " operator string length is wrong for \"%s\"\n",
				         entry->name);
			}
		}

		initialized = TRUE;
	}

	/* Save expression, and restart index. */
	expr_expression = expression;
	expr_index = 0;

	/* Allocate a temporary token value/literals string. */
	assert(!expr_temporary);
	expr_temporary = (sc_char *)sc_malloc(strlen(expression) + 1);

	/* Reset last token to none. */
	expr_current_token = TOK_NONE;
}

static void expr_tokenize_end(void) {
	/* Deallocate temporary strings, clear expression. */
	sc_free(expr_temporary);
	expr_temporary = NULL;
	expr_expression = NULL;
	expr_index = 0;
	expr_current_token = TOK_NONE;
}


/*
 * expr_next_token_unadjusted()
 * expr_next_token()
 *
 * Return the next token from the current expression.  The initial token may
 * be adjusted into a unary +/- depending on the value of the previous token.
 */
static sc_int expr_next_token_unadjusted(sc_vartype_t *token_value) {
	sc_int c;
	assert(expr_expression);

	/* Skip any and all leading whitespace. */
	do {
		c = expr_expression[expr_index++];
	} while (sc_isspace(c) && c != NUL);

	/* Return EOS if at expression end. */
	if (c == NUL) {
		expr_index--;
		return TOK_EOS;
	}

	/*
	 * Identify and return numerics.  We deal only with unsigned numbers here;
	 * the unary +/- tokens take care of any integer sign issues.
	 */
	else if (sc_isdigit(c)) {
		sc_int value;

		sscanf(expr_expression + expr_index - 1, "%ld", &value);

		while (sc_isdigit(c) && c != NUL)
			c = expr_expression[expr_index++];
		expr_index--;

		token_value->integer = value;
		return TOK_INTEGER;
	}

	/* Identify and return variable references. */
	else if (c == PERCENT) {
		sc_int index_;

		/* Copy variable name. */
		c = expr_expression[expr_index++];
		for (index_ = 0; c != PERCENT && c != NUL;) {
			expr_temporary[index_++] = c;
			c = expr_expression[expr_index++];
		}
		expr_temporary[index_++] = NUL;

		if (c == NUL) {
			sc_error("expr_next_token_unadjusted:"
			         " warning: unterminated variable name\n");
			expr_index--;
		}

		/* Return a variable name. */
		token_value->string = expr_temporary;
		return TOK_VARIABLE;
	}

	/* Identify and return string literals. */
	else if (c == DOUBLE_QUOTE || c == SINGLE_QUOTE) {
		sc_int index_;
		sc_char quote;

		/* Copy maximal string literal. */
		quote = c;
		c = expr_expression[expr_index++];
		for (index_ = 0; c != quote && c != NUL;) {
			expr_temporary[index_++] = c;
			c = expr_expression[expr_index++];
		}
		expr_temporary[index_++] = NUL;

		if (c == NUL) {
			sc_error("expr_next_token_unadjusted:"
			         " warning: unterminated string literal\n");
			expr_index--;
		}

		/* Return string literal. */
		token_value->string = expr_temporary;
		return TOK_STRING;
	}

	/* Identify ids and other multicharacter tokens. */
	else if (sc_isalpha(c)) {
		sc_int index_, token;

		/*
		 * Copy maximal alphabetical string.  While an ident would normally
		 * be alpha followed by zero or more alnum, for Adrift purposes we
		 * use only alpha -- all idents should really be "functions", and
		 * in particular we want to see "mod7" as "mod" and 7 separately.
		 */
		for (index_ = 0; sc_isalpha(c) && c != NUL;) {
			expr_temporary[index_++] = c;
			c = expr_expression[expr_index++];
		}
		expr_index--;
		expr_temporary[index_++] = NUL;

		/*
		 * Check for a function name, and if known, return that, otherwise
		 * return a bare id.
		 */
		token = expr_multichar_search(expr_temporary, FUNCTION_TOKENS);
		if (token == TOK_NONE) {
			token_value->string = expr_temporary;
			return TOK_IDENT;
		} else
			return token;
	}

	/*
	 * Last chance check for two-character (multichar) operators, and if none
	 * then return a single-character token.
	 */
	else {
		sc_char operator_[3];
		sc_int token;

		/*
		 * Build a two-character string.  If we happen to be at the last
		 * expression character, we'll pick up the expression NUL into
		 * operator_[1], so no need to special case end of expression here.
		 */
		operator_[0] = c;
		operator_[1] = expr_expression[expr_index];
		operator_[2] = NUL;

		/* Search for this two-character operator. */
		if (operator_[0] != NUL && operator_[1] != NUL) {
			token = expr_multichar_search(operator_, OPERATOR_TOKENS);
			if (token != TOK_NONE) {
				/* Matched, so advance expression index and return this token. */
				expr_index++;
				return token;
			}
		}

		/*
		 * No match, or at last expression character; return a single character
		 * token.
		 */
		return c;
	}
}

static sc_int expr_next_token(void) {
	sc_int token;
	sc_vartype_t token_value;

	/*
	 * Get the basic next token.  We may adjust it later for unary minus/plus
	 * depending on what it is, and the prior token.
	 */
	token_value.voidp = NULL;
	token = expr_next_token_unadjusted(&token_value);

	/* Special handling for unary minus/plus signs. */
	if (token == TOK_SUBTRACT || token == TOK_ADD) {
		/*
		 * Unary minus/plus if prior token was an operator or a comparison, left
		 * parenthesis, or comma, or if there was no prior token.
		 */
		switch (expr_current_token) {
		case TOK_MOD:
		case TOK_POWER:
		case TOK_ADD:
		case TOK_SUBTRACT:
		case TOK_MULTIPLY:
		case TOK_DIVIDE:
		case TOK_AND:
		case TOK_OR:
		case TOK_EQUAL:
		case TOK_GREATER:
		case TOK_LESS:
		case TOK_NOT_EQUAL:
		case TOK_GREATER_EQ:
		case TOK_LESS_EQ:
		case TOK_LPAREN:
		case TOK_COMMA:
		case TOK_NONE:
			token = (token == TOK_SUBTRACT) ? TOK_UMINUS : TOK_UPLUS;
			break;

		default:
			break;
		}
	}

	/* Set current token to the one just found, and return it. */
	expr_current_token = token;
	expr_token_value = token_value;
	return token;
}


/*
 * expr_current_token_value()
 *
 * Return the token value of the current token.  Undefined if the current
 * token is not numeric, an id, or a variable.
 */
static void expr_current_token_value(sc_vartype_t *value) {
	/* Quick check that the value is a valid one. */
	switch (expr_current_token) {
	case TOK_INTEGER:
	case TOK_STRING:
	case TOK_VARIABLE:
	case TOK_IDENT:
		break;

	default:
		sc_fatal("expr_current_token_value:"
		         " taking undefined token value, %ld\n", expr_current_token);
	}

	/* Return value. */
	*value = expr_token_value;
}


/*
 * Evaluation values stack, uses a variable type so it can contain both
 * integers and strings, and flags strings for possible garbage collection
 * on parse errors.
 */
struct sc_stack_t {
	sc_bool is_collectible;
	sc_vartype_t value;
};
static sc_stack_t expr_eval_stack[MAX_NESTING_DEPTH];
static sc_int expr_eval_stack_index = 0;

/* Variables set to reference for %...% values. */
static sc_var_setref_t expr_varset = NULL;

/*
 * expr_eval_start()
 *
 * Reset the evaluation stack to an empty state, and register the variables
 * set to use when referencing %...% variables.
 */
static void expr_eval_start(sc_var_setref_t vars) {
	expr_eval_stack_index = 0;
	expr_varset = vars;
}


/*
 * expr_eval_garbage_collect()
 *
 * In case of parse error, empty out and free all collectible malloced
 * strings left in the evaluation array.
 */
static void expr_eval_garbage_collect(void) {
	sc_int index_;

	/*
	 * Find and free all collectible strings still in the stack.  We have to
	 * free through mutable string rather than const string.
	 */
	for (index_ = 0; index_ < expr_eval_stack_index; index_++) {
		if (expr_eval_stack[index_].is_collectible)
			sc_free(expr_eval_stack[index_].value.mutable_string);
	}

	/* Reset the stack index, for clarity and neatness. */
	expr_eval_stack_index = 0;
}


/*
 * expr_eval_push_integer()
 * expr_eval_push_string()
 * expr_eval_push_alloced_string()
 *
 * Push a value onto the values stack.  Strings are malloc'ed and copied,
 * and the copy is placed onto the stack, unless _alloced_string() is used;
 * for this case, the input string is assumed to be already malloc'ed, and
 * the caller should not subsequently free the string.
 */
static void expr_eval_push_integer(sc_int value) {
	if (expr_eval_stack_index >= MAX_NESTING_DEPTH)
		sc_fatal("expr_eval_push_integer: stack overflow\n");

	expr_eval_stack[expr_eval_stack_index].is_collectible = FALSE;
	expr_eval_stack[expr_eval_stack_index++].value.integer = value;
}

static void expr_eval_push_string(const sc_char *value) {
	sc_char *value_copy;

	if (expr_eval_stack_index >= MAX_NESTING_DEPTH)
		sc_fatal("expr_eval_push_string: stack overflow\n");

	/* Push a copy of value. */
	value_copy = (sc_char *)sc_malloc(strlen(value) + 1);
	strcpy(value_copy, value);
	expr_eval_stack[expr_eval_stack_index].is_collectible = TRUE;
	expr_eval_stack[expr_eval_stack_index++].value.mutable_string = value_copy;
}

static void expr_eval_push_alloced_string(sc_char *value) {
	if (expr_eval_stack_index >= MAX_NESTING_DEPTH)
		sc_fatal("expr_eval_push_alloced_string: stack overflow\n");

	expr_eval_stack[expr_eval_stack_index].is_collectible = TRUE;
	expr_eval_stack[expr_eval_stack_index++].value.mutable_string = value;
}


/*
 * expr_eval_pop_integer()
 * expr_eval_pop_string()
 *
 * Pop values off the values stack.  Returned strings are malloc'ed copies,
 * and the caller is responsible for freeing them.
 */
static sc_int expr_eval_pop_integer(void) {
	if (expr_eval_stack_index == 0)
		sc_fatal("expr_eval_pop_integer: stack underflow\n");

	assert(!expr_eval_stack[expr_eval_stack_index - 1].is_collectible);
	return expr_eval_stack[--expr_eval_stack_index].value.integer;
}

static sc_char *expr_eval_pop_string(void) {
	if (expr_eval_stack_index == 0)
		sc_fatal("expr_eval_pop_string: stack underflow\n");

	/* Returns mutable string rather than const string. */
	assert(expr_eval_stack[expr_eval_stack_index - 1].is_collectible);
	return expr_eval_stack[--expr_eval_stack_index].value.mutable_string;
}


/*
 * expr_eval_result()
 *
 * Return the top of the values stack as the expression result.
 */
static void expr_eval_result(sc_vartype_t *vt_rvalue) {
	if (expr_eval_stack_index != 1)
		sc_fatal("expr_eval_result: values stack not completed\n");

	/* Clear down stack and return the top value. */
	expr_eval_stack_index = 0;
	*vt_rvalue = expr_eval_stack[0].value;
}


/*
 * expr_eval_abs()
 *
 * Return the absolute value of the given sc_int.  Replacement for labs(),
 * avoids tying sc_int to long types too closely.
 */
static sc_int expr_eval_abs(sc_int value) {
	return value < 0 ? -value : value;
}

/*
 * expr_eval_action
 *
 * Evaluate the effect of a token into the values stack.
 */
static void expr_eval_action(CONTEXT, sc_int token) {
	sc_vartype_t token_value;

	switch (token) {
	/* Handle tokens representing stack pushes. */
	case TOK_INTEGER:
		expr_current_token_value(&token_value);
		expr_eval_push_integer(token_value.integer);
		break;

	case TOK_STRING:
		expr_current_token_value(&token_value);
		expr_eval_push_string(token_value.string);
		break;

	case TOK_VARIABLE: {
		sc_vartype_t vt_rvalue;
		sc_int type;

		expr_current_token_value(&token_value);
		if (!var_get(expr_varset, token_value.string, &type, &vt_rvalue)) {
			sc_error("expr_eval_action:"
			         " undefined variable, %s\n", token_value.string);
			LONG_JUMP;
		}
		switch (type) {
		case VAR_INTEGER:
			expr_eval_push_integer(vt_rvalue.integer);
			break;

		case VAR_STRING:
			expr_eval_push_string(vt_rvalue.string);
			break;

		default:
			sc_fatal("expr_eval_action: bad variable type\n");
		}
		break;
	}

	/* Handle tokens representing functions returning numeric. */
	case TOK_IF: {
		sc_int test, val1, val2;

		/* Pop the test and alternatives, and push back result. */
		val2 = expr_eval_pop_integer();
		val1 = expr_eval_pop_integer();
		test = expr_eval_pop_integer();
		expr_eval_push_integer(test ? val1 : val2);
		break;
	}

	case TOK_MAX:
	case TOK_MIN: {
		sc_int argument_count, index_, result;

		/* Get argument count off the top of the stack. */
		argument_count = expr_eval_pop_integer();
		assert(argument_count > 0);

		/* Find the max or min of these stacked values. */
		result = expr_eval_pop_integer();
		for (index_ = 1; index_ < argument_count; index_++) {
			sc_int next;

			next = expr_eval_pop_integer();
			switch (token) {
			case TOK_MAX:
				result = (next > result) ? next : result;
				break;

			case TOK_MIN:
				result = (next < result) ? next : result;
				break;

			default:
				sc_fatal("expr_eval_action: bad token, %ld\n", token);
			}
		}

		/* Push back the result. */
		expr_eval_push_integer(result);
		break;
	}

	case TOK_EITHER: {
		sc_int argument_count, pick, index_;
		sc_int result = 0;

		/* Get argument count off the top of the stack. */
		argument_count = expr_eval_pop_integer();
		assert(argument_count > 0);

		/*
		 * Pick one of the top N items at random, then unstack all N and
		 * push back the value of the one picked.
		 */
		pick = sc_rand() % argument_count;
		for (index_ = 0; index_ < argument_count; index_++) {
			sc_int val;

			val = expr_eval_pop_integer();
			if (index_ == pick)
				result = val;
		}

		/* Push back the result. */
		expr_eval_push_integer(result);
		break;
	}

	case TOK_INSTR: {
		sc_char *val1, *val2, *search;
		sc_int result;

		/* Extract the two values to work on. */
		val2 = expr_eval_pop_string();
		val1 = expr_eval_pop_string();

		/*
		 * Search for the second in the first.  The result is the character
		 * position, starting at 1, or 0 if not found.  Then free the popped
		 * strings, and push back the result.
		 */
		search = (val1[0] != NUL) ? strstr(val1, val2) : NULL;
		result = (!search) ? 0 : search - val1 + 1;
		sc_free(val1);
		sc_free(val2);
		expr_eval_push_integer(result);
		break;
	}

	case TOK_LEN: {
		sc_char *val;
		sc_int result;

		/* Pop the top string, and push back its length. */
		val = expr_eval_pop_string();
		result = strlen(val);
		sc_free(val);
		expr_eval_push_integer(result);
		break;
	}

	case TOK_VAL: {
		sc_char *val;
		sc_int result = 0;

		/*
		 * Extract the string at stack top, and try to convert, returning
		 * zero if conversion fails.  Free the popped string, and push back
		 * the result.
		 */
		val = expr_eval_pop_string();
		sscanf(val, "%ld", &result);
		sc_free(val);
		expr_eval_push_integer(result);
		break;
	}

	/* Handle tokens representing unary numeric operations. */
	case TOK_UMINUS:
		expr_eval_push_integer(-(expr_eval_pop_integer()));
		break;

	case TOK_UPLUS:
		break;

	case TOK_ABS:
		expr_eval_push_integer(expr_eval_abs(expr_eval_pop_integer()));
		break;

	/* Handle tokens representing most binary numeric operations. */
	case TOK_ADD:
	case TOK_SUBTRACT:
	case TOK_MULTIPLY:
	case TOK_AND:
	case TOK_OR:
	case TOK_EQUAL:
	case TOK_GREATER:
	case TOK_LESS:
	case TOK_NOT_EQUAL:
	case TOK_GREATER_EQ:
	case TOK_LESS_EQ:
	case TOK_RANDOM: {
		sc_int val1, val2, result = 0;

		/* Extract the two values to work on. */
		val2 = expr_eval_pop_integer();
		val1 = expr_eval_pop_integer();

		/* Generate the result value. */
		switch (token) {
		case TOK_ADD:
			result = val1 + val2;
			break;
		case TOK_SUBTRACT:
			result = val1 - val2;
			break;
		case TOK_MULTIPLY:
			result = val1 * val2;
			break;
		case TOK_AND:
			result = val1 && val2;
			break;
		case TOK_OR:
			result = val1 || val2;
			break;
		case TOK_EQUAL:
			result = val1 == val2;
			break;
		case TOK_GREATER:
			result = val1 > val2;
			break;
		case TOK_LESS:
			result = val1 < val2;
			break;
		case TOK_NOT_EQUAL:
			result = val1 != val2;
			break;
		case TOK_GREATER_EQ:
			result = val1 >= val2;
			break;
		case TOK_LESS_EQ:
			result = val1 <= val2;
			break;
		case TOK_RANDOM:
			result = sc_randomint(val1, val2);
			break;
		default:
			sc_fatal("expr_eval_action: bad token, %ld\n", token);
		}

		/* Put result back at top of stack. */
		expr_eval_push_integer(result);
		break;
	}

	/* Handle division and modulus separately; they're "eccentric". */
	case TOK_DIVIDE:
	case TOK_MOD: {
		sc_int val1, val2, x, y, result = 0;

		/* Extract the two values to work on, complain about division by 0. */
		val2 = expr_eval_pop_integer();
		val1 = expr_eval_pop_integer();
		if (val2 == 0) {
			sc_error("expr_eval_action: attempt to divide by zero\n");
			expr_eval_push_integer(result);
			break;
		}

		/*
		 * ANSI/ISO C only defines integer division for positive values.
		 * Negative values usually work consistently across platforms, but are
		 * not guaranteed.  For maximum portability, then, here we'll work
		 * carefully with positive integers only.
		 */
		x = expr_eval_abs(val1);
		y = expr_eval_abs(val2);

		/* Generate the result value. */
		switch (token) {
		case TOK_DIVIDE:
			/*
			 * Adrift's division apparently works by dividing using floating
			 * point, then applying (asymmetrical) rounding, so we have to do
			 * the same here.
			 */
			result = ((val1 < 0) == (val2 < 0))
			         ? ((x / y) + (((x % y) * 2 >= y) ? 1 : 0))
			         : -((x / y) + (((x % y) * 2 >  y) ? 1 : 0));
			break;

		case TOK_MOD:
			/*
			 * Adrift also breaks numerical consistency by defining mod in a
			 * conventional (non-rounded), way, so that A=(AdivB)*B+AmodB
			 * does not hold.
			 */
			result = (val1 < 0) ? -(x % y) : (x % y);
			break;

		default:
			sc_fatal("expr_eval_action: bad token, %ld\n", token);
		}

		/* Put result back at top of stack. */
		expr_eval_push_integer(result);
		break;
	}

	/* Handle power individually, to avoid needing a maths library. */
	case TOK_POWER: {
		sc_int val1, val2, result;

		/* Extract the two values to work on. */
		val2 = expr_eval_pop_integer();
		val1 = expr_eval_pop_integer();

		/* Handle negative and zero power values first, as special cases. */
		if (val2 == 0)
			result = 1;
		else if (val2 < 0) {
			if (val1 == 0) {
				sc_error("expr_eval_action: attempt to divide by zero\n");
				result = 0;
			} else if (val1 == 1)
				result = val1;
			else if (val1 == -1)
				result = (-val2 & 1) ? val1 : -val1;
			else
				result = 0;
		} else {
			/* Raise to positive powers using the Russian Peasant algorithm. */
			while ((val2 & 1) == 0) {
				val1 = val1 * val1;
				val2 >>= 1;
			}

			result = val1;
			val2 >>= 1;
			while (val2 > 0) {
				val1 = val1 * val1;
				if (val2 & 1)
					result = result * val1;
				val2 >>= 1;
			}
		}

		/* Put result back at top of stack. */
		expr_eval_push_integer(result);
		break;
	}

	/* Handle tokens representing functions returning string. */
	case TOK_LEFT:
	case TOK_RIGHT: {
		sc_char *text;
		sc_int length;

		/*
		 * Extract the text and length.  If length is longer than text, or
		 * negative, do nothing.
		 */
		length = expr_eval_pop_integer();
		text = expr_eval_pop_string();
		if (length < 0 || length >= (sc_int) strlen(text)) {
			expr_eval_push_alloced_string(text);
			break;
		}

		/*
		 * Take the left or right segment -- for left, the operation is a
		 * simple truncation; for right, it's a memmove.
		 */
		switch (token) {
		case TOK_LEFT:
			text[length] = NUL;
			break;

		case TOK_RIGHT:
			memmove(text, text + strlen(text) - length, length + 1);
			break;

		default:
			sc_fatal("expr_eval_action: bad token, %ld\n", token);
		}

		/* Put result back at top of stack. */
		expr_eval_push_alloced_string(text);
		break;
	}

	case TOK_MID: {
		sc_char *text;
		sc_int length, start, limit;

		/*
		 * Extract the text, start, and length, re-basing start from 1 to 0,
		 * and calculate the limit on characters available for the move.
		 */
		length = expr_eval_pop_integer();
		start = expr_eval_pop_integer() - 1;
		text = expr_eval_pop_string();
		limit = strlen(text);

		/*
		 * Clamp ranges that roam outside the available text -- start less
		 * than 0 to 0, and greater than len(text) to len(text), and length
		 * less than 0 to 0, and off string end to string end.
		 */
		if (start < 0)
			start = 0;
		else if (start > limit)
			start = limit;
		if (length < 0)
			length = 0;
		else if (length > limit - start)
			length = limit - start;

		/* Move substring, terminate, and put back at top of stack. */
		memmove(text, text + start, length + 1);
		text[length] = NUL;
		expr_eval_push_alloced_string(text);
		break;
	}

	case TOK_STR: {
		sc_int val;
		sc_char buffer[32];

		/*
		 * Extract the value, convert it, and push back the resulting string.
		 * The leading space on positive values matches the Runner.
		 */
		val = expr_eval_pop_integer();
		sprintf(buffer, "% ld", val);
		expr_eval_push_string(buffer);
		break;
	}


	/* Handle tokens representing unary string operations. */
	case TOK_UPPER:
	case TOK_LOWER:
	case TOK_PROPER: {
		sc_char *text;
		sc_int index_;

		/* Extract the value to work on. */
		text = expr_eval_pop_string();

		/* Convert the entire string in place -- it's malloc'ed. */
		for (index_ = 0; text[index_] != NUL; index_++) {
			switch (token) {
			case TOK_UPPER:
				text[index_] = sc_toupper(text[index_]);
				break;

			case TOK_LOWER:
				text[index_] = sc_tolower(text[index_]);
				break;

			case TOK_PROPER:
				if (index_ == 0 || sc_isspace(text[index_ - 1]))
					text[index_] = sc_toupper(text[index_]);
				else
					text[index_] = sc_tolower(text[index_]);
				break;

			default:
				sc_fatal("expr_eval_action: bad token, %ld\n", token);
			}
		}

		/* Put result back at top of stack. */
		expr_eval_push_alloced_string(text);
		break;
	}

	/* Handle token representing binary string operation. */
	case TOK_CONCATENATE: {
		sc_char *text1, *text2;

		/* Extract the two texts to work on. */
		text2 = expr_eval_pop_string();
		text1 = expr_eval_pop_string();

		/*
		 * Resize text1 to be long enough for both, and concatenate, then
		 * free text2, and push back the concatenation.
		 */
		text1 = (sc_char *)sc_realloc(text1, strlen(text1) + strlen(text2) + 1);
		strcat(text1, text2);
		sc_free(text2);
		expr_eval_push_alloced_string(text1);
		break;
	}

	default:
		sc_fatal("expr_eval_action: bad token, %ld\n", token);
	}
}


/* Predictive parser lookahead token. */
static sc_int expr_parse_lookahead = TOK_NONE;

/* Forward declaration of factor parsers and string expression parser. */
static void expr_parse_numeric_factor(CONTEXT);
static void expr_parse_string_factor(CONTEXT);
static void expr_parse_string_expr(CONTEXT);

/*
 * expr_parse_match
 *
 * Match a token to the lookahead, then advance lookahead.
 */
static void expr_parse_match(CONTEXT, sc_int token) {
	if (expr_parse_lookahead == token)
		expr_parse_lookahead = expr_next_token();
	else {
		/* Syntax error. */
		sc_error("expr_parse_match: syntax error,"
		         " expected %ld, got %ld\n", expr_parse_lookahead, token);
		LONG_JUMP;
	}
}


/*
 * Numeric operator precedence table.  Operators are in order of precedence,
 * with the highest being a factor.  Each precedence entry permits several
 * listed tokens.  The end of the table (highest precedence) is marked by
 * a list with no operators (although in practice we need to put a TOK_NONE
 * in here since some C compilers won't accept { } as an empty initializer).
 */
struct sc_precedence_entry_t {
	const sc_int token_count;
	const sc_int tokens[6];
};
#if 0
/*
 * Conventional (BASIC, C) precedence table for the parser.  Exponentiation
 * has the highest precedence, then multiplicative operations, additive,
 * comparisons, and boolean combiners.
 */
static const sc_precedence_entry_t PRECEDENCE_TABLE[] = {
	{1, {TOK_OR}},
	{1, {TOK_AND}},
	{2, {TOK_EQUAL, TOK_NOT_EQUAL}},
	{4, {TOK_GREATER, TOK_LESS, TOK_GREATER_EQ, TOK_LESS_EQ}},
	{2, {TOK_ADD, TOK_SUBTRACT}},
	{3, {TOK_MULTIPLY, TOK_DIVIDE, TOK_MOD}},
	{1, {TOK_POWER}},
	{0, {TOK_NONE}}
};
#else
/*
 * Adrift-like precedence table for the parser.  Exponentiation and modulus
 * operations seem to be implemented at the same level as addition and
 * subtraction, and boolean 'and' and 'or' have equal precedence.
 */
static const sc_precedence_entry_t PRECEDENCE_TABLE[] = {
	{2, {TOK_OR, TOK_AND}},
	{
		6, {
			TOK_EQUAL, TOK_NOT_EQUAL,
			TOK_GREATER, TOK_LESS, TOK_GREATER_EQ, TOK_LESS_EQ
		}
	},
	{4, {TOK_ADD, TOK_SUBTRACT, TOK_POWER, TOK_MOD}},
	{2, {TOK_MULTIPLY, TOK_DIVIDE}},
	{0, {TOK_NONE}}
};
#endif


/*
 * expr_parse_contains_token()
 *
 * Helper for expr_parse_numeric_element().  Search the token list for the
 * entry passed in, and return TRUE if it contains the given token.
 */
static int expr_parse_contains_token(const sc_precedence_entry_t *entry, sc_int token) {
	sc_bool is_matched;
	sc_int index_;

	/* Search the entry's token list for the token passed in. */
	is_matched = FALSE;
	for (index_ = 0; index_ < entry->token_count; index_++) {
		if (entry->tokens[index_] == token) {
			is_matched = TRUE;
			break;
		}
	}

	return is_matched;
}


/*
 * expr_parse_numeric_element()
 *
 * Parse numeric expression elements.  This function uses the precedence table
 * to match tokens, then decide whether, and how, to recurse into itself, or
 * whether to parse a highest-precedence factor.
 */
static void expr_parse_numeric_element(CONTEXT, sc_int precedence) {
	const sc_precedence_entry_t *entry;

	/* See if the level passed in has listed tokens. */
	entry = PRECEDENCE_TABLE + precedence;
	if (entry->token_count == 0) {
		/* Precedence levels that hit the table end are factors. */
		CALL0(expr_parse_numeric_factor);
		return;
	}

	/*
	 * Parse initial higher-precedence factor, then others that associate
	 * with the given level.
	 */
	CALL1(expr_parse_numeric_element, precedence + 1);
	while (expr_parse_contains_token(entry, expr_parse_lookahead)) {
		sc_int token;

		/* Note token and match, parse next level, then action this token. */
		token = expr_parse_lookahead;
		CALL1(expr_parse_match, token);
		CALL1(expr_parse_numeric_element, precedence + 1);
		CALL1(expr_eval_action, token);
	}
}


/*
 * expr_parse_numeric_expr
 *
 * Parse a complete numeric (sub-)expression.
 */
static void expr_parse_numeric_expr(CONTEXT) {
	/* Call the parser of the lowest precedence operators. */
	CALL1(expr_parse_numeric_element, 0);
}


/*
 * expr_parse_numeric_factor()
 *
 * Parse a numeric expression factor.
 */
static void expr_parse_numeric_factor(CONTEXT) {
	/* Handle factors based on lookahead token. */
	switch (expr_parse_lookahead) {
	/* Handle straightforward factors first. */
	case TOK_LPAREN:
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		break;

	case TOK_UMINUS:
		CALL1(expr_parse_match, TOK_UMINUS);
		CALL0(expr_parse_numeric_factor);
		CALL1(expr_eval_action, TOK_UMINUS);
		break;

	case TOK_UPLUS:
		CALL1(expr_parse_match, TOK_UPLUS);
		CALL0(expr_parse_numeric_factor);
		break;

	case TOK_INTEGER:
		CALL1(expr_eval_action, TOK_INTEGER);
		CALL1(expr_parse_match, TOK_INTEGER);
		break;

	case TOK_VARIABLE: {
		sc_vartype_t token_value, vt_rvalue;
		sc_int type;

		expr_current_token_value(&token_value);
		if (!var_get(expr_varset, token_value.string, &type, &vt_rvalue)) {
			sc_error("expr_parse_numeric_factor:"
			         " undefined variable, %s\n", token_value.string);
			LONG_JUMP;
		}
		if (type != VAR_INTEGER) {
			sc_error("expr_parse_numeric_factor:"
			         " string variable in numeric context, %s\n",
			         token_value.string);
			LONG_JUMP;
		}
		CALL1(expr_eval_action, TOK_VARIABLE);
		CALL1(expr_parse_match, TOK_VARIABLE);
		break;
	}

	/* Handle functions as factors. */
	case TOK_ABS:
		/* Parse as "abs (val)". */
		CALL1(expr_parse_match, TOK_ABS);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_ABS);
		break;

	case TOK_IF:
		/* Parse as "if (boolean, val1, val2)". */
		CALL1(expr_parse_match, TOK_IF);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_IF);
		break;

	case TOK_RANDOM:
		/* Parse as "random (low, high)". */
		CALL1(expr_parse_match, TOK_RANDOM);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_RANDOM);
		break;

	case TOK_MAX:
	case TOK_MIN:
	case TOK_EITHER:
		/* Parse as "<func> (val1[,val2[,val3...]]])". */
	{
		sc_int token, argument_count;

		/* Match up the function name and opening parenthesis. */
		token = expr_parse_lookahead;
		CALL1(expr_parse_match, token);
		CALL1(expr_parse_match, TOK_LPAREN);

		/* Count variable number of arguments as they are stacked. */
		CALL0(expr_parse_numeric_expr);
		argument_count = 1;
		while (expr_parse_lookahead == TOK_COMMA) {
			CALL1(expr_parse_match, TOK_COMMA);
			CALL0(expr_parse_numeric_expr);
			argument_count++;
		}
		CALL1(expr_parse_match, TOK_RPAREN);

		/* Push additional value -- the count of arguments. */
		expr_eval_push_integer(argument_count);
		CALL1(expr_eval_action, token);
		break;
	}

	case TOK_INSTR:
		/* Parse as "instr (val1, val2)". */
		CALL1(expr_parse_match, TOK_INSTR);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_INSTR);
		break;

	case TOK_LEN:
		/* Parse as "len (val)". */
		CALL1(expr_parse_match, TOK_LEN);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_LEN);
		break;

	case TOK_VAL:
		/* Parse as "val (val)". */
		CALL1(expr_parse_match, TOK_VAL);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_VAL);
		break;

	case TOK_IDENT:
		/* Unrecognized function-type token. */
		sc_error("expr_parse_numeric_factor: syntax error, unknown ident\n");
		LONG_JUMP;

	default:
		/* Syntax error. */
		sc_error("expr_parse_numeric_factor:"
		         " syntax error, unexpected token, %ld\n", expr_parse_lookahead);
		LONG_JUMP;
	}
}


/*
 * expr_parse_string_expr()
 *
 * Parse a complete string (sub-)expression.
 */
static void expr_parse_string_expr(CONTEXT) {
	/*
	 * Parse a string factor, then all repeated concatenations.  Because the '+'
	 * and '&' are context sensitive, we have to invent/translate them into the
	 * otherwise unused TOK_CONCATENATE for evaluation.
	 */
	CALL0(expr_parse_string_factor);
	while (expr_parse_lookahead == TOK_AND || expr_parse_lookahead == TOK_ADD) {
		CALL1(expr_parse_match, expr_parse_lookahead);
		CALL0(expr_parse_string_factor);
		CALL1(expr_eval_action, TOK_CONCATENATE);
	}
}


/*
 * expr_parse_string_factor()
 *
 * Parse a string expression factor.
 */
static void expr_parse_string_factor(CONTEXT) {
	/* Handle factors based on lookahead token. */
	switch (expr_parse_lookahead) {
	/* Handle straightforward factors first. */
	case TOK_LPAREN:
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		break;

	case TOK_STRING:
		CALL1(expr_eval_action, TOK_STRING);
		CALL1(expr_parse_match, TOK_STRING);
		break;

	case TOK_VARIABLE: {
		sc_vartype_t token_value, vt_rvalue;
		sc_int type;

		expr_current_token_value(&token_value);
		if (!var_get(expr_varset, token_value.string, &type, &vt_rvalue)) {
			sc_error("expr_parse_string_factor:"
			         " undefined variable, %s\n", token_value.string);
			LONG_JUMP;
		}
		if (type != VAR_STRING) {
			sc_error("expr_parse_string_factor:"
			         " numeric variable in string context, %s\n",
			         token_value.string);
			LONG_JUMP;
		}
		CALL1(expr_eval_action, TOK_VARIABLE);
		CALL1(expr_parse_match, TOK_VARIABLE);
		break;
	}

	/* Handle functions as factors. */
	case TOK_UPPER:
	case TOK_LOWER:
	case TOK_PROPER:
		/* Parse as "<func> (text)". */
	{
		sc_int token;

		token = expr_parse_lookahead;
		CALL1(expr_parse_match, token);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, token);
		break;
	}

	case TOK_LEFT:
	case TOK_RIGHT:
		/* Parse as "<func> (text,length)". */
	{
		sc_int token;

		token = expr_parse_lookahead;
		CALL1(expr_parse_match, token);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, token);
		break;
	}

	case TOK_MID:
		/* Parse as "mid (text,start,length)". */
		CALL1(expr_parse_match, TOK_MID);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_string_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_COMMA);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_MID);
		break;

	case TOK_STR:
		/* Parse as "str (val)". */
		CALL1(expr_parse_match, TOK_STR);
		CALL1(expr_parse_match, TOK_LPAREN);
		CALL0(expr_parse_numeric_expr);
		CALL1(expr_parse_match, TOK_RPAREN);
		CALL1(expr_eval_action, TOK_STR);
		break;

	case TOK_IDENT:
		/* Unrecognized function-type token. */
		sc_error("expr_parse_string_factor: syntax error, unknown ident\n");
		LONG_JUMP;

	default:
		/* Syntax error. */
		sc_error("expr_parse_string_factor:"
		         " syntax error, unexpected token, %ld\n", expr_parse_lookahead);
		LONG_JUMP;
	}
}


/*
 * expr_evaluate_expression()
 *
 * Parse a string expression into a runtime values stack.  Return the
 * value of the expression.
 */
static sc_bool expr_evaluate_expression(const sc_char *expression, sc_var_setref_t vars,
		sc_int assign_type, sc_vartype_t *vt_rvalue) {
	assert(assign_type == VAR_INTEGER || assign_type == VAR_STRING);
	Context context;

	/* Reset values stack and start tokenizer. */
	expr_eval_start(vars);
	expr_tokenize_start(expression);

	// Try parsing an expression, and ensure it ends at string end. */
	expr_parse_lookahead = expr_next_token();
	if (assign_type == VAR_STRING)
		expr_parse_string_expr(context);
	else
		expr_parse_numeric_expr(context);
	if (!context._break)
		expr_parse_match(context, TOK_EOS);

	if (context._break) {
		/* Parse error -- clean up tokenizer, collect garbage, and fail. */
		expr_tokenize_end();
		expr_eval_garbage_collect();
		return FALSE;
	}

	/* Clean up tokenizer and return successfully with result. */
	expr_tokenize_end();
	expr_eval_result(vt_rvalue);
	return TRUE;
}


/*
 * expr_eval_numeric_expression()
 * expr_eval_string_expression()
 *
 * Public interfaces to expression evaluation.  Evaluate an expression, and
 * assign the result to either a numeric or a string.  For string expressions,
 * the return value is malloc'ed, and the caller is responsible for freeing
 * it.
 */
sc_bool expr_eval_numeric_expression(const sc_char *expression, sc_var_setref_t vars, sc_int *rvalue) {
	sc_vartype_t vt_rvalue;
	sc_bool status;
	assert(expression && vars && rvalue);

	/* Evaluate numeric expression, and return value if valid. */
	status = expr_evaluate_expression(expression, vars, VAR_INTEGER, &vt_rvalue);
	if (status)
		*rvalue = vt_rvalue.integer;
	return status;
}

sc_bool expr_eval_string_expression(const sc_char *expression, sc_var_setref_t vars, sc_char **rvalue) {
	sc_vartype_t vt_rvalue;
	sc_bool status;
	assert(expression && vars && rvalue);

	/* Evaluate string expression, and return value if valid. */
	status = expr_evaluate_expression(expression, vars, VAR_STRING, &vt_rvalue);
	if (status)
		*rvalue = vt_rvalue.mutable_string;
	return status;
}

} // End of namespace Adrift
} // End of namespace Glk
