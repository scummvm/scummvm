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
#include "glk/adrift/scgamest.h"
#include "glk/jumps.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Some of the "finer" points of pattern matching in relation to "*"
 *   wildcards, and %text%, are unknown.
 *
 * o The inclusion of part or all of prefixes in %character% and %object%
 *   matching may be right; then again, it may not be.
 */

/* Assorted definitions and constants. */
static const sc_char NUL = '\0';
static const sc_char MINUS = '-';
static const sc_char PLUS = '+';
static const sc_char PERCENT = '%';
static const sc_char *const WHITESPACE = "\t\n\v\f\r ";

/* Pattern matching trace flag. */
static sc_bool uip_trace = FALSE;

/* Enumeration of tokens.  TOK_NONE represents a non-occurring token. */
enum sc_uip_tok_t {
	TOK_NONE = 0,
	TOK_CHOICE, TOK_CHOICE_END, TOK_OPTIONAL, TOK_OPTIONAL_END,
	TOK_ALTERNATES_SEPARATOR,
	TOK_WILDCARD, TOK_WHITESPACE, TOK_WORD, TOK_VARIABLE,
	TOK_CHARACTER_REFERENCE, TOK_OBJECT_REFERENCE, TOK_NUMBER_REFERENCE,
	TOK_TEXT_REFERENCE, TOK_EOS
};

/*
 * Small table tying token strings to tokens.  Anything not whitespace and
 * not caught by the table is a plain TOK_WORD.
 */
struct sc_uip_token_entry_t {
	const sc_char *const name;
	const sc_int length;
	const sc_uip_tok_t token;
};

static const sc_uip_token_entry_t UIP_TOKENS[] = {
	{"[", 1, TOK_CHOICE}, {"]", 1, TOK_CHOICE_END},
	{"{", 1, TOK_OPTIONAL}, {"}", 1, TOK_OPTIONAL_END},
	{"/", 1, TOK_ALTERNATES_SEPARATOR},
	{"*", 1, TOK_WILDCARD},
	{"%character%", 11, TOK_CHARACTER_REFERENCE},
	{"%object%", 8, TOK_OBJECT_REFERENCE},
	{"%number%", 8, TOK_NUMBER_REFERENCE},
	{"%text%", 6, TOK_TEXT_REFERENCE},
	{NULL, 0, TOK_NONE}
};


/*
 * Tokenizer variables.  The temporary is used for keeping word token values.
 * For improved performance, we'll set it to indicate a static buffer if
 * short enough, otherwise it's allocated.
 */
static const sc_char *uip_pattern = NULL;
static sc_int uip_index = 0;
static const sc_char *uip_token_value;
enum { UIP_ALLOCATION_AVOIDANCE_SIZE = 128 };
static sc_char uip_static_temporary[UIP_ALLOCATION_AVOIDANCE_SIZE];
static sc_char *uip_temporary = NULL;


/*
 * uip_tokenize_start()
 * uip_tokenize_end()
 *
 * Start and wrap up pattern string tokenization.
 */
static void uip_tokenize_start(const sc_char *pattern) {
	static sc_bool initialized = FALSE;
	sc_int required;

	/* On first call only, verify the string lengths in the table. */
	if (!initialized) {
		const sc_uip_token_entry_t *entry;

		/* Compare table lengths with string lengths. */
		for (entry = UIP_TOKENS; entry->name; entry++) {
			if (entry->length != (sc_int) strlen(entry->name)) {
				sc_fatal("uip_tokenize_start:"
				         " table string length is wrong for \"%s\"\n",
				         entry->name);
			}
		}

		initialized = TRUE;
	}

	/* Save pattern, and restart index. */
	uip_pattern = pattern;
	uip_index = 0;

	/* Set up temporary; static if long enough, otherwise allocated. */
	required = strlen(pattern) + 1;
	uip_temporary = (required > UIP_ALLOCATION_AVOIDANCE_SIZE)
	                ? (sc_char *)sc_malloc(required) : uip_static_temporary;
}

static void uip_tokenize_end(void) {
	/* Deallocate temporary if required, and clear pattern and index. */
	if (uip_temporary != uip_static_temporary)
		sc_free(uip_temporary);
	uip_temporary = NULL;
	uip_pattern = NULL;
	uip_index = 0;
}


/*
 * uip_next_token()
 *
 * Return the next token from the current pattern.
 */
static sc_uip_tok_t uip_next_token(void) {
	const sc_uip_token_entry_t *entry;
	sc_char close;
	assert(uip_pattern);

	/* Get next character, return EOS if at pattern end. */
	if (uip_pattern[uip_index] == NUL) {
		uip_token_value = NULL;
		return TOK_EOS;
	}

	/* If whitespace, skip it, then return a whitespace token. */
	if (sc_isspace(uip_pattern[uip_index])) {
		uip_index++;
		while (sc_isspace(uip_pattern[uip_index])
		        && uip_pattern[uip_index] != NUL)
			uip_index++;
		uip_token_value = NULL;
		return TOK_WHITESPACE;
	}

	/* Search the table for matching strings. */
	for (entry = UIP_TOKENS; entry->name; entry++) {
		if (strncmp(uip_pattern + uip_index, entry->name, entry->length) == 0)
			break;
	}
	if (entry->name) {
		/* Advance over string, and return token. */
		uip_index += entry->length;
		uip_token_value = NULL;
		return entry->token;
	}

	/*
	 * Search for a non-special variable reference.  This is apparently an
	 * Adrift extension to the standard pattern match, allowing %user_var% to
	 * be used in patterns.  If found, return a variable with the name as the
	 * token value.  We can't interpolate the value into the string either
	 * here or earlier, so we have to save the variable's name, and retrieve
	 * it when we come to try the match.
	 */
	if (sscanf(uip_pattern + uip_index, "%%%[^%]%c", uip_temporary, &close) == 2
	        && close == PERCENT) {
		uip_index += strlen(uip_temporary) + 2;
		uip_token_value = uip_temporary;
		return TOK_VARIABLE;
	}

	/*
	 * Return a word.  This is a contiguous run of non-pattern-special, non-
	 * whitespace, non-percent characters
	 */
	sscanf(uip_pattern + uip_index, "%[^][/{}*% \f\n\r\t\v]", uip_temporary);
	uip_token_value = uip_temporary;
	uip_index += strlen(uip_temporary);
	return TOK_WORD;
}


/*
 * uip_current_token_value()
 *
 * Return the token value of the current token.  It is an error to call
 * here if the current token is not a TOK_WORD or TOK_VARIABLE.
 */
static const sc_char *uip_current_token_value(void) {
	/* If the token value is NULL, the current token isn't a word. */
	if (!uip_token_value) {
		sc_fatal("uip_current_token_value:"
		         " attempt to take undefined token value\n");
	}

	/* Return value. */
	return uip_token_value;
}


/*
 * Parsed pattern tree node definition.   The tree is a left child, right
 * sibling representation, with token type, and word at nodes of type TOK_WORD.
 * NODE_UNUSED must be zero to ensure that the statically allocated array that
 * forms the node pool appears initially as containing only unused nodes.
 */
enum sc_pttype_t {
	NODE_UNUSED = 0,
	NODE_CHOICE, NODE_OPTIONAL, NODE_WILDCARD, NODE_WHITESPACE,
	NODE_CHARACTER_REFERENCE, NODE_OBJECT_REFERENCE, NODE_TEXT_REFERENCE,
	NODE_NUMBER_REFERENCE, NODE_WORD, NODE_VARIABLE, NODE_LIST, NODE_EOS
};
struct sc_ptnode_s {
	struct sc_ptnode_s *left_child;
	struct sc_ptnode_s *right_sibling;

	sc_pttype_t type;
	sc_char *word;
	sc_bool is_allocated;
};
typedef sc_ptnode_s sc_ptnode_t;
typedef sc_ptnode_t *sc_ptnoderef_t;

/* Predictive parser lookahead token. */
static sc_uip_tok_t uip_parse_lookahead = TOK_NONE;

/* Parse tree for cleanup, and forward declaration of pattern list parser. */
static sc_ptnoderef_t uip_parse_tree = NULL;

/*
 * Pool of statically allocated nodes, for faster allocations.  Nodes are
 * first allocated from here, then by straight malloc() if this pool is empty.
 * An average game's peak node allocation seems to be around 40-50 nodes, so
 * allowing 128 here should be plenty.
 */
enum { UIP_NODE_POOL_SIZE = 128 };
static sc_ptnode_t uip_node_pool[UIP_NODE_POOL_SIZE];
static sc_int uip_node_pool_cursor = 0;
static sc_int uip_node_pool_available = UIP_NODE_POOL_SIZE;

/*
 * Words held at nodes are usually short (15 chars covers 95% of English), so
 * to avoid a lot of small allocations we use a pool of short strings, used
 * first, then by straight malloc() should the pool empty.
 */
enum { UIP_WORD_POOL_SIZE = 64, UIP_SHORT_WORD_SIZE = 16 };
struct sc_ptshortword_t {
	sc_bool is_in_use;
	sc_char word[UIP_SHORT_WORD_SIZE];
};
typedef sc_ptshortword_t *sc_ptshortwordref_t;
static sc_ptshortword_t uip_word_pool[UIP_WORD_POOL_SIZE];
static sc_int uip_word_pool_cursor = 0;
static sc_int uip_word_pool_available = UIP_WORD_POOL_SIZE;

// Forward declarations
static void uip_parse_list(CONTEXT, sc_ptnoderef_t list);


/*
 * uip_parse_matc
 *
 * Match a token to the lookahead, then advance lookahead.
 */
static void uip_parse_match(CONTEXT, sc_uip_tok_t token) {
	if (uip_parse_lookahead == token)
		uip_parse_lookahead = uip_next_token();
	else {
		/* Syntax error. */
		sc_error("uip_parse_match: syntax error, expected %ld, got %ld\n",
		         (sc_int) uip_parse_lookahead, (sc_int) token);
		LONG_JUMP;
	}
}


/*
 * uip_new_word()
 *
 * Return a string containing a copy of the word.  Uses a ring allocator to
 * allocate initially from static storage, for performance.  If this is
 * exhausted, backs off to standard allocation.
 */
static sc_char *uip_new_word(const sc_char *word) {
	sc_int required;

	/*
	 * Unless the pool is empty, search forwards from the next cursor position
	 * until an unused slot is found, or until the index wraps to the cursor.
	 */
	required = strlen(word) + 1;
	if (uip_word_pool_available > 0 && required <= UIP_SHORT_WORD_SIZE) {
		sc_int index_;
		sc_ptshortwordref_t shortword;

		index_ = (uip_word_pool_cursor + 1) % UIP_WORD_POOL_SIZE;
		while (index_ != uip_word_pool_cursor) {
			if (!uip_word_pool[index_].is_in_use)
				break;
			index_ = (index_ + 1) % UIP_WORD_POOL_SIZE;
		}

		if (uip_word_pool[index_].is_in_use)
			sc_fatal("uip_new_word: no free slot found in the words pool\n");

		/* Use the slot and update the pool cursor and free count. */
		shortword = uip_word_pool + index_;
		strcpy(shortword->word, word);
		shortword->is_in_use = TRUE;

		uip_word_pool_cursor = index_;
		uip_word_pool_available--;

		/* Return the address of the copied string. */
		return shortword->word;
	} else {
		sc_char *word_copy;

		/* Fall back to less efficient allocations. */
		word_copy = (sc_char *)sc_malloc(required);
		strcpy(word_copy, word);
		return word_copy;
	}
}


/*
 * uip_free_word()
 *
 * If the word was allocated, free its memory; if not, find its short word
 * pool entry and return it to the pool.
 */
static void uip_free_word(sc_char *word) {
	const sc_char *first_in_pool, *last_in_pool;

	/* Obtain the range of valid addresses for words from the word pool. */
	first_in_pool = uip_word_pool[0].word;
	last_in_pool = uip_word_pool[UIP_WORD_POOL_SIZE - 1].word;

	/* If from the pool, mark the entry as no longer in use, otherwise free. */
	if (word >= first_in_pool && word <= last_in_pool) {
		sc_int index_;
		sc_ptshortwordref_t shortword;

		/*
		 * Calculate the index to the word pool entry from which this short
		 * word was allocated.
		 */
		index_ = (word - first_in_pool) / sizeof(uip_word_pool[0]);
		shortword = uip_word_pool + index_;
		assert(shortword->word == word);

		shortword->is_in_use = FALSE;
		uip_word_pool_available++;
	} else
		sc_free(word);
}


/*
 * uip_new_node()
 *
 * Create a new node, populated with an initial type.  Uses a ring allocator
 * to allocate initially from static storage, for performance.  If this is
 * exhausted, backs off to standard allocation.
 */
static sc_ptnoderef_t uip_new_node(sc_pttype_t type) {
	sc_ptnoderef_t node;

	/*
	 * Unless the pool is empty, search forwards from the next cursor position
	 * until an unused slot is found, or until the index wraps to the cursor.
	 */
	if (uip_node_pool_available > 0) {
		sc_int index_;

		index_ = (uip_node_pool_cursor + 1) % UIP_NODE_POOL_SIZE;
		while (index_ != uip_node_pool_cursor) {
			if (uip_node_pool[index_].type == NODE_UNUSED)
				break;
			index_ = (index_ + 1) % UIP_NODE_POOL_SIZE;
		}

		if (uip_node_pool[index_].type != NODE_UNUSED)
			sc_fatal("uip_new_node: no free slot found in the nodes pool\n");

		/* Use the slot and update the pool cursor and free count. */
		node = uip_node_pool + index_;
		node->is_allocated = FALSE;

		uip_node_pool_cursor = index_;
		uip_node_pool_available--;
	} else {
		/* Fall back to less efficient allocations. */
		node = (sc_ptnoderef_t)sc_malloc(sizeof(*node));
		node->is_allocated = TRUE;
	}

	/* Fill in the remaining fields and return the new node. */
	node->left_child = NULL;
	node->right_sibling = NULL;
	node->type = type;
	node->word = NULL;

	return node;
}


/*
 * uip_destroy_node()
 *
 * Destroy a node, and any allocated word memory.  If the node was allocated,
 * free its memory; if not, return it to the pool.
 */
static void uip_destroy_node(sc_ptnoderef_t node) {
	/* Free any word contained at this node. */
	if (node->word)
		uip_free_word(node->word);

	/*
	 * If the node was allocated, poison memory and free it.  If it came from
	 * the node pool, set it to unused and update the availability count for
	 * the pool.
	 */
	if (node->is_allocated) {
		memset(node, 0xaa, sizeof(*node));
		sc_free(node);
	} else {
		node->type = NODE_UNUSED;
		uip_node_pool_available++;
	}
}


/*
 * uip_parse_new_list()
 * uip_parse_alternatives()
 *
 * Parse a set of .../.../... alternatives for choices and optionals.  The
 * first function is a helper, returning a newly constructed parsed list.
 */
static sc_ptnoderef_t uip_parse_new_list(CONTEXT) {
	sc_ptnoderef_t list;

	/* Create a new list node, parse into it, and return it. */
	list = uip_new_node(NODE_LIST);
	R0CALL1(uip_parse_list, list);
	return list;
}

static void uip_parse_alternatives(CONTEXT, sc_ptnoderef_t node) {
	sc_ptnoderef_t child;

	/* Parse initial alternative, then add other listed alternatives. */
	FUNC0(uip_parse_new_list, node->left_child);
	child = node->left_child;
	while (uip_parse_lookahead == TOK_ALTERNATES_SEPARATOR) {
		CALL1(uip_parse_match, TOK_ALTERNATES_SEPARATOR);
		FUNC0(uip_parse_new_list, child->right_sibling);
		child = child->right_sibling;
	}
}


/*
 * uip_parse_element()
 *
 * Parse a single pattern element.
 */
static sc_ptnoderef_t uip_parse_element(CONTEXT) {
	sc_ptnoderef_t node = NULL;
	sc_uip_tok_t token;

	/* Handle pattern element based on lookahead token. */
	switch (uip_parse_lookahead) {
	case TOK_WHITESPACE:
		R0CALL1(uip_parse_match, TOK_WHITESPACE);
		node = uip_new_node(NODE_WHITESPACE);
		break;

	case TOK_CHOICE:
		/* Parse a [...[/.../...]] choice. */
		R0CALL1(uip_parse_match, TOK_CHOICE);
		node = uip_new_node(NODE_CHOICE);
		R0CALL1(uip_parse_alternatives, node);
		R0CALL1(uip_parse_match, TOK_CHOICE_END);
		break;

	case TOK_OPTIONAL:
		/* Parse a {...[/.../...]} optional element. */
		R0CALL1(uip_parse_match, TOK_OPTIONAL);
		node = uip_new_node(NODE_OPTIONAL);
		R0CALL1(uip_parse_alternatives, node);
		R0CALL1(uip_parse_match, TOK_OPTIONAL_END);
		break;

	case TOK_WILDCARD:
	case TOK_CHARACTER_REFERENCE:
	case TOK_OBJECT_REFERENCE:
	case TOK_NUMBER_REFERENCE:
	case TOK_TEXT_REFERENCE:
		/* Parse %mumble% references and * wildcards. */
		token = uip_parse_lookahead;
		R0CALL1(uip_parse_match, token);
		switch (token) {
		case TOK_WILDCARD:
			node = uip_new_node(NODE_WILDCARD);
			break;
		case TOK_CHARACTER_REFERENCE:
			node = uip_new_node(NODE_CHARACTER_REFERENCE);
			break;
		case TOK_OBJECT_REFERENCE:
			node = uip_new_node(NODE_OBJECT_REFERENCE);
			break;
		case TOK_NUMBER_REFERENCE:
			node = uip_new_node(NODE_NUMBER_REFERENCE);
			break;
		case TOK_TEXT_REFERENCE:
			node = uip_new_node(NODE_TEXT_REFERENCE);
			break;
		default:
			sc_fatal("uip_parse_element: invalid token, %ld\n", (sc_int) token);
		}
		break;

	case TOK_WORD: {
		const sc_char *token_value;
		sc_char *word;

		/* Take a copy of the token's word value. */
		token_value = uip_current_token_value();
		word = uip_new_word(token_value);

		/* Store details in a word node. */
		R0CALL1(uip_parse_match, TOK_WORD);
		node = uip_new_node(NODE_WORD);
		node->word = word;
		break;
	}

	case TOK_VARIABLE: {
		const sc_char *token_value;
		sc_char *name;

		/* Take a copy of the token's variable name value. */
		token_value = uip_current_token_value();
		name = uip_new_word(token_value);

		/* Store details in a variable node, overloading word. */
		R0CALL1(uip_parse_match, TOK_VARIABLE);
		node = uip_new_node(NODE_VARIABLE);
		node->word = name;
		break;
	}

	default:
		/* Syntax error. */
		sc_error("uip_parse_element: syntax error,"
		         " unexpected token, %ld\n", (sc_int) uip_parse_lookahead);
		LONG_JUMP0;
	}

	/* Return the newly created node. */
	assert(node);
	return node;
}


/*
 * uip_parse_list()
 *
 * Parse a list of pattern elements.
 */
static void uip_parse_list(CONTEXT, sc_ptnoderef_t list) {
	sc_ptnoderef_t child, node;

	/* Add elements until a list terminator token is encountered. */
	child = list;
	while (TRUE) {
		switch (uip_parse_lookahead) {
		case TOK_CHOICE_END:
		case TOK_OPTIONAL_END:
		case TOK_ALTERNATES_SEPARATOR:
			/* Terminate list building and return. */
			return;

		case TOK_EOS:
			/* Place EOS at the appropriate link and return. */
			node = uip_new_node(NODE_EOS);
			if (child == list)
				child->left_child = node;
			else
				child->right_sibling = node;
			return;

		default:
			/* Add the next node at the appropriate link. */
			FUNC0(uip_parse_element, node);
			if (child == list) {
				child->left_child = node;
				child = child->left_child;
			} else {
				/*
				 * Make a special case of a choice or option next to another
				 * choice or option.  In this case, add an (invented) whitespace
				 * node, to ensure a match with suitable input.
				 */
				if ((child->type == NODE_OPTIONAL || child->type == NODE_CHOICE)
				        && (node->type == NODE_OPTIONAL || node->type == NODE_CHOICE)) {
					sc_ptnoderef_t whitespace;

					/* Interpose invented whitespace. */
					whitespace = uip_new_node(NODE_WHITESPACE);
					child->right_sibling = whitespace;
					child = child->right_sibling;
				}

				child->right_sibling = node;
				child = child->right_sibling;
			}
			continue;
		}
	}
}


/*
 * uip_destroy_tree()
 *
 * Free and destroy a parsed pattern tree.
 */
static void uip_destroy_tree(sc_ptnoderef_t node) {
	if (node) {
		/* Recursively destroy siblings, then left child. */
		uip_destroy_tree(node->right_sibling);
		uip_destroy_tree(node->left_child);

		/* Destroy the node itself. */
		uip_destroy_node(node);
	}
}


/*
 * uip_debug_dump_node()
 * uip_debug_dump()
 *
 * Print out a pattern match tree.
 */
static void uip_debug_dump_node(sc_ptnoderef_t node, sc_int depth) {
	/* End recursion on null node. */
	if (node) {
		sc_int index_;

		sc_trace(" ");
		for (index_ = 0; index_ < depth; index_++)
			sc_trace("  ");

		sc_trace("%p", (void *) node);
		switch (node->type) {
		case NODE_CHOICE:
			sc_trace(", choice");
			break;
		case NODE_OPTIONAL:
			sc_trace(", optional");
			break;
		case NODE_WILDCARD:
			sc_trace(", wildcard");
			break;
		case NODE_WHITESPACE:
			sc_trace(", whitespace");
			break;
		case NODE_CHARACTER_REFERENCE:
			sc_trace(", character");
			break;
		case NODE_OBJECT_REFERENCE:
			sc_trace(", object");
			break;
		case NODE_TEXT_REFERENCE:
			sc_trace(", text");
			break;
		case NODE_NUMBER_REFERENCE:
			sc_trace(", number");
			break;
		case NODE_WORD:
			sc_trace(", word \"%s\"", node->word);
			break;
		case NODE_VARIABLE:
			sc_trace(", variable \"%s\"", node->word);
			break;
		case NODE_LIST:
			sc_trace(", list");
			break;
		case NODE_EOS:
			sc_trace(", <eos>");
			break;
		default:
			sc_trace(", unknown type %ld", (sc_int) node->type);
			break;
		}
		if (node->left_child)
			sc_trace(", left child %p", (void *) node->left_child);
		if (node->right_sibling)
			sc_trace(", right sibling %p", (void *) node->right_sibling);
		sc_trace("\n");

		/* Recursively dump left child, then siblings. */
		uip_debug_dump_node(node->left_child, depth + 1);
		uip_debug_dump_node(node->right_sibling, depth);
	}
}

static void uip_debug_dump(void) {
	sc_trace("UIParser: debug dump follows...\n");
	if (uip_parse_tree) {
		sc_trace("uip_parse_tree = {\n");
		uip_debug_dump_node(uip_parse_tree, 0);
		sc_trace("}\n");
	} else
		sc_trace("uip_parse_tree = (nil)\n");
}


/* String matching variables. */
static const sc_char *uip_string = NULL;
static sc_int uip_posn = 0;
static sc_gameref_t uip_game = NULL;

/*
 * uip_match_start()
 * uip_match_end()
 *
 * Set up a string for matching to a pattern tree, and wrap up matching.
 */
static void uip_match_start(const sc_char *string, sc_gameref_t game) {
	/* Save string, and restart index. */
	uip_string = string;
	uip_posn = 0;

	/* Save the game we're working on. */
	uip_game = game;
}

static void uip_match_end(void) {
	/* Clear match target string, and variable set. */
	uip_string = NULL;
	uip_posn = 0;
	uip_game = NULL;
}


/*
 * uip_get_game()
 *
 * Safety wrapper to ensure module code sees a valid game when it requires
 * one.
 */
static sc_gameref_t uip_get_game(void) {
	assert(gs_is_game_valid(uip_game));
	return uip_game;
}


/* Forward declaration of low level node matcher. */
static sc_bool uip_match_node(sc_ptnoderef_t node);

/*
 * uip_match_eos()
 * uip_match_word()
 * uip_match_variable()
 * uip_match_whitespace()
 * uip_match_list()
 * uip_match_alternatives()
 * uip_match_choice()
 * uip_match_optional()
 * uip_match_wildcard()
 *
 * Text element and list/choice element match functions.  Return TRUE, and
 * advance position if necessary, on match, FALSE on no match, with position
 * unchanged.
 */
static sc_bool uip_match_eos(void) {
	/* Check that we hit the string's end. */
	return uip_string[uip_posn] == NUL;
}

static sc_bool uip_match_word(sc_ptnoderef_t node) {
	sc_int length;
	const sc_char *word;

	/* Get the word to match. */
	assert(node->word);
	word = node->word;

	/* Compare string text with this node's word, ignore case. */
	length = strlen(word);
	if (sc_strncasecmp(uip_string + uip_posn, word, length) == 0) {
		/* Word match, advance position and return. */
		uip_posn += length;
		return TRUE;
	}

	/* No match. */
	return FALSE;
}

static sc_bool uip_match_variable(sc_ptnoderef_t node) {
	const sc_gameref_t game = uip_get_game();
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int type;
	sc_vartype_t vt_rvalue;
	const sc_char *name;

	/* Get the variable name to match, from overloaded word. */
	assert(node->word);
	name = node->word;

	/* Get the variable's value. */
	if (var_get(vars, name, &type, &vt_rvalue)) {
		sc_int length;

		/* Compare the value against the current string position. */
		switch (type) {
		case VAR_INTEGER: {
			sc_char value[32];

			/* Compare numeric against the current string position. */
			sprintf(value, "%ld", vt_rvalue.integer);
			length = strlen(value);
			if (strncmp(uip_string + uip_posn, value, length) == 0) {
				/* Integer match, advance position and return. */
				uip_posn += length;
				return TRUE;
			}
			break;
		}

		case VAR_STRING:
			/* Compare string value against the current string position. */
			length = strlen(vt_rvalue.string);
			if (sc_strncasecmp(uip_string + uip_posn,
			                   vt_rvalue.string, length) == 0) {
				/* String match, advance position and return. */
				uip_posn += length;
				return TRUE;
			}
			break;

		default:
			sc_fatal("uip_match_variable: invalid variable type, %ld\n", type);
		}
	}

	/* No match, or no such variable. */
	return FALSE;
}

static sc_bool uip_match_whitespace(void) {
	/* If next character is space, read whitespace and return. */
	if (sc_isspace(uip_string[uip_posn])) {
		/* Space match, advance position and return. */
		while (uip_string[uip_posn] != NUL && sc_isspace(uip_string[uip_posn]))
			uip_posn++;
		return TRUE;
	}

	/*
	 * No match.  However, if we're trying to match space, this is a word
	 * boundary.  So... even though we're not sitting on a space, if the string
	 * prior character is whitespace, "double-match" the space.
	 *
	 * Also, match if we haven't yet matched any text.  In effect, this means
	 * leading spaces on patterns will be ignored.
	 *
	 * TODO Is this what we want to happen?  It seems harmless, even useful.
	 */
	if (uip_posn == 0 || sc_isspace(uip_string[uip_posn - 1]))
		return TRUE;

	/*
	 * And that's not all.  We also want to match whitespace if we're at the end
	 * of a string (another word boundary).  This will permit patterns that end
	 * in optional elements to succeed since options and wildcards always match,
	 * even if to no text.
	 */
	if (uip_string[uip_posn] == NUL)
		return TRUE;

	/* No match.  Really. */
	return FALSE;
}

static sc_bool uip_match_list(sc_ptnoderef_t node) {
	sc_ptnoderef_t child;

	/*
	 * If this list is empty, fail the match.  This special-case handling is
	 * what catches constructed temporary lists for wildcard-like items that
	 * don't actually encompass anything.
	 */
	if (!node->left_child)
		return FALSE;

	/* Match everything listed sequentially. */
	for (child = node->left_child; child; child = child->right_sibling) {
		if (!uip_match_node(child)) {
			/* No match. */
			return FALSE;
		}
	}

	/* Matched. */
	return TRUE;
}

static sc_bool uip_match_alternatives(sc_ptnoderef_t node) {
	sc_ptnoderef_t child;
	sc_int start_posn, extent;
	sc_bool matched;

	/* Note the start position for rewind between tries. */
	start_posn = uip_posn;

	/*
	 * Try a match on each of the children, looking to see which one moves the
	 * position on the furthest.  Match on this one.  This is a "maximal munch".
	 */
	extent = uip_posn;
	matched = FALSE;
	for (child = node->left_child; child; child = child->right_sibling) {
		uip_posn = start_posn;
		if (uip_match_node(child)) {
			/* Matched. */
			matched = TRUE;
			if (uip_posn > extent)
				extent = uip_posn;
		}
	}

	/* If matched, set position to extent; if not, back to start. */
	uip_posn = matched ? extent : start_posn;

	/* Return match status. */
	return matched;
}

static sc_bool uip_match_choice(sc_ptnoderef_t node) {
	/*
	 * Return the result of matching alternatives.  The choice will therefore
	 * fail if none of the alternatives match.
	 */
	return uip_match_alternatives(node);
}

static sc_bool uip_match_optional(sc_ptnoderef_t node) {
	sc_int start_posn;
	sc_ptnoderef_t list;
	sc_bool matched;

	/* Note the start position for rewind on empty match. */
	start_posn = uip_posn;

	/*
	 * Look ahead to see if we can match to nothing, and still have the main
	 * pattern match.  If we can, we'll go with this.  It's a "minimal munch"-ish
	 * strategy, but seems to be what Adrift does in this situation.
	 */
	list = uip_new_node(NODE_LIST);
	list->left_child = node->right_sibling;

	/* Match on the temporary list. */
	matched = uip_match_node(list);

	/* Free the temporary list node. */
	uip_destroy_node(list);

	/*
	 * If the temporary matched and consumed text, rewind position to match
	 * nothing.  If it didn't, match alternatives to consume anything that may
	 * match our options.
	 */
	if (matched && uip_posn > start_posn)
		uip_posn = start_posn;
	else
		uip_match_alternatives(node);

	/* Return TRUE no matter what. */
	return TRUE;
}

static sc_bool uip_match_wildcard(sc_ptnoderef_t node) {
	sc_int start_posn, limit, index_;
	sc_bool matched;
	sc_ptnoderef_t list;

	/*
	 * At least one game uses patterns like "thing******...".  Why?  Who knows.
	 * But if we're in a list of wildcards, and not the first, ignore the call;
	 * only the final one needs handling.
	 */
	if (node->right_sibling && node->right_sibling->type == NODE_WILDCARD)
		return TRUE;

	/* Note the start position for rewind on no match. */
	start_posn = uip_posn;

	/*
	 * To make life a little easier, we'll match on the tree to the right of
	 * this node by constructing a temporary list node, containing stuff to the
	 * right of the wildcard, and then matching on that.
	 */
	list = uip_new_node(NODE_LIST);
	list->left_child = node->right_sibling;

	/*
	 * Repeatedly try to match the rest of the tree at successive character
	 * positions, and stop if we succeed.  This is a "minimal munch", which may
	 * or may not be the right thing to be doing here.
	 *
	 * When scanning forward, take care to include the NUL, needed to match
	 * TOK_EOS.
	 */
	matched = FALSE;
	limit = strlen(uip_string) + 1;
	for (index_ = uip_posn + 1; index_ < limit; index_++) {
		uip_posn = index_;
		if (uip_match_node(list)) {
			/* Wildcard match at this point. */
			uip_posn = index_;
			matched = TRUE;
			break;
		}
	}

	/* Free the temporary list node. */
	uip_destroy_node(list);

	/* If we didn't match in the loop, restore position. */
	if (!matched)
		uip_posn = start_posn;

	/* Return TRUE whether we matched text or not. */
	return TRUE;
}


/*
 * uip_match_number()
 * uip_match_text()
 *
 * Attempt to match a number, or a word, from the string.
 */
static sc_bool uip_match_number(void) {
	const sc_gameref_t game = uip_get_game();
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int number;

	/* Attempt to read a number from input. */
	if (sscanf(uip_string + uip_posn, "%ld", &number) == 1) {
		/* Advance position over the number. */
		while (uip_string[uip_posn] == MINUS || uip_string[uip_posn] == PLUS)
			uip_posn++;
		while (sc_isdigit(uip_string[uip_posn]))
			uip_posn++;

		/* Set number reference in variables and return. */
		var_set_ref_number(vars, number);
		return TRUE;
	}

	/* No match. */
	return FALSE;
}

static sc_bool uip_match_text(sc_ptnoderef_t node) {
	const sc_gameref_t game = uip_get_game();
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int start_posn, limit, index_;
	sc_bool matched;
	sc_ptnoderef_t list;

	/* Note the start position for rewind on no match. */
	start_posn = uip_posn;

	/*
	 * As with wildcards, create a temporary list of the stuff to the right of
	 * the reference node, and match on that.
	 */
	list = uip_new_node(NODE_LIST);
	list->left_child = node->right_sibling;

	/*
	 * Again, as with wildcards, repeatedly try to match the rest of the tree at
	 * successive character positions, stopping if we succeed.
	 */
	matched = FALSE;
	limit = strlen(uip_string) + 1;
	for (index_ = uip_posn + 1; index_ < limit; index_++) {
		uip_posn = index_;
		if (uip_match_node(list)) {
			/* Text reference match at this point. */
			uip_posn = index_;
			matched = TRUE;
			break;
		}
	}

	/* Free the temporary list node. */
	uip_destroy_node(list);

	/* See if we found a match in the loop. */
	if (matched) {
		sc_char *string;

		/* Found a match; create a string and save the text. */
		string = (sc_char *)sc_malloc(uip_posn - start_posn + 1);
		memcpy(string, uip_string + start_posn, uip_posn - start_posn);
		string[uip_posn - start_posn] = NUL;

		/*
		 * Adrift seems to save referenced text as all-lowercase; we need to do
		 * the same.
		 */
		for (index_ = 0; string[index_] != NUL; index_++)
			string[index_] = sc_tolower(string[index_]);
		var_set_ref_text(vars, string);
		sc_free(string);

		/* Return TRUE since we matched text. */
		return TRUE;
	} else {
		/* We didn't match in the loop; restore position. */
		uip_posn = start_posn;

		/* Return FALSE on no match. */
		return FALSE;
	}
}


/*
 * uip_skip_article()
 *
 * Skip over any "a"/"an"/"the"/"some" at the head of a string.  Helper for
 * %character% and %object% matchers.  Returns the revised string position.
 */
static sc_int uip_skip_article(const sc_char *string, sc_int start) {
	sc_int posn;

	/* Skip over articles. */
	posn = start;
	if (sc_compare_word(string + posn, "a", 1))
		posn += 1;
	else if (sc_compare_word(string + posn, "an", 2))
		posn += 2;
	else if (sc_compare_word(string + posn, "the", 3))
		posn += 3;
	else if (sc_compare_word(string + posn, "some", 4))
		posn += 4;

	/* Skip any whitespace, and return. */
	while (sc_isspace(string[posn]) && string[posn] != NUL)
		posn++;
	return posn;
}


/*
 * uip_compare_reference()
 *
 * Helper for %character% and %object% matchers.  Matches multiple words
 * if necessary, at the current position.  Returns zero if the string
 * didn't match, otherwise the length of the current position that matched
 * the words passed in (the new value of uip_posn on match).
 */
static sc_int uip_compare_reference(const sc_char *words) {
	sc_int wpos, posn;

	/* Skip articles and lead in space on words and string. */
	wpos = uip_skip_article(words, 0);
	posn = uip_skip_article(uip_string, uip_posn);

	/* Match characters from words with the string at position. */
	while (TRUE) {
		/* Any character mismatch means no words match. */
		if (sc_tolower(words[wpos]) != sc_tolower(uip_string[posn]))
			return 0;

		/* Move to next character in each. */
		wpos++;
		posn++;

		/*
		 * If at space, advance over whitespace in words list.  Stop when we
		 * hit the end of the words list.
		 */
		while (sc_isspace(words[wpos]) && words[wpos] != NUL)
			wpos++;
		if (words[wpos] == NUL)
			break;

		/*
		 * About to match another word, so advance over whitespace in the
		 * current string too.
		 */
		while (sc_isspace(uip_string[posn]) && uip_string[posn] != NUL)
			posn++;
	}

	/*
	 * We reached the end of words.  If we're at the end of the match string, or
	 * at spaces, we've matched.
	 */
	if (sc_isspace(uip_string[posn]) || uip_string[posn] == NUL)
		return posn;

	/* More text after the match, so it's not quite a match. */
	return 0;
}


/*
 * uip_compare_prefixed_name()
 *
 * Helper for %character% and %object% matchers.  Attempts a reference
 * match against both the prefixed name, and if that fails, the plain name.
 * Returns the extent of the match, or zero if no match.
 */
static sc_int uip_compare_prefixed_name(const sc_char *prefix, const sc_char *name) {
	sc_char buffer[UIP_SHORT_WORD_SIZE + UIP_SHORT_WORD_SIZE + 1];
	sc_char *string;
	sc_int required, extent;

	/* Create a prefixed string, using the local buffer if possible. */
	required = strlen(prefix) + strlen(name) + 2;
	string = required > (sc_int) sizeof(buffer) ? (sc_char *)sc_malloc(required) : buffer;
	sprintf(string, "%s %s", prefix, name);

	/* Check against the prefixed name first, free string if required. */
	extent = uip_compare_reference(string);
	if (string != buffer)
		sc_free(string);

	/* If no match there, retry with just the plain name. */
	if (extent == 0)
		extent = uip_compare_reference(name);

	/* Return the count of characters consumed in matching. */
	return extent;
}


/*
 * uip_match_remainder()
 *
 * Helper for %character% and %object% matchers.  Matches the remainder
 * of a pattern, to resolve the difference between, say, "table leg" and
 * "table".
 */
static sc_bool uip_match_remainder(sc_ptnoderef_t node, sc_int extent) {
	sc_ptnoderef_t list;
	sc_int start_posn;
	sc_bool matched;

	/* Note the start position, then advance to the given extent. */
	start_posn = uip_posn;
	uip_posn = extent;

	/*
	 * Try to match everything after the node passed in, at this position in the
	 * string.
	 */
	list = uip_new_node(NODE_LIST);
	list->left_child = node->right_sibling;

	/* Match on the temporary list. */
	matched = uip_match_node(list);

	/* Free the temporary list node, and restore position. */
	uip_destroy_node(list);
	uip_posn = start_posn;

	/* Return TRUE if the pattern remainder matched. */
	return matched;
}


/*
 * uip_match_character()
 *
 * Match a %character% reference.  This function searches all NPC names and
 * aliases for possible matches, and sets the game npc_references flag
 * for any that match.  The final one to match is also stored in variables.
 */
static sc_bool uip_match_character(sc_ptnoderef_t node) {
	const sc_gameref_t game = uip_get_game();
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int npc_count, npc, max_extent;

	if (uip_trace)
		sc_trace("UIParser: attempting to match %%character%%\n");

	/* Clear all current character references. */
	gs_clear_npc_references(game);

	/* Iterate characters, looking for a name or alias match. */
	max_extent = 0;
	npc_count = gs_npc_count(game);
	for (npc = 0; npc < npc_count; npc++) {
		sc_vartype_t vt_key[4];
		const sc_char *prefix, *name;
		sc_int alias_count, alias, extent;

		/* Get the NPC's prefix and name. */
		vt_key[0].string = "NPCs";
		vt_key[1].integer = npc;
		vt_key[2].string = "Prefix";
		prefix = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);

		if (uip_trace)
			sc_trace("UIParser: trying %s\n", name);

		/* Compare this name, both prefixed and not. */
		extent = uip_compare_prefixed_name(prefix, name);
		if (extent > 0 && uip_match_remainder(node, extent)) {
			if (uip_trace)
				sc_trace("UIParser: matched\n");

			/* Increase the maximum match extent if required. */
			max_extent = (extent > max_extent) ? extent : max_extent;

			/* Save match in variables and game. */
			var_set_ref_character(vars, npc);
			game->npc_references[npc] = TRUE;
		}

		/* Now compare against all NPC aliases. */
		vt_key[2].string = "Alias";
		alias_count = prop_get_child_count(bundle, "I<-sis", vt_key);

		for (alias = 0; alias < alias_count; alias++) {
			const sc_char *alias_name;

			/*
			 * Get the NPC alias.  Version 3.9 games introduce empty aliases,
			 * so check here.
			 */
			vt_key[3].integer = alias;
			alias_name = prop_get_string(bundle, "S<-sisi", vt_key);
			if (sc_strempty(alias_name))
				continue;

			if (uip_trace)
				sc_trace("UIParser: trying alias %s\n", alias_name);

			/* Compare this alias name, both prefixed and not. */
			extent = uip_compare_prefixed_name(prefix, alias_name);
			if (extent > 0 && uip_match_remainder(node, extent)) {
				if (uip_trace)
					sc_trace("UIParser: matched\n");

				/* Increase the maximum match extent if required. */
				max_extent = (extent > max_extent) ? extent : max_extent;

				/* Save match in variables and game. */
				var_set_ref_character(vars, npc);
				game->npc_references[npc] = TRUE;
			}
		}
	}

	/* On match, advance position and return successfully. */
	if (max_extent > 0) {
		uip_posn = max_extent;
		return TRUE;
	}

	/* No match. */
	return FALSE;
}


/*
 * uip_match_object()
 *
 * Match an %object% reference.  This function searches all object names and
 * aliases for possible matches, and sets the game object_references flag
 * for any that match.  The final one to match is also stored in variables.
 */
static sc_bool uip_match_object(sc_ptnoderef_t node) {
	const sc_gameref_t game = uip_get_game();
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int object_count, object, max_extent;

	if (uip_trace)
		sc_trace("UIParser: attempting to match %%object%%\n");

	/* Clear all current object references. */
	gs_clear_object_references(game);

	/* Iterate objects, looking for a name or alias match. */
	max_extent = 0;
	object_count = gs_object_count(game);
	for (object = 0; object < object_count; object++) {
		sc_vartype_t vt_key[4];
		const sc_char *prefix, *name;
		sc_int alias_count, alias, extent;

		/* Get the object's prefix and name. */
		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Prefix";
		prefix = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Short";
		name = prop_get_string(bundle, "S<-sis", vt_key);

		if (uip_trace)
			sc_trace("UIParser: trying %s\n", name);

		/* Compare this name, both prefixed and not. */
		extent = uip_compare_prefixed_name(prefix, name);
		if (extent > 0 && uip_match_remainder(node, extent)) {
			if (uip_trace)
				sc_trace("UIParser: matched\n");

			/* Increase the maximum match extent if required. */
			max_extent = (extent > max_extent) ? extent : max_extent;

			/* Save match in variables and game. */
			var_set_ref_object(vars, object);
			game->object_references[object] = TRUE;
		}

		/* Now compare against all object aliases. */
		vt_key[2].string = "Alias";
		alias_count = prop_get_child_count(bundle, "I<-sis", vt_key);

		for (alias = 0; alias < alias_count; alias++) {
			const sc_char *alias_name;

			/*
			 * Get the object alias.  Version 3.9 games introduce empty aliases,
			 * so check here.
			 */
			vt_key[3].integer = alias;
			alias_name = prop_get_string(bundle, "S<-sisi", vt_key);
			if (sc_strempty(alias_name))
				continue;

			if (uip_trace)
				sc_trace("UIParser: trying alias %s\n", alias_name);

			/* Compare this alias name, both prefixed and not. */
			extent = uip_compare_prefixed_name(prefix, alias_name);
			if (extent > 0 && uip_match_remainder(node, extent)) {
				if (uip_trace)
					sc_trace("UIParser: matched\n");

				/* Increase the maximum match extent if required. */
				max_extent = (extent > max_extent) ? extent : max_extent;

				/* Save match in variables and game. */
				var_set_ref_object(vars, object);
				game->object_references[object] = TRUE;
			}
		}
	}

	/* On match, advance position and return successfully. */
	if (max_extent > 0) {
		uip_posn = max_extent;
		return TRUE;
	}

	/* No match. */
	return FALSE;
}


/*
 * uip_match_node()
 *
 * Attempt to match the given node to the current match string/position.
 * Return TRUE, with position advanced, on match, FALSE on fail with the
 * position unchanged.
 */
static sc_bool uip_match_node(sc_ptnoderef_t node) {
	sc_bool match = FALSE;

	/* Match depending on node type. */
	switch (node->type) {
	case NODE_EOS:
		match = uip_match_eos();
		break;
	case NODE_WORD:
		match = uip_match_word(node);
		break;
	case NODE_VARIABLE:
		match = uip_match_variable(node);
		break;
	case NODE_WHITESPACE:
		match = uip_match_whitespace();
		break;
	case NODE_LIST:
		match = uip_match_list(node);
		break;
	case NODE_CHOICE:
		match = uip_match_choice(node);
		break;
	case NODE_OPTIONAL:
		match = uip_match_optional(node);
		break;
	case NODE_WILDCARD:
		match = uip_match_wildcard(node);
		break;
	case NODE_CHARACTER_REFERENCE:
		match = uip_match_character(node);
		break;
	case NODE_OBJECT_REFERENCE:
		match = uip_match_object(node);
		break;
	case NODE_NUMBER_REFERENCE:
		match = uip_match_number();
		break;
	case NODE_TEXT_REFERENCE:
		match = uip_match_text(node);
		break;
	default:
		sc_fatal("uip_match_node: invalid type, %ld\n", (sc_int) node->type);
	}

	return match;
}


/*
 * uip_cleanse_string()
 * uip_free_cleansed_string()
 *
 * Given a string, copy it to the given buffer, and trim leading and trailing
 * spaces from it.  If the string does not fit the buffer, malloc enough for
 * a string copy and use that instead.  The caller needs to free this
 * allocation if it happens (detectable by comparing the return value to the
 * buffer passed in), or call uip_free_cleansed_string.
 */
static sc_char *uip_cleanse_string(const sc_char *original, sc_char *buffer, sc_int length) {
	sc_int required;
	sc_char *string;

	/*
	 * Use the supplied buffer if it is long enough, otherwise allocate, and
	 * copy the string.
	 */
	required = strlen(original) + 1;
	string = (required < length) ? buffer : (sc_char *)sc_malloc(required);
	strcpy(string, original);

	/* Trim, and return the string. */
	sc_trim_string(string);
	return string;
}

static sc_char *uip_free_cleansed_string(sc_char *string, const sc_char *buffer) {
	/* Free if the string was allocated by the function above. */
	if (string != buffer)
		sc_free(string);

	/* Always returns NULL, for the syntactic convenience of the caller. */
	return NULL;
}


/*
 * uip_debug_trace()
 *
 * Set pattern match tracing on/off.
 */
void uip_debug_trace(sc_bool flag) {
	uip_trace = flag;
}


/*
 * uip_match()
 *
 * Match a string to a pattern, and return TRUE on match, FALSE otherwise.
 * For performance, this function uses a local buffer to try to avoid the
 * need to copy each of the pattern and match strings passed in.
 */
sc_bool uip_match(const sc_char *pattern, const sc_char *string, sc_gameref_t game) {
	static sc_char *cleansed = nullptr;
	sc_char buffer[UIP_ALLOCATION_AVOIDANCE_SIZE];
	sc_bool match;
	Context context;
	assert(pattern && string && game);

	/* Start tokenizer. */
	cleansed = uip_cleanse_string(pattern, buffer, sizeof(buffer));
	if (uip_trace)
		sc_trace("UIParser: pattern \"%s\"\n", cleansed);
	uip_tokenize_start(cleansed);

	// Try parsing the pattern into a match tree
	uip_parse_lookahead = uip_next_token();
	uip_parse_tree = uip_new_node(NODE_LIST);
	uip_parse_list(context, uip_parse_tree);

	if (context._break) {
		// Parse error -- clean up and fail
		uip_tokenize_end();
		uip_destroy_tree(uip_parse_tree);
		uip_parse_tree = NULL;
		cleansed = uip_free_cleansed_string(cleansed, buffer);
		return FALSE;
	} else {
		uip_tokenize_end();
		cleansed = uip_free_cleansed_string(cleansed, buffer);
	}

	/* Dump out the pattern tree if requested. */
	if (if_get_trace_flag(SC_DUMP_PARSER_TREES))
		uip_debug_dump();

	/* Match the string to the pattern tree. */
	cleansed = uip_cleanse_string(string, buffer, sizeof(buffer));
	if (uip_trace)
		sc_trace("UIParser: string \"%s\"\n", cleansed);
	uip_match_start(cleansed, game);
	match = uip_match_node(uip_parse_tree);

	/* Clean up matching and free the parsed pattern tree. */
	uip_match_end();
	cleansed = uip_free_cleansed_string(cleansed, buffer);
	uip_destroy_tree(uip_parse_tree);
	uip_parse_tree = NULL;

	/* Return result of matching. */
	if (uip_trace)
		sc_trace("UIParser: %s\n", match ? "MATCHED!" : "No match");
	return match;
}


/*
 * uip_replace_pronouns()
 *
 * Replaces pronouns by their respective object or NPC names, and returns the
 * resulting string to the caller, or NULL if no pronouns were replaced.  The
 * return string is malloc'ed, so the caller needs to remember to free it.
 */
sc_char *uip_replace_pronouns(sc_gameref_t game, const sc_char *string) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_int buffer_allocation;
	sc_char *buffer;
	const sc_char *current;
	assert(string);

	if (uip_trace)
		sc_trace("UIParser: pronoun search \"%s\"\n", string);

	/* Begin with a NULL buffer for lazy allocation. */
	buffer_allocation = 0;
	buffer = NULL;

	/* Search for pronouns until no more string remains. */
	current = string + strspn(string, WHITESPACE);
	while (current[0] != NUL) {
		sc_vartype_t vt_key[3];
		sc_int object, npc, extent;
		const sc_char *prefix, *name;

		/* Initially, no object or NPC, no names, and a zero extent. */
		object = npc = -1;
		prefix = name = NULL;
		extent = 0;

		/*
		 * Search for pronouns where we have an assigned object or NPC.  We
		 * can't be sure of getting plurality right, and it's not always
		 * intuitive even in English -- is "a pair of scissors" an "it", or
		 * a "them"?  Because of this, we just treat "it" and "them" equally
		 * for now.
		 */
		if (game->it_object != -1 && sc_compare_word(current, "it", 2)) {
			object = game->it_object;
			extent = 2;
		} else if (game->it_object != -1 && sc_compare_word(current, "them", 4)) {
			object = game->it_object;
			extent = 4;
		} else if (game->him_npc != -1 && sc_compare_word(current, "him", 3)) {
			npc = game->him_npc;
			extent = 3;
		} else if (game->her_npc != -1 && sc_compare_word(current, "her", 3)) {
			npc = game->her_npc;
			extent = 3;
		} else if (game->it_npc != -1 && sc_compare_word(current, "it", 2)) {
			npc = game->it_npc;
			extent = 2;
		}

		/* Assign prefix and name to the full object or NPC name, if any. */
		if (object > -1) {
			vt_key[0].string = "Objects";
			vt_key[1].integer = object;
			vt_key[2].string = "Prefix";
			prefix = prop_get_string(bundle, "S<-sis", vt_key);
			vt_key[2].string = "Short";
			name = prop_get_string(bundle, "S<-sis", vt_key);
		} else if (npc > -1) {
			vt_key[0].string = "NPCs";
			vt_key[1].integer = npc;
			vt_key[2].string = "Prefix";
			prefix = prop_get_string(bundle, "S<-sis", vt_key);
			vt_key[2].string = "Name";
			name = prop_get_string(bundle, "S<-sis", vt_key);
		}

		/*
		 * If a pronoun was found, prefix and name indicate what to insert, and
		 * extent shows how much of the buffer to replace with them.
		 */
		if (prefix && name && extent > 0) {
			sc_char *position;
			sc_int prefix_length, name_length, length, final_length;

			/*
			 * If not yet allocated, allocate a buffer now, and copy the input
			 * string into it.  Then switch current to the equivalent location
			 * in the allocated buffer; basic copy-on-write.
			 */
			if (!buffer) {
				buffer_allocation = strlen(string) + 1;
				buffer = (sc_char *)sc_malloc(buffer_allocation);
				strcpy(buffer, string);
				current = buffer + (current - string);
			}

			/*
			 * If necessary, grow the output buffer for the replacement,
			 * remembering to adjust current for the new buffer allocated.
			 * At the same time, note the last character index for the move.
			 */
			prefix_length = strlen(prefix);
			name_length = strlen(name);
			length = prefix_length + name_length + 1;
			if (length > extent) {
				sc_int offset;

				offset = current - buffer;
				buffer_allocation += length - extent;
				buffer = (sc_char *)sc_realloc(buffer, buffer_allocation);
				current = buffer + offset;
				final_length = length;
			} else
				final_length = extent;

			/* Insert the replacement strings into the buffer. */
			position = buffer + (current - buffer);
			memmove(position + length,
			        position + extent,
			        buffer_allocation - (current - buffer) - final_length);
			memcpy(position, prefix, prefix_length);
			position[prefix_length] = ' ';
			memcpy(position + prefix_length + 1, name, name_length);

			/* Adjust current to skip over the replacement. */
			current += length;

			if (uip_trace)
				sc_trace("Parser: pronoun \"%s\"\n", buffer);
		} else {
			/* If no match, advance current over the unmatched word. */
			current += strcspn(current, WHITESPACE);
		}

		/* Set current to the next word start. */
		current += strspn(current, WHITESPACE);
	}

	/* Return the final string, or NULL if no pronoun replacements. */
	return buffer;
}


/*
 * uip_assign_pronouns()
 *
 * Search a player command for object and NPC names, and assign any found to
 * game pronouns.  The string is searched from front to back, assigning
 * pronouns for objects or NPC names as found.  Later ones will overwrite
 * earlier ones if there is more than one in the string.
 */
void uip_assign_pronouns(sc_gameref_t game, const sc_char *string) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_char *current;
	sc_int saved_ref_object, saved_ref_character;
	assert(string);

	if (uip_trace)
		sc_trace("UIParser: pronoun assignment \"%s\"\n", string);

	/* Save var references so we can restore them later. */
	saved_ref_object = var_get_ref_object(vars);
	saved_ref_character = var_get_ref_character(vars);

	/* Search for object and NPC names until no more string remains. */
	current = string + strspn(string, WHITESPACE);
	while (current[0] != NUL) {
		if (uip_match("%object% *", current, game)) {
			sc_int count, index_, object;

			/*
			 * "Disambiguate" by rejecting objects that the player hasn't seen
			 * or can't see.  If the reference is unique, assign to the 'it'
			 * object pronoun.
			 */
			count = 0;
			object = -1;
			for (index_ = 0; index_ < gs_object_count(game); index_++) {
				if (game->object_references[index_]
				        && gs_object_seen(game, index_)
				        && obj_indirectly_in_room(game,
				                                  index_, gs_playerroom(game))) {
					count++;
					object = index_;
				}
			}

			if (count == 1) {
				game->it_object = object;
				game->it_npc = -1;

				if (uip_trace)
					sc_trace("UIParser: object 'it/them' assigned %ld\n", object);
			}
		}

		if (uip_match("%character% *", current, game)) {
			sc_int count, index_, npc;

			/* Do the same "disambiguation" as for objects above. */
			count = 0;
			npc = -1;
			for (index_ = 0; index_ < gs_npc_count(game); index_++) {
				if (game->npc_references[index_]
				        && gs_npc_seen(game, index_)
				        && npc_in_room(game, index_, gs_playerroom(game))) {
					count++;
					npc = index_;
				}
			}

			if (count == 1) {
				sc_vartype_t vt_key[3];
				sc_int version, gender;

				/*
				 * Version 3.8 games lack NPC gender information, so for this
				 * case set "him"/"her" on each match, and never set "it"; this
				 * matches the version 3.8 runner.
				 */
				vt_key[0].string = "Version";
				version = prop_get_integer(bundle, "I<-s", vt_key);
				if (version == TAF_VERSION_380) {
					game->him_npc = npc;
					game->her_npc = npc;
					game->it_npc = -1;

					if (uip_trace) {
						sc_trace("UIParser: 3.8 pronouns"
						         " 'him' and 'her' assigned %ld\n", npc);
					}
				} else {
					/* Find the NPC gender, so we know the pronoun to assign. */
					vt_key[0].string = "NPCs";
					vt_key[1].integer = npc;
					vt_key[2].string = "Gender";
					gender = prop_get_integer(bundle, "I<-sis", vt_key);

					switch (gender) {
					case NPC_MALE:
						game->him_npc = npc;
						break;
					case NPC_FEMALE:
						game->her_npc = npc;
						break;
					case NPC_NEUTER:
						game->it_npc = npc;
						game->it_object = -1;
						break;
					default:
						sc_error("uip_assign_pronouns:"
						         " unknown gender, %ld\n", gender);
					}

					if (uip_trace)
						sc_trace("UIParser: NPC 'him/her/it' assigned %ld\n", npc);
				}
			}
		}

		/*
		 * Advance the string position by a complete word.  This saves a lot
		 * of time -- there's no point looking for an object or NPC name in
		 * mid-word, and anyway it's not the right thing to do.
		 */
		current += strcspn(current, WHITESPACE);
		current += strspn(current, WHITESPACE);
	}

	/* Restore variables references. */
	var_set_ref_object(vars, saved_ref_object);
	var_set_ref_character(vars, saved_ref_character);
}

} // End of namespace Adrift
} // End of namespace Glk
