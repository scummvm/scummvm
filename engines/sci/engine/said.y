%{
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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/state.h"


// Bison generates an empty switch statement that gives a warning in MSVC.
// This disables that warning.
#ifdef _MSC_VER
#pragma warning(disable:4065)
#endif


namespace Sci {

#define SAID_BRANCH_NULL 0

#define MAX_SAID_TOKENS 128

// Maximum number of words to be expected in a parsed sentence
#define AUGMENT_MAX_WORDS 64


#define ANYWORD 0xfff

#define WORD_TYPE_BASE 0x141
#define WORD_TYPE_REF 0x144
#define WORD_TYPE_SYNTACTIC_SUGAR 0x145

#define AUGMENT_SENTENCE_PART_BRACKETS 0x152

// Minor numbers
#define AUGMENT_SENTENCE_MINOR_MATCH_PHRASE 0x14c
#define AUGMENT_SENTENCE_MINOR_MATCH_WORD 0x153
#define AUGMENT_SENTENCE_MINOR_RECURSE 0x144
#define AUGMENT_SENTENCE_MINOR_PARENTHESES 0x14f


#undef YYDEBUG /*1*/
//#define SAID_DEBUG*/
//#define SCI_DEBUG_PARSE_TREE_AUGMENTATION // uncomment to debug parse tree augmentation


#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
#define scidprintf printf
#else
void print_nothing(...) { }
#define scidprintf print_nothing
#endif


static char *said_parse_error;

static int said_token;
static int said_tokens_nr;
static int said_tokens[MAX_SAID_TOKENS];
static int said_blessed;  // increminated by said_top_branch

static int said_tree_pos; // Set to 0 if we're out of space
#define SAID_TREE_START 4; // Reserve space for the 4 top nodes

#define VALUE_IGNORE -424242

static parse_tree_node_t said_tree[VOCAB_TREE_NODES];

typedef int wgroup_t;
typedef int tree_t;
typedef int said_spec_t;

static tree_t said_aug_branch(int, int, tree_t, tree_t);
static tree_t said_attach_branch(tree_t, tree_t);
/*
static tree_t said_wgroup_branch(wgroup_t);
*/
static said_spec_t said_top_branch(tree_t);
static tree_t said_paren(tree_t, tree_t);
static tree_t said_value(int, tree_t);
static tree_t said_terminal(int);

static int yylex(void);

static int yyerror(const char *s) {
	said_parse_error = strdup(s);
	return 1; /* Abort */
}

%}

%token WGROUP /* Word group */
%token YY_COMMA     /* 0xf0 */
%token YY_AMP       /* 0xf1 */
%token YY_SLASH     /* 0xf2 */
%token YY_PARENO    /* 0xf3 */
%token YY_PARENC    /* 0xf4 */
%token YY_BRACKETSO /* 0xf5 */
%token YY_BRACKETSC /* 0xf6 */
%token YY_HASH      /* 0xf7 */
%token YY_LT        /* 0xf8 */
%token YY_GT        /* 0xf9 */
%token YY_BRACKETSO_LT /* special token used to imitate LR(2) behaviour */
%token YY_BRACKETSO_SLASH /* special token used to imitate LR(2) behaviour */
%token YY_LT_BRACKETSO /* special token used to imitate LR(2) behaviour */
%token YY_LT_PARENO /* special token used to imitate LR(2) behaviour */

%%

saidspec :	  leftspec optcont
			{ $$ = said_top_branch(said_attach_branch($1, $2)); }
		| leftspec midspec optcont
			{ $$ = said_top_branch(said_attach_branch($1, said_attach_branch($2, $3))); }
		| leftspec midspec rightspec optcont
			{ $$ = said_top_branch(said_attach_branch($1, said_attach_branch($2, said_attach_branch($3, $4)))); }
		;


optcont :	 /* empty */
			{ $$ = SAID_BRANCH_NULL; }
		| YY_GT
			{ $$ = said_paren(said_value(0x14b, said_value(0xf900, said_terminal(0xf900))), SAID_BRANCH_NULL); }
		;



leftspec :	/* empty */
			{ $$ = SAID_BRANCH_NULL; }
		| expr
			{ $$ = said_paren(said_value(0x141, said_value(0x149, $1)), SAID_BRANCH_NULL); }
		;



midspec :	 YY_SLASH expr
			{ $$ = said_aug_branch(0x142, 0x14a, $2, SAID_BRANCH_NULL); }
		| YY_BRACKETSO_SLASH YY_SLASH expr YY_BRACKETSC
			{ $$ = said_aug_branch(0x152, 0x142, said_aug_branch(0x142, 0x14a, $3, SAID_BRANCH_NULL), SAID_BRANCH_NULL); }
		| YY_SLASH
			{ $$ = SAID_BRANCH_NULL; }
		;



rightspec :	 YY_SLASH expr
			{ $$ = said_aug_branch(0x143, 0x14a, $2, SAID_BRANCH_NULL); }
		| YY_BRACKETSO_SLASH YY_SLASH expr YY_BRACKETSC
			{ $$ = said_aug_branch(0x152, 0x143, said_aug_branch(0x143, 0x14a, $3, SAID_BRANCH_NULL), SAID_BRANCH_NULL); }
		| YY_SLASH
			{ $$ = SAID_BRANCH_NULL; }
		;


word :		 WGROUP
			{ $$ = said_paren(said_value(0x141, said_value(0x153, said_terminal($1))), SAID_BRANCH_NULL); }
		;


cwordset :	wordset
			{ $$ = said_aug_branch(0x141, 0x14f, $1, SAID_BRANCH_NULL); }
		| YY_BRACKETSO wordset YY_BRACKETSC
			{ $$ = said_aug_branch(0x141, 0x14f, said_aug_branch(0x152, 0x14c, said_aug_branch(0x141, 0x14f, $2, SAID_BRANCH_NULL), SAID_BRANCH_NULL), SAID_BRANCH_NULL); }
		;


wordset :	 word
			{ $$ = $1; }
		| YY_PARENO expr YY_PARENC
			{ $$ = said_aug_branch(0x141, 0x14c, $2, SAID_BRANCH_NULL); }
		| wordset YY_COMMA wordset
			{ $$ = said_attach_branch($1, $3); }
		| wordset YY_BRACKETSO_LT wordrefset YY_BRACKETSC
			{ $$ = said_attach_branch($1, $3); }
		| wordset YY_COMMA YY_BRACKETSO wordset YY_BRACKETSC
			{ $$ = said_attach_branch($1, $3); }
		;


expr :		 cwordset cwordrefset
			{ $$ = said_attach_branch($1, $2); }
		| cwordset
			{ $$ = $1; }
		| cwordrefset
			{ $$ = $1; }
		;


cwordrefset :	 wordrefset
			{ $$ = $1; }
		| YY_BRACKETSO_LT wordrefset YY_BRACKETSC
			{ $$ = said_aug_branch(0x152, 0x144, $2, SAID_BRANCH_NULL); }
		| wordrefset YY_BRACKETSO_LT wordrefset YY_BRACKETSC
			{ $$ = said_attach_branch($1, said_aug_branch(0x152, 0x144, $3, SAID_BRANCH_NULL)); }
		;


wordrefset :	YY_LT word recref
			{ $$ = said_aug_branch(0x144, 0x14f, $2, $3); }
		| YY_LT_PARENO YY_PARENO expr YY_PARENC
		{ $$ = said_aug_branch(0x144, 0x14f, said_aug_branch(0x141, 0x144, $2, SAID_BRANCH_NULL), SAID_BRANCH_NULL); }
		| YY_LT wordset
			{ $$ = said_aug_branch(0x144, 0x14f, $2, SAID_BRANCH_NULL); }
		| YY_LT_BRACKETSO YY_BRACKETSO wordset YY_BRACKETSC
			{ $$ = said_aug_branch(0x152, 0x144, said_aug_branch(0x144, 0x14f, $3, SAID_BRANCH_NULL), SAID_BRANCH_NULL); }
		;


recref :	YY_LT wordset recref
			{ $$ = said_aug_branch(0x141, 0x144, said_aug_branch(0x144, 0x14f, $2, SAID_BRANCH_NULL), $3); }
		| YY_LT wordset
			{ $$ = said_aug_branch(0x141, 0x144, said_aug_branch(0x144, 0x14f, $2, SAID_BRANCH_NULL), SAID_BRANCH_NULL); }
		| YY_LT_PARENO YY_PARENO expr YY_PARENC
		{ $$ = said_aug_branch(0x141, 0x14c, $2, SAID_BRANCH_NULL); }
		;

%%

int parse_yy_token_lookup[] = {YY_COMMA, YY_AMP, YY_SLASH, YY_PARENO, YY_PARENC, YY_BRACKETSO, YY_BRACKETSC, YY_HASH, YY_LT, YY_GT};

static int yylex(void) {
	int retval = said_tokens[said_token++];

	if (retval < SAID_LONG(SAID_FIRST)) {
		yylval = retval;
		retval = WGROUP;
	} else {
		retval >>= 8;

		if (retval == SAID_TERM)
			retval = 0;
		else {
			assert(retval >= SAID_FIRST);
			retval = parse_yy_token_lookup[retval - SAID_FIRST];
			if (retval == YY_BRACKETSO) {
				if ((said_tokens[said_token] >> 8) == SAID_LT)
					retval = YY_BRACKETSO_LT;
				else
					if ((said_tokens[said_token] >> 8) == SAID_SLASH)
						retval = YY_BRACKETSO_SLASH;
			} else if (retval == YY_LT && (said_tokens[said_token] >> 8) == SAID_BRACKO) {
				retval = YY_LT_BRACKETSO;
			} else if (retval == YY_LT && (said_tokens[said_token] >> 8) == SAID_PARENO) {
				retval = YY_LT_PARENO;
			}
		}
	}

	return retval;
}

static int said_next_node() {
	return ((said_tree_pos == 0) || (said_tree_pos >= VOCAB_TREE_NODES)) ? said_tree_pos = 0 : said_tree_pos++;
}

#define SAID_NEXT_NODE said_next_node()

static int said_leaf_node(tree_t pos, int value) {
	said_tree[pos].type = PARSE_TREE_NODE_LEAF;

	if (value != VALUE_IGNORE)
		said_tree[pos].content.value = value;

	return pos;
}

static int said_branch_node(tree_t pos, int left, int right) {
	said_tree[pos].type = PARSE_TREE_NODE_BRANCH;

	if (left != VALUE_IGNORE)
		said_tree[pos].content.branches[0] = left;

	if (right != VALUE_IGNORE)
		said_tree[pos].content.branches[1] = right;

	return pos;
}

static tree_t said_paren(tree_t t1, tree_t t2) {
	if (t1)
		return said_branch_node(SAID_NEXT_NODE, t1, t2);
	else
		return t2;
}

static tree_t said_value(int val, tree_t t) {
	return said_branch_node(SAID_NEXT_NODE, said_leaf_node(SAID_NEXT_NODE, val), t);

}

static tree_t said_terminal(int val) {
	return said_leaf_node(SAID_NEXT_NODE, val);
}

static tree_t said_aug_branch(int n1, int n2, tree_t t1, tree_t t2) {
	int retval;

	retval = said_branch_node(SAID_NEXT_NODE,
				said_branch_node(SAID_NEXT_NODE,
					said_leaf_node(SAID_NEXT_NODE, n1),
						said_branch_node(SAID_NEXT_NODE,
							said_leaf_node(SAID_NEXT_NODE, n2),
						t1)
					),
				t2);

#ifdef SAID_DEBUG
	fprintf(stderr, "AUG(0x%x, 0x%x, [%04x], [%04x]) = [%04x]\n", n1, n2, t1, t2, retval);
#endif

	return retval;
}

static tree_t said_attach_branch(tree_t base, tree_t attacheant) {
#ifdef SAID_DEBUG
	fprintf(stderr, "ATT2([%04x], [%04x]) = [%04x]\n", base, attacheant, base);
#endif

	if (!attacheant)
		return base;
	if (!base)
		return attacheant;

	if (!base)
		return 0; // Happens if we're out of space

	said_branch_node(base, VALUE_IGNORE, attacheant);

	return base;
}

static said_spec_t said_top_branch(tree_t first) {
#ifdef SAID_DEBUG
	fprintf(stderr, "TOP([%04x])\n", first);
#endif
	said_branch_node(0, 1, 2);
	said_leaf_node(1, 0x141); // Magic number #1
	said_branch_node(2, 3, first);
	said_leaf_node(3, 0x13f); // Magic number #2

	++said_blessed;

	return 0;
}

static int said_parse_spec(EngineState *s, byte *spec) {
	int nextitem;

	said_parse_error = NULL;
	said_token = 0;
	said_tokens_nr = 0;
	said_blessed = 0;

	said_tree_pos = SAID_TREE_START;

	do {
		nextitem = *spec++;
		if (nextitem < SAID_FIRST)
			said_tokens[said_tokens_nr++] = nextitem << 8 | *spec++;
		else
			said_tokens[said_tokens_nr++] = SAID_LONG(nextitem);

	} while ((nextitem != SAID_TERM) && (said_tokens_nr < MAX_SAID_TOKENS));

	if (nextitem == SAID_TERM)
		yyparse();
	else {
		warning("SAID spec is too long");
		return 1;
	}

	if (said_parse_error) {
		warning("Error while parsing SAID spec: %s", said_parse_error);
		free(said_parse_error);
		return 1;
	}

	if (said_tree_pos == 0) {
		warning("Out of tree space while parsing SAID spec");
		return 1;
	}

	if (said_blessed != 1) {
		warning("Found multiple top branches");
		return 1;
	}

	return 0;
}

/**********************/
/**** Augmentation ****/
/**********************/

// primitive functions

#define AUG_READ_BRANCH(a, br, p) \
	if (tree[p].type != PARSE_TREE_NODE_BRANCH) \
		return 0; \
	a = tree[p].content.branches[br];

#define AUG_READ_VALUE(a, p) \
	if (tree[p].type != PARSE_TREE_NODE_LEAF) \
		return 0; \
	a = tree[p].content.value;

#define AUG_ASSERT(i) \
	if (!i) return 0;

static int aug_get_next_sibling(parse_tree_node_t *tree, int pos, int *first, int *second) {
	// Returns the next sibling relative to the specified position in 'tree',
	// sets *first and *second to its augment node values, returns the new position
	// or 0 if there was no next sibling
	int seek, valpos;

	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(seek, 0, pos);
	AUG_ASSERT(seek);

	// Now retrieve first value
	AUG_READ_BRANCH(valpos, 0, seek);
	AUG_ASSERT(valpos);
	AUG_READ_VALUE(*first, valpos);

	// Get second value
	AUG_READ_BRANCH(seek, 1, seek);
	AUG_ASSERT(seek);
	AUG_READ_BRANCH(valpos, 0, seek);
	AUG_ASSERT(valpos);
	AUG_READ_VALUE(*second, valpos);

	return pos;
}

static int aug_get_wgroup(parse_tree_node_t *tree, int pos) {
	// Returns 0 if pos in tree is not the root of a 3-element list, otherwise
	// it returns the last element (which, in practice, is the word group
	int val;

	AUG_READ_BRANCH(pos, 0, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);
	AUG_READ_VALUE(val, pos);

	return val;
}

static int aug_get_base_node(parse_tree_node_t *tree) {
	int startpos = 0;
	AUG_READ_BRANCH(startpos, 1, startpos);

	return startpos;
}

// semi-primitive functions

static int aug_get_first_child(parse_tree_node_t *tree, int pos, int *first, int *second) {
	// like aug_get_next_sibling, except that it recurses into the tree and
	// finds the first child (usually *not* Ayanami Rei) of the current branch
	// rather than its next sibling.
	AUG_READ_BRANCH(pos, 0, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);

	return aug_get_next_sibling(tree, pos, first, second);
}

static void aug_find_words_recursively(parse_tree_node_t *tree, int startpos, int *base_words, int *base_words_nr,
			   int *ref_words, int *ref_words_nr, int maxwords, int refbranch) {
	// Finds and lists all base (141) and reference (144) words */
	int major, minor;
	int word;
	int pos = aug_get_first_child(tree, startpos, &major, &minor);

	//if (major == WORD_TYPE_REF)
	//	refbranch = 1;

	while (pos) {
		if ((word = aug_get_wgroup(tree, pos))) { // found a word
			if (!refbranch && major == WORD_TYPE_BASE) {
				if ((*base_words_nr) == maxwords) {
					warning("Out of regular words");
					return; // return gracefully
				}

				base_words[*base_words_nr] = word; // register word
				++(*base_words_nr);

			}
			if (major == WORD_TYPE_REF || refbranch) {
				if ((*ref_words_nr) == maxwords) {
					warning("Out of reference words");
					return; // return gracefully
				}

				ref_words[*ref_words_nr] = word; // register word
				++(*ref_words_nr);

			}
			if (major != WORD_TYPE_SYNTACTIC_SUGAR && major != WORD_TYPE_BASE && major != WORD_TYPE_REF)
				warning("aug_find_words_recursively(): Unknown word type %03x", major);

		} else // Did NOT find a word group: Attempt to recurse
			aug_find_words_recursively(tree, pos, base_words, base_words_nr,
						   ref_words, ref_words_nr, maxwords, refbranch || major == WORD_TYPE_REF);

		pos = aug_get_next_sibling(tree, pos, &major, &minor);
	}
}


static void aug_find_words(parse_tree_node_t *tree, int startpos, int *base_words, int *base_words_nr,
				int *ref_words, int *ref_words_nr, int maxwords) {
	// initializing wrapper for aug_find_words_recursively()
	*base_words_nr = 0;
	*ref_words_nr = 0;

	aug_find_words_recursively(tree, startpos, base_words, base_words_nr, ref_words, ref_words_nr, maxwords, 0);
}


static int aug_contains_word(int *list, int length, int word) {
	int i;

	if (word == ANYWORD)
		return (length);

	for (i = 0; i < length; i++)
		if (list[i] == word)
			return 1;

	return 0;
}


static int augment_sentence_expression(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset,
					int parse_branch, int major, int minor, int *base_words, int base_words_nr,
					int *ref_words, int ref_words_nr);

static int augment_match_expression_p(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset,
					int parse_basepos, int major, int minor,
			   int *base_words, int base_words_nr, int *ref_words, int ref_words_nr) {
	int cmajor, cminor, cpos;
	cpos = aug_get_first_child(saidt, augment_pos, &cmajor, &cminor);
	if (!cpos) {
		warning("augment_match_expression_p(): Empty condition");
		return 1;
	}

	scidprintf("Attempting to match (%03x %03x (%03x %03x\n", major, minor, cmajor, cminor);

	if ((major == WORD_TYPE_BASE) && (minor == AUGMENT_SENTENCE_MINOR_RECURSE))
		return augment_match_expression_p(saidt, cpos, parset, parse_basepos, cmajor, cminor,
						  base_words, base_words_nr, ref_words, ref_words_nr);

	switch (major) {

	case WORD_TYPE_BASE:
		while (cpos) {
			if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_WORD) {
				int word = aug_get_wgroup(saidt, cpos);
				scidprintf("Looking for word %03x\n", word);

				if (aug_contains_word(base_words, base_words_nr, word))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_PHRASE) {
				if (augment_sentence_expression(saidt, cpos, parset, parse_basepos, cmajor, cminor,
								base_words, base_words_nr, ref_words, ref_words_nr))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_PARENTHESES) {
				int gc_major, gc_minor;
				int gchild = aug_get_first_child(saidt, cpos, &gc_major, &gc_minor);

				while (gchild) {
					if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, major,
									minor, base_words, base_words_nr,
								       ref_words, ref_words_nr))
						return 1;
					gchild = aug_get_next_sibling(saidt, gchild, &gc_major, &gc_minor);
				}
			} else
				warning("augment_match_expression_p(): Unknown type 141 minor number %3x", cminor);

			cpos = aug_get_next_sibling(saidt, cpos, &cmajor, &cminor);

		}
		break;

	case WORD_TYPE_REF:
		while (cpos) {
			if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_WORD) {
				int word = aug_get_wgroup(saidt, cpos);
				scidprintf("Looking for refword %03x\n", word);

				if (aug_contains_word(ref_words, ref_words_nr, word))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_PHRASE) {
				if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, cmajor, cminor,
							       base_words, base_words_nr, ref_words, ref_words_nr))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_PARENTHESES) {
				int gc_major, gc_minor;
				int gchild = aug_get_first_child(saidt, cpos, &gc_major, &gc_minor);

				while (gchild) {
					if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, major,
									minor, base_words, base_words_nr,
									ref_words, ref_words_nr))
						return 1;
					gchild = aug_get_next_sibling(saidt, gchild, &gc_major, &gc_minor);
				}
			} else
				warning("augment_match_expression_p(): Unknown type 144 minor number %3x", cminor);

			cpos = aug_get_next_sibling(saidt, cpos, &cmajor, &cminor);

		}
		break;

	case AUGMENT_SENTENCE_PART_BRACKETS:
		if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, cmajor, cminor,
					       base_words, base_words_nr, ref_words, ref_words_nr))
			return 1;

		scidprintf("Didn't match subexpression; checking sub-bracked predicate %03x\n", cmajor);

		switch (cmajor) {
		case WORD_TYPE_BASE:
			if (!base_words_nr)
				return 1;
			break;

		case WORD_TYPE_REF:
			if (!ref_words_nr)
				return 1;
			break;

		default:
			warning("augment_match_expression_p(): (subp1) Unkonwn sub-bracket predicate %03x", cmajor);
		}

		break;

	default:
		warning("augment_match_expression_p(): Unknown predicate %03x", major);

	}

	scidprintf("Generic failure\n");

	return 0;
}

static int augment_sentence_expression(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset,
					int parse_branch, int major, int minor, int *base_words, int base_words_nr,
					int *ref_words, int ref_words_nr) {
	int check_major, check_minor;
	int check_pos = aug_get_first_child(saidt, augment_pos, &check_major, &check_minor);
	do {
		if (!(augment_match_expression_p(saidt, check_pos, parset, parse_branch, check_major, check_minor,
						base_words, base_words_nr, ref_words, ref_words_nr)))
			return 0;
	} while ((check_pos = aug_get_next_sibling(saidt, check_pos, &check_major, &check_minor)));

	return 1;
}

static int augment_sentence_part(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset, int parse_basepos, int major, int minor) {
	int pmajor, pminor;
	int parse_branch = parse_basepos;
	int optional = 0;
	int foundwords = 0;

	scidprintf("Augmenting (%03x %03x\n", major, minor);

	if (major == AUGMENT_SENTENCE_PART_BRACKETS) { // '[/ foo]' is true if '/foo' or if there
						       // exists no x for which '/x' is true
		if ((augment_pos = aug_get_first_child(saidt, augment_pos, &major, &minor))) {
			scidprintf("Optional part: Now augmenting (%03x %03x\n", major, minor);
			optional = 1;
		} else {
			scidprintf("Matched empty optional expression\n");
			return 1;
		}
	}

	if ((major < 0x141) || (major > 0x143)) {
		scidprintf("augment_sentence_part(): Unexpected sentence part major number %03x\n", major);
		return 0;
	}

	while ((parse_branch = aug_get_next_sibling(parset, parse_branch, &pmajor, &pminor))) {
		if (pmajor == major) { // found matching sentence part
			int success;
			int base_words_nr;
			int ref_words_nr;
			int base_words[AUGMENT_MAX_WORDS];
			int ref_words[AUGMENT_MAX_WORDS];
#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
			int i;
#endif

			scidprintf("Found match with pminor = %03x\n", pminor);
			aug_find_words(parset, parse_branch, base_words, &base_words_nr, ref_words, &ref_words_nr, AUGMENT_MAX_WORDS);
			foundwords |= (ref_words_nr | base_words_nr);
#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
			printf("%d base words:", base_words_nr);
			for (i = 0; i < base_words_nr; i++)
				printf(" %03x", base_words[i]);
			printf("\n%d reference words:", ref_words_nr);
			for (i = 0; i < ref_words_nr; i++)
				printf(" %03x", ref_words[i]);
			printf("\n");
#endif

			success = augment_sentence_expression(saidt, augment_pos, parset, parse_basepos, major, minor,
							      base_words, base_words_nr, ref_words, ref_words_nr);

			if (success) {
				scidprintf("SUCCESS on augmenting (%03x %03x\n", major, minor);
				return 1;
			}
		}
	}

	if (optional && (foundwords == 0)) {
		scidprintf("Found no words and optional branch => SUCCESS on augmenting (%03x %03x\n", major, minor);
		return 1;
	}
	scidprintf("FAILURE on augmenting (%03x %03x\n", major, minor);

	return 0;
}

static int augment_parse_nodes(parse_tree_node_t *parset, parse_tree_node_t *saidt) {
	int augment_basepos = 0;
	int parse_basepos;
	int major, minor;
	int dontclaim = 0;

	parse_basepos = aug_get_base_node(parset);
	if (!parse_basepos) {
		warning("augment_parse_nodes(): Parse tree is corrupt");
		return 0;
	}

	augment_basepos = aug_get_base_node(saidt);
	if (!augment_basepos) {
		warning("augment_parse_nodes(): Said tree is corrupt");
		return 0;
	}

	while ((augment_basepos = aug_get_next_sibling(saidt, augment_basepos, &major, &minor))) {
		if ((major == 0x14b) && (minor == SAID_LONG(SAID_GT)))
			dontclaim = 1; // special case
		else // normal sentence part
			if (!(augment_sentence_part(saidt, augment_basepos, parset, parse_basepos, major, minor))) {
				scidprintf("Returning failure\n");
				return 0; // fail
			}
	}

	scidprintf("Returning success with dontclaim=%d\n", dontclaim);

	if (dontclaim)
		return SAID_PARTIAL_MATCH;
	else
		return 1; // full match
}


/*******************/
/**** Main code ****/
/*******************/

int said(EngineState *s, byte *spec, int verbose) {
	int retval;

	parse_tree_node_t *parse_tree_ptr = s->parser_nodes;

	if (s->parser_valid) {
		if (said_parse_spec(s, spec)) {
			warning("Offending spec was: ");
			s->_vocabulary->decypherSaidBlock(spec);
			return SAID_NO_MATCH;
		}

		if (verbose)
			vocab_dump_parse_tree("Said-tree", said_tree); // Nothing better to do yet
		retval = augment_parse_nodes(parse_tree_ptr, &(said_tree[0]));

		if (!retval)
			return SAID_NO_MATCH;
		else if (retval != SAID_PARTIAL_MATCH)
			return SAID_FULL_MATCH;
		else
			return SAID_PARTIAL_MATCH;
	}

	return SAID_NO_MATCH;
}


#ifdef SAID_DEBUG_PROGRAM
int main (int argc, char *argv) {
	byte block[] = {0x01, 0x00, 0xf8, 0xf5, 0x02, 0x01, 0xf6, 0xf2, 0x02, 0x01, 0xf2, 0x01, 0x03, 0xff};
	EngineState s;

	s.parser_valid = 1;
	said(&s, block);
}
#endif

} // End of namespace Sci
