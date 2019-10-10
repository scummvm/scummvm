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

#include "glk/jacl/jacl.h"
#include "glk/jacl/language.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"

namespace Glk {
namespace JACL {

#ifdef GLK
extern uint                     status_width, status_height;
extern winid_t                  statuswin;
#endif

#ifdef __NDS__
extern int                      screen_width;
extern int                      screen_depth;
#endif

extern struct object_type       *object[];
extern struct integer_type      *integer_table;
extern struct integer_type      *integer[];
extern struct cinteger_type     *cinteger_table;
extern struct attribute_type    *attribute_table;
extern struct string_type       *string_table;
extern struct string_type       *cstring_table;
extern struct function_type     *function_table;
extern struct function_type     *executing_function;
extern struct command_type      *completion_list;
extern struct word_type         *grammar_table;
extern struct synonym_type      *synonym_table;
extern struct filter_type       *filter_table;

extern char                     function_name[];
extern char                     temp_buffer[];
extern char                     error_buffer[];
extern char                     integer_buffer[16];

#ifndef GLK
#ifndef __NDS__
extern char                     game_url[];
extern char                     user_id[];
#endif
#endif

extern int                      noun[];
extern int                      quoted[];
extern int                      percented[];
extern const char               *word[];

extern int                      resolved_attribute;

extern int                      objects;
extern int                      integers;
extern int                      player;
extern int                      oec;
extern int                      *object_element_address;
extern int                      *object_backup_address;

extern int                      value_resolved;

char                            macro_function[84];
int                             value_has_been_resolved;

int *container_resolve(const char *container_name) {
	container_name = arg_text_of(container_name);

	/* IN JACL, A 'CONTAINER' IS ANYTHING THAT CAN STORE AN INTEGER */
	struct integer_type *resolved_integer;

	if ((resolved_integer = integer_resolve(container_name)) != NULL)
		return (&resolved_integer->value);
	else if (object_element_resolve(container_name))
		return (object_element_address);
	else if (!strcmp(container_name, "noun1"))
		return (&noun[0]);
	else if (!strcmp(container_name, "noun2"))
		return (&noun[1]);
	else if (!strcmp(container_name, "noun3"))
		return (&noun[2]);
	else if (!strcmp(container_name, "noun4"))
		return (&noun[3]);
	else if (!strcmp(container_name, "player"))
		return (&player);
	else if (!strcmp(container_name, "here"))
		return (&object[player]->PARENT);
	else
		return ((int *) NULL);
}

const char *var_text_of_word(int wordnumber) {
	const char *value;

	if (percented[wordnumber] == FALSE) {
		return (word[wordnumber]);
	} else {
		value_has_been_resolved = TRUE;
		value = arg_text_of(word[wordnumber]);
		while (value_has_been_resolved && percented[wordnumber]) {
			value = arg_text_of(value);
			percented[wordnumber]--;
		}

		return (value);
	}
}

const char *arg_text_of_word(int wordnumber) {
	const char *value;

	if (quoted[wordnumber] == 1) {
		return (word[wordnumber]);
	} else {
		value_has_been_resolved = TRUE;
		value = arg_text_of(word[wordnumber]);
		while (value_has_been_resolved && percented[wordnumber]) {
			value = arg_text_of(value);
			percented[wordnumber]--;
		}

		return (value);
	}
}

const char *text_of_word(int wordnumber) {
	const char *value;

	if (quoted[wordnumber] == 1) {
		return (word[wordnumber]);
	} else {
		value_has_been_resolved = TRUE;
		value = text_of(word[wordnumber]);
		while (value_has_been_resolved && percented[wordnumber]) {
			value = text_of(value);
			percented[wordnumber]--;
		}

		return (value);
	}
}

const char *text_of(const char *string) {
	struct integer_type *resolved_integer;
	struct cinteger_type *resolved_cinteger;
	struct string_type *resolved_string;
	struct string_type *resolved_cstring;
	char            *return_string;

	int             index;

	/* CHECK IF THE SUPPLIED STRING IS THE NAME OF A STRING CONSTANT,
	 * IF NOT, RETURN THE STRING LITERAL */
	if ((return_string = macro_resolve(string)) != NULL) {
		value_has_been_resolved = FALSE;
		return (return_string);
	} else if ((resolved_integer = integer_resolve(string)) != NULL) {
		value_has_been_resolved = FALSE;
		integer_buffer[0] = 0;
		sprintf(integer_buffer, "%d", resolved_integer->value);
		return (integer_buffer);
	} else if ((resolved_cinteger = cinteger_resolve(string)) != NULL) {
		value_has_been_resolved = FALSE;
		integer_buffer[0] = 0;
		sprintf(integer_buffer, "%d", resolved_cinteger->value);
		return (integer_buffer);
	} else if (object_element_resolve(string)) {
		value_has_been_resolved = FALSE;
		integer_buffer[0] = 0;
		sprintf(integer_buffer, "%d", oec);
		return (integer_buffer);
	} else if ((index = object_resolve(string)) != -1) {
		value_has_been_resolved = FALSE;
		if (index < 1 || index > objects) {
			badptrrun(string, index);
			return ("");
		} else {
			return (object[index]->label);
		}
	} else if ((resolved_string = string_resolve(string)) != NULL) {
		return (resolved_string->value);
	} else if ((resolved_cstring = cstring_resolve(string)) != NULL) {
		return (resolved_cstring->value);
	} else if (function_resolve(string) != NULL) {
		value_has_been_resolved = FALSE;
		sprintf(integer_buffer, "%d", execute(string));
		return (integer_buffer);
#ifndef GLK
#ifndef __NDS__
	} else if (!strcmp(string, "$url")) {
		value_has_been_resolved = FALSE;
		return (game_url);
	} else if (!strcmp(string, "$user_id")) {
		value_has_been_resolved = FALSE;
		return (user_id);
#endif
#endif
	} else {
		value_has_been_resolved = FALSE;
		return (string);
	}
}

const char *arg_text_of(const char *string) {
	struct string_type *resolved_string;
	struct string_type *resolved_cstring;
	char               *macro_text;

	/* CHECK IF THE SUPPLIED STRING IS THE NAME OF A STRING CONSTANT,
	 * IF NOT, RETURN THE STRING LITERAL */
	if ((macro_text = macro_resolve(string)) != NULL) {
		value_has_been_resolved = FALSE;
		return (macro_text);
	} else if ((resolved_string = string_resolve(string)) != NULL) {
		return (resolved_string->value);
	} else if ((resolved_cstring = cstring_resolve(string)) != NULL) {
		value_has_been_resolved = FALSE;
		return (resolved_cstring->value);
	} else {
		value_has_been_resolved = FALSE;
		return (string);
	}
}

int validate(const char *string) {
	int             index,
	                count;

	if (string == NULL) {
		return (FALSE);
	}

	/* CHECK IF THE SUPPLIED STRING IS A VALID INTEGER */
	count = strlen(string);

	/* LOOP OVER THE WHOLE STRING MAKING SURE THAT EACH CHARACTER IS EITHER
	 * A DIGIT OR A MINUS SIGN */
	for (index = 0; index < count; index++) {
		if (!Common::isDigit((int) * (string + index)) && string[index] != '-') {
			//printf ("'%c' is not a digit\n", *(string + index));
			return (FALSE);
		}
	}

	return (TRUE);
}

long value_of(const char *value, int run_time) {
	long            compare;

	value_resolved = TRUE;

	value = arg_text_of(value);

	/* RETURN THE INTEGER VALUE OF A STRING */
	struct integer_type *resolved_integer;
	struct cinteger_type *resolved_cinteger;

	if (!strcmp(value, "**held")) {
		return (FALSE);
	} else if (!strcmp(value, "**here")) {
		return (FALSE);
	} else if (!strcmp(value, "**anywhere")) {
		return (FALSE);
	} else if (!strcmp(value, "**present")) {
		return (FALSE);
	} else if (!strcmp(value, "*held")) {
		return (FALSE);
	} else if (!strcmp(value, "*here")) {
		return (FALSE);
	} else if (!strcmp(value, "*anywhere")) {
		return (FALSE);
	} else if (!strcmp(value, "*present")) {
		return (FALSE);
	} else if (!strcmp(value, "random")) {
		return random_number();
#ifdef GLK
	} else if (!strcmp(value, "status_height")) {
		g_vm->glk_window_get_size(statuswin, &status_width, &status_height);
		return status_height;
	} else if (!strcmp(value, "status_width")) {
		g_vm->glk_window_get_size(statuswin, &status_width, &status_height);
		return status_width;
#else
#ifdef __NDS__
	} else if (!strcmp(value, "status_height")) {
		return screen_depth;
	} else if (!strcmp(value, "status_width")) {
		return screen_width;
#else
	} else if (!strcmp(value, "status_height")) {
		value_resolved = FALSE;
		return -1;
	} else if (!strcmp(value, "status_width")) {
		value_resolved = FALSE;
		return -1;
#endif
#endif
	} else if (!strcmp(value, "unixtime")) {
		return g_system->getMillis() / 1000;
	} else if (validate(value)) {
		return (atoi(value));
	} else if ((resolved_cinteger = cinteger_resolve(value)) != NULL) {
		return (resolved_cinteger->value);
	} else if ((resolved_integer = integer_resolve(value)) != NULL) {
		return (resolved_integer->value);
	} else if (function_resolve(value) != NULL) {
		return (execute(value));
	} else if (object_element_resolve(value)) {
		return (oec);
	} else if ((compare = attribute_resolve(value))) {
		resolved_attribute = SYSTEM_ATTRIBUTE;
		return (compare);
	} else if ((compare = user_attribute_resolve(value))) {
		resolved_attribute = USER_ATTRIBUTE;
		return (compare);
	} else if ((compare = object_resolve(value)) != -1) {
		return (compare);
	} else if (*value == '@') {
		return (count_resolve(value));
	} else {
		if (run_time) {
			unkvarrun(value);
		}
		value_resolved = FALSE;
		return (-1);
	}
}

struct integer_type *integer_resolve(const char *name) {
	int             index,
	                iterator,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	strncpy(expression, name, 80);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '[') {
			/* THIS MAY STILL BE AN OBJECT ELEMENT IF A CLOSING ] */
			/* IS FOUND BEFORE AN OPENING ( */
			expression[index] = 0;
			delimiter = index + 1;
			/* LOOK FOR THE CLOSING ], BUT IF YOU FIND A ( FIRST */
			/* THEN THIS EXPRESSION IS NOT AN ARRAY */
			for (iterator = counter; iterator > 0; iterator--) {
				if (expression[iterator] == ']') {
					expression[iterator] = 0;
					break;
				} else if (expression[iterator] == '(') {
					/* NOT A VARIABLE ARRAY */
					return (FALSE);
				}
			}
			break;
		} else if (expression[index] == '<') {
			/* HIT A < BEFORE A [ THEREFORE */
			/* IS A FUNCTION CALL, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == '(') {
			/* HIT A ( BEFORE A [ THEREFORE */
			/* IS AN OBJECT ELEMENT, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == ' ')
			return (NULL);
	}

	// NO DELIMITER FOUND, TRY AS UNINDEXED VARIABLE
	if (delimiter == 0) {
		return (integer_resolve_indexed(name, 0));
	}

	// NO STRING BEFORE DELIMITER
	if (delimiter == 1) {
		return (NULL);
	}

	counter = value_of(&expression[delimiter], TRUE);

	if (counter > -1) {
		return (integer_resolve_indexed(expression, counter));
	} else {
		/* INDEX OUT OF RANGE */
		return (NULL);
	}
}

struct integer_type *integer_resolve_indexed(const char *name, int index) {
	struct integer_type *pointer = integer_table;

	if (pointer == NULL)
		return (NULL);

	do {
		if (!strcmp(name, pointer->name)) {
			if (index == 0) {
				return (pointer);
			} else {
				/* THIS VARIABLE DOES MATCH, BUT WERE NOT AT THE
				 * RIGHT INDEX YET SO MOVE ON */
				pointer = pointer->next_integer;
				index--;
			}
		} else
			pointer = pointer->next_integer;
	} while (pointer != NULL);

	/* IF index != 0, INDEX OUT OF RANGE, OTHERWISE NOT VARIABLE */
	return (NULL);
}

struct cinteger_type *cinteger_resolve(const char *name) {
	int             index,
	                iterator,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	strncpy(expression, name, 80);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '[') {
			/* THIS MAY STILL BE AN OBJECT ELEMENT IF A CLOSING ] */
			/* IS FOUND BEFORE AN OPENING ( */
			expression[index] = 0;
			delimiter = index + 1;
			/* LOOK FOR THE CLOSING ], BUT IF YOU FIND A ( FIRST */
			/* THEN THIS EXPRESSION IS NOT AN ARRAY */
			for (iterator = counter; iterator > 0; iterator--) {
				if (expression[iterator] == ']') {
					expression[iterator] = 0;
					break;
				} else if (expression[iterator] == '(') {
					/* NOT A CONSTANT ARRAY */
					return (FALSE);
				}
			}
			break;
		} else if (expression[index] == '<') {
			/* HIT A < BEFORE A [ THEREFORE */
			/* IS A FUNCTION CALL, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == '(') {
			/* HIT A ( BEFORE A [ THEREFORE */
			/* IS AN OBJECT ELEMENT, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == ' ')
			return (NULL);
	}

	// NO DELIMITER FOUND, TRY AS UNINDEXED CONSTANT
	if (delimiter == 0) {
		return (cinteger_resolve_indexed(name, 0));
	}

	// NO STRING BEFORE DELIMITER
	if (delimiter == 1) {
		return (NULL);
	}

	counter = value_of(&expression[delimiter], TRUE);

	if (counter > -1) {
		return (cinteger_resolve_indexed(expression, counter));
	} else {
		/* INDEX OUT OF RANGE */
		return (NULL);
	}
}

struct cinteger_type *cinteger_resolve_indexed(const char *name, int index) {
	struct cinteger_type *pointer = cinteger_table;

	if (pointer == NULL)
		return (NULL);

	do {
		if (!strcmp(name, pointer->name)) {
			if (index == 0) {
				return (pointer);
			} else {
				/* THIS VARIABLE DOES MATCH, BUT WERE NOT AT THE
				 * RIGHT INDEX YET SO MOVE ON */
				pointer = pointer->next_cinteger;
				index--;
			}
		} else
			pointer = pointer->next_cinteger;
	} while (pointer != NULL);

	/* IF index != 0, INDEX OUT OF RANGE, OTHERWISE NOT VARIABLE */
	return (NULL);
}

struct string_type *string_resolve(const char *name) {
	int             index,
	                iterator,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	strncpy(expression, name, 80);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '[') {
			expression[index] = 0;
			delimiter = index + 1;
			for (iterator = counter; iterator > 0; iterator--) {
				if (expression[iterator] == ']') {
					expression[iterator] = 0;
					break;
				}
			}
			break;
		} else if (expression[index] == '<') {
			/* HIT A < BEFORE A [ THEREFORE */
			/* IS A FUNCTION CALL, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == '(') {
			/* HIT A ( BEFORE A [ THEREFORE */
			/* IS AN OBJECT ELEMENT, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == ' ')
			return (NULL);
	}

	if (delimiter == 0) {
		/* NO DELIMITER FOUND, TRY AS UNINDEXED VARIABLE */
		return (string_resolve_indexed(name, 0));
	}

	if (delimiter == 1) {
		/* NO STRING BEFORE DELIMITER */
		return (NULL);
	}

	counter = value_of(&expression[delimiter], TRUE);

	if (counter > -1) {
		return (string_resolve_indexed(expression, counter));
	} else
		return (NULL);
}

struct string_type *string_resolve_indexed(const char *name, int index) {
	struct string_type *pointer = string_table;

	if (pointer == NULL)
		return (NULL);

	do {
		if (!strcmp(name, pointer->name)) {
			if (index == 0) {
				return (pointer);
			} else {
				/* THIS STRING DOES MATCH, BUT WERE NOT AT THE
				 * RIGHT INDEX YET SO MOVE ON */
				pointer = pointer->next_string;
				index--;
			}
		} else {
			pointer = pointer->next_string;
		}
	} while (pointer != NULL);

	return (NULL);
}

struct string_type *cstring_resolve(const char *name) {
	int             index,
	                iterator,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	strncpy(expression, name, 80);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '[') {
			expression[index] = 0;
			delimiter = index + 1;
			for (iterator = counter; iterator > 0; iterator--) {
				if (expression[iterator] == ']') {
					expression[iterator] = 0;
					break;
				}
			}
			break;
		} else if (expression[index] == '<') {
			/* HIT A < BEFORE A [ THEREFORE */
			/* IS A FUNCTION CALL, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == '(') {
			/* HIT A ( BEFORE A [ THEREFORE */
			/* IS AN OBJECT ELEMENT, NOT AN ARRAY */
			return (NULL);
		} else if (expression[index] == ' ')
			return (NULL);
	}

	if (delimiter == 0) {
		/* NO DELIMITER FOUND, TRY AS UNINDEXED VARIABLE */
		return (cstring_resolve_indexed(name, 0));
	}

	if (delimiter == 1) {
		/* NO STRING BEFORE DELIMITER */
		return (NULL);
	}

	counter = value_of(&expression[delimiter], TRUE);

	if (counter > -1) {
		return (cstring_resolve_indexed(expression, counter));
	} else
		return (NULL);
}

struct string_type *cstring_resolve_indexed(const char *name, int index) {
	struct string_type *pointer = cstring_table;

	if (pointer == NULL)
		return (NULL);

	do {
		if (!strcmp(name, pointer->name)) {
			if (index == 0) {
				return (pointer);
			} else {
				/* THIS STRING DOES MATCH, BUT WERE NOT AT THE
				 * RIGHT INDEX YET SO MOVE ON */
				pointer = pointer->next_string;
				index--;
			}
		} else {
			pointer = pointer->next_string;
		}
	} while (pointer != NULL);

	return (NULL);
}

struct function_type *function_resolve(const char *name) {
	const char      *full_name;
	char            core_name[84];
	int             index;

	struct function_type *pointer = function_table;

	if (function_table == NULL)
		return (NULL);

	/* STRIP ARGUMENTS OFF FIRST, THEN EXPAND RESOLVE NAME */
	index = 0;

	while (*name && index < 80) {
		if (*name == '<') {
			break;
		} else {
			core_name[index++] = *name++;
		}
	}
	core_name[index] = 0;

	/* GET A POINTER TO A STRING THAT REPRESENTS THE EXPANDED NAME OF THE FUNCTION */
	full_name = (const char *)expand_function(core_name);

	/* LOOP THROUGH ALL THE FUNCTIONS LOOKING FOR A FUNCTION THAT
	 * HAS THIS EXPANDED FULL NAME */
	do {
		if (!strcmp(full_name, pointer->name))
			return (pointer);
		else
			pointer = pointer->next_function;
	} while (pointer != NULL);

	/* RETURN A POINTER TO THE STRUCTURE THAT ENCAPSULATES THE FUNCTION */
	return (NULL);
}

const char *expand_function(const char *name) {
	/* THIS FUNCTION TAKES A SCOPE FUNCTION CALL SUCH AS noun1.function
	 * AND REOLVE THE ACTUAL FUNCTION NAME SUCH AS function_key */
	int             index,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	strncpy(expression, name, 80);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '.') {
			expression[index] = 0;
			delimiter = index + 1;
			break;
		}
	}

	if (delimiter == FALSE) {
		/* THIS FUNCTION DOESN'T CONTAIN A '.', SO RETURN IT AS IS */
		return (arg_text_of(name));
	}

	/* THE ORIGINAL STRING IS NOW CUT INTO TWO STRINGS:
	 * expression.delimiter */

	index = value_of(expression, TRUE);

	if (index < 1 || index > objects) {
		return ((const char *) name);
	}

	if (cinteger_resolve(&expression[delimiter]) != NULL ||
	        integer_resolve(&expression[delimiter]) != NULL ||
	        object_element_resolve(&expression[delimiter])) {
		/* THE DELIMETER RESOLVES TO A CONSTANT, VARIABLE OR OBJECT
		 * ELEMENT, SO TAKE NOTE OF THAT */
		sprintf(function_name, "%ld", value_of(&expression[delimiter], TRUE));
	} else {
		strcpy(function_name, &expression[delimiter]);
	}
	strcat(function_name, "_");
	strcat(function_name, object[index]->label);

	return ((const char *) function_name);
}

char *macro_resolve(const char *testString) {
	int             index,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	strncpy(expression, testString, 80);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '{' || expression[index] == '}') {
			expression[index] = 0;
			if (!delimiter)
				delimiter = index + 1;
		}
	}

	if (delimiter == FALSE)
		return (NULL);

	if (*expression != 0) {
		index = value_of(expression, TRUE);
	} else {
		index = 0;
	}

	if (!strcmp(&expression[delimiter], "list")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (list_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "plain")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (plain_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "long")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (long_output(index));
		}
	} else if (!strcmp(&expression[delimiter], "sub")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (sub_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "obj")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (obj_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "that")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (that_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "it")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (it_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "doesnt")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (doesnt_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "does")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (does_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "isnt")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (isnt_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "is")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (is_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "the")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (sentence_output(index, FALSE));
		}
	} else if (!strcmp(&expression[delimiter], "s")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			if (object[index]->attributes & PLURAL) {
				strcpy(temp_buffer, "");
			} else {
				strcpy(temp_buffer, "s");
			}
			return (temp_buffer);
		}
	} else if (!strcmp(&expression[delimiter], "names")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (object_names(index, temp_buffer));
		}
	} else if (!strcmp(&expression[delimiter], "label")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (object[index]->label);
		}
	} else if (!strcmp(&expression[delimiter], "List")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (list_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Plain")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (plain_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Sub")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (sub_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Obj")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (obj_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "That")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (that_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "It")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (it_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Doesnt")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (doesnt_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Does")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (does_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Isnt")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (isnt_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "Is")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (is_output(index, TRUE));
		}
	} else if (!strcmp(&expression[delimiter], "The")) {
		if (index < 1 || index > objects) {
			badptrrun(expression, index);
			return (NULL);
		} else {
			return (sentence_output(index, TRUE));
		}
	} else {
		strcpy(macro_function, "+macro_");
		strcat(macro_function, &expression[delimiter]);
		strcat(macro_function, "<");
		sprintf(temp_buffer, "%d", index);
		strcat(macro_function, temp_buffer);

		// BUILD THE FUNCTION NAME AND PASS THE OBJECT AS
		// THE ONLY ARGUMENT
		if (execute(macro_function)) {
			return (string_resolve("return_value")->value);
		}
	}

	return (NULL);
}

int count_resolve(const char *testString) {
	struct function_type    *resolved_function = NULL;

	if (*(testString + 1) == 0) {
		// @ ON ITS OWN, SO RETURN THE CALL COUNT OF THE CURRENTLY EXECUTING
		// FUNCTION
		return (executing_function->call_count);
	} else if ((resolved_function = function_resolve(testString + 1)) != NULL) {
		return (resolved_function->call_count);
	} else {
		return array_length_resolve(testString);
	}
}

int array_length_resolve(const char *testString) {
	int             counter = 0;
	const char      *array_name = &testString[1];

	struct integer_type *integer_pointer = integer_table;
	struct cinteger_type *cinteger_pointer = cinteger_table;
	struct string_type *string_pointer = string_table;
	struct string_type *cstring_pointer = cstring_table;

	if (integer_pointer != NULL) {
		do {
			if (!strcmp(array_name, integer_pointer->name)) {
				counter++;
			}
			integer_pointer = integer_pointer->next_integer;
		} while (integer_pointer != NULL);
	}

	/* IF ONE OR MORE INTEGERS WITH THIS NAME WERE FOUND
	   RETURN THE COUNT */
	if (counter)
		return (counter);

	if (string_pointer != NULL) {
		do {
			if (!strcmp(array_name, string_pointer->name)) {
				counter++;
			}
			string_pointer = string_pointer->next_string;
		} while (string_pointer != NULL);
	}

	/* IF ONE OR MORE STRINGS WITH THIS NAME WERE FOUND
	   RETURN THE COUNT */
	if (counter)
		return (counter);

	if (cinteger_pointer != NULL) {
		do {
			if (!strcmp(array_name, cinteger_pointer->name)) {
				counter++;
			}
			cinteger_pointer = cinteger_pointer->next_cinteger;
		} while (cinteger_pointer != NULL);
	}

	/* IF ONE OR MORE INTEGER CONSTANTS WITH THIS NAME WERE FOUND
	   RETURN THE COUNT */
	if (counter)
		return (counter);

	if (cstring_pointer != NULL) {
		do {
			if (!strcmp(array_name, cstring_pointer->name)) {
				counter++;
			}
			cstring_pointer = cstring_pointer->next_string;
		} while (cstring_pointer != NULL);
	}

	/* IF ONE OR MORE STRING CONSTANTS WITH THIS NAME WERE FOUND
	   RETURN THE COUNT */
	if (counter)
		return (counter);

	/* NO VARIABLES OR STRINGS FOUND */
	return (0);
}

int object_element_resolve(const char *testString) {
	int             index,
	                iterator,
	                counter;
	int             delimiter = 0;
	char            expression[84];

	struct integer_type *resolved_integer;
	struct cinteger_type *resolved_cinteger;

	strncpy(expression, testString, 80);

	//sprintf(temp_buffer, "incoming = %s^", testString);
	//write_text (temp_buffer);

	counter = strlen(expression);

	for (index = 0; index < counter; index++) {
		if (expression[index] == '(') {
			expression[index] = 0;
			delimiter = index + 1;
			for (iterator = counter; iterator > 0; iterator--) {
				if (expression[iterator] == ')') {
					expression[iterator] = 0;
					break;
				}
			}
			break;
		} else if (expression[index] == '<') {
			/* HIT A < BEFORE A [ THEREFORE */
			/* IS A FUNCTION CALL, NOT AN ARRAY */
			return (FALSE);
		} else if (expression[index] == '[') {
			/* HIT A [ BEFORE A ( THEREFORE */
			/* THIS EXPRESSION IS AN ARRAY, NOT AN OBJECT ELEMENT */
			/* UNLESS A CLOSING ] IS FOUND BEFORE THE OPENING ( */
			/* ie. COULD BE AN array[index](element) FORMAT */
			/* SEARCH FORWARD... */
			for (; index < counter; index++) {
				if (expression[index] == ']') {
					/* BREAK OUT AND KEEP LOOKING FOR A ( */
					break;
				} else if (expression[index] == '(') {
					/* THIS EXPRESSION IS DEFINITELY AN ARRAY WITH AN */
					/* OBJECT ELEMENT AS THE INDEX */
					return (FALSE);
				}
			}
		} else if (expression[index] == ' ')
			return (FALSE);
	}

	// NO DELIMITER FOUND OR NO STRING BEFORE DELIMITER
	if (delimiter == FALSE || delimiter == 1)
		return (FALSE);

	index = object_resolve(expression);

	if (index == -1) {
		//sprintf(temp_buffer, "expression %s is not an object^", expression);
		//write_text(temp_buffer);

		// COULDN'T BE RESOLVED AS AN OBJECT, TRY AS A VARIABLE
		if ((resolved_integer = integer_resolve(expression)) != NULL) {
			index = resolved_integer->value;
		} else if ((resolved_cinteger = cinteger_resolve(expression)) != NULL) {
			index = resolved_cinteger->value;
		}
	}

	if (index < 1 || index > objects) {
		badptrrun(expression, index);
		return (FALSE);
	}

	counter = value_of(&expression[delimiter], TRUE);

	if (counter < 0 || counter > 15) {
		sprintf(error_buffer,
		        "ERROR: In function \"%s\", element \"%s\" out of range (%d).^",
		        executing_function->name, &expression[delimiter], counter);
		write_text(error_buffer);
		return (FALSE);
	} else {
		oec = object[index]->integer[counter];
		object_element_address = &object[index]->integer[counter];
		return (TRUE);
	}
}

int object_resolve(const char *object_string) {
	int             index;

	if (!strcmp(object_string, "noun1"))
		return (noun[0]);
	else if (!strcmp(object_string, "noun2"))
		return (noun[1]);
	else if (!strcmp(object_string, "noun3"))
		return (noun[2]);
	else if (!strcmp(object_string, "noun4"))
		return (noun[3]);
	else if (!strcmp(object_string, "player"))
		return (player);
	else if (!strcmp(object_string, "here"))
		return (HERE);
	else if (!strcmp(object_string, "self") ||
	         !strcmp(object_string, "this")) {
		if (executing_function != NULL && executing_function->self == 0) {
			sprintf(error_buffer,
			        "ERROR: Reference to 'self' from global function \"%s\".^",
			        executing_function->name);
			write_text(error_buffer);
		} else
			return (executing_function->self);
	} else {
		for (index = 1; index <= objects; index++) {
			if (!strcmp(object_string, object[index]->label))
				return (index);
		}
	}

	return (-1);
}

long attribute_resolve(const char *attribute) {
	long            bit_mask;

	if (!strcmp(attribute, "VISITED"))
		return (VISITED);
	else if (!strcmp(attribute, "DARK"))
		return (DARK);
	else if (!strcmp(attribute, "ON_WATER"))
		return (ON_WATER);
	else if (!strcmp(attribute, "UNDER_WATER"))
		return (UNDER_WATER);
	else if (!strcmp(attribute, "WITHOUT_AIR"))
		return (WITHOUT_AIR);
	else if (!strcmp(attribute, "OUTDOORS"))
		return (OUTDOORS);
	else if (!strcmp(attribute, "MID_AIR"))
		return (MID_AIR);
	else if (!strcmp(attribute, "TIGHT_ROPE"))
		return (TIGHT_ROPE);
	else if (!strcmp(attribute, "POLLUTED"))
		return (POLLUTED);
	else if (!strcmp(attribute, "SOLVED"))
		return (SOLVED);
	else if (!strcmp(attribute, "MID_WATER"))
		return (MID_WATER);
	else if (!strcmp(attribute, "DARKNESS")) {
		bit_mask = DARKNESS;
		if (check_light(HERE)) {
			bit_mask = ~bit_mask;
			object[HERE]->attributes = object[HERE]->attributes & bit_mask;
		} else {
			object[HERE]->attributes = object[HERE]->attributes | bit_mask;
		}
		return (DARKNESS);
	} else if (!strcmp(attribute, "MAPPED"))
		return (MAPPED);
	else if (!strcmp(attribute, "KNOWN"))
		return (KNOWN);
	else if (!strcmp(attribute, "CLOSED"))
		return (CLOSED);
	else if (!strcmp(attribute, "LOCKED"))
		return (LOCKED);
	else if (!strcmp(attribute, "DEAD"))
		return (DEAD);
	else if (!strcmp(attribute, "IGNITABLE"))
		return (IGNITABLE);
	else if (!strcmp(attribute, "WORN"))
		return (WORN);
	else if (!strcmp(attribute, "CONCEALING"))
		return (CONCEALING);
	else if (!strcmp(attribute, "LUMINOUS"))
		return (LUMINOUS);
	else if (!strcmp(attribute, "WEARABLE"))
		return (WEARABLE);
	else if (!strcmp(attribute, "CLOSABLE"))
		return (CLOSABLE);
	else if (!strcmp(attribute, "LOCKABLE"))
		return (LOCKABLE);
	else if (!strcmp(attribute, "ANIMATE"))
		return (ANIMATE);
	else if (!strcmp(attribute, "LIQUID"))
		return (LIQUID);
	else if (!strcmp(attribute, "CONTAINER"))
		return (CONTAINER);
	else if (!strcmp(attribute, "SURFACE"))
		return (SURFACE);
	else if (!strcmp(attribute, "PLURAL"))
		return (PLURAL);
	else if (!strcmp(attribute, "FLAMMABLE"))
		return (FLAMMABLE);
	else if (!strcmp(attribute, "BURNING"))
		return (BURNING);
	else if (!strcmp(attribute, "LOCATION"))
		return (LOCATION);
	else if (!strcmp(attribute, "ON"))
		return (ON);
	else if (!strcmp(attribute, "DAMAGED"))
		return (DAMAGED);
	else if (!strcmp(attribute, "FEMALE"))
		return (FEMALE);
	else if (!strcmp(attribute, "POSSESSIVE"))
		return (POSSESSIVE);
	else if (!strcmp(attribute, "OUT_OF_REACH"))
		return (OUT_OF_REACH);
	else if (!strcmp(attribute, "TOUCHED"))
		return (TOUCHED);
	else if (!strcmp(attribute, "SCORED"))
		return (SCORED);
	else if (!strcmp(attribute, "SITTING"))
		return (SITTING);
	else if (!strcmp(attribute, "NPC"))
		return (NPC);
	else if (!strcmp(attribute, "DONE"))
		return (DONE);
	else if (!strcmp(attribute, "GAS"))
		return (MAPPED);
	else if (!strcmp(attribute, "NO_TAB"))
		return (NO_TAB);
	else if (!strcmp(attribute, "NOT_IMPORTANT"))
		return (NOT_IMPORTANT);
	else
		return (0);
}

long user_attribute_resolve(const char *name) {
	struct attribute_type *pointer = attribute_table;

	if (pointer == NULL)
		return (0);

	do {
		if (!strcmp(name, pointer->name)) {
			return (pointer->value);
		} else
			pointer = pointer->next_attribute;
	} while (pointer != NULL);

	/* ATTRIBUTE NOT FOUND */
	return (0);
}

} // End of namespace JACL
} // End of namespace Glk
