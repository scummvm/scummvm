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
** strict Greibach normal form (strict GNF), and to test SCI input against
** that grammar, writing an appropriate node tree if successful.
*/

#include "sci/tools.h"
#include "sci/vocabulary.h"
#include "sci/console.h"

namespace Sci {

#define TOKEN_OPAREN 0xff000000
#define TOKEN_CPAREN 0xfe000000
#define TOKEN_TERMINAL_CLASS 0x10000
#define TOKEN_TERMINAL_GROUP 0x20000
#define TOKEN_STUFFING_WORD 0x40000
#define TOKEN_NON_NT (TOKEN_OPAREN | TOKEN_TERMINAL_CLASS | TOKEN_TERMINAL_GROUP | TOKEN_STUFFING_WORD)
#define TOKEN_TERMINAL (TOKEN_TERMINAL_CLASS | TOKEN_TERMINAL_GROUP)

static int _allocd_rules = 0;	// FIXME: Avoid non-const global vars

int getAllocatedRulesCount() { return _allocd_rules; }

static void vocab_print_rule(parse_rule_t *rule) {
	int i;
	int wspace = 0;

	if (!rule) {
		sciprintf("NULL rule");
		return;
	}

	sciprintf("[%03x] -> ", rule->id);

	if (!rule->length)
		sciprintf("e");

	for (i = 0; i < rule->length; i++) {
		uint token = rule->data[i];

		if (token == TOKEN_OPAREN) {
			if (i == rule->first_special)
				sciprintf("_");

			sciprintf("(");
			wspace = 0;
		} else if (token == TOKEN_CPAREN) {
			if (i == rule->first_special)
				sciprintf("_");

			sciprintf(")");
			wspace = 0;
		} else {
			if (wspace)
				sciprintf(" ");

			if (i == rule->first_special)
				sciprintf("_");
			if (token & TOKEN_TERMINAL_CLASS)
				sciprintf("C(%04x)", token & 0xffff);
			else if (token & TOKEN_TERMINAL_GROUP)
				sciprintf("G(%04x)", token & 0xffff);
			else if (token & TOKEN_STUFFING_WORD)
				sciprintf("%03x", token & 0xffff);
			else
				sciprintf("[%03x]", token); /* non-terminal */
			wspace = 1;
		}

		if (i == rule->first_special)
			sciprintf("_");
	}
	sciprintf(" [%d specials]", rule->specials_nr);
}

static void _vfree(parse_rule_t *rule) {
	free(rule);
	--_allocd_rules;
	rule = NULL;
}

static parse_rule_t *_vdup(parse_rule_t *a) {
	parse_rule_t *rule = (parse_rule_t*)malloc(sizeof(int) * (a->length + 4));

	rule->id = a->id;
	rule->length = a->length;
	rule->specials_nr = a->specials_nr;
	rule->first_special = a->first_special;
	++_allocd_rules;

	memcpy(rule->data, a->data, sizeof(int) * a->length);

	return rule;
}

static parse_rule_t *_vinsert(parse_rule_t *turkey, parse_rule_t *stuffing) {
	int firstnt = turkey->first_special;
	parse_rule_t *rule;

	while ((firstnt < turkey->length) && (turkey->data[firstnt] & TOKEN_NON_NT))
		firstnt++;

	if ((firstnt == turkey->length) || (turkey->data[firstnt] != stuffing->id))
		return NULL;

	rule = (parse_rule_t*)malloc(sizeof(int) * (turkey->length - 1 + stuffing->length + 4));
	rule->id = turkey->id;
	rule->specials_nr = turkey->specials_nr + stuffing->specials_nr - 1;
	rule->first_special = firstnt + stuffing->first_special;
	rule->length = turkey->length - 1 + stuffing->length;
	++_allocd_rules;

	if (firstnt > 0)
		memcpy(rule->data, turkey->data, sizeof(int) * firstnt);

	memcpy(&(rule->data[firstnt]), stuffing->data, sizeof(int) * stuffing->length);
	if (firstnt < turkey->length - 1)
		memcpy(&(rule->data[firstnt + stuffing->length]), &(turkey->data[firstnt + 1]), sizeof(int) * (turkey->length - firstnt - 1));

	return rule;
}

static parse_rule_t *_vbuild_rule(const parse_tree_branch_t *branch) {
	parse_rule_t *rule;
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

	rule = (parse_rule_t*)malloc(sizeof(int) * (4 + tokens));

	++_allocd_rules;
	rule->id = branch->id;
	rule->specials_nr = tokenpos >> 1;
	rule->length = tokens;
	rule->first_special = 0;

	tokens = 0;
	for (i = 0; i < tokenpos; i += 2) {
		int type = branch->data[i];
		int value = branch->data[i + 1];

		if (type == VOCAB_TREE_NODE_COMPARE_TYPE)
			rule->data[tokens++] = value | TOKEN_TERMINAL_CLASS;
		else if (type == VOCAB_TREE_NODE_COMPARE_GROUP)
			rule->data[tokens++] = value | TOKEN_TERMINAL_GROUP;
		else if (type == VOCAB_TREE_NODE_FORCE_STORAGE)
			rule->data[tokens++] = value | TOKEN_STUFFING_WORD;
		else { // normal inductive rule
			rule->data[tokens++] = TOKEN_OPAREN;
			rule->data[tokens++] = type | TOKEN_STUFFING_WORD;
			rule->data[tokens++] = value | TOKEN_STUFFING_WORD;

			if (i == 0)
				rule->first_special = tokens;

			rule->data[tokens++] = value; // The non-terminal
			rule->data[tokens++] = TOKEN_CPAREN;
		}
	}

	return rule;
}

static parse_rule_t *_vsatisfy_rule(parse_rule_t *rule, const ResultWord &input) {
	int dep;

	if (!rule->specials_nr)
		return NULL;

	dep = rule->data[rule->first_special];

	if (((dep & TOKEN_TERMINAL_CLASS) && ((dep & 0xffff) & input._class)) ||
			((dep & TOKEN_TERMINAL_GROUP) && ((dep & 0xffff) & input._group))) {
		parse_rule_t *retval = (parse_rule_t*)malloc(sizeof(int) * (4 + rule->length));
		++_allocd_rules;
		retval->id = rule->id;
		retval->specials_nr = rule->specials_nr - 1;
		retval->length = rule->length;
		memcpy(retval->data, rule->data, sizeof(int) * retval->length);
		retval->data[rule->first_special] = TOKEN_STUFFING_WORD | input._group;
		retval->first_special = 0;

		if (retval->specials_nr) { // find first special, if it exists
			int tmp, i = rule->first_special;

			while ((i < rule->length)&& ((tmp = retval->data[i]) & TOKEN_NON_NT) && !(tmp & TOKEN_TERMINAL))
				++i;

			if (i < rule->length)
				retval->first_special = i;
		}

		return retval;
	} else
		return NULL;
}

void Vocabulary::freeRuleList(parse_rule_list_t *list) {
	if (list) {
		_vfree(list->rule);
		freeRuleList(list->next); // Yep, this is slow and memory-intensive.
		free(list);
	}
}

static int _rules_equal_p(parse_rule_t *r1, parse_rule_t *r2) {
	if ((r1->id != r2->id) || (r1->length != r2->length) || (r1->first_special != r2->first_special))
		return 0;

	return !(memcmp(r1->data, r2->data, sizeof(int) * r1->length));
}

static parse_rule_list_t *_vocab_add_rule(parse_rule_list_t *list, parse_rule_t *rule) {
	parse_rule_list_t *new_elem;
	int term;

	if (!rule)
		return list;

	new_elem = (parse_rule_list_t*)malloc(sizeof(parse_rule_list_t));
	term = rule->data[rule->first_special];

	new_elem->rule = rule;
	new_elem->next = NULL;
	new_elem->terminal = term = ((term & TOKEN_TERMINAL) ? term : 0);

	if (!list)
		return new_elem;
	else {
/*		if (term < list->terminal) {
			new_elem->next = list;
			return new_elem;
		} else {*/
		parse_rule_list_t *seeker = list;

		while (seeker->next/* && seeker->next->terminal <= term*/) {
			if (seeker->next->terminal == term) {
				if (_rules_equal_p(seeker->next->rule, rule)) {
					_vfree(rule);
					free(new_elem);
					return list; // No duplicate rules
				}
			}
			seeker = seeker->next;
		}

		new_elem->next = seeker->next;
		seeker->next = new_elem;
		return list;
	}
}

static void _vprl(parse_rule_list_t *list, int pos) {
	if (list) {
		sciprintf("R%03d: ", pos);
		vocab_print_rule(list->rule);
		sciprintf("\n");
		_vprl(list->next, pos + 1);
	} else {
		sciprintf("%d rules total.\n", pos);
	}
}

void vocab_print_rule_list(parse_rule_list_t *list) {
	_vprl(list, 0);
}

static parse_rule_list_t *_vocab_split_rule_list(parse_rule_list_t *list) {
	if (!list->next || (list->next->terminal)) {
		parse_rule_list_t *tmp = list->next;
		list->next = NULL;
		return tmp;
	} else
		return _vocab_split_rule_list(list->next);
}

static void _vocab_free_empty_rule_list(parse_rule_list_t *list) {
	if (list->next)
		_vocab_free_empty_rule_list(list->next);

	free(list);
}

static parse_rule_list_t *_vocab_merge_rule_lists(parse_rule_list_t *l1, parse_rule_list_t *l2) {
	parse_rule_list_t *retval = l1, *seeker = l2;
	while (seeker) {
		retval = _vocab_add_rule(retval, seeker->rule);
		seeker = seeker->next;
	}
	_vocab_free_empty_rule_list(l2);

	return retval;
}

static int _vocab_rule_list_length(parse_rule_list_t *list) {
	return ((list) ? _vocab_rule_list_length(list->next) + 1 : 0);
}

static parse_rule_list_t *_vocab_clone_rule_list_by_id(parse_rule_list_t *list, int id) {
	parse_rule_list_t *result = NULL;
	parse_rule_list_t *seeker = list;

	while (seeker) {
		if (seeker->rule->id == id) {
			result = _vocab_add_rule(result, _vdup(seeker->rule));
		}
		seeker = seeker->next;
	}

	return result;
}

parse_rule_list_t *Vocabulary::buildGNF(bool verbose) {
	int iterations = 0;
	int last_termrules, termrules = 0;
	int ntrules_nr;
	parse_rule_list_t *ntlist = NULL;
	parse_rule_list_t *tlist, *new_tlist;
	Console *con = ((SciEngine *)g_engine)->getSciDebugger();

	for (uint i = 1; i < _parserBranches.size(); i++) { // branch rule 0 is treated specially
		parse_rule_t *rule = _vbuild_rule(&_parserBranches[i]);
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
		parse_rule_list_t *new_new_tlist = NULL;
		parse_rule_list_t *ntseeker, *tseeker;
		last_termrules = termrules;

		ntseeker = ntlist;
		while (ntseeker) {
			tseeker = new_tlist;

			while (tseeker) {
				parse_rule_t *newrule = _vinsert(ntseeker->rule, tseeker->rule);
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
		vocab_print_rule_list(tlist);
		con->DebugPrintf("%d allocd rules\n", getAllocatedRulesCount());
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

static int _vbpt_write_subexpression(parse_tree_node_t *nodes, int *pos, parse_rule_t *rule, int rulepos, int writepos) {
	uint token;

	while ((token = ((rulepos < rule->length) ? rule->data[rulepos++] : TOKEN_CPAREN)) != TOKEN_CPAREN) {
		uint nexttoken = (rulepos < rule->length) ? rule->data[rulepos] : TOKEN_CPAREN;
		if (token == TOKEN_OPAREN) {
			int wpold;
			int writepos2 = _vbpt_pareno(nodes, pos, wpold = writepos);
			rulepos = _vbpt_write_subexpression(nodes, pos, rule, rulepos, writepos2);
			nexttoken = (rulepos < rule->length) ? rule->data[rulepos] : TOKEN_CPAREN;
			if (nexttoken != TOKEN_CPAREN)
				writepos = _vbpt_parenc(nodes, pos, wpold);
		} else if (token & TOKEN_STUFFING_WORD) {
			if (nexttoken == TOKEN_CPAREN)
				writepos = _vbpt_terminate(nodes, pos, writepos, token & 0xffff);
			else
				writepos = _vbpt_append(nodes, pos, writepos, token & 0xffff);
		} else {
			sciprintf("\nError in parser (grammar.cpp, _vbpt_write_subexpression()): Rule data broken in rule ");
			vocab_print_rule(rule);
			sciprintf(", at token position %d\n", *pos);
			return rulepos;
		}
	}

	return rulepos;
}

int Vocabulary::parseGNF(parse_tree_node_t *nodes, const ResultWordList &words, bool verbose) {
	Console *con = ((SciEngine *)g_engine)->getSciDebugger();
	// Get the start rules:
	parse_rule_list_t *work = _vocab_clone_rule_list_by_id(_parserRules, _parserBranches[0].data[1]);
	parse_rule_list_t *results = NULL;
	int word = 0;
	const int words_nr = words.size();
	ResultWordList::const_iterator word_iter = words.begin();

	for (word_iter = words.begin(); word_iter != words.end(); ++word_iter, ++word) {
		parse_rule_list_t *new_work = NULL;
		parse_rule_list_t *reduced_rules = NULL;
		parse_rule_list_t *seeker, *subseeker;

		if (verbose)
			con->DebugPrintf("Adding word %d...\n", word);

		seeker = work;
		while (seeker) {
			if (seeker->rule->specials_nr <= (words_nr - word))
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
				if (seeker->rule->specials_nr) {
					int my_id = seeker->rule->data[seeker->rule->first_special];

					subseeker = _parserRules;
					while (subseeker) {
						if (subseeker->rule->id == my_id)
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
		vocab_print_rule_list(results);
		con->DebugPrintf("\n");
	}

	// now use the first result
	{
		int temp, pos;

		nodes[0].type = kParseTreeBranchNode;
		nodes[0].content.branches[0] = 1;
		nodes[0].content.branches[1] = 2;

		nodes[1].type = kParseTreeLeafNode;
		nodes[1].content.value = 0x141;

		nodes[2].type = kParseTreeBranchNode;
		nodes[2].content.branches[0] = 0;
		nodes[2].content.branches[1] = 0;

		pos = 2;

		temp = _vbpt_append(nodes, &pos, 2, _parserBranches[0].id);
		//_vbpt_write_subexpression(nodes, &pos, results[_vocab_rule_list_length(results)].rule, 0, temp);
		_vbpt_write_subexpression(nodes, &pos, results->rule, 0, temp);
	}

	freeRuleList(results);

	return 0;
}

} // End of namespace Sci
