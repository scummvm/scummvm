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

#ifndef GLK_TADS_TADS2_REGEX
#define GLK_TADS_TADS2_REGEX

#include "common/array.h"
#include "engines/glk/tads/tads2/ler.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * state ID
 */
typedef int re_state_id;

/**
 * invalid state ID - used to mark null machines
 */
#define RE_STATE_INVALID   ((re_state_id)-1)

/**
 * first valid state ID
 */
#define RE_STATE_FIRST_VALID  ((re_state_id)0)


/**
 *   Group register structure.  Each register keeps track of the starting
 *   and ending offset of the group's text.
 */
struct re_group_register {
	const char *start_ofs;
	const char *end_ofs;
};

/**
 * number of group registers we keep
 */
#define RE_GROUP_REG_CNT  10

/**
 * Denormalized state transition tuple.  Each tuple represents the
 * complete set of transitions out of a particular state.  A particular
 * state can have one character transition, or two epsilon transitions.
 * Note that we don't need to store the state ID in the tuple, because
 * the state ID is the index of the tuple in an array of state tuples.
 */
struct re_tuple {
	// the character we must match to transition to the target state
	char ch;

	// the target states
	re_state_id next_state_1;
	re_state_id next_state_2;

	// character range match table, if used
	unsigned char *char_range;

	// flags
	byte flags;
};


/**
 * Tuple flags
 */
enum {
	// this state is the start of a group - the 'ch' value is the group ID
	RE_STATE_GROUP_BEGIN = 0x02,

	// this state is the end of a group - 'ch' is the group ID */
	RE_STATE_GROUP_END = 0x04
};

/**
 * Status codes
 */
typedef enum {
	// success
	RE_STATUS_SUCCESS = 0,

	// compilation error - group nesting too deep
	RE_STATUS_GROUP_NESTING_TOO_DEEP
} re_status_t;


/**
 * Regular expression compilation.  This tracks the state of the compilation and
 * stores the resources associated with the compiled expression.
 */
class re_context {
	/**
	 * A machine description.  Machines are fully described by their initial
	 * and final state ID's.
	 */
	struct re_machine {
		re_state_id init;		///< the machine's initial state
		re_state_id final;		///< the machine's final state

		re_machine() : init(0), final(0) {}

		/**
		 * Build a null machine
		 */
		void build_null_machine() {
			init = final = RE_STATE_INVALID;
		}

		/**
		 * Determine if a machine is null
		 */
		bool isNull() const {
			return (init == RE_STATE_INVALID);
		}
	};
private:
	/**
	 * Reset compiler - clears states and tuples
	 */
	void reset();

	/**
	 * Set a transition from a state to a given destination state
	 */
	void set_trans(re_state_id id, re_state_id dest_id, char ch);

	/**
	 * Initialize a new machine, giving it an initial and final state
	 */
	void init_machine(re_machine *machine);

	/**
	 * Build a character recognizer
	 */
	void build_char(re_machine *machine, char ch);

	/**
	 * Build a character range recognizer.  'range' is a 256-bit (32-byte) bit vector.
	 */
	void build_char_range(re_machine *machine, unsigned char *range, int exclusion);

	/**
	 * Build a group recognizer.  This is almost the same as a character
	 * recognizer, but matches a previous group rather than a literal character.
	 */
	void build_group_matcher(re_machine *machine, int group_num);

	/**
	 *   Build a concatenation recognizer
	 */
	void build_concat(re_machine *new_machine, re_machine *lhs, re_machine *rhs);

	/**
	 * Build a group machine.  sub_machine contains the machine that
	 * expresses the group's contents; we'll fill in new_machine with a
	 * newly-created machine that encloses and marks the group.
	 */
	void build_group(re_machine *new_machine, re_machine *sub_machine, int group_id);

	/**
	 * Build an alternation recognizer
	 */
	void build_alter(re_machine *new_machine, re_machine *lhs, re_machine *rhs);

	/**
	 * Build a closure recognizer
	 */
	void build_closure(re_machine *new_machine, re_machine *sub, char specifier);

	/**
	 * Concatenate the second machine onto the first machine, replacing the
	 * first machine with the resulting machine.  If the first machine is a
	 * null machine (created with re_build_null_machine), we'll simply copy
	 * the second machine into the first.
	 */
	void concat_onto(re_machine *dest, re_machine *rhs);

	/**
	 * Alternate the second machine onto the first machine, replacing the
	 * first machine with the resulting machine.  If the first machine is a
	 * null machine, this simply replaces the first machine with the second
	 * machine.  If the second machine is null, this simply leaves the first
	 * machine unchanged.
	 */
	void alternate_onto(re_machine *dest, re_machine *rhs);

	/**
	 * Compile an expression
	 */
	re_status_t compile(const char *expr, size_t exprlen, re_machine *result_machine);

	/**
	 * Note a group position if appropriate
	 */
	void note_group(re_group_register *regs, re_state_id id, const char *p);

	/**
	 * Determine if a character is part of a word.  We consider letters and
	 * numbers to be word characters.
	 */
	bool is_word_char(char c) const;

	/**
	 * Match a string to a compiled expression.  Returns the length of the
	 * match if successful, or -1 if no match was found.
	 */
	int match(const char *entire_str, const char *str, size_t origlen,
		const re_machine *machine, re_group_register *regs);

	/**
	 * Search for a regular expression within a string.  Returns -1 if the string
	 * cannot be found, otherwise returns the offset from the start of the string
	 * to be searched of the start of the first match for the pattern.
	 */
	int search(const char *str, size_t len, const re_machine *machine,
		re_group_register *regs, int *result_len);

	/**
	 * Make a copy of a search string in our private buffer.
	 */
	void save_search_str(const char *str, size_t len);
public:
	errcxdef *_errctx;			///< error context
	re_state_id _next_state;	///< next available state ID

	/**
	 * The array of transition tuples.  We'll allocate this array and
	 * expand it as necessary.
	 */
	Common::Array<re_tuple> _tuple_arr;

	// current group ID
	int _cur_group;

	// group registers
	re_group_register _regs[RE_GROUP_REG_CNT];

	/**
	 * Buffer for retaining a copy of the last string we scanned.  We
	 * retain our own copy of each string, and point the group registers
	 * into this copy rather than the caller's original string -- this
	 * ensures that the group registers remain valid even after the
	 * caller has deallocated the original string.
	 */
	char *_strbuf;

	/**
	 * length of the string currently in the buffer
	 */
	size_t _curlen;

	/**
	 * size of the buffer allocated to strbuf
	 */
	size_t _strbufsiz;
public:
	/**
	 * Constructor.  The memory for the context structure itself
	 * must be allocated and maintained by the caller.
	 */
	re_context(errcxdef *errctx);

	/**
	 * Destructor
	 */
	~re_context();

	/**
	 * Allocate a new state ID
	 */
	re_state_id alloc_state();

	/**
	 * Compile an expression and search for a match within the given string.
	 * Returns the offset of the match, or -1 if no match was found.
	 */
	int compile_and_search(const char *pattern, size_t patlen,
		const char *searchstr, size_t searchlen, int *result_len);

	/**
	 * Compile an expression and check for a match.  Returns the length of the match
	 * if we found a match, -1 if we found no match.  This is not a search function;
	 * we merely match the leading substring of the given string to the given pattern.
	 */
	int compile_and_match(const char *pattern, size_t patlen,
		const char *searchstr, size_t searchlen);
};

} // End of namespace TADS2
} // End of namespace TADS
} // Engine of namespace GLK

#endif
