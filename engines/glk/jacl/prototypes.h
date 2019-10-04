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

namespace Glk {
namespace JACL {

#ifdef GLK
strid_t open_glk_file(uint usage, uint mode, char *filename);
glui32 glk_get_bin_line_stream(strid_t file_stream, char *buffer, glui32 max_length);
glui32 parse_utf8(unsigned char *buf, glui32 buflen, glui32 *out, glui32 outlen);
void convert_to_utf8(glui32 *text, int len);
glui32 parse_utf8(unsigned char *buf, glui32 buflen, glui32 *out, glui32 outlen);
#else
void update_parameters();
#endif

int bearing(double x1, double y1, double x2, double y2);
int distance(double x1, double y1, double x2, double y2);
int strcondition();
int and_strcondition();
int logic_test(int first);
int str_test(int first);
int first_available(int list_number);
int validate(char *string);
int noun_resolve(struct word_type *scope_word, int finding_from, int noun_number);
int get_from_object(struct word_type *scope_word, int noun_number);
int is_direct_child_of_from(int child);
int is_child_of_from(int child);
int verify_from_object(int from_object);
int find_parent(int index);
int scope(int index, char *expected, int restricted = 0);
int object_element_resolve(char *textstring);
int execute(char *funcname);
int object_resolve(char object_string[]);
int random_number();
void log_error(char *message, int console);
int parent_of(int parent, int child, int restricted);
int grand_of(int child, int objs_only);
int check_light(int where);
int find_route(int fromRoom, int toRoom, int known);
int exit_function(int return_code);
int count_resolve(char *textstring);
void jacl_set_window(winid_t new_window);
void create_cstring(char *name, char *value);
void create_string(char *name, char *value);
void create_integer(char *name, int value);
void create_cinteger(char *name, int value);
void scripting();
void undoing();
void walking_thru();
void create_paths(char *full_path);
int get_key();
char get_character(char *message);
int get_yes_or_no();
void get_string(char *string_buffer);
int get_number(int insist, int low, int high);
int save_interaction(char *filename);
int restore_interaction(char *filename);
void jacl_encrypt(char *string);
void jacl_decrypt(char *string);
void log_message(char *message, int console);
void set_them(int noun_number);
void preparse();
void inspect(int object_num);
void add_all(struct word_type *scope_word, int noun_number);
void add_to_list(int noun_number, int resolved_object);
void call_functions(char *base_name);
int build_object_list(struct word_type *scope_word, int noun_number);
long value_of(char *value, int run_time = 0);
long attribute_resolve(char *attribute);
long user_attribute_resolve(char *name);
struct word_type *exact_match(struct word_type *pointer);
struct word_type *object_match(struct word_type *iterator, int noun_number);
struct integer_type *integer_resolve(char *name);
struct integer_type *integer_resolve_indexed(char *name, int index);
struct function_type *function_resolve(char *name);
struct string_type *string_resolve(char *name);
struct string_type *string_resolve_indexed(char *name, int index);
struct string_type *cstring_resolve(char *name);
struct string_type *cstring_resolve_indexed(char *name, int index);
struct cinteger_type *cinteger_resolve(char *name);
struct cinteger_type *cinteger_resolve_indexed(char *name, int index);
int array_length_resolve(char *textstring);
int legal_label_check(char *word, int line, int type);
char *object_names(int object_index, char *names_buffer);
char *arg_text_of(char *string);
char *arg_text_of_word(int wordnumber);
char *var_text_of_word(int wordnumber);
char *text_of(char *string);
char *text_of_word(int wordnumber);
char *expand_function(char *name);
int *container_resolve(char *container_name);
int condition();
int and_condition();
void free_from(struct word_type *x);
void eachturn();
int jacl_whitespace(char character);
int get_here();
char *stripwhite(char *string);
void command_encapsulate();
void encapsulate();
void jacl_truncate();
void parser();
void diagnose();
void look_around();
char *macro_resolve(char *textstring);
char *plain_output(int index, int capital);
char *sub_output(int index, int capital);
char *obj_output(int index, int capital);
char *that_output(int index, int capital);
char *sentence_output(int index, int capital);
char *isnt_output(int index, bool unused = false);
char *is_output(int index, bool unused = false);
char *it_output(int index, bool unused = false);
char *doesnt_output(int index, bool unused = false);
char *does_output(int index, bool unused = false);
char *list_output(int index, int capital);
char *long_output(int index);
void terminate(int code);
void set_arguments(char *function_call);
void pop_stack();
void push_stack(int32 file_pointer);
void pop_proxy();
void push_proxy();
void write_text(char *string_buffer);
void status_line();
void newline();
int  save_game(frefid_t saveref);
int  restore_game(frefid_t saveref, int warn);
void write_integer(strid_t stream, int x);
int  read_integer(strid_t stream);
void write_long(strid_t stream, long x);
long read_long(strid_t stream);
void save_game_state();
void restore_game_state();
void add_cstring(char *name, char *value);
void clear_cstring(char *name);
void add_cinteger(char *name, int value);
void clear_cinteger(char *name);
void restart_game();
void read_gamefile();
void new_position(double x1, double y1, double bearing, double velocity);
void build_grammar_table(struct word_type *pointer);
void unkvalerr(int line, int wordno);
void totalerrs(int errors);
void unkatterr(int line, int wordno);
void unkfunrun(char *name);
void nofnamerr(int line);
void nongloberr(int line);
void unkkeyerr(int line, int wordno);
void maxatterr(int line, int wordno);
void unkattrun(int wordno);
void badptrrun(char *name, int value);
void badplrrun(int value);
void badparrun();
void notintrun();
void noproprun(int unused = 0);
void noproperr(int line);
void noobjerr(int line);
void unkobjerr(int line, int wordno);
void unkobjrun(int wordno);
void unkdirrun(int wordno);
void unkscorun(char *scope);
void unkstrrun(char *variable);
void unkvarrun(char *variable);
void outofmem();
void set_defaults();
void no_it();
void more(char *message);
int jpp();
int process_file(char *sourceFile1, char *sourceFile2);
char *strip_return(char *string);
char *object_generator(char *text, int state);
char *verb_generator(char *text, int state);
void add_word(char *word);
void create_language_constants();
int select_next();
void jacl_sleep(unsigned int mseconds);

} // End of namespace JACL
} // End of namespace Glk
