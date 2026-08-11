/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/jacl/jacl.h"
#include "glk/jacl/language.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"

namespace Glk {
namespace JACL {

#define FIRST_LIST      noun_number-2
#define SECOND_LIST     noun_number

int                             object_list[4][MAX_OBJECTS];
int                             multiple_resolved[MAX_OBJECTS];

// THIS IS THE NUMBER OF OBJECTS LEFT IN THE LIST
int                             list_size[4];

// THIS IS THE INDEX OF THE FINAL OBJECT
int                             max_size[4];

/* object_list[] HAS THE FOLLOWING INDEXES:
 * noun1            : 0
 * noun2            : 1
 * noun1 EXCEPTIONS : 2
 * noun2 EXCEPTIONS : 3
 */

int                             selection;

int                             matches = 0;
int                             highest_confidence = 0;
int                             prime_suspect = 0;
int                             done = 0;
int                             backup_pointer = 0;
int                             everything = 0;

int                             confidence[MAX_OBJECTS];
int                             possible_objects[MAX_OBJECTS];

int                             it;
int                             them[MAX_OBJECTS];
int                             her;
int                             him;
int                             parent;

int                             custom_error;

int                             oops_word;
int                             last_exact;

char                            *expected_scope[3];

// THIS ARRAY DEFINES THE OBJECTS THAT THE CURRENT OBJECTS ARE
// SUPPOSED TO BE A CHILD OF
int                             from_objects[MAX_OBJECTS];

int                             after_from;
const char                      *from_word;

int                             object_expected = FALSE;

char                            default_function[84];
char                            object_name[84];

char                            base_function[84];
char                            before_function[84];
char                            after_function[84];
char                            local_after_function[84];

extern char                     text_buffer[];
extern char                     function_name[];
extern char                     temp_buffer[];
extern char                     error_buffer[];
extern char                     override_[];
extern const char               *word[];

extern int                      quoted[];

extern struct object_type       *object[];
extern int                      objects;

extern int                      noun[];
extern int                      wp;
extern int                      player;

extern struct word_type         *grammar_table;
extern struct function_type     *executing_function;
extern struct object_type       *object[];
extern struct variable_type     *variable[];

void parser() {
	// THIS FUNCTION COMPARES THE WORDS IN THE PLAYER'S COMMAND TO THE
	// GRAMMAR TREE OF POSSIBLE COMMANDS

	struct word_type *pointer;
	struct word_type *matched_word = nullptr;

	int             index;
	int             current_noun = 0;

	// RESET TO START OF PROCESSING

	////printf("--- in parser\n");

	// THIS IS USED TO STORE THE LAST EXACT WORD MATCH IN THE PLAYERS COMMAND
	last_exact = -1;
	after_from = -1;
	from_objects[0] = 0;

	noun[0] = FALSE;
	noun[1] = FALSE;

	// RESET BOTH THE LISTS TO BE EMPTY
	//printf("--- reset lists\n");
	for (index = 0; index < 4; index++) {
		list_size[index] = 0;
		max_size[index] = 0;
	}

	//printf("--- clear $integer\n");
	clear_cinteger("$integer");
	//printf("--- clear $string\n");
	clear_cstring("$string");
	//printf("--- clear action\n");
	clear_cstring("action");

	if (grammar_table == nullptr) {
		// THERE ARE NO USER DEFINED COMMANDS AVAILABLE, SO THE USER'S
		// COMMAND IS INEVITABLY INVALID
		//printf("--- no grammar table\n");
		INTERRUPTED->value = TRUE;
		diagnose();
		return;
	}

	//printf("--- set pointer to grammar table\n");
	// START AT THE TOP OF THE GRAMMAR TREE
	pointer = grammar_table;

	while (word[wp] != nullptr && pointer != nullptr) {
		//printf("--- wp = %d\n", wp);
		//printf("--- word[%d] = %s\n", wp, word[wp]);
		object_expected = FALSE;

		if (!strcmp(cstring_resolve("THEN_WORD")->value, word[wp])) {
			// CONSIDER THIS THE END OF THIS COMMAND AS 'THEN' IS
			// TREATED LIKE A FULL STOP
			break;
		} else if ((matched_word = exact_match(pointer)) != nullptr) {
			// THIS WORD WAS AN EXACT MATCH FOR ONE OF THE POSSIBLE WORDS
			// AT THE CURRENT GRAMMAR TREE LEVEL - MOVE ON!
			pointer = matched_word;
			pointer = pointer->first_child;
		} else if ((matched_word = object_match(pointer, current_noun)) != nullptr) {
			// THIS WAS AN OBJECT PLACE HOLDER AT THIS GRAMMAR LEVEL AND
			// THIS POINT IN THE PLAYER'S COMMAND COULD BE RESOLVED TO
			// AT LEAST ONE OBJECT

			if (list_size[current_noun] > 1) {
				// MULTIPLE OBJECTS WERE RETURNED
				if (matched_word->word[1] != '*') {
					if (last_exact == -1) {
						write_text(cstring_resolve("NO_MULTI_START")->value);
					} else {
						Common::sprintf_s(error_buffer, 1024, cstring_resolve("NO_MULTI_VERB")->value, word[last_exact]);
						write_text(error_buffer);
					}

					INTERRUPTED->value = TRUE;
					return;
				}
			}

			// STORE THE EXPECTED SCOPE FOR LATER CHECKING
			expected_scope[current_noun] = (char *)&matched_word->word;
			//printf("--- expected scope for noun%d is %s\n", current_noun, expected_scope[current_noun]);

			// THE NEXT MATCH OR GROUP OF MATCHES SHOULD BE IN THE SECOND
			// LIST OF OBJECTS
			current_noun++;

			// PUSH ON FROM THE POINT A MATCH WAS FOUND....
			pointer = matched_word;
			pointer = pointer->first_child;
		} else {
			// THIS IS AN UNKNOWN WORD
			if (oops_word == -1 && word[wp] != nullptr) {
				oops_word = wp;
			}

			if (custom_error == TRUE) {
				// THERE HAS BEEN SOME CUSTOM ERROR DISPLAYED ALREADY
				// SO JUST RETURN
				TIME->value = FALSE;
				INTERRUPTED->value = TRUE;
				return;
			} else {
				// THERE ARE NO MORE POSIBILITIES, THE WORD CAN'T BE
				// USED IN THIS CONTEXT
				INTERRUPTED->value = TRUE;
				diagnose();
				return;
			}
		}
	};

	if (pointer == nullptr) {
		// THIS CAN ONLY HAPPEN IF MOVING THE POINTER TO ITS
		// FIRST CHILD RESULTS IN A NULL - AN INCOMPLETE
		// GRAMMAR STATEMENT.
		log_error(INCOMPLETE_GRAMMAR, PLUS_STDOUT);
		INTERRUPTED->value = TRUE;
		return;
	}

	//printf("--- first list has %d objects\n", list_size[0]);
	//printf("--- first list has %d max\n", max_size[0]);
	//printf("--- second list has %d objects\n", list_size[1]);
	//printf("--- second list has %d max\n", max_size[1]);

	/* THE PLAYER'S MOVE HAS NO MORE WORDS, AND A BRANCH OF
	 * THE GRAMMAR TREE HAS BEEN FOUND THAT SO FAR MATCHES
	 * THIS MOVE. SCAN THROUGH ALL THE CHILDREN OF THE FINAL
	 * WORD OF THE PLAYER'S COMMAND AND FIND A FUNCTION BASE.
	 * (FUNCTION BASES BEGIN WITH '>'). IF THIS IS FOUND,
	 * CALL THE APPROPRIATE JACL FUNCTIONS IN THE GAME CODE
	 * IF NO FUNCTION BASE IS FOUND, MORE WORDS WERE EXPECTED
	 * IN ORDER TO CONSTRUCT A COMPLETE COMMAND. SHOW ERROR. */
	do {
		if (pointer->word[0] == '>') {
			//printf("--- found action %s\n", pointer->word);
			/* CALL ALL THE APPROPRIATE FUNCTIONS FOR EACH OF */
			/* THE OBJECTS IN THE SET */
			add_cstring("action", &pointer->word[1]);

			if (list_size[0] > 1) {
				/* FIRST IS MULTI, PRESUME SECOND IS SINGLE OR ZERO AS YOU
				 * CAN HAVE COMMANDS WITH TWO MULTIPLE OBJECT REFERENCES */
				noun[1] = first_available(1);

				for (index = 0; index < max_size[0]; index++) {
					/* CALL ALL THE FUNCTIONS ONCE FOR EACH OJECT */
					if (object_list[0][index] != 0) {
						noun[0] = object_list[0][index];

						if (MULTI_PREFIX->value) {
							// DISPLAY THE OBJECT BEING ACTED ON
							plain_output(noun[0], FALSE);
							write_text(temp_buffer);
							write_text(": ");
						}

						//printf("--- 1 calling functions for %s\n", object[noun[0]]->label);
						call_functions(pointer->word);

						/* IF INTERRUPTED BY SOME SPECIAL CONDITION, DON'T
						 * PERFORM THIS COMMAND FOR THE REMAINING OBJECTS */
						if (INTERRUPTED->value) {
							break;
						}
					}
				}
			} else {
				// FIRST LIST ISN'T MULTI
				//printf("--- first list isn't multi\n");
				if (list_size[1] > 1) {
					//printf("--- second list is multi\n");
					// ONLY SECOND IS MULTI
					noun[0] = first_available(0);

					for (index = 0; index < max_size[1]; index++) {
						/* CALL ALL THE FUNCTIONS ONCE FOR EACH OJECT */
						if (object_list[1][index] != 0) {
							noun[1] = object_list[1][index];

							if (MULTI_PREFIX->value) {
								// DISPLAY THE OBJECT BEING ACTED ON
								plain_output(noun[0], FALSE);
								write_text(temp_buffer);
								write_text(": ");
							}

							//printf("--- 2 calling functions for %s\n", object[noun[1]]->label);
							call_functions(pointer->word);

							/* IF INTERRUPTED BY SOME SPECIAL CONDITION, DON'T
							 * PERFORM THIS COMMAND FOR THE REMAINING OBJECTS */
							if (INTERRUPTED->value) {
								break;
							}
						}
					}
				} else {
					//printf("--- second list isn't multi\n");
					// NEITHER OBJECT REFERENCE IS MULTI
					if (list_size[0] == 0) {
						noun[0] = 0;
						noun[1] = 0;

						/* THIS IS AN OBJECT-LESS COMMAND */
						call_functions(pointer->word);
					} else {
						noun[0] = first_available(0);
						noun[1] = first_available(1);

						//printf("--- 3 calling functions for %s and %s\n", object[noun[0]]->label, object[noun[1]]->label);
						call_functions(pointer->word);
					}
				}
			}
			if (TIME->value == FALSE) {
				/* IS THIS ENOUGH TO INDICATE THAT THE OTHER COMMANDS
				 * SHOULDN'T BE PERFORMED??? */
				//INTERRUPTED->value = TRUE;
			}
			return;
		} else {
			//printf("--- move to next grammar sibling\n");
			// MOVE THROUGH THE OPTIONS AT THIS LEVEL OF THE GRAMMAR TREE
			// TO FIND THE ACTION THAT MATCHES THIS COMMAND
			if (pointer->next_sibling == nullptr) {
				break;
			} else {
				pointer = pointer->next_sibling;
			}
		}
	} while (TRUE);

	/* THERE IS NO FUNCTION AS A CHILD OF THE LAST WORD OF THE PLAYER'S
	 * COMMAND SO DISPLAY AN IN-GAME ERROR TO THE PLAYER. THIS IS LIKELY
	 * TO BE AN INCOMPLETE COMMAND */
	INTERRUPTED->value = TRUE;
	diagnose();
	return;
}

/* THIS FUNCTION RETURNS THE FIRST OBJECT IN THE SPECIFIED LIST
 * OR 0 IF IT IS EMPTY. THE FIRST OBJECT IN THE LIST IS THE FIRST
 * ELEMENT THAT IS NON-ZERO BEFORE THE max-size IS REACHED. */
int first_available(int list_number) {
	int index;

	if (list_size[list_number] == 0) return (0);

	//printf("--- looking for next object in list\n");
	for (index = 0; index < max_size[list_number]; index++) {
		if (object_list[list_number][index] != 0) {
			//printf("--- returning object %s\n", object[object_list[list_number][index]]->label);
			return (object_list[list_number][index]);
		}
	}

	//printf("--- no objects left in list\n");
	/* NO OBJECTS LEFT IN THE LIST */
	return (0);
}

void call_functions(const char *base_name) {
	/* THIS FUNCTION CALLS ALL THE APPROPRIATE JACL FUNCTIONS TO RESPOND
	 * TO A PLAYER'S COMMAND GIVEN A BASE FUNCTION NAME AND THE CURRENT
	 * VALUE OF noun1 AND noun2 */

	/* THE DEFAULT IS THAT THE COMMAND IS SUCCESSFUL AND THAT TIME SHOULD
	 * PASS. IF THE COMMAND FAILS, 'TIME' WILL BE SET TO FALSE */
	TIME->value = TRUE;

	Common::strlcpy(base_function, base_name + 1, 81);
	Common::strcat_s(base_function, "_");

	Common::strlcpy(override_, base_function, 81);

	Common::strcpy_s(before_function, "+before_");
	Common::strcat_s(before_function, base_name + 1);

	Common::strcpy_s(after_function, "+after_");
	Common::strcat_s(after_function, base_name + 1);

	Common::strcpy_s(local_after_function, "after_");
	Common::strcat_s(local_after_function, base_name + 1);
	if (noun[1] != FALSE) {
		Common::strcat_s(local_after_function, "_");
		Common::strcat_s(local_after_function, object[noun[1]]->label);
	}
	if (noun[0] != FALSE) {
		Common::strcat_s(local_after_function, "_");
		Common::strcat_s(local_after_function, object[noun[0]]->label);
	}

	/* THIS IS CALLED IF AN 'override' COMMAND IS EXECUTED
	 * IN A LIBRARY FUNCTION BUT THE OBJECT-OR-LOCATION-SPECIFIC
	 * OVERRIDE DOES NOT EXIST. IT IS SET TO '+default_func' */
	Common::strcpy_s(default_function, "+default_");
	Common::strcat_s(default_function, base_name + 1);

	/* EXECUTE THE GLOBAL *DEFAULT* BEFORE FUNCTION
	 * AND RETURN IF IT RETURNS TRUE */
	if (execute("+before") != FALSE)
		return;

	/* EXECUTE THE VERB-SPECIFIC BEFORE
	   FUNCTION AND RETURN IF IT RETURNS TRUE */
	if (execute(before_function) != FALSE)
		return;

	if (noun[0] == FALSE) { /* USER'S COMMAND HAS NO NOUNS */
		Common::strcat_s(base_function, object[HERE]->label);
		/* EXECUTE THE FUNCTION 'func_here' */
		if (execute(base_function) == FALSE) {
			/* THIS LOCATION-SPECIFIC FUNCTION DOES NOT
			 * EXIST OR HAS ISSUED A 'break false' COMMAND.
			 * EXECUTE THE FUNCTION '+func'
			 * WITH THE POSSIBILITY OF
			 * EXECUTING THE FUNCTION 'func_override_here'
			 * IF AN 'override' COMMAND IS FOUND IN '+func'
			 * IF THIS OVERRIDE FUNCTION ISN'T FOUND
			 * THE DEFAULT FUNCTION WILL BE EXECUTED */

			/* PREPARE THE OVERRIDE FUNCTION NAME IN CASE IT
			 * IS NEEDED */
			Common::strcat_s(override_, 81, "override_");
			Common::strcat_s(override_, 81, object[HERE]->label);

			/* CREATE THE FUNCTION NAME '+func' */
			Common::strcpy_s(base_function, "+");
			Common::strcat_s(base_function, base_name + 1);

			/* CALL THE LIBRARY'S DEFAULT BEHAVIOR */
			if (execute(base_function) == FALSE)
				unkfunrun(base_function);
		}
	} else if (noun[1] == FALSE) { /* USER'S COMMAND HAS ONE NOUN */
		Common::strcat_s(base_function, object[noun[0]]->label);
		/* EXECUTE THE FUNCTION 'func_noun1' */
		if (execute(base_function) == FALSE) {
			/* THIS OBJECT-SPECIFIC FUNCTION DOES NOT
			 * EXIST OR HAS ISSUED A 'break false' COMMAND.
			 * EXECUTE THE FUNCTION '+func'
			 * WITH THE POSSIBILITY OF
			 * EXECUTING THE FUNCTION 'func_override_noun1'
			 * IF AN 'override' COMMAND IS FOUND IN '+func'
			 * IF THIS OVERRIDE FUNCTION ISN'T FOUND
			 * THE DEFAULT FUNCTION WILL BE EXECUTED */

			/* PREPARE THE OVERRIDE FUNCTION NAME IN CASE IT
			 * IS NEEDED */
			Common::strcat_s(override_, 81, "override_");
			Common::strcat_s(override_, 81, object[noun[0]]->label);

			/* CREATE THE FUNCTION NAME '+func' */
			Common::strcpy_s(base_function, "+");
			Common::strcat_s(base_function, base_name + 1);

			/* CALL THE LIBRARY'S DEFAULT BEHAVIOR */
			if (execute(base_function) == FALSE)
				unkfunrun(base_function);
		}
	} else { /* USER'S COMMAND HAS TWO NOUNS */
		Common::strcat_s(base_function, object[noun[1]]->label);
		Common::strcat_s(base_function, "_");
		Common::strcat_s(base_function, object[noun[0]]->label);
		/* EXECUTE THE FUNCTION 'func_noun2_noun1'
		 * IE give_to_fred THAT IS ASSOCIATED WITH
		 * THE OBJECT flint_stone */
		if (execute(base_function) == FALSE) {
			/* THIS OBJECTS-SPECIFIC FUNCTION DOES NOT
			 * EXIST OR HAS ISSUED A 'break false' COMMAND.
			 * EXECUTE THE FUNCTION '+func'
			 * WITH THE POSSIBILITY OF
			 * EXECUTING THE FUNCTION 'func_override_noun2_noun1'
			 * IF AN 'override' COMMAND IS FOUND IN '+func'
			 * IF THIS OVERRIDE FUNCTION ISN'T FOUND
			 * THE DEFAULT FUNCTION WILL BE EXECUTED */

			/* PREPARE THE OVERRIDE FUNCTION NAME IN CASE IT
			 * IS NEEDED */
			Common::strcat_s(override_, 81, object[noun[1]]->label);
			Common::strcat_s(override_, 81, "_override_");
			Common::strcat_s(override_, 81, object[noun[0]]->label);

			/* CREATE THE FUNCTION NAME '+func' */
			Common::strcpy_s(base_function, "+");
			Common::strcat_s(base_function, base_name + 1);

			/* CALL THE LIBRARY'S DEFAULT BEHAVIOR */
			if (execute(base_function) == FALSE)
				unkfunrun(base_function);
		}
	}

	/* EXECUTE THE LOCAL AFTER FUNCTION
	 * AND RETURN IF IT RETURNS TRUE */
	if (execute(local_after_function) != FALSE)
		return;

	/* EXECUTE THE VERB-SPECIFIC AFTER
	 * FUNCTION AND RETURN IF IT RETURNS TRUE */
	if (execute(after_function) != FALSE)
		return;

	/* EXECUTE THE GLOBAL *DEFAULT* AFTER FUNCTION
	 * AND RETURN IF IT RETURNS TRUE */
	if (execute("+after") != FALSE)
		return;


	//sprintf(temp_buffer, "TIME = %d", TIME->value);
	//log_error(temp_buffer, PLUS_STDERR);

	if (TIME->value) {
		//printf("--- %s\n", base_function);
		eachturn();
	}

	return;
}

struct word_type *object_match(struct word_type *iterator, int noun_number) {
	/* THIS FUNCTION LOOPS THROUGH ALL THE POSIBILITIES IN THE CURRENT LEVEL
	 * OF THE GRAMMAR TREE TO SEE IF THERE ARE ANY OBJECT PLACE HOLDERS */

	/* STORE WHETHER BY THE END AN OBJECT PLACEHOLDER WAS ENCOUNTERED AND
	 * DISPLAY A MESSAGE IF NO OBJECTS MATCHED */
	int object_was_option = FALSE;

	do {
		/* THIS LOOP MEANS THAT CERTAIN ERRORS SUCH AS TAKING FROM A
		 * CLOSED CONTAINER CAN OCCUR MORE THAN ONCE */
		if (iterator->word[0] == '*') {
			object_was_option = TRUE;
			if (build_object_list(iterator, noun_number)) {
				/* RETURN THE POINT IN THE GRAMMAR TREE THAT MATCHED TO
				* CONTINUE ON FROM */
				//printf("--- returned TRUE from build_object_list\n");
				return (iterator);
			}
		}

		if (custom_error == TRUE) {
			/* AN ERROR OCCURRED IN THE FIRST OBJECT PLACEHOLDER, DON'T
			 * TRY ANY OTHERS */
			return (nullptr);
		}
	} while ((iterator = iterator->next_sibling) != nullptr);

	/* THERE WERE NO OBJECT PLACE HOLDERS OR, IF THERE WERE, NO
	 * MATCHING OBJECTS COULD BE RESOLVED */
	//printf("--- returning null from object_match\n");

	if (object_was_option) {
		/* NO OBJECT MATCHED, BUT AN OBJECT WAS VALID AT THIS POINT IN THE
		 * PLAYER'S COMMAND */
		diagnose();
		custom_error = TRUE;
	}

	return (nullptr);
}

struct word_type *exact_match(struct word_type *pointer) {
	/* THIS FUNCTION LOOPS THROUGH ALL THE POSIBILITIES IN THE CURRENT LEVEL
	 * OF THE GRAMMAR TREE TO SEE IF THERE ARE ANY EXACT MATCHES WITH THE
	 * CURRENT WORD IN THE PLAYER'S COMMAND.
	 * AN EXACT MATCH IS ANYTHING THAT ISN'T AN OBJECT PLACE HOLDER. */
	struct word_type *iterator = pointer;

	do {
		if (iterator->word[0] == '*') {
			/* THIS IS AN OBJECT PLACE HOLDER, THEREFORE IGNORE AND
			 * KEEP LOOKING FOR EXACT MATCHES */
		} else if (!strcmp(iterator->word, "$string")) {
			add_cstring("$string", word[wp]);
			last_exact = wp;
			wp++;
			return (iterator);
		} else if (!strcmp(iterator->word, "$integer") &&
		           validate(word[wp])) {
			add_cinteger("$integer", atoi(word[wp]));
			last_exact = wp;
			wp++;
			return (iterator);
		} else if (!strcmp(word[wp], iterator->word)) {
			last_exact = wp;
			wp++;
			return (iterator);
		}
	} while ((iterator = iterator->next_sibling) != nullptr);

	/* THERE WERE NO EXACT MATCHES, SO RETURN FALSE */
	return (nullptr);
}

int is_terminator(struct word_type *scope_word) {
	struct word_type *terminator = scope_word->first_child;

	if (terminator != nullptr) {
		/* THERE MAY NO BE ANY MORE POSSIBLE WORDS IN THIS COMMAND
		 * BUT THERE SHOULD ALWAYS AT LEAST BE A BASE FUNCTION NAME */
		do {
			/* LOOP THROUGH ALL WORDS IN THE NEXT LEVEL OF THE
			 * GRAMMAR TABLE. THESE ARE THE WORDS THAT MARK THE END
			 * OF THE OBJECT REFERENCE PART OF THE COMMAND */
			if (!strcmp(word[wp], terminator->word)
			        || (!strcmp(terminator->word, "$integer")
			            && validate(word[wp]))) {
				return (TRUE);
			}
		} while ((terminator = terminator->next_sibling) != nullptr);
	}

	return (FALSE);
}

int build_object_list(struct word_type *scope_word, int noun_number) {
	/* THIS FUNCTION BUILDS A LIST OF OBJECTS FROM THE PLAYER'S COMMAND
	 * AND RETURNS THE NUMBER OF OBJECTS IN THAT LIST */

	int             index, counter;
	int             resolved_object;
	const char      *except_word;

	//printf("--- entering build object list starting at %s with a scope_word of %s\n", word[wp], scope_word->word);
	/* LOOK AHEAD FOR A FROM CLAUSE AND STORE from_object IF SO */
	if (get_from_object(scope_word, noun_number) == FALSE) {
		/* THERE WAS AN ERROR, AND A MESSAGE HAS ALREADY BEEN
		 * DISPLAYED */
		//printf("--- from had an error\n");
		return (FALSE);
	}

	while (word[wp] != nullptr) {
		/* LOOP THROUGH WORDS IN THE PLAYER'S INPUT ENDING WHEN EITHER
		 * THERE ARE NO MORE WORDS OR ONE OF THE CHILD NODES OF THE
		 * CURRENT scope_word NODE IS REACHED INDICATING THERE ARE NO
		 * MORE OBJECTS TO RESOLVE */

		if (!strcmp(word[wp], cstring_resolve("BUT_WORD")->value) ||
		        !strcmp(word[wp], cstring_resolve("EXCEPT_WORD")->value)) {
			/* START ADDING ALL FUTURE RESOLVED OBJECTS TO A SECOND LIST
			 * TO REMOVE FROM THE FIRST */
			except_word = word[wp];

			wp++;

			if (word[wp] != nullptr && !strcmp(word[wp], cstring_resolve("FOR_WORD")->value)) {
				/* SKIP PAST THE WORD 'FOR' */
				wp++;
			}

			/* LOOK FORWARD FOR A FROM CLAUSE */
			if (get_from_object(scope_word, noun_number) == FALSE) {
				/* THERE WAS AN ERROR, AND A MESSAGE HAS ALREADY BEEN
				 * DISPLAYED */
				return (FALSE);
			}

			/* MOVE TO THE SECOND LIST THAT WILL ULTIMATELY BE SUBTRACTED
			 * FROM THE FIRST LIST */
			if (noun_number < 2) {
				/* CREATE A 'them' SET THAT CAN BE REFERRED TO IN THE
				 * 'except' CLAUSE */
				set_them(noun_number);
				/* JUMP TO THE 'EXCEPT' LIST THAT CORRESPONDS TO THIS
				 * RESOLVED LIST */
				noun_number = noun_number + 2;
			} else {
				Common::sprintf_s(error_buffer, 1024, cstring_resolve("DOUBLE_EXCEPT")->value, except_word);
				write_text(error_buffer);
				custom_error = TRUE;
				return (FALSE);
			}
		} else if (after_from != -1 && !strcmp(word[wp], cstring_resolve("FROM_WORD")->value)) {
			/* SET THE WORD POINTER TO AFTER THE ALREADY-PROCESSED FROM
			 * CLAUSE (IF ONE EXISTED) AND CONTINUE */
			wp = after_from;
			//printf("--- hit from in processing moving on to word '%s'\n", word[wp]);

			/* IF NO OBJECTS WERE MATCHED BY THE TIME WE HIT FROM THEN EITHER
			 * PRESUME THE PLAYER MEANT 'ALL FROM' OR PRINT AN ERROR */
			if (list_size[noun_number] == 0) {
				//printf("--- adding all due to empty list.\n");
				add_all(scope_word, noun_number);
			}

			/* LOOK FOR THE NEXT FROM CLAUSE */
			if (get_from_object(scope_word, noun_number) == FALSE) {
				/* THERE WAS AN ERROR, AND A MESSAGE HAS ALREADY BEEN
				 * DISPLAYED */
				return (FALSE);
			}
		} else if (!strcmp("then", word[wp])) {
			break;
		} else if (is_terminator(scope_word)) {
			/* THERE ARE NO MORE OBJECTS TO RESOLVE */

			//printf("--- %s is a build list terminator\n", word[wp]);
			break;
		} else if (!strcmp(word[wp], "comma") ||
		           !strcmp(word[wp], cstring_resolve("AND_WORD")->value)) {
			/* JUST MOVE ONTO THE NEXT WORD AND SEE WHAT COME NEXT */
			wp++;
		} else {
			/* CALL noun_resolve TO FETCH THE FIRST MATCHING OBJECT */
			/* FALSE INDICATES THAT WE ARE NOT LOOKING FOR A FROM OBJECT */
			resolved_object = noun_resolve(scope_word, FALSE, noun_number);

			if (resolved_object == -1) {
				/* THERE WERE MULTIPLE MATCHES DUE TO PLURAL NAME
				 * BEING USED */
				index = 0;

				while (multiple_resolved[index] != 0) {
					/* ADD ALL THE RESOLVED OBJECTS TO THE LIST */
					add_to_list(noun_number, multiple_resolved[index]);
					index++;
				}
			} else if (resolved_object) {
				/* ADD IT TO THE LIST */
				add_to_list(noun_number, resolved_object);
			} else {
				/* NO OBJECTS COULD BE RESOLVED, THIS MIGHT NOT BE A BAD
				 * THING YET IF THERE ARE OTHER OBJECT PLACEHOLDERS TO
				 * COME THAT ARE LESS RESTRICTIVE */
				return (FALSE);
			}
		}
	}

	if (noun_number > 1 && list_size[noun_number] != 0) {
		/* A SECOND EXCEPTION LIST EXISTS, SUBTRACT IT FROM THE FIRST */
		//printf("--- there are some exceptions.\n");

		//printf("--- first list: %d, second list: %d\n", max_size[FIRST_LIST], max_size[SECOND_LIST]);
		/* LOOP THROUGH ALL THE ITEMS IN THE SECOND LIST */
		for (index = 0; index < max_size[SECOND_LIST]; index++) {
			if (object_list[SECOND_LIST][index] != 0) {
				/* THIS OBJECT IS A REAL OBJECT SO LOOP THROUGH ALL THE ITEMS
				 * IN THE FIRST LIST */
				//printf("--- exception object is %s\n", object[object_list[SECOND_LIST][index]]->label);
				for (counter = 0; counter < max_size[FIRST_LIST]; counter++) {
					/* LOOP THROUGH ALL THE OBJECTS IN THE FIRST LIST
					 * IF AN OBJECT FROM THE SECOND LIST EXISTS IN THE FIRST
					 * LIST, REMOVE IT */
					//printf("--- comparing %s = %s\n", object[object_list[FIRST_LIST][counter]]->label, object[object_list[SECOND_LIST][index]]->label);
					if (object_list[FIRST_LIST][counter] ==
					        object_list[SECOND_LIST][index]) {

						//printf("--- removing object %s\n", object[object_list[FIRST_LIST][counter]]->label);
						object_list[FIRST_LIST][counter] = 0;
						list_size[FIRST_LIST]--;
					}
				}
			}
		}
	}

	if (noun_number > 1) {
		/* IF THERE WERE EXCEPTIONS, MOVE BACK TO THE FIRST LIST */
		noun_number = FIRST_LIST;
	}

	/* THE RETURN TRUE IF AN OBJECT COULD BE RESOLVED */
	if (list_size[noun_number] != 0) {
		/* SET THEM ARRAY */
		set_them(noun_number);

		return (TRUE);
	} else {
		/* THE LIST IS NOW EMPTY, DISPLAY ' I DON'T SEE WHAT YOU
		 * ARE REFERRING TO.' ERROR */
		if (!strcmp(scope_word->word, "*held") ||
		        !strcmp(scope_word->word, "**held")) {
			write_text(cstring_resolve("NONE_HELD")->value);
		} else {
			write_text(cstring_resolve("NO_OBJECTS")->value);
		}

		custom_error = TRUE;
		return (FALSE);
	}
}

void set_them(int noun_number) {
	int index, counter;

	if (list_size[noun_number] == 1) {
		/* THERE IS ONLY ONE OBJECT LEFT IN THE LIST, FIND IT AND ADD
		 * TO THEM IF REQUIRED */
		for (index = 0; index < max_size[noun_number]; index++) {
			if (object_list[noun_number][index] != 0) {
				if (object[object_list[noun_number][index]]->attributes & PLURAL) {
					them[0] = object_list[noun_number][index];
					them[1] = 0;
					break;
				}
			}
		}
	} else {
		/* THERE IS MORE THAN ONE OBJECT IN THE LIST SO COPY IT TO THE
		 * 'THEM' LIST */
		counter = 0;

		for (index = 0; index < max_size[noun_number]; index++) {
			if (object_list[noun_number][index] != 0) {
				them[counter] = object_list[noun_number][index];
				//printf("--- storing %s in them list\n", object[them[counter]]->label);
				counter++;
			}
		}

		/* NULL TERMINATE THE LIST */
		them[counter] = 0;
	}
}

void add_all(struct word_type *scope_word, int noun_number) {
	int index;

	//printf("--- trying to add all\n");

	for (index = 1; index <= objects; index++) {
		if ((object[index]->MASS < HEAVY) &&
		        !(object[index]->attributes & LOCATION)) {
			if (is_direct_child_of_from(index) &&
			        scope(index, scope_word->word, RESTRICT)) {
				//printf("--- objects parent is %s\n", object[object[index]->PARENT]->label);
				add_to_list(noun_number, index);
			}
		}
	}
}

int is_child_of_from(int child) {
	/* THIS FUNCTION DETERMINES IF THE PASSED OBJECT IS A CHILD OF ANY OF
	 * THE RESOLVED 'FROM' OBJECTS, OR ANY OBJECT IN A FROM OBJECT */
	int index = 0;

	if (from_objects[0] == 0) {
		/* THERE HAS BEEN NO FROM CLAUSE */
		return (TRUE);
	}

	while (from_objects[index] != 0) {
		//printf("--- in is_child from object is %s, child is %s\n", object[from_objects[index]]->label, object[child]->label);
		/* THIS OLD WAY OF DOING THINGS ALLOWS SPECIFIC 'take thing from box'
		 * WHEN thing IS INSIDE SOMETHING ELSE INSIDE box. THAT IS KINDA COOL
		 * BUT NOT PARTICULARLY NECESSARY. BY ONLY CHECKING IMMEDIATE CHILDREN
		 * THE NON-RESTRICTIVE VERSION OF parent_of CAN BE USED FROM SCOPE
		 * WHEN SAYING take all from box BECAUSE THIS FROM STOPS OBJECT IN
		 * OBJECTS FROM BEING TAKING FROM SOMETHING, SO scope DOESN'T HAVE
		 * TO. THIS BEHAVIOUR WOULD NOT BE NECESSARY WHEN NOT taking all from */

		//if (parent_of(from_objects[index], child, RESTRICT)) {
		if (object[child]->PARENT == from_objects[index]) {
			//printf("--- %s is in %s\n", object[child]->label, object[from_objects[index]]->label);
			return (TRUE);
		}
		index++;
	}

	return (FALSE);
}

int is_direct_child_of_from(int child) {
	/* THIS FUNCTION DETERMINES IF THE PASSED OBJECT IS A CHILD OF ANY OF
	 * THE RESOLVED 'FROM' OBJECTS, OR ANY OBJECT IN A FROM OBJECT */
	int index = 0;

	if (from_objects[0] == 0) {
		/* THERE HAS BEEN NO FROM CLAUSE */
		return (TRUE);
	}

	while (from_objects[index] != 0) {
		//printf("--- in is_direct from object is %s\n", object[from_objects[index]]->label);
		if (object[child]->PARENT == from_objects[index]) {
			//printf("--- object %s is in the from object\n", object[child]->label);
			return (TRUE);
		}
		index++;
	}

	return (FALSE);
}

int get_from_object(struct word_type *scope_word, int noun_number) {
	/* THIS FUNCTION LOOKS AHEAD TO FIND IF THE CURRENT OBJECT REFERENCE
	 * IS QUALIFIED BY A 'FROM' WORD. IT RETURNS FALSE ON AN ERROR
	 * CONDITION AND TRUE OTHERWISE, REGARDLESS OF WHETHER A FROM OBJECT
	 * IS SPECIFIED */

	int index, counter, from_object;

	/* TAKE A COPY OF THE CURRENT VALUE OF wp */
	int backup = wp;

	/* SET TERMINATOR TO THE FIRST OF THE TERMINATING WORDS */
	struct word_type *terminator = scope_word->first_child;

	/* SEE IF 'FROM' IS ONE OF THE TERMINATORS OF THIS CURRENT OBJECT
	 * PLACEHOLDER. IF SO, DON'T LOOK FOR A FROM OBJECT */
	if (terminator != nullptr) {
		//printf("--- checking if terminator word (%s) is from\n", terminator->word);
		if (!strcmp(cstring_resolve("FROM_WORD")->value, terminator->word)) {
			//printf("--- from is a terminator, don't get a from object\n");
			return (TRUE);
		}
		while ((terminator = terminator->next_sibling) != nullptr);
	}

	/* LOOP FROM THE CURRENT WORD TO THE NEXT TERMINATOR AND LOOK FOR THE
	 * WORD 'FROM' AND STORE THE FOLLOWING OBJECT */
	while (word[wp] != nullptr) {
		//printf("--- from loop checking %s\n", word[wp]);
		if (!strcmp(word[wp], cstring_resolve("FROM_WORD")->value)) {
			from_word = word[wp];
			wp++;

			/* scope_word FOR THE CURRENT OBJECT IS PASSED ONLY SO
			 * noun_resolve CAN FIND OUT THE APPROPRIATE TERMINATORS
			 * THE ACCEPTABLE SCOPE FOR THE FROM OBJECT SHOULD BE
			 * AT LEAST *present */
			/* TRUE INDICATES THAT WE ARE LOOKING FOR A FROM OBJECT */
			from_object = noun_resolve(scope_word, TRUE, noun_number);

			/* STORE THE wp FROM AFTER THE RESOLVED FROM OBJECT SO
			 * WE CAN JUMP FORWARD TO HERE AGAIN WHEN WE HIT THIS
			 * FROM CLAUSE IN PROCESSING (THIS FUNCTION IS A LOOK-AHEAD */
			after_from = wp;

			//printf("--- looked forward and found a from object of %s\n", object[from_object]->label);

			if (from_object == -1) {
				/* THERE WERE MULTIPLE MATCHES DUE TO PLURAL NAME
				 * BEING USED */
				index = 0;
				counter = 0;
				/* LOOK THROUGH ALL THE OBJECTS RESOLVED AND CHECK THAT
				 * THEY ARE ALL VALID FROM OBJECTS */
				while (multiple_resolved[index] != 0) {
					if (verify_from_object(multiple_resolved[index]) == FALSE) {
						/* AS SOON AS ONE IS BAD, ABORT THE FROM CLAUSE */
						return (FALSE);
					} else {
						from_objects[counter] = multiple_resolved[index];
						counter++;
					}
					index++;
				}

				/* NULL TERMINATE THE LIST */
				from_objects[counter] = 0;

				/* OBJECTS HAVE BEEN SET AND ARE VALID, RESTORE THE WORD
				 * POINTER */
				wp = backup;
				return (TRUE);
			} else if (from_object) {
				if (verify_from_object(from_object) == FALSE) {
					return (FALSE);
				} else {
					/* ADD THIS OBJECT TO THE NULL TERMINATED LIST */
					from_objects[0] = from_object;
					from_objects[1] = 0;

					/* OBJECT HAS BEEN SET AND IS VALID, RESTORE THE WORD
					* POINTER */
					wp = backup;
					return (TRUE);
				}
			} else {
				/* FROM UNKNOWN OBJECT, DISPLAY ERROR */
				diagnose();
				custom_error = TRUE;
				return (FALSE);
			}
			//printf("--- finished processing from clause, next word is %s\n", word[wp]);
		} else if (!strcmp(cstring_resolve("EXCEPT_WORD")->value, word[wp]) ||
		           !strcmp(cstring_resolve("BUT_WORD")->value, word[wp])) {
			/* THIS IS THE LIMIT OF THE EFFECT ANY FROM OBJECT ANYWAY,
			 * SO TREAT IT LIKE A TERMINATOR */
			//printf("--- %s is a get_from_object except word\n", word[wp]);
			wp = backup;
			return (TRUE);
		} else if (is_terminator(scope_word) || !strcmp("then", word[wp])) {
			/* THERE ARE NO MORE OBJECTS TO RESOLVE */

			//printf("--- %s is a get_from_object terminator\n", word[wp]);
			wp = backup;
			return (TRUE);
		}
		wp++;
	}

	/* HIT THE END OF THE PLAYER'S COMMAND BEFORE A 'FROM' WORD */
	//printf("--- no from clause specified in this block\n");
	wp = backup;
	return (TRUE);
}

int verify_from_object(int from_object) {
	//printf("--- from object is %s\n", object[from_object]->label);
	//if (!(object[from_object]->attributes & CONTAINER) &&
	//  !(object[from_object]->attributes & SURFACE) &&
	//  !(object[from_object]->attributes & ANIMATE)) {
	//  sprintf (error_buffer, FROM_NON_CONTAINER, from_word);
	//  write_text (error_buffer);
	//  custom_error = TRUE;
	//  return (FALSE);
	//} else if (object[from_object]->attributes & CLOSED) {
	if (object[from_object]->attributes & CONTAINER && object[from_object]->attributes & CLOSED) {
		//printf("--- container is concealing\n");
		if (object[from_object]->attributes & FEMALE) {
			Common::sprintf_s(error_buffer, 1024, cstring_resolve("CONTAINER_CLOSED_FEM")->value, sentence_output(from_object, TRUE));
		} else {
			Common::sprintf_s(error_buffer, 1024, cstring_resolve("CONTAINER_CLOSED")->value, sentence_output(from_object, TRUE));
		}
		write_text(error_buffer);
		custom_error = TRUE;
		return (FALSE);
		/* IF THE PERSON IS CONCEALING, THEN THE OBJECT CAN'T BE REFERRED TO
		 * IF THE PERSON IS POSSESSIVE LET THE LIBRARY HANDLE THE RESPONSE
		} else if (object[from_object]->attributes & POSSESSIVE) {
		    //printf("--- container is closed\n");
		    sprintf (error_buffer, PERSON_POSSESSIVE, sentence_output(from_object, TRUE));
		    write_text(error_buffer);
		    custom_error = TRUE;
		    return (FALSE);
		} else if (object[from_object]->attributes & CONCEALING) {
		    //printf("--- container is closed\n");
		    sprintf (error_buffer, PERSON_CONCEALING, sentence_output(from_object, TRUE));
		    write_text(error_buffer);
		    custom_error = TRUE;
		    return (FALSE);*/
	}

	//printf("--- set from object just fine\n");
	return (TRUE);
}

void add_to_list(int noun_number, int resolved_object) {
	/* ADD THIS OBJECT TO THE OBJECT LIST DEPENDING */
	/* AND SET IT, THEM, HER AND HIM */
	if (!(object[resolved_object]->attributes & ANIMATE))
		it = resolved_object;
	if (object[resolved_object]->attributes & ANIMATE
	        && object[resolved_object]->attributes & FEMALE)
		her = resolved_object;
	if (object[resolved_object]->attributes & ANIMATE
	        && !(object[resolved_object]->attributes & FEMALE))
		him = resolved_object;

	//printf("--- adding_object %s to list %d at index %d\n", object[resolved_object]->label, noun_number, max_size[noun_number]);
	object_list[noun_number][max_size[noun_number]] = resolved_object;
	list_size[noun_number]++;
	max_size[noun_number]++;
}

int noun_resolve(struct word_type *scope_word, int finding_from, int noun_number) {
	/* THIS FUNCTION STARTS LOOKING AT THE PLAYER'S COMMAND FROM wp ONWARDS
	 * AND LOOKS FOR OBJECTS IN THE SCOPE SPECIFIED BY THE GRAMMAR ELEMENT
	 * POINTED TO BY THE PASSED pointer */

	/* THIS IS SET TO TRUE WHEN THE CURRENT WORD MATCHES THE CURRENT OBJECT */
	int             object_matched;

	/* THIS IS SET TO > 0 WHEN THE PLURAL FORM OF AN OBJECT IS USED */
	int             return_limit = 0;

	int             index;
	int             counter;
	int             first_word = TRUE;

	struct word_type *terminator = scope_word->first_child;
	struct name_type *current_name;

	matches = 0;
	highest_confidence = 0;
	prime_suspect = 0;
	done = FALSE;
	backup_pointer = wp;
	everything = FALSE;

	if (word[wp] == nullptr) {
		/* NOTHING TO RESOLVE... */
		return (FALSE);
	}

	/* SET THE CONFIDENCE FOR EACH OBJECT TO 1 TO INDICATE THAT THEY ARE
	 * ALL EQUALLY POSSIBLE AT THE MOMENT. SET TO ZERO TO DISCOUNT */
	for (index = 1; index <= objects; index++)
		confidence[index] = 1;

	/* CLEAR THE OBJECT NAME BEFORE BUILDING IT WAS WE GO ALONG */
	object_name[0] = 0;

	/* CHECK FOR A QUANTITY QUALIFIER */
	if (validate(word[wp])) {
		/* FIRST WORD IS AN INTEGER AND SECOND WORD IS 'OF' SO
		 * TREAT THIS AS A LIMIT QUALIFIER BEFORE STARTING TO
		 * PROCESS THE REST OF THE WORDS */
		if (word[wp + 1] != nullptr && !strcmp(word[wp + 1], cstring_resolve("OF_WORD")->value)) {
			return_limit = atoi(word[wp]);

			/* MAKE SURE THE RETURN LIMIT IS SOMETHING SENSIBLE */
			if (return_limit < 1) {
				return_limit = 1;
			}

			object_expected = TRUE;
			Common::strcpy_s(object_name, word[wp]);
			Common::strcat_s(object_name, " ");
			Common::strcat_s(object_name, cstring_resolve("OF_WORD")->value);

			/* MOVE THE WORD POINTER TO AFTER THE 'OF' */
			wp = wp + 2;
		}
		/* IF AN INTEGER IS NOT FOLLOWED BY 'OF', PRESUME IT IS AN OBJECT
		 * NAME */
	}

	// CLEAR THE ERROR BUFFER AND USE IT TO STORE ALL THE WORDS THE PLAYER
	// HAS USED TO REFER TO THE OBJECT
	error_buffer[0] = 0;

	while (word[wp] != nullptr) {
		// ADD THE WORDS USED TO error_buffer FOR POSSIBLE USE
		// IN A DISABMIGUATE EMESSAGE
		if (first_word == FALSE) {
			Common::strcat_s(error_buffer, 1024, " ");
		}
		Common::strcat_s(error_buffer, 1024, word[wp]);
		first_word = FALSE;

		/* LOOP THROUGH WORDS IN THE PLAYER'S INPUT */

		/* RESET TERMINATOR TO THE FIRST OF THE TERMINATING WORDS */
		terminator = scope_word->first_child;

		if (terminator != nullptr) {
			/* THERE MAY NO BE ANY MORE POSSIBLE WORDS IN THIS COMMAND
			 * BUT THERE SHOULD ALWAYS AT LEAST BE A BASE FUNCTION NAME */
			do {
				/* LOOP THROUGH ALL WORDS IN THE NEXT LEVEL OF THE
				 * GRAMMAR TABLE. THESE ARE THE WORDS THAT MARK THE END
				 * OF THE OBJECT REFERENCE PART OF THE COMMAND */
				//printf("--- checking terminator word %s\n", terminator->word);
				if (!strcmp(word[wp], terminator->word)
				        || (!strcmp(word[wp], cstring_resolve("FROM_WORD")->value))
				        || (!strcmp(word[wp], cstring_resolve("AND_WORD")->value))
				        || (!strcmp(word[wp], "comma"))
				        || (!strcmp(word[wp], cstring_resolve("BUT_WORD")->value))
				        || (!strcmp(word[wp], cstring_resolve("THEN_WORD")->value))
				        || (!strcmp(word[wp], cstring_resolve("EXCEPT_WORD")->value))
				        || (!strcmp(terminator->word, "$integer")
				            && validate(word[wp]))) {
					if (!matches) {
						/* A TERMINATOR HAS BEEN FOUND BEFORE A
						 * SINGLE MATCHING OBJECT NAME. */
						return (FALSE);
					} else {
						/* A TERMINATOR HAS BEEN FOUND, BUT NOT
						 * BEFORE MATCHING OBJECT NAMES. JUMP FORWARD TO
						 * RESOLVING INTO OBJECT NUMBER(S) */
						done = TRUE;
						break;
					}
				}
			} while ((terminator = terminator->next_sibling) != nullptr);
		}

		if (done == TRUE) {
			/* A TERMINATING WORD HAS BEEN REACHED, SO DON'T TEST THIS
			 * WORD AGAINST THE OBJECT NAMES, JUST MOVE ON TO CHOSING
			 * AN OBJECT. */
			//printf("--- %s is a terminator\n", word[wp]);
			break;
		}

		//puts("--- passed checking for a terminator");

		/* ADD THE CURRENT WORD TO THE NAME OF THE OBJECT THE PLAYER
		 * IS TRYING TO REFER TO FOR USE IN AN ERROR MESSAGE IF
		 * LATER REQUIRED */
		if (object_name[0] != 0)
			Common::strcat_s(object_name, " ");

		Common::strcat_s(object_name, word[wp]);

		if (!strcmp("everything", word[wp])) {
			/* ALL THIS NEEDS TO SIGNIFY IS THAT IT IS OKAY TO RETURN MULTIPLE
			 * RESULTS. EVERYTHING ELSE SHOULD TAKE CARE OF ITSELF GIVEN ALL
			 * OBJECTS START OF WITH A CONFIDENCE OF 1, AND ALL OBJECTS WITH
			 * A CONFIDENCE > 0 ARE RETURNED WHEN return_limit > 1 */
			if (return_limit == FALSE) {
				return_limit = MAX_OBJECTS;
			}

			/* THIS IS USED TO ALTER THE BEHAVIOUR OF SCOPE SELECTION LATER */
			everything = TRUE;

			matches = 0;

			//printf("--- entering for loop\n");
			for (index = 1; index <= objects; index++) {
				if (confidence[index] != FALSE) {
					matches++;
				}
			}
			//printf("--- exiting for loop\n");

			if (word[wp + 1] != nullptr && !strcmp(cstring_resolve("OF_WORD")->value, word[wp + 1])) {
				/* MOVE PAST THE 'OF' IF IT IS NEXT */
				wp++;
			}
			//printf("--- finished setting matches to %d for all\n", matches);
		} else {
			/* THE CURRENT WORD IS NOT ONE OF THE NEXT POSSIBLE WORDS IN THE
			 * PLAYER'S COMMAND OR ANY OF THE SPECIAL MEANING WORDS, THEREFORE
			 * TEST IT AGAINST ALL OF THE NAMES OF ALL OF THE OBJECTS. */
			for (index = 1; index <= objects; index++) {
				if (!confidence[index]) {
					/* SKIP OBJECTS THAT HAVE ALREADY
					 * BEEN EXCLUDED BY A PREVIOUS WORD */
					continue;
				}

				/* NEXT WORD IN PLAYERS INPUT IS YET TO
				 * BE TESTED AGAINST ALL THIS OBJECT'S NAMES */
				object_matched = FALSE;

				if (!strcmp(cstring_resolve("IT_WORD")->value, word[wp]) ||
				        !strcmp(cstring_resolve("ITSELF_WORD")->value, word[wp])) {
					if (it == FALSE) {
						no_it();
						return (FALSE);
					} else {
						if (index == it) {
							object_matched = TRUE;
						}
					}
				} else if (!strcmp(cstring_resolve("HER_WORD")->value, word[wp]) ||
				           !strcmp(cstring_resolve("HERSELF_WORD")->value, word[wp])) {
					if (her == FALSE) {
						no_it();
						return (FALSE);
					} else {
						if (index == her) {
							object_matched = TRUE;
						}
					}
				} else if (!strcmp(cstring_resolve("HIM_WORD")->value, word[wp]) ||
				           !strcmp(cstring_resolve("HIMSELF_WORD")->value, word[wp])) {
					if (him == FALSE) {
						no_it();
						return (FALSE);
					} else {
						if (index == him) {
							object_matched = TRUE;
						}
					}
				} else if (!strcmp(cstring_resolve("THEM_WORD")->value, word[wp]) ||
				           !strcmp(cstring_resolve("THEMSELVES_WORD")->value, word[wp]) ||
				           !strcmp(cstring_resolve("ONES_WORD")->value, word[wp])) {
					/* THIS NEED ONLY BE THE SIZE OF 'THEM', BUT NO HARM
					 * IN MAKING IT THE FULL SIZE */
					if (return_limit == FALSE) {
						return_limit = MAX_OBJECTS;
					}

					/* LOOP THROUGH ALL THE OBJECT IN THE 'THEM' ARRAY AND
					 * SEE IF THIS OBJECT IS PRESENT */
					counter = 0;

					while (them[counter] != 0) {
						if (them[counter] == index) {
							//printf("--- found previous them object %s\n", object[index]->label);
							object_matched = TRUE;
							break;
						}
						counter++;
					}
					/*} else if (!strcmp("1", word[wp])) {
					    return_limit = 1;

					    // LOOP THROUGH ALL THE OBJECT IN THE 'THEM' ARRAY AND
					    // SEE IF THIS OBJECT IS PRESENT
					    counter = 0;

					    while (them[counter] != 0) {
					        if (them[counter] == index) {
					            //printf("--- found previous them object %s\n", object[index]->label);
					            object_matched = TRUE;
					            break;
					        }
					        counter++;
					    }*/
				} else {
					current_name = object[index]->first_name;

					while (current_name != nullptr) {
						/* LOOP THROUGH ALL THE CURRENT OBJECTS NAMES */
						if (!strcmp(word[wp], current_name->name)) {
							/* CURRENT WORD MATCHES THE CURRENT NAME
							 *OF THE CURRENT OBJECT */
							//printf("--- %s is a name match of object %d\n", word[wp], index);
							object_matched = TRUE;

							/* MOVE ON TO NEXT OBJECT, THIS OBJECT SHOULD
							 * NOT HAVE THE SAME NAME TWICE */
							break;
						}
						current_name = current_name->next_name;
					}

					/* NOW LOOP THROUGH ALL THE OJBECTS PLURAL NAMES */
					current_name = object[index]->first_plural;

					while (current_name != nullptr) {
						/* LOOP THROUGH ALL THE CURRENT OBJECTS NAMES */
						if (!strcmp(word[wp], current_name->name)) {
							/* CURRENT WORD MATCHES THE CURRENT NAME
							 *OF THE CURRENT OBJECT */
							//printf("--- %s is a plural name match of object %d\n", word[wp], index);
							object_matched = TRUE;

							/* IT IS NOW OKAY FOR THIS FUNCTION TO RETURN MORE
							 * THAT ONE MATCHING OBJECT AS THE PLURAL FORM
							 * HAS BEEN USED. THIS IS INDICATED BY RETURNING
							 * -1 TO INDICATED THAT THE OBJECT LIST IS STORED
							 * IN A GLOBALLY ACCESSIBLE ARRAY */
							if (return_limit == FALSE) {
								return_limit = MAX_OBJECTS;
							}

							/* MOVE ON TO NEXT OBJECT, THIS OBJECT SHOULD
							 * NOT HAVE THE SAME NAME TWICE */
							break;
						}
						current_name = current_name->next_name;
					}
				}

				if (object_matched) {
					/* THE CURRENT WORD MATCHES ONE OF THE NAMES OF THE
					 * CURRENT OBJECT. */
					if (confidence[index] == 1) {
						/* OBJECT HAD NOT YET BEEN COUNTED AS MATCH SO INCREMENT
						 * THE NUMBER OF OBJECTS THAT MATCH SO FAR. */
						matches++;
					}
					if (confidence[index] != FALSE) {
						/* IF OBJECT HAS NOT BEEN EXCLUDED, INCREMENT THE
						 * NUMBER OF NAMES IT HAS MATCHING. */
						confidence[index]++;
					}
				} else {
					/* THE CURRENT WORD IS NOT ONE OF THE NAMES OF THE
					 * CURRENT OBJECT.
					 * IF THE OBJECT HAD PREVIOUSLY BEEN A CANDIDATE, DECREMENT
					 * THE NUMBER OF MATCHING OBJECTS. */
					if (confidence[index] > 1)
						matches--;

					/* AS THE CURRENT WORD DID NOT MATCH ANY OF THE NAMES OF
					 * THE THIS OBJECT, EXCLUDE IT FROM CONTENTION. */
					confidence[index] = FALSE;
				}
			} /* MOVE ON TO NEXT OBJECT FOR IN LOOP */
		}

		if (!matches) {
			/* IF THERE ARE NO REMAINING MATCHES DON'T MOVE ON TO THE NEXT
			 * WORD IN THE PLAYER'S INPUT. */
			//printf("--- %s isn't a name match for any object\n", word[wp]);

			/* THIS WORD IS A LIKELY BE INCORRECT AS IT DIDN'T MATCH
			 * ANY OBJECTS */
			if (oops_word == -1 && word[wp] != nullptr) {
				oops_word = wp;
			}

			break;
		}

		/* MOVE ON TO THE NEXT WORD IN THE PLAYER'S INPUT. */
		//printf("--- moving on to next word\n");
		wp++;
	}

	//printf("--- finished processing words\n");

	/***************************************************
	 * FINISHED LOOKING THROUGH THE PLAYER'S INPUT NOW *
	 * MOVE THE POINTER BACK FOR PARSER TO PROCESS     *
	 * THIS SHOULDN'T BE DONE FOR A WORD LIKE 'AND'    *
	 ***************************************************/

	if (return_limit == FALSE) {
		/* THE RETURN LIMIT WAS NEVER SET, SO TREAT THIS AS AN IMPLICIT 1 */
		return_limit = 1;
	}

	if (matches == 0) {
		/* THE PLAYER'S INPUT COULD NOT BE RESOLVED TO A SINGLE OBJECT
		 * BUT NO TERMINATING WORD WAS USED BEFORE A NON-TERMINATING
		 * WORD IN A PLACE WHERE AN OBJECT COULD BE SPECIFIED ---
		 * IN OTHER WORDS, PRESUME THE PLAYER WAS TRYING TO REFER TO
		 * AN OBJECT BUT SOMEHOW GOT IT WRONG */

		//printf("--- matches = 0\n");
		/* THIS VARIABLE IS USED TO CHANGE THE BEHAVIOR OF diagnose()
		 * SHOULD THIS COMMAND TURN OUT TO BE INVALID */
		object_expected = TRUE;

		wp = backup_pointer;    /* BACK UP THE CURRENT WORD POINTER.
								 * SO THE PARSER CAN INVESTIAGE THE
								 * POSIBILITY THAT THIS ISN'T A
								 * REFERENCE TO AN OBJECT AT ALL */

		return (FALSE);         /* RETURN TO PARSER WITH NO MATCHING
								 * OBJECT HAVING BEEN FOUND. */
	}

	//printf("--- starting with %d matches\n", matches);

	/* LOOP THROUGH ALL OBJECTS AND REMOVE FROM ANY OBJECT THAT IS NOT IN THE
	 * CURRENT LOCATION, PROVIDED THE VERB DOESN'T ALLOW THE OBJECT TO
	 * BE *ANYWHERE. */
	for (index = 1; index <= objects; index++) {
		if (confidence[index] != FALSE && strcmp(scope_word->word, "*anywhere") && strcmp(scope_word->word, "**anywhere") && strcmp(scope_word->word, "*location")) {
			if (scope(index, "*present", UNRESTRICT) == FALSE) {
				matches--;
				confidence[index] = FALSE;
				//printf("--- removing %s for not being present\n", object[index]->label);
			}
		}

		/* IF THE OBJECT IS IMPLICITLY INCLUDED DUE TO A MULTIPLE RETURN
		 * LIMIT, REMOVE IT IF IT IS PART OF THE SCENERY */
		if (confidence[index] == 1 && return_limit > 1) {
			if ((object[index]->MASS >= HEAVY) ||
			        (object[index]->attributes & LOCATION)) {
				matches--;
				confidence[index] = 0;
				//printf("--- removing %s for being scenery\n", object[index]->label);
			}
		}

		/* FOR ALL CONTENDERS, CALCULATE THE CONFIDENCE AS A PERCENTAGE,
		 * SO LONG AS WE ARE STILL LOOKING FOR A SINGLE OBJECT */
		if (confidence[index] != FALSE && return_limit == 1) {
			current_name = object[index]->first_name;
			counter = 0;
			while (current_name != nullptr) {
				counter++;
				current_name = current_name->next_name;
			}
			confidence[index] = ((confidence[index] - 1) * 100) / counter;
		}
	}

	// CALCULATE THE HIGHEST CONFIDENCE OF ALL THE POSSIBLE OBJECTS
	highest_confidence = 0;

	for (index = 1; index <= objects; index++) {
		if (confidence[index] > highest_confidence) {
			highest_confidence = confidence[index];
		}
	}

	/* REMOVE ANY OBJECTS THAT ARE NOT IN THEIR VERB'S SPECIFIED SCOPE */
	for (index = 1; index <= objects; index++) {
		if (confidence[index] != FALSE) {
			if (scope(index, "*present", UNRESTRICT) != FALSE) {
				// TAKE SPECIAL NOT OF AN OBJECT THAT IS PRESENT
				// AND AT LEAST EQUAL FOR THE HIGHEST CONFIDENCE
				// IN CASE NO OBJECT ARE LEFT AFTER SPECIFIED SCOPE
				// IS USED TO FILTER THE LIST */
				if (confidence[index] == highest_confidence) {
					prime_suspect = index;
				}
				//printf("--- storing %s as prime_suspect\n", object[index]->label);
			}

			if (finding_from) {
				if (strcmp(scope_word->word, "*anywhere") && strcmp(scope_word->word, "**anywhere")) {
					if (scope(index, "*present") == FALSE) {
						matches--;
						confidence[index] = FALSE;
						continue;
					}
				}
			} else if (scope(index, scope_word->word, (everything && !from_objects[0])) == FALSE) {
				/* IF everything IS TRUE, scope IS RESTRICTED */
				//printf("--- removing %s due to regular scope\n", object[index]->label);
				matches--;
				confidence[index] = FALSE;
				continue;
			}

			/* CHECK IF THIS OBJECT IS IN ACCORDANCE WITH ANY FROM CLAUSE
			 * THAT MAY OR MAY NOT HAVE USED */
			if (is_child_of_from(index) == FALSE) {
				matches--;
				confidence[index] = FALSE;
				//printf("--- removing %s due to from clause\n", object[index]->label);
			}
		}
	}

	//printf("--- there are %d matches left\n", matches);

	/* THIS LOOP REMOVES ANY OBJECT THAT ARE NOT EQUAL TO THE HIGHEST
	 * CONFIDENCE UNLESS A PLURAL NAME WAS USED. IN THAT CASE, ONLY
	 * EXCLUDE OBJECTS THAT DO NOT HAVE ONE OF THE NAMES SUPPLIED */
	if (matches > 1 && return_limit == 1) {
		// CALCULATE THE HIGHEST CONFIDENCE NOW THAT OBJECTS
		// NOT IN SCOPE HAVE BEEN REMOVED
		highest_confidence = 0;

		for (index = 1; index <= objects; index++) {
			if (confidence[index] > highest_confidence) {
				highest_confidence = confidence[index];
			}
		}

		//printf("--- removing lower confidence objects\n");
		for (index = 1; index <= objects; index++) {
			/* REMOVE ANY OBJECTS THAT ARE NOT EQUAL
			   TO THE HIGHEST CONFIDENCE. */
			if (confidence[index] != FALSE) {
				if (confidence[index] < highest_confidence) {
					//printf("--- removing %s due to confidence of %d being under %d\n", object[index]->label, confidence[index], highest_confidence);
					confidence[index] = FALSE;
					matches--;
				}
			}
		}
	}

	if (matches == 0) {
		/* IF THIS LEAVES NO OBJECT, RETURN THE OBJECT THAT WAS PRESENT
		 * AND SET THE APPROPRIATE POINTERS. */
		if (prime_suspect != FALSE) {
			return (prime_suspect);
		} else {
			object_expected = TRUE;
			wp = backup_pointer;
			return (FALSE);
		}
	}

	if (matches == 1) {
		/* IF ONLY ONE POSSIBILITY REMAINS, RETURN THIS OBJECT AND SET THE
		 * APPROPRIATE POINTERS. */
		//printf("--- only one object left\n");
		for (index = 1; index <= objects; index++) {
			if (confidence[index] != FALSE) {
				return (index);
			}
		}
	}

	if (return_limit > 1) {
		/* THE PLURAL NAME OF AN OBJECT WAS USED OR A QUANTITY QUALIFIER */

		counter = 0;

		//printf("--- return_limit == TRUE\n");

		for (index = 1; index <= objects; index++) {
			if (confidence[index] != FALSE) {
				/* ADD EACH OBJECT TO multiple_resolved UNTIL
				 * THE return_limit IS REACHED */
				multiple_resolved[counter] = index;
				return_limit--;
				//printf("--- adding %s to multiple_resolved\n", object[index]->label);
				counter++;

				if (return_limit == 0) {
					break;
				}
			}
		}

		/* NULL TERMINATE THE LIST */
		multiple_resolved[counter] = 0;

		//printf("--- returning multiple objects\n");
		/* RETURN -1 TO INDICATED THERE ARE MULTIPLE OBJECTS */
		return (-1);
	}

	/* AN AMBIGUOUS REFERENCE WAS MADE. ATTEMPT TO CALL ALL THE disambiguate
	 * FUNCTIONS TO SEE IF ANY OF THE OBJECT WANTS TO TAKE PREFERENCE IN
	 * THIS CIRCUMSTANCE */

	/*
	int situation = noun_number;

	if (finding_from) {
		situation += 4;
	}

	for (index = 1; index <= objects; index++) {
	    if (confidence[index] != FALSE) {
	        strcpy(function_name, "disambiguate");
	        strcat(function_name, "_");
	        strcat(function_name, object[index]->label);
	        strcat(function_name, "<");
	        sprintf(temp_buffer, "%d", situation);
	        strcat(function_name, temp_buffer);

	        // CALL THE DISAMBIGUATION FUNCTION ATTACHED TO EACH OF THE
	        // POSSIBLE OBJECTS
	        int return_code = execute (function_name);

	        if (return_code == 1) {
	            // THIS OBJECT CAN CLAIMED OWNERSHIP OF THIS COMMAND
	            return (index);
	        } else if (return_code == -1) {
	            // THIS OBJECT HAS REJECTED OWNERSHIP OF THIS COMMAND
	            confidence[index] = FALSE;
	            matches--;
	        }
	    }
	}
	*/

	// CHECK IF ALL THE OBJECTS WERE REJECTED
	if (matches == 0) {
		return (prime_suspect);
	}

	// CHECK IF ONLY ONE OBJECT NOW REMAINS
	if (matches == 1) {
		/* IF ONLY ONE POSSIBILITY REMAINS, RETURN THIS OBJECT AND SET THE
		 * APPROPRIATE POINTERS. */
		//printf("--- only one object left\n");
		for (index = 1; index <= objects; index++) {
			if (confidence[index] != FALSE) {
				return (index);
			}
		}
	}

#if defined GLK || defined __NDS__
	/* NO OBJECT HAS CLAIMED OWNERSHIP, PROMPT THE PLAYER TO SPECIFY
	 * WHICH ONE THEY REQUIRE. */
	counter = 1;
	write_text(cstring_resolve("BY")->value);
	write_text(error_buffer);
	write_text(cstring_resolve("REFERRING_TO")->value);
	for (index = 1; index <= objects; index++) {
		if (confidence[index] != FALSE) {
			possible_objects[counter] = index;
			Common::sprintf_s(text_buffer, 1024, "  [%d] ", counter);
			write_text(text_buffer);
			sentence_output(index, 0);
			write_text(temp_buffer);
			matches--;
			if (counter < 9)
				counter++;
			write_text("^");
		}
	}

	/* GET A NUMBER: don't insist, low = 1, high = counter */
	selection = get_number(FALSE, 1, counter - 1);

	if (selection == -1) {
		write_text(cstring_resolve("INVALID_SELECTION")->value);
		custom_error = TRUE;
		return (FALSE);
	} else {
		write_text("^");
		return (possible_objects[selection]);
	}
#else
	/* IF MORE THAT ONE OBJECT STILL REMAINS, PROMPT THE PLAYER TO SPECIFY
	 * WHICH ONE THEY REQUIRE. */
	write_text(cstring_resolve("WHEN_YOU_SAY")->value);
	write_text(error_buffer);
	write_text(cstring_resolve("MUST_SPECIFY")->value);
	for (index = 1; index <= objects; index++) {
		if (confidence[index] != FALSE) {
			sentence_output(index, 0);
			write_text(temp_buffer);
			matches--;
			if (matches == 0) {
				write_text(".");
				break;
			} else if (matches == 1) {
				write_text(cstring_resolve("OR_WORD")->value);
			} else
				write_text(", ");
		}
	}
	write_text("^");

	custom_error = TRUE;
	return (FALSE);
#endif
}

void diagnose() {
	if (custom_error) {
		TIME->value = FALSE;
		return;
	}
	if (word[wp] == nullptr)
		write_text(cstring_resolve("INCOMPLETE_SENTENCE")->value);
	else if (object_expected && wp != 0) {
		write_text(cstring_resolve("UNKNOWN_OBJECT")->value);
		write_text(object_name);
		write_text(cstring_resolve("UNKNOWN_OBJECT_END")->value);
	} else {
		write_text(cstring_resolve("CANT_USE_WORD")->value);
		write_text(word[wp]);
		write_text(cstring_resolve("IN_CONTEXT")->value);
	}
	TIME->value = FALSE;
}

int scope(int index, const char *expected, int restricted) {
	/* THIS FUNCTION DETERMINES IF THE SPECIFIED OBJECT IS IN THE SPECIFIED
	 * SCOPE - IT RETURNS TRUE IF SO, FALSE IF NOT. */

	int temp = 0;

	/* WHEN THE ARGUMENT restricted IS TRUE IT HAS A MORE LIMITED
	 * SENSE OF WHAT IS ACCEPTABLE */

	if (!strcmp(expected, "*held") || !strcmp(expected, "**held")) {
		if (object[index]->PARENT == HELD) {
			return (TRUE);
		} else if (object[index]->MASS >= HEAVY) {
			/* ALLOW AND OBJECT TO BE CONSIDERED HELD IF IT HAS A
			 * MASS OF HEAVY OR GREATER AND ITS DIRECT PARENT IS
			 * BEING HELD. IE, A LABEL ON A JAR CAN BE SHOWN BECAUSE
			 * THE JAR IS BEING HELD. */
			temp = object[index]->PARENT;
			if (temp > 0 && temp < objects) {
				if (object[temp]->PARENT == HELD) {
					return (TRUE);
				}
			}
			return (FALSE);
		} else {
			return (FALSE);
		}
	} else if (!strcmp(expected, "*location")) {
		if (object[index]->attributes & LOCATION) {
			return (TRUE);
		} else {
			return (FALSE);
		}
	} else if (!strcmp(expected, "*here") || !strcmp(expected, "**here")) {
		if (object[index]->PARENT == HERE || index == HERE) {
			/* THE OBJECT IN QUESTION IS IN THE PLAYER'S CURRENT LOCATION
			 * OR IS THE PLAYER'S CURRENT LOCATION. */
			return (TRUE);
		} else if (object[index]->PARENT == HELD) {
			/* IT IS ONLY A PROBLEM FOR THE OBJECT TO BE AN IMMEDIATE CHILD
			 * OF THE PLAYER. THE PLAYER CAN STILL TAKE AN OBJECT THAT IS IN
			 * SOMETHING THEY ARE CARRYING */
			return (FALSE);
		} else {
			/* IS THE OBJECT A CHILD OF THE CURRENT LOCATION SOMEHOW */
			return (parent_of(HERE, index, restricted));
		}
	} else if (!strcmp(expected, "*anywhere") || !strcmp(expected, "**anywhere")) {
		return (TRUE);
	} else if (!strcmp(expected, "*inside") || !strcmp(expected, "**inside")) {
		if (object_list[0][0] > 0 && object_list[0][0] < objects) {
			return (parent_of(object_list[0][0], index, restricted));
		} else {
			// THERE IS NO PREVIOUS OBJECT SO TREAT THIS LIKE A *here
			return (parent_of(HERE, index, restricted));
		}
	} else if (!strcmp(expected, "*present") || !strcmp(expected, "**present")) {
		if (index == HERE) {
			return (TRUE);
		} else {
			if (find_parent(index)) {
				return (TRUE);
			} else {
				return (FALSE);
			}
		}
	} else {
		unkscorun(expected);
		return (FALSE);
	}
}

int find_parent(int index) {
	/* THIS FUNCTION WILL SET THE GLOBAL VARIABLE parent TO
	 * THE OBJECT THAT IS AT THE TOP OF THE POSSESSION TREE.
	 * IT WILL RETURN TRUE IF THE OBJECT IS VISIBLE TO THE
	 * PLAYER */
	//printf("--- find parent of %s\n", object[index]->label);

	if (!(object[index]->attributes & LOCATION) &&
	        object[index]->PARENT != NOWHERE) {

		parent = object[index]->PARENT;
		//printf("--- parent is %s\n", object[parent]->label);

		if (index == parent) {
			/* THIS OBJECT HAS ITS PARENT SET TO ITSELF */
			Common::sprintf_s(error_buffer, 1024, SELF_REFERENCE, executing_function->name, object[index]->label);
			log_error(error_buffer, PLUS_STDOUT);
			return (FALSE);
		} else  if (!(object[parent]->attributes & LOCATION)
		            && ((object[parent]->attributes & CLOSED && object[parent]->attributes & CONTAINER)
		                || object[parent]->attributes & CONCEALING)) {
			//printf("--- %s is closed, so return FALSE\n", object[parent]->label);
			return (FALSE);
		} else if (parent == HERE || parent == HELD) {
			/* THE OBJECT IS THE PLAYER'S CURRENT LOCATION OR BEING HELD */
			return (TRUE);
		} else {
			if (object[parent]->attributes & LOCATION) {
				//printf("--- %s is a location, so dont recuse\n", object[parent]->label);
				return (FALSE);
			} else {
				//printf("--- %s isn't a location, so recuse\n", object[parent]->label);
				return (find_parent(parent));
			}
		}
	} else {
		if (index == HERE)
			/* THE OBJECT IS THE PLAYER'S CURRENT LOCATION. */
			return (TRUE);
		else
			return (FALSE);
	}
}

int parent_of(int parent_, int child, int restricted) {
	/* THIS FUNCTION WILL CLIMB THE OBJECT TREE STARTING AT 'CHILD' UNTIL
	 * 'PARENT' IS REACHED (RETURN TRUE), OR THE TOP OF THE TREE OR A CLOSED
	 * OR CONCEALING OBJECT IS REACHED (RETURN FALSE). */

	/* restricted ARGUMENT TELLS FUNCTION TO IGNORE OBJECT IF IT IS IN AN
	 * OBJECT WITH A mass OF heavy OR LESS THAT IS NOT THE SUPPLIED
	 * PARENT ie. DON'T ACCEPT OBJECTS IN SUB OBJECTS */

	int             index;

	//printf("--- parent_ is %s, child is %s\n", object[parent_]->label, object[child]->label);
	if (child == parent_) {
		return (TRUE);
	} else if (!(object[child]->attributes & LOCATION) &&
	           object[child]->PARENT != NOWHERE) {
		/* STORE THE CHILDS PARENT OBJECT */
		index = object[child]->PARENT;
		//printf("--- %s is the parent_ of %s\n", object[index]->label, object[child]->label);

		if (index == child) {
			/* THIS CHILD HAS IT'S PARENT SET TO ITSELF */
			Common::sprintf_s(error_buffer, 1024, SELF_REFERENCE, executing_function->name, object[index]->label);
			log_error(error_buffer, PLUS_STDOUT);
			//printf("--- self parent_.\n");
			return (FALSE);
		} else  if (!(object[index]->attributes & LOCATION)
		            && ((object[index]->attributes & CLOSED && object[index]->attributes & CONTAINER)
		                || object[index]->attributes & CONCEALING)) {
			/* THE CHILDS PARENT IS CLOSED OR CONCEALING - CAN'T BE SEEN */
			//printf("--- parent_ %s is closed\n", object[index]->label);
			return (FALSE);
		} else if (restricted && object[index]->MASS < HEAVY && index != parent_) {
			//printf("--- scenery object.\n");
			return (FALSE);
		} else {
			//printf("--- comparing %s with %s\n", object[index]->label, object[parent_]->label);
			if (index == parent_) {
				/* YES, IS PARENT OF CHILD */
				return (TRUE);
			} else if (object[index]->attributes & LOCATION) {
				return (FALSE);
			} else {
				/* KEEP LOOKING UP THE TREE TILL THE CHILD HAS NO MORE
				 * PARENTS */
				return (parent_of(parent_, index, restricted));
			}
		}
	} else {
		/* THE SPECIFIED OBJECT HAS NO PARENT */
		return (FALSE);
	}
}

} // End of namespace JACL
} // End of namespace Glk
