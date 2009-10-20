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

/* Functionality to transform the context-free SCI grammar rules into
 * strict Greibach normal form (strict GNF), and to test SCI input against
 * that grammar, writing an appropriate node tree if successful.
 */

#include "sci/vocabulary.h"
#include "sci/console.h"
#include "common/array.h"

namespace Sci {

#define TOKEN_OPAREN 0xff000000
#define TOKEN_CPAREN 0xfe000000
#define TOKEN_TERMINAL_CLASS 0x10000
#define TOKEN_TERMINAL_GROUP 0x20000
#define TOKEN_STUFFING_WORD 0x40000
#define TOKEN_NON_NT (TOKEN_OPAREN | TOKEN_TERMINAL_CLASS | TOKEN_TERMINAL_GROUP | TOKEN_STUFFING_WORD)
#define TOKEN_TERMINAL (TOKEN_TERMINAL_CLASS | TOKEN_TERMINAL_GROUP)

static int _allocd_rules = 0;	// FIXME: Avoid non-const global vars

struct ParseRule {
	int _id; /**< non-terminal ID */
	uint _firstSpecial; /**< first terminal or non-terminal */
	uint _numSpecials; /**< number of terminals and non-terminals */
	Common::Array<int> _data;	/**< actual data */

	~ParseRule() {
		--_allocd_rules;
	}

	// FIXME remove this one again?
	bool operator==(const ParseRule &other) const {
		return _id == other._id &&
			_firstSpecial == other._firstSpecial &&
			_numSpecials == other._numSpecials &&
			_data == other._data;
	}
};


struct ParseRuleList {
	int terminal; /**< Terminal character this rule matches against or 0 for a non-terminal rule */
	ParseRule *rule;
	ParseRuleList *next;

	void print() const;

	ParseRuleList(ParseRule *r) : rule(r), next(0) {
		int term = rule->_data[rule->_firstSpecial];
		terminal = ((term & TOKEN_TERMINAL) ? term : 0);
	}

	~ParseRuleList() {
		delete rule;
		delete next;
	}
};


static void vocab_print_rule(ParseRule *rule) {
	int wspace = 0;

	if (!rule) {
		warning("NULL rule");
		return;
	}

	printf("[%03x] -> ", rule->_id);

	if (rule->_data.empty())
		printf("e");

	for (uint i = 0; i < rule->_data.size(); i++) {
		uint token = rule->_data[i];

		if (token == TOKEN_OPAREN) {
			if (i == rule->_firstSpecial)
				printf("_");

			printf("(");
			wspace = 0;
		} else if (token == TOKEN_CPAREN) {
			if (i == rule->_firstSpecial)
				printf("_");

			printf(")");
			wspace = 0;
		} else {
			if (wspace)
				printf(" ");

			if (i == rule->_firstSpecial)
				printf("_");
			if (token & TOKEN_TERMINAL_CLASS)
				printf("C(%04x)", token & 0xffff);
			else if (token & TOKEN_TERMINAL_GROUP)
				printf("G(%04x)", token & 0xffff);
			else if (token & TOKEN_STUFFING_WORD)
				printf("%03x", token & 0xffff);
			else
				printf("[%03x]", token); /* non-terminal */
			wspace = 1;
		}

		if (i == rule->_firstSpecial)
			printf("_");
	}
	printf(" [%d specials]", rule->_numSpecials);
}

static ParseRule *_vdup(ParseRule *a) {
	++_allocd_rules;
	return new ParseRule(*a);
}

static ParseRule *_vinsert(ParseRule *turkey, ParseRule *stuffing) {
	uint firstnt = turkey->_firstSpecial;

	// Search for first TOKEN_NON_NT in 'turkey'
	while ((firstnt < turkey->_data.size()) && (turkey->_data[firstnt] & TOKEN_NON_NT))
		firstnt++;

	// If no TOKEN_NON_NT found, or if it doesn't match the id of 'stuffing', abort.
	if ((firstnt == turkey->_data.size()) || (turkey->_data[firstnt] != stuffing->_id))
		return NULL;

	// Create a new rule as a copy of 'turkey', where the token firstnt has been substituted
	// by the rule 'stuffing'.
	++_allocd_rules;

	ParseRule *rule = new ParseRule(*turkey);
	rule->_numSpecials += stuffing->_numSpecials - 1;
	rule->_firstSpecial = firstnt + stuffing->_firstSpecial;
	rule->_data.resize(turkey->_data.size() - 1 + stuffing->_data.size());

	// Replace rule->_data[firstnt] by all of stuffing->_data
	Common::copy(stuffing->_data.begin(), stuffing->_data.end(), rule->_data.begin() + firstnt);

	if (firstnt < turkey->_data.size() - 1)
		Common::copy(turkey->_data.begin() + firstnt + 1, turkey->_data.end(),
				rule->_data.begin() + firstnt + stuffing->_data.size());

	return rule;
}

static ParseRule *_vbuild_rule(const parse_tree_branch_t *branch) {
	int tokens = 0, tokenpos = 0, i;

	while (tokenpos < 10 && branch->data[tokenpos]) {
		int type = branch->data[tokenpos];
		tokenpos += 2;

		if ((type == VOCAB_TREE_NODE_COMPARE_TYPE) || (type == VOCAB_TREE_NODE_COMPARE_GROUP) || (type == VOCAB_TREE_NODE_FORCE_STORAGE))
			++tokens;
		else if (type > VOCAB_TREE_NODE_LAST_WORD_STORAGE)
			tokens += 5;
		else
			return NULL; // invalid
	}

	ParseRule *rule = new ParseRule();

	++_allocd_rules;
	rule->_id = branch->id;
	rule->_numSpecials = tokenpos >> 1;
	rule->_data.resize(tokens);
	rule->_firstSpecial = 0;

	tokens = 0;
	for (i = 0; i < tokenpos; i += 2) {
		int type = branch->data[i];
		int value = branch->data[i + 1];

		if (type == VOCAB_TREE_NODE_COMPARE_TYPE)
			rule->_data[tokens++] = value | TOKEN_TERMINAL_CLASS;
		else if (type == VOCAB_TREE_NODE_COMPARE_GROUP)
			rule->_data[tokens++] = value | TOKEN_TERMINAL_GROUP;
		else if (type == VOCAB_TREE_NODE_FORCE_STORAGE)
			rule->_data[tokens++] = value | TOKEN_STUFFING_WORD;
		else { // normal inductive rule
			rule->_data[tokens++] = TOKEN_OPAREN;
			rule->_data[tokens++] = type | TOKEN_STUFFING_WORD;
			rule->_data[tokens++] = value | TOKEN_STUFFING_WORD;

			if (i == 0)
				rule->_firstSpecial = tokens;

			rule->_data[tokens++] = value; // The non-terminal
			rule->_data[tokens++] = TOKEN_CPAREN;
		}
	}

	return rule;
}

static ParseRule *_vsatisfy_rule(ParseRule *rule, const ResultWord &input) {
	int dep;

	if (!rule->_numSpecials)
		return NULL;

	dep = rule->_data[rule->_firstSpecial];

	if (((dep & TOKEN_TERMINAL_CLASS) && ((dep & 0xffff) & input._class)) ||
			((dep & TOKEN_TERMINAL_GROUP) && ((dep & 0xffff) & input._group))) {
		ParseRule *retval = new ParseRule(*rule);
		++_allocd_rules;
		retval->_data[rule->_firstSpecial] = TOKEN_STUFFING_WORD | input._group;
		retval->_numSpecials--;
		retval->_firstSpecial = 0;

		if (retval->_numSpecials) { // find first special, if it exists
			for (uint i = rule->_firstSpecial; i < retval->_data.size(); ++i) {
				int tmp = retval->_data[i];
				if (!(tmp & TOKEN_NON_NT) || (tmp & TOKEN_TERMINAL)) {
					retval->_firstSpecial = i;
					break;
				}
			}
		}

		return retval;
	} else
		return NULL;
}

void Vocabulary::freeRuleList(ParseRuleList *list) {
	delete list;
}

static ParseRuleList *_vocab_add_rule(ParseRuleList *list, ParseRule *rule) {
		return list;

	ParseRuleList *new_elem = new ParseRuleList(rule);

	if (list) {
		const int term = new_elem->terminal;
/*		if (term < list->terminal) {
			new_elem->next = list;
			return new_elem;
		} else {*/
		ParseRuleList *seeker = list;

		while (seeker->next/* && seeker->next->terminal <= term*/) {
			if (seeker->next->terminal == term) {
				if (*(seeker->next->rule) == *rule) {
					delete rule;
					delete new_elem;
					return list; // No duplicate rules
				}
			}
			seeker = seeker->next;
		}

		new_elem->next = seeker->next;
		seeker->next = new_elem;
		return list;
	} else {
		return new_elem;
	}
}

void ParseRuleList::print() const {
	const ParseRuleList *list = this;
	int pos = 0;
	while (list) {
		printf("R%03d: ", pos);
		vocab_print_rule(list->rule);
		printf("\n");
		list = list->next;
		pos++;
	}
	printf("%d rules total.\n", pos);
}

static ParseRuleList *_vocab_split_rule_list(ParseRuleList *list) {
	if (!list->next || (list->next->terminal)) {
		ParseRuleList *tmp = list->next;
		list->next = NULL;
		return tmp;
	} else
		return _vocab_split_rule_list(list->next);
}

static void _vocab_free_empty_rule_list(ParseRuleList *list) {
	if (list->next)
		_vocab_free_empty_rule_list(list->next);
	list->next = 0;
	delete list;
}

static ParseRuleList *_vocab_merge_rule_lists(ParseRuleList *l1, ParseRuleList *l2) {
	ParseRuleList *retval = l1, *seeker = l2;
	while (seeker) {
		retval = _vocab_add_rule(retval, seeker->rule);
		seeker = seeker->next;
	}
	_vocab_free_empty_rule_list(l2);

	return retval;
}

static int _vocab_rule_list_length(ParseRuleList *list) {
	return ((list) ? _vocab_rule_list_length(list->next) + 1 : 0);
}

static ParseRuleList *_vocab_clone_rule_list_by_id(ParseRuleList *list, int id) {
	ParseRuleList *result = NULL;
	ParseRuleList *seeker = list;

	while (seeker) {
		if (seeker->rule->_id == id) {
			result = _vocab_add_rule(result, _vdup(seeker->rule));
		}
		seeker = seeker->next;
	}

	return result;
}

ParseRuleList *Vocabulary::buildGNF(bool verbose) {
	int iterations = 0;
	int last_termrules, termrules = 0;
	int ntrules_nr;
	ParseRuleList *ntlist = NULL;
	ParseRuleList *tlist, *new_tlist;
	Console *con = ((SciEngine *)g_engine)->getSciDebugger();

	for (uint i = 1; i < _parserBranches.size(); i++) { // branch rule 0 is treated specially
		ParseRule *rule = _vbuild_rule(&_parserBranches[i]);
		if (!rule)
			return NULL;
		ntlist = _vocab_add_rule(ntlist, rule);
	}

	tlist = _vocab_split_rule_list(ntlist);
	ntrules_nr = _vocab_rule_list_length(ntlist);

	if (verbose)
		con->DebugPrintf("Starting with %d rules\n", ntrules_nr);

	new_tlist = tlist;
	tlist = NULL;

	do {
		ParseRuleList *new_new_tlist = NULL;
		ParseRuleList *ntseeker, *tseeker;
		last_termrules = termrules;

		ntseeker = ntlist;
		while (ntseeker) {
			tseeker = new_tlist;

			while (tseeker) {
				ParseRule *newrule = _vinsert(ntseeker->rule, tseeker->rule);
				if (newrule)
					new_new_tlist = _vocab_add_rule(new_new_tlist, newrule);
				tseeker = tseeker->next;
			}

			ntseeker = ntseeker->next;
		}

		tlist = _vocab_merge_rule_lists(tlist, new_tlist);
		new_tlist = new_new_tlist;
		termrules = _vocab_rule_list_length(new_new_tlist);

		if (verbose)
			con->DebugPrintf("After iteration #%d: %d new term rules\n", ++iterations, termrules);

	} while (termrules && (iterations < 30));

	freeRuleList(ntlist);

	if (verbose) {
		con->DebugPrintf("\nGNF rules:\n");
		tlist->print();
		con->DebugPrintf("%d allocd rules\n", _allocd_rules);
		con->DebugPrintf("Freeing rule list...\n");
		freeRuleList(tlist);
		return NULL;
	}

	return tlist;
}

static int _vbpt_pareno(parse_tree_node_t *nodes, int *pos, int base) {
	// Opens parentheses
	nodes[base].content.branches[0] = (*pos) + 1;
	nodes[++(*pos)].type = kParseTreeBranchNode;
	nodes[*pos].content.branches[0] = 0;
	nodes[*pos].content.branches[1] = 0;
	return *pos;
}

static int _vbpt_parenc(parse_tree_node_t *nodes, int *pos, int paren) {
	// Closes parentheses for appending
	nodes[paren].content.branches[1] = ++(*pos);
	nodes[*pos].type = kParseTreeBranchNode;
	nodes[*pos].content.branches[0] = 0;
	nodes[*pos].content.branches[1] = 0;
	return *pos;
}

static int _vbpt_append(parse_tree_node_t *nodes, int *pos, int base, int value) {
	// writes one value to an existing base node and creates a successor node for writing
	nodes[base].content.branches[0] = ++(*pos);
	nodes[*pos].type = kParseTreeLeafNode;
	nodes[*pos].content.value = value;
	nodes[base].content.branches[1] = ++(*pos);
	nodes[*pos].type = kParseTreeBranchNode;
	nodes[*pos].content.branches[0] = 0;
	nodes[*pos].content.branches[1] = 0;
	return *pos;
}

static int _vbpt_terminate(parse_tree_node_t *nodes, int *pos, int base, int value) {
	// Terminates, overwriting a nextwrite forknode
	nodes[base].type = kParseTreeLeafNode;
	nodes[base].content.value = value;
	return *pos;
}

static int _vbpt_write_subexpression(parse_tree_node_t *nodes, int *pos, ParseRule *rule, uint rulepos, int writepos) {
	uint token;

	while ((token = ((rulepos < rule->_data.size()) ? rule->_data[rulepos++] : TOKEN_CPAREN)) != TOKEN_CPAREN) {
		uint nexttoken = (rulepos < rule->_data.size()) ? rule->_data[rulepos] : TOKEN_CPAREN;
		if (token == TOKEN_OPAREN) {
			int writepos2 = _vbpt_pareno(nodes, pos, writepos);
			rulepos = _vbpt_write_subexpression(nodes, pos, rule, rulepos, writepos2);
			nexttoken = (rulepos < rule->_data.size()) ? rule->_data[rulepos] : TOKEN_CPAREN;
			if (nexttoken != TOKEN_CPAREN)
				writepos = _vbpt_parenc(nodes, pos, writepos);
		} else if (token & TOKEN_STUFFING_WORD) {
			if (nexttoken == TOKEN_CPAREN)
				writepos = _vbpt_terminate(nodes, pos, writepos, token & 0xffff);
			else
				writepos = _vbpt_append(nodes, pos, writepos, token & 0xffff);
		} else {
			printf("\nError in parser (grammar.cpp, _vbpt_write_subexpression()): Rule data broken in rule ");
			vocab_print_rule(rule);
			printf(", at token position %d\n", *pos);
			return rulepos;
		}
	}

	return rulepos;
}

int Vocabulary::parseGNF(const ResultWordList &words, bool verbose) {
	Console *con = ((SciEngine *)g_engine)->getSciDebugger();
	// Get the start rules:
	ParseRuleList *work = _vocab_clone_rule_list_by_id(_parserRules, _parserBranches[0].data[1]);
	ParseRuleList *results = NULL;
	uint word = 0;
	const uint words_nr = words.size();
	ResultWordList::const_iterator word_iter = words.begin();

	for (word_iter = words.begin(); word_iter != words.end(); ++word_iter, ++word) {
		ParseRuleList *new_work = NULL;
		ParseRuleList *reduced_rules = NULL;
		ParseRuleList *seeker, *subseeker;

		if (verbose)
			con->DebugPrintf("Adding word %d...\n", word);

		seeker = work;
		while (seeker) {
			if (seeker->rule->_numSpecials <= (words_nr - word))
				reduced_rules = _vocab_add_rule(reduced_rules, _vsatisfy_rule(seeker->rule, *word_iter));

			seeker = seeker->next;
		}

		if (reduced_rules == NULL) {
			freeRuleList(work);
			if (verbose)
				con->DebugPrintf("No results.\n");
			return 1;
		}

		freeRuleList(work);

		if (word + 1 < words_nr) {
			seeker = reduced_rules;

			while (seeker) {
				if (seeker->rule->_numSpecials) {
					int my_id = seeker->rule->_data[seeker->rule->_firstSpecial];

					subseeker = _parserRules;
					while (subseeker) {
						if (subseeker->rule->_id == my_id)
							new_work = _vocab_add_rule(new_work, _vinsert(seeker->rule, subseeker->rule));

						subseeker = subseeker->next;
					}
				}

				seeker = seeker->next;
			}
			freeRuleList(reduced_rules);
		} else // last word
			new_work = reduced_rules;

		work = new_work;
		if (verbose)
			con->DebugPrintf("Now at %d candidates\n", _vocab_rule_list_length(work));
		if (work == NULL) {
			if (verbose)
				con->DebugPrintf("No results.\n");
			return 1;
		}
	}

	results = work;

	if (verbose) {
		con->DebugPrintf("All results (excluding the surrounding '(141 %03x' and ')'):\n", _parserBranches[0].id);
		results->print();
		con->DebugPrintf("\n");
	}

	// now use the first result
	{
		int temp, pos;

		_parserNodes[0].type = kParseTreeBranchNode;
		_parserNodes[0].content.branches[0] = 1;
		_parserNodes[0].content.branches[1] = 2;

		_parserNodes[1].type = kParseTreeLeafNode;
		_parserNodes[1].content.value = 0x141;

		_parserNodes[2].type = kParseTreeBranchNode;
		_parserNodes[2].content.branches[0] = 0;
		_parserNodes[2].content.branches[1] = 0;

		pos = 2;

		temp = _vbpt_append(_parserNodes, &pos, 2, _parserBranches[0].id);
		//_vbpt_write_subexpression(nodes, &pos, results[_vocab_rule_list_length(results)].rule, 0, temp);
		_vbpt_write_subexpression(_parserNodes, &pos, results->rule, 0, temp);
	}

	freeRuleList(results);

	return 0;
}

} // End of namespace Sci
