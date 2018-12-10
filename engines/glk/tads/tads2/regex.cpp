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

/*
Regular Expression Parser and Recognizer for TADS
Function
  Parses and recognizes regular expressions
Notes
  Regular expression syntax:

	 abc|def    either abc or def
	 (abc)      abc
	 abc+       abc, abcc, abccc, ...
	 abc*     ab, abc, abcc, ...
	 abc?       ab or abc
	 .          any single character
	 abc$       abc at the end of the line
	 ^abc       abc at the beginning of the line
	 %^abc      literally ^abc
	 [abcx-z]   matches a, b, c, x, y, or z
	 [^abcx-z]  matches any character except a, b, c, x, y, or z
	 [^]-q]     matches any character except ], -, or q

  Note that using ']' or '-' in a character range expression requires
  special ordering.  If ']' is to be used, it must be the first character
  after the '^', if present, within the brackets.  If '-' is to be used,
  it must be the first character after the '^' and/or ']', if present.

  '%' is used to escape the special characters: | . ( ) * ? + ^ $ % [
  (We use '%' rather than a backslash because it's less trouble to
  enter in a TADS string -- a backslash needs to be quoted with another
  backslash, which is error-prone and hard to read.  '%' doesn't need
  any special quoting in a TADS string, which makes it a lot more
  readable.)

  In addition, '%' is used to introduce additional special sequences:

	 %1         text matching first parenthesized expression
	 %9         text matching ninth parenthesized experssion
	 %<         matches at the beginning of a word only
	 %>         matches at the end of a word only
	 %w         matches any word character
	 %W         matches any non-word character
	 %b         matches at any word boundary (beginning or end of word)
	 %B         matches except at a word boundary

  For the word matching sequences, a word is any sequence of letters and
  numbers.
*/

#include "engines/glk/tads/tads2/regex.h"
#include "engines/glk/tads/tads2/ler.h"
#include "engines/glk/tads/tads2/os.h"
//#include "engines/glk/tads/tads2/std.h"
#//include "engines/glk/tads/tads2/ler.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * A "machine" (i.e., a finite state automaton) is a set of state
 * transition tuples.  A tuple has three elements: the state ID, the ID
 * of the state that we transition to, and the condition for the
 * transition.  The condition is simply the character that we must match
 * to make the transition, or a special distinguished symbol "epsilon,"
 * which refers to a transition with no input character consumed.
 * 
 * The primitive elements of our machines guarantee that we never have
 * more than two transitions out of a particular state, so we can
 * denormalize the representation of a state by storing the two possible
 * tuples for that state in a single combined tuple.  This has the
 * performance advantage that we can use the state ID as an index into
 * an array of state tuples.
 * 
 * A particular machine always has a single initial and single final
 * (successful) state, so we can define a machine by its initial and
 * final state ID's.  
 */
enum {
	// the special symbol value for "epsilon"
	RE_EPSILON            = '\001',

	// the special symbol value for a wildcard character
	RE_WILDCARD           = '\002',

	// special symbol values for beginning and end of text
	RE_TEXT_BEGIN         = '\003',
	RE_TEXT_END           = '\004',

	// special symbol values for start and end of a word
	RE_WORD_BEGIN         = '\005',
	RE_WORD_END           = '\006',

	// special symbols for word-char and non-word-char
	RE_WORD_CHAR          = '\007',
	RE_NON_WORD_CHAR      = '\010',

	// special symbols for word-boundary and non-word-boundary
	RE_WORD_BOUNDARY      = '\011',
	RE_NON_WORD_BOUNDARY  = '\012',

	// special symbol for a character range/exclusion range
	RE_RANGE              = '\013',
	RE_RANGE_EXCL         = '\014',

	// a range of special symbol values for group matchers
	RE_GROUP_MATCH_0      = '\015',
	RE_GROUP_MATCH_9      = (RE_GROUP_MATCH_0 + 9)
};

re_context::re_context(errcxdef *errctx) {
	// save the error context
	_errctx = errctx;
	
	// clear states
	_next_state = RE_STATE_FIRST_VALID;

	// clear groups
	_cur_group = 0;

	// no string buffer yet
	_strbuf = 0;
}

re_context::~re_context() {
	// reset state
	reset();

	// if we allocated a string buffer, delete it
	if (_strbuf != 0) {
		mchfre(_strbuf);
		_strbuf = nullptr;
	}
}

void re_context::reset() {
	int i;
	
	// delete any range tables we've allocated
	for (i = 0 ; i < _next_state ; ++i) {
		if (_tuple_arr[i].char_range != 0) {
			mchfre(_tuple_arr[i].char_range);
			_tuple_arr[i].char_range = 0;
		}
	}

	// clear states
	_next_state = RE_STATE_FIRST_VALID;

	// clear groups
	_cur_group = 0;
}

re_state_id re_context::alloc_state() {
	// If we don't have enough room for another state, expand the array 
	if (_next_state >= (int)_tuple_arr.size()) {
		// bump the size by a bit
		_tuple_arr.resize(_tuple_arr.size() + 100);
	}

	// initialize the next state
	_tuple_arr[_next_state].next_state_1 = RE_STATE_INVALID;
	_tuple_arr[_next_state].next_state_2 = RE_STATE_INVALID;
	_tuple_arr[_next_state].ch = RE_EPSILON;
	_tuple_arr[_next_state].flags = 0;
	_tuple_arr[_next_state].char_range = 0;

	// return the new state's ID
	return _next_state++;
}

void re_context::set_trans(re_state_id id, re_state_id dest_id, char ch) {
	re_tuple *tuple;
	
	/* 
	 * get the tuple containing the transitions for this state ID - the
	 * state ID is the index of the state's transition tuple in the array 
	 */
	tuple = &_tuple_arr[id];

	/*
	 * If the first state pointer hasn't been set yet, set it to the new
	 * destination.  Otherwise, set the second state pointer.
	 * 
	 * Only set the character on setting the first state.  When setting
	 * the second state, we must assume that the character for the state
	 * has already been set, since any given state can have only one
	 * character setting.  
	 */
	if (tuple->next_state_1 == RE_STATE_INVALID) {
		/* 
		 * set the character ID, unless the state has been marked with a
		 * special flag which indicates that the character value has
		 * another meaning (in particular, a group marker) 
		 */
		if (!(tuple->flags & (RE_STATE_GROUP_BEGIN | RE_STATE_GROUP_END)))
			tuple->ch = ch;

		// set the first transition
		tuple->next_state_1 = dest_id;
	} else {
		// set only the second transition state - don't set the character
		tuple->next_state_2 = dest_id;
	}
}

void re_context::init_machine(re_machine *machine) {
	machine->init = alloc_state();
	machine->final = alloc_state();
}

void re_context::build_char(re_machine *machine, char ch) {
	// initialize our new machine
	init_machine(machine);

	// allocate a transition tuple for the new state
	set_trans(machine->init, machine->final, ch);
}

void re_context::build_char_range(re_machine *machine, unsigned char *range, int exclusion) {
	unsigned char *range_copy;
	
	// initialize our new machine
	init_machine(machine);

	// allocate a transition table for the new state
	set_trans(machine->init, machine->final, (char)(exclusion ? RE_RANGE_EXCL : RE_RANGE));

	// allocate a copy of the range bit vector
	range_copy = (unsigned char *)mchalo(_errctx, 32, "regex range");

	// copy the caller's range
	memcpy(range_copy, range, 32);

	// store it in the tuple
	_tuple_arr[machine->init].char_range = range_copy;
}

void re_context::build_group_matcher(re_machine *machine, int group_num) {
	// initialize our new machine
	init_machine(machine);

	/* 
	 * Allocate a transition tuple for the new state, using the group ID
	 * as the character code.  Store the special code for a group
	 * recognizer rather than the normal literal character code.  
	 */
	set_trans(machine->init, machine->final, (char)(group_num + RE_GROUP_MATCH_0));
}

void re_context::build_concat(re_machine *new_machine, re_machine *lhs, re_machine *rhs) {
	// initialize the new machine
	init_machine(new_machine);

	/* 
	 * Set up an epsilon transition from the new machine's initial state
	 * to the first submachine's initial state 
	 */
	set_trans(new_machine->init, lhs->init, RE_EPSILON);

	/*
	 * Set up an epsilon transition from the first submachine's final
	 * state to the second submachine's initial state 
	 */
	set_trans(lhs->final, rhs->init, RE_EPSILON);

	/*
	 * Set up an epsilon transition from the second submachine's final
	 * state to our new machine's final state 
	 */
	set_trans(rhs->final, new_machine->final, RE_EPSILON);
}

void re_context::build_group(re_machine *new_machine, re_machine *sub_machine, int group_id) {
	// initialize the container machine
	init_machine(new_machine);

	/* 
	 * set up an epsilon transition from the new machine's initial state
	 * into the initial state of the group, and another transition from
	 * the group's final state into the container's final state 
	 */
	set_trans(new_machine->init, sub_machine->init, RE_EPSILON);
	set_trans(sub_machine->final, new_machine->final, RE_EPSILON);

	// Mark the initial and final states of the group machine as being group markers
	_tuple_arr[new_machine->init].flags |= RE_STATE_GROUP_BEGIN;
	_tuple_arr[new_machine->final].flags |= RE_STATE_GROUP_END;

	// store the group ID in the 'ch' member of the start and end states
	_tuple_arr[new_machine->init].ch = group_id;
	_tuple_arr[new_machine->final].ch = group_id;
}

void re_context::build_alter(re_machine *new_machine, re_machine *lhs, re_machine *rhs) {
	// initialize the new machine
	init_machine(new_machine);

	/*
	 * Set up an epsilon transition from our new machine's initial state
	 * to the initial state of each submachine 
	 */
	set_trans(new_machine->init, lhs->init, RE_EPSILON);
	set_trans(new_machine->init, rhs->init, RE_EPSILON);

	/*
	 * Set up an epsilon transition from the final state of each
	 * submachine to our final state 
	 */
	set_trans(lhs->final, new_machine->final, RE_EPSILON);
	set_trans(rhs->final, new_machine->final, RE_EPSILON);
}

void re_context::build_closure(re_machine *new_machine, re_machine *sub, char specifier) {
	// initialize the new machine
	init_machine(new_machine);

	/* 
	 * Set up an epsilon transition from our initial state to the submachine's initial
	 * state, and from the submachine's final state to our final state 
	 */
	set_trans(new_machine->init, sub->init, RE_EPSILON);
	set_trans(sub->final, new_machine->final, RE_EPSILON);

	/*
	 * If this is an unbounded closure ('*' or '+', but not '?'), set up
	 * the loop transition that takes us from the new machine's final
	 * state back to its initial state.  We don't do this on the
	 * zero-or-one closure, because we can only match the expression
	 * once.  
	 */
	if (specifier != '?')
		set_trans(sub->final, sub->init, RE_EPSILON);

	/*
	 * If this is a zero-or-one closure or a zero-or-more closure, set
	 * up an epsilon transition from our initial state to our final
	 * state, since we can skip the entire subexpression.  We don't do
	 * this on the one-or-more closure, because we can't skip the
	 * subexpression in this case.  
	 */
	if (specifier != '+')
		set_trans(new_machine->init, new_machine->final, RE_EPSILON);
}

void re_context::concat_onto(re_machine *dest, re_machine *rhs) {
	// check for a null destination machine
	if (dest->isNull()) {
		/* 
		 * the first machine is null - simply copy the second machine
		 * onto the first unchanged 
		 */
		*dest = *rhs;
	} else {
		re_machine new_machine;
		
		// build the concatenated machine
		build_concat(&new_machine, dest, rhs);

		// copy the concatenated machine onto the first machine
		*dest = new_machine;
	}
}

void re_context::alternate_onto(re_machine *dest, re_machine *rhs) {
	// check to see if the first machine is null
	if (dest->isNull()) {
		/* 
		 * the first machine is null - simply copy the second machine
		 * onto the first 
		 */
		*dest = *rhs;
	} else {
		/* 
		 * if the second machine is null, don't do anything; otherwise,
		 * build the alternation 
		 */
		if (!rhs->isNull()) {
			re_machine new_machine;
			
			// build the alternation
			build_alter(&new_machine, dest, rhs);

			// replace the first machine with the alternation
			*dest = new_machine;
		}
	}
}

/**
 * Set a bit in a bit vector.
 */
#define re_set_bit(set, bit) \
	(((unsigned char *)(set))[(bit) >> 3] |= (1 << ((bit) & 7)))

/**
 * Test a bit in a bit vector 
 */
#define re_is_bit_set(set, bit) \
	((((unsigned char *)(set))[(bit) >> 3] & (1 << ((bit) & 7))) != 0)

re_status_t re_context::compile(const char *expr, size_t exprlen, re_machine *result_machine) {
	re_machine cur_machine;
	re_machine alter_machine;
	re_machine new_machine;
	size_t group_stack_level;
	struct {
		re_machine old_cur;
		re_machine old_alter;
		int group_id;
	} group_stack[50];

	// reset everything
	reset();

	// start out with no current machine and no alternate machine
	cur_machine.build_null_machine();
	alter_machine.build_null_machine();

	// nothing on the stack yet
	group_stack_level = 0;

	// loop until we run out of expression to parse
	for (; exprlen != 0 ; ++expr, --exprlen) {
		switch(*expr) {
		case '^':
			/*
			 * beginning of line - if we're not at the beginning of the
			 * current expression (i.e., we already have some
			 * concatentations accumulated), treat it as an ordinary
			 * character 
			 */
			if (!cur_machine.isNull())
				goto normal_char;

			// build a new start-of-text recognizer
			build_char(&new_machine, RE_TEXT_BEGIN);

			/* 
			 * concatenate it onto the string - note that this can't
			 * have any postfix operators 
			 */
			concat_onto(&cur_machine, &new_machine);
			break;

		case '$':
			/*
			 * End of line specifier - if there's anything left after
			 * the '$' other than a close parens or alternation
			 * specifier, great it as a normal character 
			 */
			if (exprlen > 1
				&& (*(expr+1) != ')' && *(expr+1) != '|'))
				goto normal_char;

			// build a new end-of-text recognizer
			build_char(&new_machine, RE_TEXT_END);

			/* 
			 * concatenate it onto the string - note that this can't
			 * have any postfix operators 
			 */
			concat_onto(&cur_machine, &new_machine);
			break;
			
		case '(':
			/* 
			 * Add a nesting level.  Push the current machine and
			 * alternate machines onto the group stack, and clear
			 * everything out for the new group. 
			 */
			if (group_stack_level > sizeof(group_stack)/sizeof(group_stack[0])) {
				/* we cannot proceed - return an error */
				return RE_STATUS_GROUP_NESTING_TOO_DEEP;
			}

			// save the current state on the stack
			group_stack[group_stack_level].old_cur = cur_machine;
			group_stack[group_stack_level].old_alter = alter_machine;

			/* 
			 * Assign the group a group ID - groups are numbered in
			 * order of their opening (left) parentheses, so we want to
			 * assign a group number now.  We won't actually need to
			 * know the group number until we get to the matching close
			 * paren, but we need to assign it now, so store it in the
			 * group stack. 
			 */
			group_stack[group_stack_level].group_id = _cur_group;

			// consume the group number
			_cur_group++;

			// push the level
			++group_stack_level;

			// start the new group with empty machines
			cur_machine.build_null_machine();
			alter_machine.build_null_machine();
			break;

		case ')':
			// if there's nothing on the stack, ignore this
			if (group_stack_level == 0)
				break;

			// take a level off the stack
			--group_stack_level;

			/* 
			 * Remove a nesting level.  If we have a pending alternate
			 * expression, build the alternation expression.  This will
			 * leave the entire group expression in alter_machine,
			 * regardless of whether an alternation was in progress or
			 * not.  
			 */
			alternate_onto(&alter_machine, &cur_machine);

			/*
			 * Create a group machine that encloses the group and marks
			 * it with a group number.  We assigned the group number
			 * when we parsed the open paren, so read that group number
			 * from the stack.
			 * 
			 * Note that this will leave 'new_machine' with the entire
			 * group machine.  
			 */
			build_group(&new_machine, &alter_machine,
				group_stack[group_stack_level].group_id);

			/*
			 * Pop the stack - restore the alternation and current
			 * machines that were in progress before the group started. 
			 */
			cur_machine = group_stack[group_stack_level].old_cur;
			alter_machine = group_stack[group_stack_level].old_alter;

			/*
			 * Check the group expression (in new_machine) for postfix
			 * expressions 
			 */
			goto apply_postfix;

		case '|':
			/* 
			 * Start a new alternation.  This ends the current
			 * alternation; if we have a previous pending alternate,
			 * build an alternation machine out of the previous
			 * alternate and the current machine and move that to the
			 * alternate; otherwise, simply move the current machine to
			 * the pending alternate. 
			 */
			alternate_onto(&alter_machine, &cur_machine);

			/* 
			 * the alternation starts out with a blank slate, so null
			 * out the current machine 
			 */
			cur_machine.build_null_machine();
			break;

		case '%':
			// quoted character - skip the quote mark and see what we have 
			++expr;
			--exprlen;

			// check to see if we're at the end of the expression
			if (exprlen == 0) {
				/* 
				 * end of the string - ignore it, but undo the extra
				 * increment of the expression index so that we exit the
				 * enclosing loop properly 
				 */
				--expr;
				++exprlen;
				break;
			}

			// see what we have
			switch(*expr) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				// group match - build a new literal group recognizer
				build_group_matcher(&new_machine, (int)(*expr - '1'));

				// apply any postfix expression to the group recognizer
				goto apply_postfix;

			case '<':
				// build a beginning-of-word recognizer
				build_char(&new_machine, RE_WORD_BEGIN);

				// it can't be postfixed - just concatenate it
				concat_onto(&cur_machine, &new_machine);
				break;

			case '>':
				// build an end-of-word recognizer */
				build_char(&new_machine, RE_WORD_END);

				// it can't be postfixed - just concatenate it
				concat_onto(&cur_machine, &new_machine);
				break;

			case 'w':
				// word character
				build_char(&new_machine, RE_WORD_CHAR);
				goto apply_postfix;

			case 'W':
				// non-word character
				build_char(&new_machine, RE_NON_WORD_CHAR);
				goto apply_postfix;

			case 'b':
				// word boundary
				build_char(&new_machine, RE_WORD_BOUNDARY);

				// it can't be postfixed
				concat_onto(&cur_machine, &new_machine);
				break;

			case 'B':
				// not a word boundary
				build_char(&new_machine, RE_NON_WORD_BOUNDARY);

				// it can't be postfixed
				concat_onto(&cur_machine, &new_machine);
				break;

			default:
				// build a new literal character recognizer
				build_char(&new_machine, *expr);

				// apply any postfix expression to the character
				goto apply_postfix;
			}
			break;

		case '.':
			/* 
			 * wildcard character - build a single character recognizer
			 * for the special wildcard symbol, then go check it for a
			 * postfix operator 
			 */
			build_char(&new_machine, RE_WILDCARD);
			goto apply_postfix;
			break;

		case '[': {
			// range expression
			int is_exclusive = false;
			unsigned char set[32];

			// clear out the set of characters in the range
			memset(set, 0, sizeof(set));

			// first, skip the open bracket
			++expr;
			--exprlen;

			// check to see if starts with the exclusion character
			if (exprlen != 0 && *expr == '^') {
				// skip the exclusion specifier
				++expr;
				--exprlen;

				// note it
				is_exclusive = true;
			}

			// if the first character is a ']', include it in the range 
			if (exprlen != 0 && *expr == ']') {
				re_set_bit(set, (int)']');
				++expr;
				--exprlen;
			}

			// if the next character is a '-', include it in the range        
			if (exprlen != 0 && *expr == '-') {
				re_set_bit(set, (int)'-');
				++expr;
				--exprlen;
			}

			// scan the character set
			while (exprlen != 0 && *expr != ']') {
				int ch;

				// note this character
				ch = (int)(unsigned char)*expr;

				// set it
				re_set_bit(set, ch);

				// skip this character of the expression
				++expr;
				--exprlen;

				// check for a range
				if (exprlen != 0 && *expr == '-') {
					int ch2;
	
					// skip the '-'
					++expr;
					--exprlen;
					if (exprlen != 0) {
						// get the other end of the range
						ch2 = (int)(unsigned char)*expr;

						// skip the second character
						++expr;
						--exprlen;

						// if the range is reversed, swap it
						if (ch > ch2)
							SWAP(ch, ch2);

						// fill in the range
						for ( ; ch <= ch2 ; ++ch)
							re_set_bit(set, ch);
					}
				}
			}

			// create a character range machine
			build_char_range(&new_machine, set, is_exclusive);

			// apply any postfix operator
			goto apply_postfix;
			break;
		}

		default:
		normal_char:
			/* 
			 * it's an ordinary character - build a single character
			 * recognizer machine, and then concatenate it onto any
			 * existing machine 
			 */
			build_char(&new_machine, *expr);

		apply_postfix:
			/*
			 * Check for a postfix operator, and apply it to the machine
			 * in 'new_machine' if present.  In any case, concatenate
			 * the 'new_machine' (modified by a postix operator or not)
			 * to the current machien.  
			 */
			if (exprlen > 1) {
				switch(*(expr+1)) {
				case '*':
				case '+':
				case '?':
					/*
					 * We have a postfix closure operator.  Build a new
					 * closure machine out of 'new_machine'.  
					 */
					{
						re_machine closure_machine;
						
						// move onto the closure operator
						++expr;
						--exprlen;
						
						// build the closure machine
						build_closure(&closure_machine, &new_machine, *expr);
						
						// replace the original machine with the closure
						new_machine = closure_machine;
						
						/* 
						 * skip any redundant closure symbols, keeping
						 * only the first one we saw 
						 */
						while (exprlen > 1 && (*(expr+1) == '?'
											   || *(expr+1) == '+'
											   || *(expr+1) == '*')) {
							++expr;
							--exprlen;
						}
					}
					break;
					
				default:
					/* no postfix operator */
					break;
				}
			}

			/*
			 * Concatenate the new machine onto the current machine
			 * under construction.  
			 */
			concat_onto(&cur_machine, &new_machine);
			break;
		}
	}

	// complete any pending alternation
	alternate_onto(&alter_machine, &cur_machine);

	// store the resulting machine in the caller's machine descriptor
	*result_machine = alter_machine;

	// no errors encountered
	return RE_STATUS_SUCCESS;
}

void re_context::note_group(re_group_register *regs, re_state_id id, const char *p) {
	int group_index;

	/*
	 * Check to see if this is a valid state and it's a group marker -
	 * if not, there's nothing to do 
	 */
	if (id == RE_STATE_INVALID
		|| !(_tuple_arr[id].flags
			 & (RE_STATE_GROUP_BEGIN | RE_STATE_GROUP_END))
		|| (group_index = (int)_tuple_arr[id].ch) >= RE_GROUP_REG_CNT)
		return;

	// It's a valid group marker - note the appropriate register value 
	if ((_tuple_arr[id].flags & RE_STATE_GROUP_BEGIN) != 0)
		regs[group_index].start_ofs = p;
	else
		regs[group_index].end_ofs = p;
}

bool re_context::is_word_char(char c) const {
	return Common::isAlnum(c);
}

int re_context::match(const char *entire_str, const char *str, size_t origlen,
		const re_machine *machine, re_group_register *regs) {
	re_state_id cur_state;
	const char *p;
	size_t curlen;

	// start at the machine's initial state
	cur_state = machine->init;

	// start at the beginning of the string
	p = str;
	curlen = origlen;

	// note any group involved in the initial state
	note_group(regs, cur_state, p);

	/* 
	 * if we're starting in the final state, immediately return success
	 * with a zero-length match 
	 */
	if (cur_state == machine->final) {
		// return success with a zero-length match
		return 0;
	}

	// run the machine
	for (;;) {
		re_tuple *tuple;

		// get the tuple for this state
		tuple = &_tuple_arr[cur_state];

		// if this is a group state, adjust the group registers
		note_group(regs, cur_state, p);
		
		// see what kind of state we're in
		if (!(tuple->flags & (RE_STATE_GROUP_BEGIN | RE_STATE_GROUP_END))
			&& tuple->ch != RE_EPSILON) {
			/*
			 * This is a character or group recognizer state.  If we
			 * match the character or group, continue on to the next
			 * state; otherwise, return failure.
			 */
			switch(tuple->ch) {
			case RE_GROUP_MATCH_0:
			case RE_GROUP_MATCH_0 + 1:
			case RE_GROUP_MATCH_0 + 2:
			case RE_GROUP_MATCH_0 + 3:
			case RE_GROUP_MATCH_0 + 4:
			case RE_GROUP_MATCH_0 + 5:
			case RE_GROUP_MATCH_0 + 6:
			case RE_GROUP_MATCH_0 + 7:
			case RE_GROUP_MATCH_0 + 8:
			case RE_GROUP_MATCH_0 + 9: {
				int group_num;
				re_group_register *group_reg;
				size_t reg_len;

				// it's a group - get the group number
				group_num = tuple->ch - RE_GROUP_MATCH_0;
				group_reg = &regs[group_num];
					
				/* 
				 * if this register isn't defined, there's nothing
				 * to match, so fail 
				 */
				if (group_reg->start_ofs == 0 || group_reg->end_ofs == 0)
					return -1;
					
				// calculate the length of the register value
				reg_len = group_reg->end_ofs - group_reg->start_ofs;

				// if we don't have enough left to match, it fails
				if (curlen < reg_len)
					return -1;

				// if the string doesn't match exactly, we fail
				if (memcmp(p, group_reg->start_ofs, reg_len) != 0)
					return -1;
					
				/*
				 * It matches exactly - skip the entire length of
				 * the register in the source string 
				 */
				p += reg_len;
				curlen -= reg_len;
				break;
			}

			case RE_TEXT_BEGIN:
				/* 
				 * Match only the exact beginning of the string - if
				 * we're anywhere else, this isn't a match.  If this
				 * succeeds, we don't skip any characters.  
				 */
				if (p != entire_str)
					return -1;
				break;

			case RE_TEXT_END:
				/*
				 * Match only the exact end of the string - if we're
				 * anywhere else, this isn't a match.  Don't skip any
				 * characters on success.  
				 */
				if (curlen != 0)
					return -1;
				break;

			case RE_WORD_BEGIN:
				/* 
				 * if the previous character is a word character, we're
				 * not at the beginning of a word 
				 */
				if (p != entire_str && is_word_char(*(p - 1)))
					return -1;

				/* 
				 * if we're at the end of the string, or the current
				 * character isn't the start of a word, we're not at the
				 * beginning of a word 
				 */
				if (curlen == 0 || !is_word_char(*p))
					return -1;
				break;

			case RE_WORD_END:
				/*
				 * if the current character is a word character, we're not
				 * at the end of a word 
				 */
				if (curlen != 0 && is_word_char(*p))
					return -1;

				/*
				 * if we're at the beginning of the string, or the
				 * previous character is not a word character, we're not
				 * at the end of a word 
				 */
				if (p == entire_str || !is_word_char(*(p - 1)))
					return -1;
				break;

			case RE_WORD_CHAR:
				/* if it's not a word character, it's a failure */
				if (curlen == 0 || !is_word_char(*p))
					return -1;

				/* skip this character of input */
				++p;
				--curlen;
				break;

			case RE_NON_WORD_CHAR:
				/* if it's a word character, it's a failure */
				if (curlen == 0 || is_word_char(*p))
					return -1;

				/* skip the input */
				++p;
				--curlen;
				break;

			case RE_WORD_BOUNDARY:
			case RE_NON_WORD_BOUNDARY:
				{
					int prev_is_word;
					int next_is_word;
					int boundary;

					/*
					 * Determine if the previous character is a word
					 * character -- if we're at the beginning of the
					 * string, it's obviously not, otherwise check its
					 * classification 
					 */
					prev_is_word = (p != entire_str
									&& is_word_char(*(p - 1)));

					/* make the same check for the current character */
					next_is_word = (curlen != 0
									&& is_word_char(*p));

					/*
					 * Determine if this is a boundary - it is if the
					 * two states are different 
					 */
					boundary = ((prev_is_word != 0) ^ (next_is_word != 0));

					/* 
					 * make sure it matches what was desired, and return
					 * failure if not 
					 */
					if ((tuple->ch == RE_WORD_BOUNDARY && !boundary)
						|| (tuple->ch == RE_NON_WORD_BOUNDARY && boundary))
						return -1;
				}
				break;

			case RE_WILDCARD:
				// make sure we have a character to match
				if (curlen == 0)
					return -1;

				// skip this character
				++p;
				--curlen;
				break;

			case RE_RANGE:
			case RE_RANGE_EXCL: {
				int match_val;

				// make sure we have a character to match
				if (curlen == 0)
					return -1;

				// see if we match
				match_val = re_is_bit_set(tuple->char_range, (int)(unsigned char)*p);
					
				// make sure we got what we wanted
				if ((tuple->ch == RE_RANGE && !match_val)
					|| (tuple->ch == RE_RANGE_EXCL && match_val))
					return -1;

				// skip this character of the input
				++p;
				--curlen;
				break;
			}

			default:
				// make sure we have an exact match
				if (curlen == 0 || tuple->ch != *p)
					return -1;

				// skip this character of the input
				++p;
				--curlen;
				break;
			}

			/* 
			 * if we got this far, we were successful - move on to the
			 * next state 
			 */
			cur_state = tuple->next_state_1;
		} else if (tuple->next_state_2 == RE_STATE_INVALID) {
			/*
			 * We have only one transition, so this state is entirely
			 * deterministic.  Simply move on to the next state. 
			 */
			cur_state = tuple->next_state_1;
		} else {
			re_machine sub_machine;
			re_group_register regs1[RE_GROUP_REG_CNT];
			re_group_register regs2[RE_GROUP_REG_CNT];
			int ret1;
			int ret2;
			
			/*
			 * This state has two possible transitions, and we don't
			 * know which one to take.  So, try both, see which one
			 * works better, and return the result.  Try the first
			 * transition first.  Note that each separate attempt must
			 * use a separate copy of the registers.  
			 */
			memcpy(regs1, regs, sizeof(regs1));
			sub_machine.init = tuple->next_state_1;
			sub_machine.final = machine->final;
			ret1 = match(entire_str, p, curlen, &sub_machine, regs1);

			/*
			 * Now try the second transition 
			 */
			memcpy(regs2, regs, sizeof(regs2));
			sub_machine.init = tuple->next_state_2;
			sub_machine.final = machine->final;
			ret2 = match(entire_str, p, curlen, &sub_machine, regs2);

			/*
			 * If they both failed, the whole thing failed.  Otherwise,
			 * return the longer of the two, plus the length we
			 * ourselves matched previously.  Note that we return the
			 * register set from the winning match.  
			 */
			if (ret1 < 0 && ret2 < 0) {
				// they both failed
				return -1;
			} else if (ret1 > ret2) {
				// use the first register set and result length
				memcpy(regs, regs1, sizeof(regs1));
				return ret1 + (p - str);
			} else {
				// use the second register set and result length
				memcpy(regs, regs2, sizeof(regs2));
				return ret2 + (p - str);
			}
		}

		// If we're in the final state, return success 
		if (cur_state == machine->final) {
			// finish off any group involved in the final state
			note_group(regs, cur_state, p);
			
			// return the length we matched
			return p - str;
		}
	}
}

int re_context::search(const char *str, size_t len, const re_machine *machine,
		re_group_register *regs, int *result_len) {
	int ofs;
	
	/*
	 * Starting at the first character in the string, search for the
	 * pattern at each subsequent character until we either find the
	 * pattern or run out of string to test. 
	 */
	for (ofs = 0 ; ofs < (int)len ; ++ofs) {
		int matchlen;
		
		// check for a match
		matchlen = match(str, str + ofs, len - ofs, machine, regs);
		if (matchlen >= 0) {
			// we found a match here - return the length and offset
			*result_len = matchlen;
			return ofs;
		}
	}

	// we didn't find a match
	return -1;
}

void re_context::save_search_str(const char *str, size_t len) {
	// if the string is empty, this is easy
	if (len == 0) {
		// nothing to store - just save the length and return
		_curlen = 0;
		return;
	}
	
	// if the current buffer isn't big enough, allocate a new one
	if (_strbuf == 0 || _strbufsiz < len) {
		/* 
		 * free any previous buffer - its contents are no longer
		 * important, since we're about to overwrite it with a new
		 * string 
		 */
		if (_strbuf != 0)
			mchfre(_strbuf);

		/* 
		 * allocate a new buffer; round up to the next 256-byte
		 * increment to make sure we're not constantly reallocating to
		 * random sizes 
		 */
		_strbufsiz = ((len + 255) & ~255);

		// allocate it
		_strbuf = (char *)mchalo(_errctx, _strbufsiz, "regex str");
	}

	// copy the string
	memcpy(_strbuf, str, len);

	// save the length
	_curlen = len;
}

int re_context::compile_and_search(const char *pattern, size_t patlen,
		const char *searchstr, size_t searchlen, int *result_len) {
	re_machine machine;
	
	// compile the expression - return failure if we get an error
	if (compile(pattern, patlen, &machine) != RE_STATUS_SUCCESS)
		return -1;

	// save the search string in our internal buffer
	save_search_str(searchstr, searchlen);

	// clear the group registers
	memset(_regs, 0, sizeof(_regs));

	/* 
	 * search for the pattern in our copy of the string - use the copy
	 * so that the group registers stay valid even if the caller
	 * deallocates the original string after we return
	 */
	return search(_strbuf, _curlen, &machine, _regs, result_len);
}

int re_context::compile_and_match(const char *pattern, size_t patlen,
		const char *searchstr, size_t searchlen) {
	re_machine machine;

	// compile the expression - return failure if we get an error
	if (compile(pattern, patlen, &machine) != RE_STATUS_SUCCESS)
		return 0;

	// save the search string in our internal buffer
	save_search_str(searchstr, searchlen);

	// clear the group registers
	memset(_regs, 0, sizeof(_regs));

	// match the string
	return match(_strbuf, _strbuf, _curlen, &machine, _regs);
}

} // End of namespace TADS2
} // End of namespace TADS
} // Engine of namespace GLK
