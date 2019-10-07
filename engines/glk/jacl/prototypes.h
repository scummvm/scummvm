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
extern strid_t open_glk_file(uint usage, uint mode, char *filename);
extern glui32 glk_get_bin_line_stream(strid_t file_stream, char *buffer, glui32 max_length);
extern glui32 parse_utf8(unsigned char *buf, glui32 buflen, glui32 *out, glui32 outlen);
extern void convert_to_utf8(glui32 *text, int len);
extern glui32 parse_utf8(unsigned char *buf, glui32 buflen, glui32 *out, glui32 outlen);
#else
extern void update_parameters();
#endif

extern int bearing(double x1, double y1, double x2, double y2);
extern int distance(double x1, double y1, double x2, double y2);
extern int strcondition();
extern int and_strcondition();
extern int logic_test(int first);
extern int str_test(int first);
extern int first_available(int list_number);
extern int validate(const char *string);
extern int noun_resolve(struct word_type *scope_word, int finding_from, int noun_number);
extern int get_from_object(struct word_type *scope_word, int noun_number);
extern int is_direct_child_of_from(int child);
extern int is_child_of_from(int child);
extern int verify_from_object(int from_object);
extern int find_parent(int index);
extern int scope(int index, const char *expected, int restricted = 0);
extern int object_element_resolve(const char *textstring);
extern int execute(const char *funcname);
extern int object_resolve(const char *object_string);
extern int random_number();
extern void log_error(const char *message, int console);
extern int parent_of(int parent_, int child, int restricted);
extern int grand_of(int child, int objs_only);
extern int check_light(int where);
extern int find_route(int fromRoom, int toRoom, int known);
extern int exit_function(int return_code);
extern int count_resolve(const char *textstring);
extern void jacl_set_window(winid_t new_window);
extern void create_cstring(const char *name, const char *value);
extern void create_string(const char *name, const char *value);
extern void create_integer(const char *name, int value);
extern void create_cinteger(const char *name, int value);
extern void scripting();
extern void undoing();
extern void walking_thru();
extern void create_paths(char *full_path);
extern int get_key();
extern char get_character(const char *message);
extern int get_yes_or_no();
extern void get_string(char *string_buffer);
extern int get_number(int insist, int low, int high);
extern int save_interaction();
extern int restore_interaction();
extern void jacl_encrypt(char *string);
extern void jacl_decrypt(char *string);
//extern void log_message(const char *message, int console);
extern void set_them(int noun_number);
extern void preparse();
extern void inspect(int object_num);
extern void add_all(struct word_type *scope_word, int noun_number);
extern void add_to_list(int noun_number, int resolved_object);
extern void call_functions(const char *base_name);
extern int build_object_list(struct word_type *scope_word, int noun_number);
extern long value_of(const char *value, int run_time = 0);
extern long attribute_resolve(const char *attribute);
extern long user_attribute_resolve(const char *name);
extern struct word_type *exact_match(struct word_type *pointer);
extern struct word_type *object_match(struct word_type *iterator, int noun_number);
extern struct integer_type *integer_resolve(const char *name);
extern struct integer_type *integer_resolve_indexed(const char *name, int index);
extern struct function_type *function_resolve(const char *name);
extern struct string_type *string_resolve(const char *name);
extern struct string_type *string_resolve_indexed(const char *name, int index);
extern struct string_type *cstring_resolve(const char *name);
extern struct string_type *cstring_resolve_indexed(const char *name, int index);
extern struct cinteger_type *cinteger_resolve(const char *name);
extern struct cinteger_type *cinteger_resolve_indexed(const char *name, int index);
extern int array_length_resolve(const char *textstring);
extern int legal_label_check(const char *word, int line, int type);
extern char *object_names(int object_index, char *names_buffer);
extern const char *arg_text_of(const char *string);
extern const char *arg_text_of_word(int wordnumber);
extern const char *var_text_of_word(int wordnumber);
extern const char *text_of(const char *string);
extern const char *text_of_word(int wordnumber);
extern const char *expand_function(const char *name);
extern int *container_resolve(const char *container_name);
extern int condition();
extern int and_condition();
extern void free_from(struct word_type *x);
extern void eachturn();
extern int jacl_whitespace(char character);
extern int get_here();
extern char *stripwhite(char *string);
extern void command_encapsulate();
extern void encapsulate();
extern void jacl_truncate();
extern void parser();
extern void diagnose();
extern void look_around();
extern char *macro_resolve(const char *textstring);
extern char *plain_output(int index, int capital);
extern char *sub_output(int index, int capital);
extern char *obj_output(int index, int capital);
extern char *that_output(int index, int capital);
extern char *sentence_output(int index, int capital);
extern char *isnt_output(int index, bool unused = false);
extern char *is_output(int index, bool unused = false);
extern char *it_output(int index, bool unused = false);
extern char *doesnt_output(int index, bool unused = false);
extern char *does_output(int index, bool unused = false);
extern char *list_output(int index, int capital);
extern char *long_output(int index);
extern void terminate(int code);
extern void set_arguments(const char *function_call);
extern void pop_stack();
extern void push_stack(int32 file_pointer);
extern void pop_proxy();
extern void push_proxy();
extern void write_text(const char *string_buffer);
extern void status_line();
extern void newline();
extern bool save_game(strid_t save);
extern bool restore_game(strid_t save, bool warn = false);
extern void write_integer(strid_t stream, int x);
extern int  read_integer(strid_t stream);
extern void write_long(strid_t stream, long x);
extern long read_long(strid_t stream);
extern void save_game_state();
extern void restore_game_state();
extern void add_cstring(const char *name, const char *value);
extern void clear_cstring(const char *name);
extern void add_cinteger(const char *name, int value);
extern void clear_cinteger(const char *name);
extern void restart_game();
extern void read_gamefile();
extern void new_position(double x1, double y1, double bearing, double velocity);
extern void build_grammar_table(struct word_type *pointer);
extern void unkvalerr(int line, int wordno);
extern void totalerrs(int errors);
extern void unkatterr(int line, int wordno);
extern void unkfunrun(const char *name);
extern void nofnamerr(int line);
extern void nongloberr(int line);
extern void unkkeyerr(int line, int wordno);
extern void maxatterr(int line, int wordno);
extern void unkattrun(int wordno);
extern void badptrrun(const char *name, int value);
extern void badplrrun(int value);
extern void badparrun();
extern void notintrun();
extern void noproprun(int unused = 0);
extern void noproperr(int line);
extern void noobjerr(int line);
extern void unkobjerr(int line, int wordno);
extern void unkobjrun(int wordno);
extern void unkdirrun(int wordno);
extern void unkscorun(const char *scope);
extern void unkstrrun(const char *variable);
extern void unkvarrun(const char *variable);
extern void outofmem();
extern void set_defaults();
extern void no_it();
extern void more(const char *message);
extern int jpp();
extern int process_file(const char *sourceFile1, char *sourceFile2);
extern char *strip_return(char *string);
extern const char *object_generator(const char *text, int state);
extern const char *verb_generator(const char *text, int state);
extern void add_word(const char *word);
extern void create_language_constants();
extern int select_next();
extern void jacl_sleep(unsigned int mseconds);

} // End of namespace JACL
} // End of namespace Glk
