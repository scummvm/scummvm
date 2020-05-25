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
#include "glk/jacl/version.h"

namespace Glk {
namespace JACL {

/* INDICATES THAT THE CURRENT '.j2' FILE BEING WORKED
 * WITH IS ENCRYPTED */
int                 encrypted = FALSE;

extern char         text_buffer[];
extern char         temp_buffer[];
extern char         prefix[];
extern char         error_buffer[];
extern const char   *word[];
extern int          quoted[];
extern int          punctuated[];
extern int          wp;

extern schanid_t                sound_channel[];

extern struct object_type       *object[];
extern struct integer_type      *integer_table;
extern struct integer_type      *integer[];
extern struct cinteger_type     *cinteger_table;
extern struct string_type       *string_table;
extern struct string_type       *cstring_table;
extern struct attribute_type    *attribute_table;
extern struct function_type     *function_table;
extern struct function_type     *executing_function;
extern struct command_type      *completion_list;
extern struct word_type         *grammar_table;
extern struct synonym_type      *synonym_table;
extern struct filter_type       *filter_table;


struct string_type *current_string = NULL;
struct integer_type *current_integer = NULL;
struct integer_type *last_system_integer = NULL;

extern struct string_type *current_cstring;
extern struct cinteger_type *current_cinteger;

extern strid_t                  game_stream;

extern int                      objects;
extern int                      integers;
extern int                      functions;
extern int                      strings;
extern int                      player;

extern int                      it;
extern int                      them[];
extern int                      her;
extern int                      him;
extern int                      parent;

extern int                      noun[];

int                             value_resolved;

void read_gamefile() {
	int             index,
	                counter,
	                errors;
	int             result;
	int             location_count = 0;
	int             object_count = 0;
	int             line = 0;
	int             self_parent = 0;

	long            start_of_file = 0;
	long            bit_mask;

	filter_type *current_filter = NULL;
	filter_type *new_filter = NULL;
	attribute_type *current_attribute = NULL;
	attribute_type *new_attribute = NULL;
	cinteger_type *resolved_cinteger = NULL;
	synonym_type *current_synonym = NULL;
	synonym_type *new_synonym = NULL;
	function_type *current_function = NULL;
	name_type *current_name = NULL;

	char            function_name[81];

	// CREATE SOME SYSTEM VARIABLES

	// THIS IS USED BY JACL FUNCTIONS TO PASS STRING VALUES BACK
	// TO THE INTERPRETER AS JACL FUNCTION CAN ONLY RETURN
	// AN INTEGER
	create_string("return_value", "");

	create_cstring("function_name", "JACL*Internal");

	// THESE ARE THE FIELDS FOR THE CSV PARSER
	create_cstring("field", "field0");
	create_cstring("field", "field1");
	create_cstring("field", "field2");
	create_cstring("field", "field3");
	create_cstring("field", "field4");
	create_cstring("field", "field5");
	create_cstring("field", "field6");
	create_cstring("field", "field7");
	create_cstring("field", "field8");
	create_cstring("field", "field9");
	create_cstring("field", "field10");
	create_cstring("field", "field11");
	create_cstring("field", "field12");
	create_cstring("field", "field13");
	create_cstring("field", "field14");
	create_cstring("field", "field15");
	create_cstring("field", "field16");
	create_cstring("field", "field17");
	create_cstring("field", "field18");
	create_cstring("field", "field19");

	create_cinteger("field_count", 0);

	create_integer("compass", 0);
	// START AT -1 AS TIME PASSES BEFORE THE FIRST PROMPT
	create_integer("total_moves", -1);
	create_integer("time", TRUE);
	create_integer("score", 0);
	create_integer("display_mode", 0);
	create_integer("internal_version", J_VERSION);
	create_integer("max_rand", 100);
	create_integer("destination", 0);
	create_integer("interrupted", 0);
	create_integer("debug", 0);
	create_integer("graphics_enabled", 0);
	create_integer("sound_enabled", 0);
	create_integer("timer_enabled", 0);
	create_integer("multi_prefix", 0);
	create_integer("notify", 1);
	create_integer("debug", 0);
	create_integer("linebreaks", 1);

	/* STORE THIS SO THE SECOND PASS KNOWS WHERE TO START
	 * SETTING VALUES FROM (EVERYTHING BEFORE THIS IN THE
	 * VARIABLE TABLE IS A SYSTEM VARIABLE */
	last_system_integer = current_integer;

	/* CREATE SOME SYSTEM CONSTANTS */
	create_cinteger("graphics_supported", 0);
	create_cinteger("sound_supported", 0);
	create_cinteger("timer_supported", 0);
	create_cinteger("GLK", 0);
	create_cinteger("CGI", 1);
	create_cinteger("NDS", 2);

	create_cinteger("interpreter", 0);

	/* TEST FOR AVAILABLE FUNCTIONALITY BEFORE EXECUTING ANY JACL CODE */

	GRAPHICS_SUPPORTED->value = (int)g_vm->glk_gestalt(gestalt_Graphics, 0);
	GRAPHICS_ENABLED->value = (int)g_vm->glk_gestalt(gestalt_Graphics, 0);
	SOUND_SUPPORTED->value = (int)g_vm->glk_gestalt(gestalt_Sound, 0);
	SOUND_ENABLED->value = (int)g_vm->glk_gestalt(gestalt_Sound, 0);
	TIMER_SUPPORTED->value = (int)g_vm->glk_gestalt(gestalt_Timer, 0);
	TIMER_ENABLED->value = (int)g_vm->glk_gestalt(gestalt_Timer, 0);

	create_cinteger("true", 1);
	create_cinteger("false", 0);
	create_cinteger("null", 0);
	create_cinteger("nowhere", 0);
	create_cinteger("heavy", HEAVY);
	create_cinteger("scenery", SCENERY);
	create_cinteger("north", NORTH_DIR);
	create_cinteger("south", SOUTH_DIR);
	create_cinteger("east", EAST_DIR);
	create_cinteger("west", WEST_DIR);
	create_cinteger("northeast", NORTHEAST_DIR);
	create_cinteger("northwest", NORTHWEST_DIR);
	create_cinteger("southeast", SOUTHEAST_DIR);
	create_cinteger("southwest", SOUTHWEST_DIR);
	create_cinteger("up", UP_DIR);
	create_cinteger("down", DOWN_DIR);
	create_cinteger("in", IN_DIR);
	create_cinteger("out", OUT_DIR);
	create_cinteger("parent", 0);
	create_cinteger("quantity", 1);
	create_cinteger("capacity", 1);
	create_cinteger("mass", 2);
	create_cinteger("bearing", 3);
	create_cinteger("velocity", 4);
	create_cinteger("next", 5);
	create_cinteger("previous", 6);
	create_cinteger("child", 7);
	create_cinteger("index", 8);
	create_cinteger("status", 9);
	create_cinteger("state", 10);
	create_cinteger("counter", 11);
	create_cinteger("points", 12);
	create_cinteger("class", 13);
	create_cinteger("x", 14);
	create_cinteger("y", 15);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("volume", 100);
	create_cinteger("timer", 500);

	set_defaults();

	/* CREATE A DUMMY FUNCTION TO BE USED WHEN AN ERROR MESSAGE
	   IS PRINTED AS A RESULT OF CODE CALLED BY THE INTERPRETER */
	if ((function_table = (struct function_type *)
	                      malloc(sizeof(struct function_type))) == NULL)
		outofmem();
	else {
		current_function = function_table;
		strcpy(current_function->name, "JACL*Internal");
		current_function->position = 0;
		current_function->self = 0;
		current_function->call_count = 0;
		current_function->call_count_backup = 0;
		current_function->next_function = NULL;
	}

	executing_function = function_table;

	errors = 0;
	objects = 0;
	integers = 0;
	functions = 0;
	strings = 0;

	g_vm->glk_stream_set_position(game_stream, start_of_file, seekmode_Start);
	result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);

	line++;

	if (!encrypted && strstr(text_buffer, "#encrypted")) {
		encrypted = TRUE;

		result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
		line++;
	}

	if (encrypted) jacl_decrypt(text_buffer);

	while (result) {
		encapsulate();
		if (word[0] == NULL);
		else if (text_buffer[0] == '{') {
			while (result) {
				result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
				line++;

				if (!encrypted && strstr(text_buffer, "#encrypted")) {
					encrypted = TRUE;
					result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
					line++;
				}
				if (encrypted) jacl_decrypt(text_buffer);
				if (text_buffer[0] == '}')
					break;
			}
		} else {
			if (!strcmp(word[0], "grammar")) {
				if (word[++wp] == NULL) {
					noproperr(line);
					errors++;
				} else {
					if (grammar_table == NULL) {
						if ((grammar_table = (struct word_type *)
						                     malloc(sizeof(struct word_type))) == NULL)
							outofmem();
						else {
							strncpy(grammar_table->word, word[wp], 40);
							grammar_table->word[40] = 0;
							grammar_table->next_sibling = NULL;
							grammar_table->first_child = NULL;
							build_grammar_table(grammar_table);
						}
					} else
						build_grammar_table(grammar_table);
				}
			} else if (!strcmp(word[0], "object")
			           || !strcmp(word[0], "location")) {
				if (word[1] == NULL) {
					noproperr(line);
					errors++;
				} else if (legal_label_check(word[1], line, OBJ_TYPE)) {
					errors++;
				} else {
					objects++;

					if (objects == MAX_OBJECTS) {
						log_error(MAXIMUM_EXCEEDED, PLUS_STDERR);
						terminate(47);
						return;
					} else {
						if ((object[objects] = (struct object_type *)
						                       malloc(sizeof(struct object_type))) == NULL)
							outofmem();

						strncpy(object[objects]->label, word[1], 40);

						object[objects]->label[40] = 0;
						object[objects]->first_plural = NULL;

						strcpy(object[objects]->described, object[objects]->label);
						strcpy(object[objects]->inventory, object[objects]->label);
						strcpy(object[objects]->article, "the");
						strcpy(object[objects]->definite, "the");
						object[objects]->attributes = FALSE;
						object[objects]->user_attributes = FALSE;

						for (counter = 0; counter < 16; counter++)
							object[objects]->integer[counter] = 0;
					}
					object[objects]->nosave = FALSE;
				}
			} else if (!strcmp(word[0], "synonym")) {
				if (word[++wp] == NULL) {
					noproperr(line);
					errors++;
				} else {
					if ((new_synonym = (struct synonym_type *)
					                   malloc(sizeof(struct synonym_type))) == NULL)
						outofmem();
					else {
						if (synonym_table == NULL) {
							synonym_table = new_synonym;
						} else {
							current_synonym->next_synonym = new_synonym;
						}
					}
					current_synonym = new_synonym;
					strncpy(current_synonym->original, word[wp], 40);
					current_synonym->original[40] = 0;
					if (word[++wp] == NULL) {
						noproperr(line);
						errors++;
					} else {
						strncpy(current_synonym->standard, word[wp], 40);
						current_synonym->standard[40] = 0;
					}
					current_synonym->next_synonym = NULL;
				}
			} else if (!strcmp(word[0], "parameter")) {
#ifdef UNUSED
				if (word[2] == NULL) {
					noproperr(line);
					errors++;
				} else {
					if ((new_parameter = (struct parameter_type *)
					                     malloc(sizeof(struct parameter_type))) == NULL)
						outofmem();
					else {
						if (parameter_table == NULL) {
							parameter_table = new_parameter;
						} else {
							current_parameter->next_parameter =
							    new_parameter;
						}
						current_parameter = new_parameter;
						strncpy(current_parameter->name, word[1], 40);
						current_parameter->name[40] = 0;
						strncpy(current_parameter->container, word[2], 40);
						current_parameter->container[40] = 0;
						current_parameter->next_parameter = NULL;
					}

					if (word[4] != NULL) {
						if (validate(word[3]))
							current_parameter->low = atoi(word[3]);
						else
							current_parameter->low = -65535;

						if (validate(word[4]))
							current_parameter->high = atoi(word[4]);
						else
							current_parameter->high = 65535;
					} else {
						current_parameter->low = -65535;
						current_parameter->high = 65535;
					}

				}
#else
				warning("parameter");
#endif
			} else if (!strcmp(word[0], "constant")) {
				if (word[2] == NULL) {
					noproperr(line);
					errors++;
				} else {
					/* CHECK IF MORE THAN ONE VALUE IS SUPPLIED AND CREATE
					   ADDITIONAL CONSTANTS IF REQUIRED */
					index = 2;

					while (word[index] != NULL && index < MAX_WORDS) {
						if (quoted[index] == TRUE || !validate(word[index])) {
							if (legal_label_check(word[1], line, CSTR_TYPE)) {
								errors++;
							} else {
								create_cstring(word[1], word[index]);
							}
						} else {
							if (legal_label_check(word[1], line, CINT_TYPE)) {
								errors++;
							} else {
								create_cinteger(word[1], value_of(word[index])/*, FALSE */);
								if (!value_resolved) {
									unkvalerr(line, index);
									errors++;
								}
							}
						}
						index++;
					}
				}
			} else if (!strcmp(word[0], "attribute")) {
				if (word[1] == NULL) {
					noproperr(line);
					errors++;
				} else if (legal_label_check(word[1], line, ATT_TYPE)) {
					errors++;
				} else if (current_attribute != NULL && current_attribute->value == 1073741824) {
					maxatterr(line, 1);
					errors++;
				} else {
					if ((new_attribute = (struct attribute_type *)
					                     malloc(sizeof(struct attribute_type))) == NULL)
						outofmem();
					else {
						if (attribute_table == NULL) {
							attribute_table = new_attribute;
							new_attribute->value = 1;
						} else {
							current_attribute->next_attribute = new_attribute;
							new_attribute->value = current_attribute->value * 2;
						}
						current_attribute = new_attribute;
						strncpy(current_attribute->name, word[1], 40);
						current_attribute->name[40] = 0;
						current_attribute->next_attribute = NULL;
					}

					/* CHECK IF MORE THAN ONE VALUE IS SUPPLIED AND CREATE
					   ADDITIONAL CONSTANTS IF REQUIRED */
					index = 2;
					while (word[index] != NULL && index < MAX_WORDS) {
						if (legal_label_check(word[index], line, ATT_TYPE)) {
							errors++;
						} else if (current_attribute != NULL && current_attribute->value == 1073741824) {
							maxatterr(line, index);
							errors++;
						} else {
							if ((new_attribute = (struct attribute_type *)
							                     malloc(sizeof(struct attribute_type))) == NULL)
								outofmem();
							else {
								current_attribute->next_attribute = new_attribute;
								new_attribute->value = current_attribute->value * 2;
								current_attribute = new_attribute;
								strncpy(current_attribute->name, word[index], 40);
								current_attribute->name[40] = 0;
								current_attribute->next_attribute = NULL;
							}
						}
						index++;
					}
				}
			} else if (!strcmp(word[0], "string")) {
				if (word[1] == NULL) {
					noproperr(line);
					errors++;
				} else if (legal_label_check(word[1], line, STR_TYPE)) {
					errors++;
				} else {
					if (word[2] == NULL) {
						create_string(word[1], "");
					} else {
						create_string(word[1], word[2]);
						index = 3;
						while (word[index] != NULL && index < MAX_WORDS) {
							create_string(word[1], word[index]);
							index++;
						}
					}
				}
			} else if (!strcmp(word[0], "filter")) {
				if (word[++wp] == NULL) {
					noproperr(line);
					errors++;
				} else {
					if ((new_filter = (struct filter_type *)
					                  malloc(sizeof(struct filter_type))) == NULL)
						outofmem();
					else {
						if (filter_table == NULL) {
							filter_table = new_filter;
						} else {
							current_filter->next_filter = new_filter;
						}
						current_filter = new_filter;
						strncpy(current_filter->word, word[wp], 40);
						current_filter->word[40] = 0;
						current_filter->next_filter = NULL;
					}
				}
			} else if (!strcmp(word[0], "string_array")) {
				if (word[2] == NULL) {
					noproperr(line);
					errors++;
				} else if (legal_label_check(word[1], line, STR_TYPE)) {
					errors++;
				} else {
					int x;

					index = value_of(word[2], FALSE);
					if (!value_resolved) {
						unkvalerr(line, 2);
						errors++;
					}

					for (x = 0; x < index; x++) {
						create_string(word[1], word[3]);
					}
				}
			} else if (!strcmp(word[0], "integer_array")) {
				if (word[2] == NULL) {
					noproperr(line);
					errors++;
				} else if (legal_label_check(word[1], line, INT_TYPE)) {
					errors++;
				} else {
					int default_value, x;

					if (word[3] != NULL) {
						default_value = value_of(word[3], FALSE);
						if (!value_resolved) {
							unkvalerr(line, 3);
							errors++;
						}
					} else {
						default_value = 0;
					}

					/* THIS IS THE NUMBER OF ARRAY ELEMENTS TO MAKE */
					index = value_of(word[2], FALSE);
					if (!value_resolved) {
						unkvalerr(line, 2);
						errors++;
					}

					for (x = 0; x < index; x++) {
						create_integer(word[1], default_value);
					}
				}
			} else if (!strcmp(word[0], "integer")) {
				if (word[1] == NULL) {
					noproperr(line);
					errors++;
				} else if (legal_label_check(word[1], line, INT_TYPE)) {
					errors++;
				} else {
					create_integer(word[1], 0);

					/* CHECK IF MORE THAN ONE VALUE IS SUPPLIED AND CREATE
					   ADDITIONAL VARIABLES IF REQUIRED */
					index = 3;
					while (word[index] != NULL && index < MAX_WORDS) {
						create_integer(word[1], 0);
						index++;
					}
				}
			}
		}

		result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
		line++;

		if (!encrypted && strstr(text_buffer, "#encrypted")) {
			encrypted = TRUE;
			result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
			line++;
		}
		if (encrypted) jacl_decrypt(text_buffer);
	}

	if (errors) {
		totalerrs(errors);
		terminate(48);
		return;
	}

	/*************************************************************************
	 * START OF SECOND PASS                                                  *
	 *************************************************************************/

	/* IF NO SIZE IS SPECIFIED FOR THE STATUS WINDOW, SET IT TO 1 */
	if (integer_resolve("status_window") == NULL) {
		create_integer("status_window", 1);
	}

	/* IF NO STRING IS SPECIFIED FOR THE COMMAND PROMPT, SET IT TO "^> " */
	if (string_resolve("command_prompt") == NULL) {
		create_string("command_prompt", "^> ");
	}

	/* IF NO STRING IS SPECIFIED FOR THE GAME_TITLE, SET IT TO THE FILENAME */
	if (cstring_resolve("game_title") == NULL) {
		create_cstring("game_title", prefix);
	}

	create_language_constants();

	/* MUST RE-DETERMINE THE POINT IN THE GAME FILE THAT ENCRYPTION STARTS */
	encrypted = FALSE;

	/* SET CURRENT VARIABLE TO POINT TO THE FIRST USER VARIABLE THAT WAS
	 * CREATED AFTER THE SYSTEM VARIABLES */
	current_integer = last_system_integer;

	line = 0;
	g_vm->glk_stream_set_position(game_stream, start_of_file, seekmode_Start);
	result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);

	line++;

	if (!encrypted && strstr(text_buffer, "#encrypted")) {
		encrypted = TRUE;
		result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
		line++;
	}
	if (encrypted) jacl_decrypt(text_buffer);

	while (result) {
		encapsulate();
		if (word[0] == NULL);
		else if (text_buffer[0] == '{') {
			word[wp]++;         /* MOVE THE START OF THE FIRST WORD ONLY
                                 * TO PAST THE '{'. */
			if (word[wp][0] == 0) {
				nofnamerr(line);
				errors++;
			} else {
				while (word[wp] != NULL && wp < MAX_WORDS) {
					if (word[wp][0] == '+') {
						strncpy(function_name, word[wp], 80);
						function_name[80] = 0;
						self_parent = 0;
					} else if (word[wp][0] == '*') {
						const char *last_underscore = (char *)NULL;

						/* ALLOW MANUAL NAMING OF ASSOCIATED FUNCTIONS */
						/* TO GIVE CLASS-LIKE BEHAVIOR */
						strncpy(function_name, word[wp] + 1, 80);
						function_name[80] = 0;

						/* LOOK FOR THE FINAL UNDERSCORE AND SEE IF */
						/* IT IS FOLLOWED BY AN OBJECT LABEL */
						last_underscore = strrchr(word[wp], '_');
						if (last_underscore != NULL) {
							self_parent = object_resolve(last_underscore + 1);
						} else {
							self_parent = 0;
						}
					} else if (object_count == 0) {
						nongloberr(line);
						errors++;
					} else {
						strncpy(function_name, word[wp], 59);
						strcat(function_name, "_");
						strcat(function_name, object[object_count]->label);
						self_parent = object_count;
					}
					if (function_table == NULL) {
						if ((function_table = (struct function_type *)
						                      malloc(sizeof(struct function_type))) == NULL)
							outofmem();
						else {
							// STORE THE NUMBER OF FUNCTION DEFINED TO
							// HELP VALIDATE SAVED GAME FILES
							functions++;

							current_function = function_table;
							strcpy(current_function->name, function_name);

							current_function->position = g_vm->glk_stream_get_position(game_stream);
							current_function->call_count = 0;
							current_function->call_count_backup = 0;
							current_function->self = self_parent;
							current_function->next_function = NULL;
						}
					} else {
						if ((current_function->next_function =
						            (struct function_type *)
						            malloc(sizeof(struct function_type))) == NULL)
							outofmem();
						else {
							// STORE THE NUMBER OF FUNCTION DEFINED TO
							// HELP VALIDATE SAVED GAME FILES
							functions++;

							current_function = current_function->next_function;
							strcpy(current_function->name, function_name);

							current_function->position = g_vm->glk_stream_get_position(game_stream);
							current_function->call_count = 0;
							current_function->call_count_backup = 0;
							current_function->self = self_parent;
							current_function->next_function = NULL;
						}
					}
					wp++;
				}
			}

			while (result) {
				result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
				line++;

				if (!encrypted && strstr(text_buffer, "#encrypted")) {
					encrypted = TRUE;
					result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
					line++;
				}
				if (encrypted) jacl_decrypt(text_buffer);
				if (text_buffer[0] == '}')
					break;
			}
		}
		else if (!strcmp(word[0], "string_array")) {
		} else if (!strcmp(word[0], "integer_array")) {
			if (word[2] == NULL) {
				noproperr(line);
				errors++;
			} else {
				int x;

				/* THIS IS THE NUMBER OF ARRAY ELEMENTS TO MAKE */
				index = value_of(word[2], FALSE);
				if (!value_resolved) {
					unkvalerr(line, 2);
					errors++;
				}

				for (x = 0; x < index; x++) {
					current_integer = current_integer->next_integer;
				}
			}
		} else if (!strcmp(word[0], "integer")) {
			if (word[2] != NULL) {
				current_integer = current_integer->next_integer;
				current_integer->value = value_of(word[2], FALSE);
				if (!value_resolved) {
					unkvalerr(line, 2);
					errors++;
				}
				index = 3;
				while (word[index] != NULL && index < MAX_WORDS) {
					current_integer = current_integer->next_integer;
					current_integer->value = value_of(word[index], FALSE);
					if (!value_resolved) {
						unkvalerr(line, index);
						errors++;
					}
					index++;
				}
			} else {
				current_integer = current_integer->next_integer;
				current_integer->value = FALSE;
			}

			/* CONSUME ALL THESE KEYWORDS TO AVOID AN UNKNOWN KEYWORD */
			/* ERROR DURING THE SECOND PASS (ALL WORK DONE IN FIRST PASS) */
		} else if (!strcmp(word[0], "constant"));
		else if (!strcmp(word[0], "string"));
		else if (!strcmp(word[0], "attribute"));
		else if (!strcmp(word[0], "parameter"));
		else if (!strcmp(word[0], "synonym"));
		else if (!strcmp(word[0], "grammar"));
		else if (!strcmp(word[0], "filter"));
		else if (!strcmp(word[0], "has")) {
			if (word[1] == NULL) {
				noproperr(line);
				errors++;
			} else if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else {
				for (index = 1; word[index] != NULL && index < MAX_WORDS; index++) {
					if ((bit_mask = attribute_resolve(word[index]))) {
						object[object_count]->attributes = object[object_count]->attributes | bit_mask;
					} else if ((bit_mask = user_attribute_resolve(word[index]))) {
						object[object_count]->user_attributes = object[object_count]->user_attributes | bit_mask;
					} else {
						unkatterr(line, index);
						errors++;
					}
				}
			}
		} else if (!strcmp(word[0], "object")
		           || !strcmp(word[0], "location")) {
			object_count++;

			if (!strcmp(word[0], "object")) {
				object[object_count]->MASS = SCENERY;
				if (location_count == 0)
					object[object_count]->PARENT = 0;
				else
					object[object_count]->PARENT = location_count;
			} else {
				location_count = object_count;
				object[object_count]->PARENT = 0;
				object[object_count]->attributes =
				    object[object_count]->attributes | LOCATION;
			}


			if ((object[object_count]->first_name =
			            (struct name_type *) malloc(sizeof(struct name_type)))
			        == NULL)
				outofmem();
			else {
				current_name = object[object_count]->first_name;
				if (word[2] != NULL) {
					strncpy(current_name->name, word[2], 40);
				} else {
					strncpy(current_name->name, object[object_count]->label, 40);
				}
				current_name->name[40] = 0;
				current_name->next_name = NULL;
			}

			wp = 3;

			while (word[wp] != NULL && wp < MAX_WORDS) {
				if ((current_name->next_name = (struct name_type *)
				                               malloc(sizeof(struct name_type))) == NULL)
					outofmem();
				else {
					current_name = current_name->next_name;
					strncpy(current_name->name, word[wp], 40);
					current_name->name[40] = 0;
					current_name->next_name = NULL;
				}
				wp++;
			}
		} else if (!strcmp(word[0], "plural")) {
			if (word[1] == NULL) {
				noproperr(line);
				errors++;
			} else {
				if ((object[object_count]->first_plural =
				            (struct name_type *) malloc(sizeof(struct name_type)))
				        == NULL)
					outofmem();
				else {
					current_name = object[object_count]->first_plural;
					strncpy(current_name->name, word[1], 40);
					current_name->name[40] = 0;
					current_name->next_name = NULL;
				}

				wp = 2;

				while (word[wp] != NULL && wp < MAX_WORDS) {
					if ((current_name->next_name = (struct name_type *)
					                               malloc(sizeof(struct name_type))) == NULL)
						outofmem();
					else {
						current_name = current_name->next_name;
						strncpy(current_name->name, word[wp], 40);
						current_name->name[40] = 0;
						current_name->next_name = NULL;
					}
					wp++;
				}
			}
		} else if (!strcmp(word[0], "static")) {
			if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else
				object[object_count]->nosave = TRUE;
		} else if (!strcmp(word[0], "player")) {
			if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else
				player = object_count;
		} else if (!strcmp(word[0], "short")) {
			if (word[2] == NULL) {
				noproperr(line);
				errors++;
			} else if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else {
				strncpy(object[object_count]->article, word[1], 10);
				object[object_count]->article[10] = 0;
				strncpy(object[object_count]->inventory, word[2], 40);
				object[object_count]->inventory[40] = 0;
			}
		} else if (!strcmp(word[0], "definite")) {
			if (word[1] == NULL) {
				noproperr(line);
				errors++;
			} else if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else {
				strncpy(object[object_count]->definite, word[1], 10);
				object[object_count]->definite[10] = 0;
			}
		} else if (!strcmp(word[0], "long")) {
			if (word[1] == NULL) {
				noproperr(line);
				errors++;
			} else if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else {
				strncpy(object[object_count]->described, word[1], 80);
				object[object_count]->described[80] = 0;
			}
		} else if ((resolved_cinteger = cinteger_resolve(word[0])) != NULL) {
			index = resolved_cinteger->value;
			if (word[1] == NULL) {
				noproperr(line);
				errors++;
			} else if (object_count == 0) {
				noobjerr(line);
				errors++;
			} else if (!strcmp(word[1], "here")) {
				object[object_count]->integer[index] = location_count;
			} else {
				object[object_count]->integer[index] = value_of(word[1], FALSE);
				if (!value_resolved) {
					unkvalerr(line, 1);
					errors++;
				}
			}
		} else {
			unkkeyerr(line, 0);
			errors++;
		}

		result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
		line++;

		if (!encrypted && strstr(text_buffer, "#encrypted")) {
			encrypted = TRUE;
			result = glk_get_bin_line_stream(game_stream, text_buffer, (glui32) 1024);
			line++;
		}
		if (encrypted) jacl_decrypt(text_buffer);
	}

	/* CREATE THE CONSTANT THE RECORDS THE TOTAL NUMBER OF OBJECTS */
	create_cinteger("objects", objects);

	/* LOOP THROUGH ALL THE OBJECTS AND CALL THEIR CONSTRUCTORS
	for (index = 1; index <= objects; index++) {
	    strcpy (function_name, "constructor_");
	    strcat (function_name, object[index]->label);
	    execute (function_name);
	}
	*/

	if (errors) {
		totalerrs(errors);
		terminate(48);
		return;
	}
}

void build_grammar_table(struct word_type *pointer) {
	do {
		if (!strcmp(word[wp], pointer->word)) {
			if (pointer->first_child == NULL && word[wp + 1] != NULL) {
				if ((pointer->first_child = (struct word_type *)
				                            malloc(sizeof(struct word_type)))
				        == NULL)
					outofmem();
				else {
					pointer = pointer->first_child;
					strncpy(pointer->word, word[++wp], 40);
					pointer->word[40] = 0;
					pointer->next_sibling = NULL;
					pointer->first_child = NULL;
				}
			} else {
				pointer = pointer->first_child;
				wp++;
			}
		} else {
			if (pointer->next_sibling == NULL) {
				if ((pointer->next_sibling = (struct word_type *)
				                             malloc(sizeof(struct word_type)))
				        == NULL)
					outofmem();
				else {
					pointer = pointer->next_sibling;
					strncpy(pointer->word, word[wp], 40);
					pointer->word[40] = 0;
					pointer->next_sibling = NULL;
					pointer->first_child = NULL;
				}
			} else
				pointer = pointer->next_sibling;
		}
	} while (word[wp] != NULL && wp < MAX_WORDS);
}

int legal_label_check(const char *label_word, int line, int type) {
	struct integer_type *integer_pointer = integer_table;
	struct cinteger_type *cinteger_pointer = cinteger_table;
	struct string_type *string_pointer = string_table;
	struct string_type *cstring_pointer = cstring_table;
	struct attribute_type *attribute_pointer = attribute_table;

	int index;

	if (!strcmp(label_word, "here") ||
	        !strcmp(label_word, "player") ||
	        !strcmp(label_word, "integer") ||
	        !strcmp(label_word, "arg") ||
	        !strcmp(label_word, "string_arg") ||
	        !strcmp(label_word, "arg") ||
	        !strcmp(label_word, "$label_word") ||
	        !strcmp(label_word, "self") ||
	        !strcmp(label_word, "this") ||
	        !strcmp(label_word, "noun1") ||
	        !strcmp(label_word, "noun2") ||
	        !strcmp(label_word, "noun3") ||
	        !strcmp(label_word, "noun4") ||
	        !strcmp(label_word, "objects") ||
	        validate(label_word)) {
		sprintf(error_buffer, ILLEGAL_LABEL, line, label_word);
		log_error(error_buffer, PLUS_STDERR);

		return (TRUE);
	}

	if (type == CSTR_TYPE) {
		if (!strcmp(label_word, "command_prompt")) {
			sprintf(error_buffer, USED_LABEL_STR, line, label_word);
			log_error(error_buffer, PLUS_STDERR);

			return (TRUE);
		}
	}

	while (integer_pointer != NULL && type != INT_TYPE) {
		if (!strcmp(label_word, integer_pointer->name)) {
			sprintf(error_buffer, USED_LABEL_INT, line, label_word);
			log_error(error_buffer, PLUS_STDERR);

			return (TRUE);
		} else
			integer_pointer = integer_pointer->next_integer;
	}


	while (cinteger_pointer != NULL && type != CINT_TYPE) {
		if (!strcmp(label_word, cinteger_pointer->name)) {
			sprintf(error_buffer, USED_LABEL_CINT, line, label_word);
			log_error(error_buffer, PLUS_STDERR);

			return (TRUE);
		} else
			cinteger_pointer = cinteger_pointer->next_cinteger;
	}

	while (string_pointer != NULL && type != STR_TYPE) {
		if (!strcmp(label_word, string_pointer->name)) {
			sprintf(error_buffer, USED_LABEL_STR, line, label_word);
			log_error(error_buffer, PLUS_STDERR);

			return (TRUE);
		} else
			string_pointer = string_pointer->next_string;
	}

	while (cstring_pointer != NULL && type != CSTR_TYPE) {
		if (!strcmp(label_word, cstring_pointer->name)) {
			sprintf(error_buffer, USED_LABEL_CSTR, line, label_word);
			log_error(error_buffer, PLUS_STDERR);

			return (TRUE);
		} else
			cstring_pointer = cstring_pointer->next_string;
	}

	/* DON'T CHECK FOR ATT_TYPE AS YOU CAN'T HAVE ATTRIBUTE ARRAYS. */
	while (attribute_pointer != NULL) {
		if (!strcmp(label_word, attribute_pointer->name)) {
			sprintf(error_buffer, USED_LABEL_ATT, line, label_word);
			write_text(error_buffer);

			return (TRUE);
		} else
			attribute_pointer = attribute_pointer->next_attribute;
	}

	for (index = 1; index <= objects; index++) {
		if (!strcmp(label_word, object[index]->label)) {
			sprintf(error_buffer, USED_LABEL_OBJ,
			        line, label_word);
			log_error(error_buffer, PLUS_STDERR);

			return (TRUE);
		}
	}

	return (FALSE);
}

void restart_game() {
	int             index;

	integer_type *curr_integer;
	integer_type *previous_integer;
	synonym_type *current_synonym;
	synonym_type *previous_synonym;
	name_type *current_name;
	name_type *next_name;
	function_type *current_function;
	function_type *previous_function;
	string_type *curr_string;
	string_type *previous_string;
	attribute_type *current_attribute;
	attribute_type *previous_attribute;
	cinteger_type *previous_cinteger;
	filter_type *current_filter;
	filter_type *previous_filter;

	if (SOUND_SUPPORTED->value) {
		/* STOP ALL SOUNDS AND SET VOLUMES BACK TO 100% */
		for (index = 0; index < 4; index++) {
			g_vm->glk_schannel_stop(sound_channel[index]);
			g_vm->glk_schannel_set_volume(sound_channel[index], 65535);

			/* STORE A COPY OF THE CURRENT VOLUME FOR ACCESS
			 * FROM JACL CODE */
			sprintf(temp_buffer, "volume[%d]", index);
			cinteger_resolve(temp_buffer)->value = 100;
		}
	}

	/* FREE ALL OBJECTS */
	for (index = 1; index <= objects; index++) {
		current_name = object[index]->first_name;
		while (current_name->next_name != NULL) {
			next_name = current_name->next_name;
			free(current_name);
			current_name = next_name;
		}
		free(current_name);
		free(object[index]);
	}

	/* FREE ALL VARIABLES */

	if (integer_table != NULL) {
		if (integer_table->next_integer != NULL) {
			do {
				curr_integer = integer_table;
				previous_integer = integer_table;
				while (curr_integer->next_integer != NULL) {
					previous_integer = curr_integer;
					curr_integer = curr_integer->next_integer;
				}
				free(curr_integer);
				previous_integer->next_integer = NULL;
			} while (previous_integer != integer_table);
		}

		free(integer_table);
		integer_table = NULL;
	}

	/* FREE ALL FUNCTIONS */
	if (function_table != NULL) {
		if (function_table->next_function != NULL) {
			do {
				current_function = function_table;
				previous_function = function_table;
				while (current_function->next_function != NULL) {
					previous_function = current_function;
					current_function = current_function->next_function;
				}
				free(current_function);
				previous_function->next_function = NULL;
			} while (previous_function != function_table);
		}

		free(function_table);
		function_table = NULL;
	}

	/* FREE ALL FILTERS */
	if (filter_table != NULL) {
		if (filter_table->next_filter != NULL) {
			do {
				current_filter = filter_table;
				previous_filter = filter_table;
				while (current_filter->next_filter != NULL) {
					previous_filter = current_filter;
					current_filter = current_filter->next_filter;
				}
				free(current_filter);
				previous_filter->next_filter = NULL;
			} while (previous_filter != filter_table);
		}

		free(filter_table);
		filter_table = NULL;
	}

	/* FREE ALL STRINGS */
	if (string_table != NULL) {
		if (string_table->next_string != NULL) {
			do {
				curr_string = string_table;
				previous_string = string_table;
				while (curr_string->next_string != NULL) {
					previous_string = curr_string;
					curr_string = curr_string->next_string;
				}
				free(curr_string);
				previous_string->next_string = NULL;
			} while (previous_string != string_table);
		}

		free(string_table);
		string_table = NULL;
	}

	/* FREE ALL ATTRIBUTES */
	if (attribute_table != NULL) {
		if (attribute_table->next_attribute != NULL) {
			do {
				current_attribute = attribute_table;
				previous_attribute = attribute_table;
				while (current_attribute->next_attribute != NULL) {
					previous_attribute = current_attribute;
					current_attribute = current_attribute->next_attribute;
				}
				free(current_attribute);
				previous_attribute->next_attribute = NULL;
			} while (previous_attribute != attribute_table);
		}

		free(attribute_table);
		attribute_table = NULL;
	}

	/* FREE ALL CONSTANTS */
	if (cinteger_table != NULL) {
		if (cinteger_table->next_cinteger != NULL) {
			do {
				current_cinteger = cinteger_table;
				previous_cinteger = cinteger_table;
				while (current_cinteger->next_cinteger != NULL) {
					previous_cinteger = current_cinteger;
					current_cinteger = current_cinteger->next_cinteger;
				}
				free(current_cinteger);
				previous_cinteger->next_cinteger = NULL;
			} while (previous_cinteger != cinteger_table);
		}

		free(cinteger_table);
		cinteger_table = NULL;
	}

	if (cstring_table != NULL) {
		if (cstring_table->next_string != NULL) {
			do {
				curr_string = cstring_table;
				previous_string = cstring_table;
				while (curr_string->next_string != NULL) {
					previous_string = curr_string;
					curr_string = curr_string->next_string;
				}
				free(curr_string);
				previous_string->next_string = NULL;
			} while (previous_string != cstring_table);
		}

		free(cstring_table);
		cstring_table = NULL;
	}

	/* FREE ALL SYNONYMS */
	if (synonym_table != NULL) {
		if (synonym_table->next_synonym != NULL) {
			do {
				current_synonym = synonym_table;
				previous_synonym = synonym_table;
				while (current_synonym->next_synonym != NULL) {
					previous_synonym = current_synonym;
					current_synonym = current_synonym->next_synonym;
				}
				free(current_synonym);
				previous_synonym->next_synonym = NULL;
			} while (previous_synonym != synonym_table);
		}
		free(synonym_table);
		synonym_table = NULL;
	}

	free_from(grammar_table);
	grammar_table = NULL;

	read_gamefile();
}

void free_from(struct word_type *x) {
	if (x) {
		free_from(x->first_child);
		free_from(x->next_sibling);
		free(x);
	}
}

void set_defaults() {
	/* RESET THE BACK-REFERENCE VARIABLES */
	them[0] = 0;
	it = 0;
	her = 0;
	him = 0;
}

void create_cinteger(const char *name, int value) {
	struct cinteger_type *new_cinteger = NULL;

	if ((new_cinteger = (struct cinteger_type *)
	                    malloc(sizeof(struct cinteger_type))) == NULL) {
		outofmem();
	} else {
		if (cinteger_table == NULL) {
			cinteger_table = new_cinteger;
		} else {
			current_cinteger->next_cinteger = new_cinteger;
		}

		current_cinteger = new_cinteger;
		strncpy(current_cinteger->name, name, 40);
		current_cinteger->name[40] = 0;
		current_cinteger->value = value;
		current_cinteger->next_cinteger = NULL;
	}
}

void create_integer(const char *name, int value) {
	struct integer_type *new_integer = NULL;

	if ((new_integer = (struct integer_type *)
	                   malloc(sizeof(struct integer_type))) == NULL) {
		outofmem();
	} else {
		/* KEEP A COUNT OF HOW MANY INTEGERS ARE DEFINED TO
		 * VALIDATE SAVED GAMES */
		integers++;

		if (integer_table == NULL) {
			integer_table = new_integer;
		} else {
			current_integer->next_integer = new_integer;
		}
		current_integer = new_integer;
		strncpy(current_integer->name, name, 40);
		current_integer->name[40] = 0;
		current_integer->value = value;
		current_integer->next_integer = NULL;
	}
}

void create_string(const char *name, const char *value) {
	struct string_type *new_string = NULL;

	if ((new_string = (struct string_type *)
	                  malloc(sizeof(struct string_type))) == NULL) {
		outofmem();
	} else {
		/* KEEP A COUNT OF HOW MANY STRINGS ARE DEFINED TO
		 * VALIDATE SAVED GAMES */
		strings++;

		if (string_table == NULL) {
			string_table = new_string;
		} else {
			current_string->next_string = new_string;
		}
		current_string = new_string;
		strncpy(current_string->name, name, 40);
		current_string->name[40] = 0;

		if (value != NULL) {
			strncpy(current_string->value, value, 255);
		} else {
			/* IF NO VALUE IS SUPPLIED, JUST NULL-TERMINATE
			 * THE STRING */
			current_string->value[0] = 0;
		}

		current_string->value[255] = 0;
		current_string->next_string = NULL;
	}
}

void create_cstring(const char *name, const char *value) {
	struct string_type *new_string = NULL;

	if ((new_string = (struct string_type *)
	                  malloc(sizeof(struct string_type))) == NULL) {
		outofmem();
	} else {
		if (cstring_table == NULL) {
			cstring_table = new_string;
		} else {
			current_cstring->next_string = new_string;
		}
		current_cstring = new_string;
		strncpy(current_cstring->name, name, 40);
		current_cstring->name[40] = 0;

		if (value != NULL) {
			strncpy(current_cstring->value, value, 255);
		} else {
			/* IF NO VALUE IS SUPPLIED, JUST NULL-TERMINATE
			 * THE STRING */
			current_cstring->value[0] = 0;
		}

		current_cstring->value[255] = 0;
		current_cstring->next_string = NULL;
	}
}

void create_language_constants() {
	/* SET THE DEFAULT LANGUAGE CONSTANTS IF ANY OR ALL OF THEM
	 * ARE MISSING FROM THE GAME THAT IS BEING LOADED. DEFAULT
	 * TO THE NATIVE_LANGUAGE SETTING IN language.h */

	if (cstring_resolve("COMMENT_IGNORED") == NULL)
		create_cstring("COMMENT_IGNORED", COMMENT_IGNORED);
	if (cstring_resolve("COMMENT_RECORDED") == NULL)
		create_cstring("COMMENT_RECORDED", COMMENT_RECORDED);
	if (cstring_resolve("YES_WORD") == NULL)
		create_cstring("YES_WORD", YES_WORD);
	if (cstring_resolve("NO_WORD") == NULL)
		create_cstring("NO_WORD", NO_WORD);
	if (cstring_resolve("YES_OR_NO") == NULL)
		create_cstring("YES_OR_NO", YES_OR_NO);
	if (cstring_resolve("INVALID_SELECTION") == NULL)
		create_cstring("INVALID_SELECTION", INVALID_SELECTION);
	if (cstring_resolve("RESTARTING") == NULL)
		create_cstring("RESTARTING", RESTARTING);
	if (cstring_resolve("RETURN_GAME") == NULL)
		create_cstring("RETURN_GAME", RETURN_GAME);
	if (cstring_resolve("SCRIPTING_ON") == NULL)
		create_cstring("SCRIPTING_ON", SCRIPTING_ON);
	if (cstring_resolve("SCRIPTING_OFF") == NULL)
		create_cstring("SCRIPTING_OFF", SCRIPTING_OFF);
	if (cstring_resolve("SCRIPTING_ALREADY_OFF") == NULL)
		create_cstring("SCRIPTING_ALREADY_OFF", SCRIPTING_ALREADY_OFF);
	if (cstring_resolve("SCRIPTING_ALREADY_ON") == NULL)
		create_cstring("SCRIPTING_ALREADY_ON", SCRIPTING_ALREADY_ON);
	if (cstring_resolve("CANT_WRITE_SCRIPT") == NULL)
		create_cstring("CANT_WRITE_SCRIPT", CANT_WRITE_SCRIPT);
	if (cstring_resolve("ERROR_READING_WALKTHRU") == NULL)
		create_cstring("ERROR_READING_WALKTHRU", ERROR_READING_WALKTHRU);
	if (cstring_resolve("BAD_OOPS") == NULL)
		create_cstring("BAD_OOPS", BAD_OOPS);
	if (cstring_resolve("CANT_CORRECT") == NULL)
		create_cstring("CANT_CORRECT", CANT_CORRECT);
	if (cstring_resolve("SURE_QUIT") == NULL)
		create_cstring("SURE_QUIT", SURE_QUIT);
	if (cstring_resolve("SURE_RESTART") == NULL)
		create_cstring("SURE_RESTART", SURE_RESTART);
	if (cstring_resolve("NOT_CLEVER") == NULL)
		create_cstring("NOT_CLEVER", NOT_CLEVER);
	if (cstring_resolve("NO_MOVES") == NULL)
		create_cstring("NO_MOVES", NO_MOVES);
	if (cstring_resolve("TYPE_NUMBER") == NULL)
		create_cstring("TYPE_NUMBER", TYPE_NUMBER);
	if (cstring_resolve("BY") == NULL)
		create_cstring("BY", BY);
	if (cstring_resolve("REFERRING_TO") == NULL)
		create_cstring("REFERRING_TO", REFERRING_TO);
	if (cstring_resolve("WALKTHRU_WORD") == NULL)
		create_cstring("WALKTHRU_WORD", WALKTHRU_WORD);
	if (cstring_resolve("INFO_WORD") == NULL)
		create_cstring("INFO_WORD", INFO_WORD);
	if (cstring_resolve("RESTART_WORD") == NULL)
		create_cstring("RESTART_WORD", RESTART_WORD);
	if (cstring_resolve("AGAIN_WORD") == NULL)
		create_cstring("AGAIN_WORD", AGAIN_WORD);
	if (cstring_resolve("SCRIPT_WORD") == NULL)
		create_cstring("SCRIPT_WORD", SCRIPT_WORD);
	if (cstring_resolve("UNSCRIPT_WORD") == NULL)
		create_cstring("UNSCRIPT_WORD", UNSCRIPT_WORD);
	if (cstring_resolve("QUIT_WORD") == NULL)
		create_cstring("QUIT_WORD", QUIT_WORD);
	if (cstring_resolve("UNDO_WORD") == NULL)
		create_cstring("UNDO_WORD", UNDO_WORD);
	if (cstring_resolve("OOPS_WORD") == NULL)
		create_cstring("OOPS_WORD", OOPS_WORD);
	if (cstring_resolve("FROM_WORD") == NULL)
		create_cstring("FROM_WORD", FROM_WORD);
	if (cstring_resolve("EXCEPT_WORD") == NULL)
		create_cstring("EXCEPT_WORD", EXCEPT_WORD);
	if (cstring_resolve("FOR_WORD") == NULL)
		create_cstring("FOR_WORD", FOR_WORD);
	if (cstring_resolve("BUT_WORD") == NULL)
		create_cstring("BUT_WORD", BUT_WORD);
	if (cstring_resolve("AND_WORD") == NULL)
		create_cstring("AND_WORD", AND_WORD);
	if (cstring_resolve("THEN_WORD") == NULL)
		create_cstring("THEN_WORD", THEN_WORD);
	if (cstring_resolve("OF_WORD") == NULL)
		create_cstring("OF_WORD", OF_WORD);
	if (cstring_resolve("SHE_WORD") == NULL)
		create_cstring("SHE_WORD", SHE_WORD);
	if (cstring_resolve("HE_WORD") == NULL)
		create_cstring("HE_WORD", HE_WORD);
	if (cstring_resolve("THAT_WORD") == NULL)
		create_cstring("THAT_WORD", THAT_WORD);
	if (cstring_resolve("THEM_WORD") == NULL)
		create_cstring("THEM_WORD", THEM_WORD);
	if (cstring_resolve("THOSE_WORD") == NULL)
		create_cstring("THOSE_WORD", THOSE_WORD);
	if (cstring_resolve("THEY_WORD") == NULL)
		create_cstring("THEY_WORD", THEY_WORD);
	if (cstring_resolve("IT_WORD") == NULL)
		create_cstring("IT_WORD", IT_WORD);
	if (cstring_resolve("ITSELF_WORD") == NULL)
		create_cstring("ITSELF_WORD", ITSELF_WORD);
	if (cstring_resolve("HIM_WORD") == NULL)
		create_cstring("HIM_WORD", HIM_WORD);
	if (cstring_resolve("HIMSELF_WORD") == NULL)
		create_cstring("HIMSELF_WORD", HIMSELF_WORD);
	if (cstring_resolve("HER_WORD") == NULL)
		create_cstring("HER_WORD", HER_WORD);
	if (cstring_resolve("HERSELF_WORD") == NULL)
		create_cstring("HERSELF_WORD", HERSELF_WORD);
	if (cstring_resolve("THEMSELVES_WORD") == NULL)
		create_cstring("THEMSELVES_WORD", THEMSELVES_WORD);
	if (cstring_resolve("YOU_WORD") == NULL)
		create_cstring("YOU_WORD", YOU_WORD);
	if (cstring_resolve("YOURSELF_WORD") == NULL)
		create_cstring("YOURSELF_WORD", YOURSELF_WORD);
	if (cstring_resolve("ONES_WORD") == NULL)
		create_cstring("ONES_WORD", ONES_WORD);
	if (cstring_resolve("NO_MULTI_VERB") == NULL)
		create_cstring("NO_MULTI_VERB", NO_MULTI_VERB);
	if (cstring_resolve("NO_MULTI_START") == NULL)
		create_cstring("NO_MULTI_START", NO_MULTI_START);
	if (cstring_resolve("PERSON_CONCEALING") == NULL)
		create_cstring("PERSON_CONCEALING", PERSON_CONCEALING);
	if (cstring_resolve("PERSON_POSSESSIVE") == NULL)
		create_cstring("PERSON_POSSESSIVE", PERSON_POSSESSIVE);
	if (cstring_resolve("CONTAINER_CLOSED") == NULL)
		create_cstring("CONTAINER_CLOSED", CONTAINER_CLOSED);
	if (cstring_resolve("CONTAINER_CLOSED_FEM") == NULL)
		create_cstring("CONTAINER_CLOSED_FEM", CONTAINER_CLOSED_FEM);
	if (cstring_resolve("FROM_NON_CONTAINER") == NULL)
		create_cstring("FROM_NON_CONTAINER", FROM_NON_CONTAINER);
	if (cstring_resolve("DOUBLE_EXCEPT") == NULL)
		create_cstring("DOUBLE_EXCEPT", DOUBLE_EXCEPT);
	if (cstring_resolve("NONE_HELD") == NULL)
		create_cstring("NONE_HELD", NONE_HELD);
	if (cstring_resolve("NO_OBJECTS") == NULL)
		create_cstring("NO_OBJECTS", NO_OBJECTS);
	if (cstring_resolve("NO_FILENAME") == NULL)
		create_cstring("NO_FILENAME", NO_FILENAME);
	if (cstring_resolve("MOVE_UNDONE") == NULL)
		create_cstring("MOVE_UNDONE", MOVE_UNDONE);
	if (cstring_resolve("NO_UNDO") == NULL)
		create_cstring("NO_UNDO", NO_UNDO);
	if (cstring_resolve("CANT_SAVE") == NULL)
		create_cstring("CANT_SAVE", CANT_SAVE);
	if (cstring_resolve("CANT_RESTORE") == NULL)
		create_cstring("CANT_RESTORE", CANT_RESTORE);
	if (cstring_resolve("GAME_SAVED") == NULL)
		create_cstring("GAME_SAVED", GAME_SAVED);
	if (cstring_resolve("INCOMPLETE_SENTENCE") == NULL)
		create_cstring("INCOMPLETE_SENTENCE", INCOMPLETE_SENTENCE);
	if (cstring_resolve("UNKNOWN_OBJECT") == NULL)
		create_cstring("UNKNOWN_OBJECT", UNKNOWN_OBJECT);
	if (cstring_resolve("UNKNOWN_OBJECT_END") == NULL)
		create_cstring("UNKNOWN_OBJECT_END", UNKNOWN_OBJECT_END);
	if (cstring_resolve("CANT_USE_WORD") == NULL)
		create_cstring("CANT_USE_WORD", CANT_USE_WORD);
	if (cstring_resolve("IN_CONTEXT") == NULL)
		create_cstring("IN_CONTEXT", IN_CONTEXT);
	if (cstring_resolve("DONT_SEE") == NULL)
		create_cstring("DONT_SEE", DONT_SEE);
	if (cstring_resolve("HERE_WORD") == NULL)
		create_cstring("HERE_WORD", HERE_WORD);
	if (cstring_resolve("BAD_SAVED_GAME") == NULL)
		create_cstring("BAD_SAVED_GAME", BAD_SAVED_GAME);
	if (cstring_resolve("ARENT") == NULL)
		create_cstring("ARENT", ARENT);
	if (cstring_resolve("ISNT") == NULL)
		create_cstring("ISNT", ISNT);
	if (cstring_resolve("ARE") == NULL)
		create_cstring("ARE", ARE);
	if (cstring_resolve("IS") == NULL)
		create_cstring("IS", IS);
	if (cstring_resolve("DONT") == NULL)
		create_cstring("DONT", DONT);
	if (cstring_resolve("DOESNT") == NULL)
		create_cstring("DOESNT", DOESNT);
	if (cstring_resolve("DO") == NULL)
		create_cstring("DO", DO);
	if (cstring_resolve("DOES") == NULL)
		create_cstring("DOES", DOES);
	if (cstring_resolve("SCORE_UP") == NULL)
		create_cstring("SCORE_UP", SCORE_UP);
	if (cstring_resolve("POINT") == NULL)
		create_cstring("POINT", POINT);
	if (cstring_resolve("POINTS") == NULL)
		create_cstring("POINTS", POINTS);
	if (cstring_resolve("STARTING") == NULL)
		create_cstring("STARTING", STARTING);
	if (cstring_resolve("NO_IT") == NULL)
		create_cstring("NO_IT", NO_IT);
	if (cstring_resolve("NO_IT_END") == NULL)
		create_cstring("NO_IT_END", NO_IT_END);
	if (cstring_resolve("BACK_REFERENCE") == NULL)
		create_cstring("BACK_REFERENCE", BACK_REFERENCE);
	if (cstring_resolve("BACK_REFERENCE_END") == NULL)
		create_cstring("BACK_REFERENCE_END", BACK_REFERENCE_END);
	if (cstring_resolve("WHEN_YOU_SAY") == NULL)
		create_cstring("WHEN_YOU_SAY", WHEN_YOU_SAY);
	if (cstring_resolve("MUST_SPECIFY") == NULL)
		create_cstring("MUST_SPECIFY", MUST_SPECIFY);
	if (cstring_resolve("OR_WORD") == NULL)
		create_cstring("OR_WORD", OR_WORD);
}

} // End of namespace JACL
} // End of namespace Glk
