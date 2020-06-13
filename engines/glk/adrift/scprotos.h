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

#include "glk/adrift/scare.h"
#include "glk/jumps.h"

namespace Glk {
namespace Adrift {

#ifndef ADRIFT_PROTOTYPES_H
#define ADRIFT_PROTOTYPES_H

/* Runtime version and emulated version, for %version% variable and so on. */
#ifndef SCARE_VERSION
# define SCARE_VERSION "1.3.10"
#endif
#ifndef SCARE_PATCH_LEVEL
# define SCARE_PATCH_LEVEL ""
#endif
#ifndef SCARE_EMULATION
# define SCARE_EMULATION 4046
#endif

/* True and false, unless already defined. */
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE (!FALSE)
#endif

/* Vartype typedef, supports relaxed typing. */
typedef union {
	sc_int integer;
	sc_bool boolean;
	const sc_char *string;
	sc_char *mutable_string;
	void *voidp;
} sc_vartype_t;

/* Standard reader and writer callback function typedefs. */
typedef sc_int(*sc_read_callbackref_t)(void *, sc_byte *, sc_int);
typedef void (*sc_write_callbackref_t)(void *, const sc_byte *, sc_int);

/*
 * Small utility and wrapper functions.  For printf wrappers, try to apply
 * gcc printf argument checking; this code is cautious about applying the
 * checks.
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
extern void sc_trace(const sc_char *format, ...)
__attribute__((__format__(__printf__, 1, 2)));
extern void sc_error(const sc_char *format, ...)
__attribute__((__format__(__printf__, 1, 2)));
extern void sc_fatal(const sc_char *format, ...)
__attribute__((__format__(__printf__, 1, 2)));
#else
extern void sc_trace(const sc_char *format, ...);
extern void sc_error(const sc_char *format, ...);
extern void sc_fatal(const sc_char *format, ...);
#endif
extern void *sc_malloc(size_t size);
extern void *sc_realloc(void *pointer, size_t size);
extern void sc_free(void *pointer);
extern void sc_set_congruential_random(void);
extern void sc_set_platform_random(void);
extern sc_bool sc_is_congruential_random(void);
extern void sc_seed_random(sc_uint new_seed);
extern sc_int sc_rand(void);
extern sc_int sc_randomint(sc_int low, sc_int high);
extern sc_bool sc_strempty(const sc_char *string);
extern sc_char *sc_trim_string(sc_char *string);
extern sc_char *sc_normalize_string(sc_char *string);
extern sc_bool sc_compare_word(const sc_char *string,
                               const sc_char *Word, sc_int length);
extern sc_uint sc_hash(const sc_char *string);

/* TAF file reader/decompressor enumerations, opaque typedef and functions. */
enum {
	TAF_VERSION_NONE = 0,
	TAF_VERSION_SAVE = 999,
	TAF_VERSION_500 = 500,
	TAF_VERSION_400 = 400,
	TAF_VERSION_390 = 390,
	TAF_VERSION_380 = 380
};

typedef struct sc_taf_s *sc_tafref_t;
extern void taf_destroy(sc_tafref_t taf);
extern sc_tafref_t taf_create(sc_read_callbackref_t callback, void *opaque);
extern sc_tafref_t taf_create_tas(sc_read_callbackref_t callback,
                                  void *opaque);
extern void taf_first_line(sc_tafref_t taf);
extern const sc_char *taf_next_line(sc_tafref_t taf);
extern sc_bool taf_more_lines(sc_tafref_t taf);
extern sc_int taf_get_game_data_length(sc_tafref_t taf);
extern sc_int taf_get_version(sc_tafref_t taf);
extern sc_bool taf_debug_is_taf_string(sc_tafref_t taf, const void *addr);
extern void taf_debug_dump(sc_tafref_t taf);

/* Properties store enumerations, opaque typedef, and functions. */
enum {
	PROP_KEY_STRING = 's',
	PROP_KEY_INTEGER = 'i'
};
enum {
	PROP_INTEGER = 'I',
	PROP_BOOLEAN = 'B',
	PROP_STRING = 'S'
};

typedef struct sc_prop_set_s *sc_prop_setref_t;
extern sc_prop_setref_t prop_create(const sc_tafref_t taf);
extern void prop_destroy(sc_prop_setref_t bundle);
extern void prop_put(sc_prop_setref_t bundle,
                     const sc_char *format, sc_vartype_t vt_value,
                     const sc_vartype_t vt_key[]);
extern sc_bool prop_get(sc_prop_setref_t bundle,
                        const sc_char *format, sc_vartype_t *vt_value,
                        const sc_vartype_t vt_key[]);
extern void prop_solidify(sc_prop_setref_t bundle);
extern sc_int prop_get_integer(sc_prop_setref_t bundle,
                               const sc_char *format,
                               const sc_vartype_t vt_key[]);
extern sc_bool prop_get_boolean(sc_prop_setref_t bundle,
                                const sc_char *format,
                                const sc_vartype_t vt_key[]);
extern const sc_char *prop_get_string(sc_prop_setref_t bundle,
                                      const sc_char *format,
                                      const sc_vartype_t vt_key[]);
extern sc_int prop_get_child_count(sc_prop_setref_t bundle,
                                   const sc_char *format,
                                   const sc_vartype_t vt_key[]);
extern void prop_adopt(sc_prop_setref_t bundle, void *addr);
extern void prop_debug_trace(sc_bool flag);
extern void prop_debug_dump(sc_prop_setref_t bundle);

/* Game parser enumeration and functions. */
enum {
	ROOMLIST_NO_ROOMS = 0,
	ROOMLIST_ONE_ROOM = 1,
	ROOMLIST_SOME_ROOMS = 2,
	ROOMLIST_ALL_ROOMS = 3,
	ROOMLIST_NPC_PART = 4
};

extern sc_bool parse_game(sc_tafref_t taf, sc_prop_setref_t bundle);
extern void parse_debug_trace(sc_bool flag);

/* Game state structure for modules that use it. */
typedef struct sc_game_s *sc_gameref_t;

/* Hint type definition, a thinly disguised pointer to task entry. */
typedef struct sc_taskstate_s *sc_hintref_t;

/* Variables set enumerations, opaque typedef, and functions. */
enum {
	TAFVAR_NUMERIC = 0,
	TAFVAR_STRING = 1
};
enum {
	VAR_INTEGER = 'I',
	VAR_STRING = 'S'
};

typedef struct sc_var_set_s *sc_var_setref_t;
extern void var_put(sc_var_setref_t vars,
                    const sc_char *name, sc_int _type, sc_vartype_t vt_value);
extern sc_bool var_get(sc_var_setref_t vars,
                       const sc_char *name, sc_int *_type,
                       sc_vartype_t *vt_rvalue);
extern void var_put_integer(sc_var_setref_t vars,
                            const sc_char *name, sc_int value);
extern sc_int var_get_integer(sc_var_setref_t vars, const sc_char *name);
extern void var_put_string(sc_var_setref_t vars,
                           const sc_char *name, const sc_char *string);
extern const sc_char *var_get_string(sc_var_setref_t vars,
                                     const sc_char *name);
extern sc_var_setref_t var_create(sc_prop_setref_t bundle);
extern void var_destroy(sc_var_setref_t vars);
extern void var_register_game(sc_var_setref_t vars, sc_gameref_t game);
extern void var_set_ref_character(sc_var_setref_t vars, sc_int character);
extern void var_set_ref_object(sc_var_setref_t vars, sc_int object);
extern void var_set_ref_number(sc_var_setref_t vars, sc_int number);
extern void var_set_ref_text(sc_var_setref_t vars, const sc_char *text);
extern sc_int var_get_ref_character(sc_var_setref_t vars);
extern sc_int var_get_ref_object(sc_var_setref_t vars);
extern sc_int var_get_ref_number(sc_var_setref_t vars);
extern const sc_char *var_get_ref_text(sc_var_setref_t vars);
extern sc_uint var_get_elapsed_seconds(sc_var_setref_t vars);
extern void var_set_elapsed_seconds(sc_var_setref_t vars, sc_uint seconds);
extern void var_debug_trace(sc_bool flag);
extern void var_debug_dump(sc_var_setref_t vars);

/* Expression evaluation functions. */
extern sc_bool expr_eval_numeric_expression(const sc_char *expression,
        sc_var_setref_t vars,
        sc_int *rvalue);
extern sc_bool expr_eval_string_expression(const sc_char *expression,
        sc_var_setref_t vars,
        sc_char **rvalue);

/* Print filtering opaque typedef and functions. */
typedef struct sc_filter_s *sc_filterref_t;
extern sc_filterref_t pf_create(void);
extern void pf_destroy(sc_filterref_t filter);
extern void pf_buffer_string(sc_filterref_t filter,
                             const sc_char *string);
extern void pf_buffer_character(sc_filterref_t filter,
                                sc_char character);
extern void pf_prepend_string(sc_filterref_t filter,
                              const sc_char *string);
extern void pf_new_sentence(sc_filterref_t filter);
extern void pf_mute(sc_filterref_t filter);
extern void pf_clear_mute(sc_filterref_t filter);
extern void pf_buffer_tag(sc_filterref_t filter, sc_int tag);
extern void pf_strip_tags(sc_char *string);
extern void pf_strip_tags_for_hints(sc_char *string);
extern sc_char *pf_filter(const sc_char *string,
                          sc_var_setref_t vars, sc_prop_setref_t bundle);
extern sc_char *pf_filter_for_info(const sc_char *string,
                                   sc_var_setref_t vars);
extern void pf_flush(sc_filterref_t filter,
                     sc_var_setref_t vars, sc_prop_setref_t bundle);
extern void pf_checkpoint(sc_filterref_t filter,
                          sc_var_setref_t vars, sc_prop_setref_t bundle);
extern const sc_char *pf_get_buffer(sc_filterref_t filter);
extern sc_char *pf_transfer_buffer(sc_filterref_t filter);
extern void pf_empty(sc_filterref_t filter);
extern sc_char *pf_escape(const sc_char *string);
extern sc_char *pf_filter_input(const sc_char *string,
                                sc_prop_setref_t bundle);
extern void pf_debug_trace(sc_bool flag);

/* Game memo opaque typedef and functions. */
typedef struct sc_memo_set_s *sc_memo_setref_t;
extern sc_memo_setref_t memo_create(void);
extern void memo_destroy(sc_memo_setref_t memento);
extern void memo_save_game(sc_memo_setref_t memento, sc_gameref_t game);
extern sc_bool memo_load_game(sc_memo_setref_t memento, sc_gameref_t game);
extern sc_bool memo_is_load_available(sc_memo_setref_t memento);
extern void memo_clear_games(sc_memo_setref_t memento);
extern void memo_save_command(sc_memo_setref_t memento,
                              const sc_char *command, sc_int timestamp,
                              sc_int turns);
extern void memo_unsave_command(sc_memo_setref_t memento);
extern sc_int memo_get_command_count(sc_memo_setref_t memento);
extern void memo_first_command(sc_memo_setref_t memento);
extern void memo_next_command(sc_memo_setref_t memento,
                              const sc_char **command, sc_int *sequence,
                              sc_int *timestamp, sc_int *turns);
extern sc_bool memo_more_commands(sc_memo_setref_t memento);
extern const sc_char *memo_find_command(sc_memo_setref_t memento,
                                        sc_int sequence);
extern void memo_clear_commands(sc_memo_setref_t memento);

/* Game state functions. */
extern sc_gameref_t gs_create(sc_var_setref_t vars, sc_prop_setref_t bundle,
                              sc_filterref_t filter);
extern sc_bool gs_is_game_valid(sc_gameref_t game);
extern void gs_copy(sc_gameref_t to, sc_gameref_t from);
extern void gs_destroy(sc_gameref_t game);

/* Game state accessors and mutators. */
extern void gs_move_player_to_room(sc_gameref_t game, sc_int Room);
extern sc_bool gs_player_in_room(sc_gameref_t game, sc_int room);
extern sc_var_setref_t gs_get_vars(sc_gameref_t gs);
extern sc_prop_setref_t gs_get_bundle(sc_gameref_t gs);
extern sc_filterref_t gs_get_filter(sc_gameref_t gs);
extern sc_memo_setref_t gs_get_memento(sc_gameref_t gs);
extern void gs_set_playerroom(sc_gameref_t gs, sc_int room);
extern void gs_set_playerposition(sc_gameref_t gs, sc_int position);
extern void gs_set_playerparent(sc_gameref_t gs, sc_int parent);
extern sc_int gs_playerroom(sc_gameref_t gs);
extern sc_int gs_playerposition(sc_gameref_t gs);
extern sc_int gs_playerparent(sc_gameref_t gs);
extern sc_int gs_event_count(sc_gameref_t gs);
extern void gs_set_event_state(sc_gameref_t gs, sc_int event, sc_int state);
extern void gs_set_event_time(sc_gameref_t gs, sc_int event, sc_int etime);
extern sc_int gs_event_state(sc_gameref_t gs, sc_int event);
extern sc_int gs_event_time(sc_gameref_t gs, sc_int event);
extern void gs_decrement_event_time(sc_gameref_t gs, sc_int event);
extern sc_int gs_room_count(sc_gameref_t gs);
extern void gs_set_room_seen(sc_gameref_t gs, sc_int Room, sc_bool seen);
extern sc_bool gs_room_seen(sc_gameref_t gs, sc_int Room);
extern sc_int gs_task_count(sc_gameref_t gs);
extern void gs_set_task_done(sc_gameref_t gs, sc_int task, sc_bool done);
extern void gs_set_task_scored(sc_gameref_t gs, sc_int task, sc_bool scored);
extern sc_bool gs_task_done(sc_gameref_t gs, sc_int task);
extern sc_bool gs_task_scored(sc_gameref_t gs, sc_int task);
extern sc_int gs_object_count(sc_gameref_t gs);
extern void gs_set_object_openness(sc_gameref_t gs,
                                   sc_int object, sc_int openness);
extern void gs_set_object_state(sc_gameref_t gs, sc_int object, sc_int state);
extern void gs_set_object_seen(sc_gameref_t gs, sc_int object, sc_bool seen);
extern void gs_set_object_unmoved(sc_gameref_t gs,
                                  sc_int object, sc_bool unmoved);
extern void gs_set_object_static_unmoved(sc_gameref_t gs,
        sc_int _object, sc_bool unmoved);
extern sc_int gs_object_openness(sc_gameref_t gs, sc_int object);
extern sc_int gs_object_state(sc_gameref_t gs, sc_int _object);
extern sc_bool gs_object_seen(sc_gameref_t gs, sc_int _object);
extern sc_bool gs_object_unmoved(sc_gameref_t gs, sc_int _object);
extern sc_bool gs_object_static_unmoved(sc_gameref_t gs, sc_int _object);
extern sc_int gs_object_position(sc_gameref_t gs, sc_int _object);
extern sc_int gs_object_parent(sc_gameref_t gs, sc_int _object);
extern void gs_object_move_onto(sc_gameref_t gs, sc_int _object, sc_int onto);
extern void gs_object_move_into(sc_gameref_t gs, sc_int _object, sc_int into);
extern void gs_object_make_hidden(sc_gameref_t gs, sc_int _object);
extern void gs_object_player_get(sc_gameref_t gs, sc_int _object);
extern void gs_object_npc_get(sc_gameref_t gs, sc_int object, sc_int npc);
extern void gs_object_player_wear(sc_gameref_t gs, sc_int object);
extern void gs_object_npc_wear(sc_gameref_t gs, sc_int object, sc_int npc);
extern void gs_object_to_room(sc_gameref_t gs, sc_int _object, sc_int room);
extern sc_int gs_npc_count(sc_gameref_t gs);
extern void gs_set_npc_location(sc_gameref_t gs, sc_int npc, sc_int _location);
extern sc_int gs_npc_location(sc_gameref_t gs, sc_int npc);
extern void gs_set_npc_position(sc_gameref_t gs, sc_int npc, sc_int position);
extern sc_int gs_npc_position(sc_gameref_t gs, sc_int npc);
extern void gs_set_npc_parent(sc_gameref_t gs, sc_int npc, sc_int parent);
extern sc_int gs_npc_parent(sc_gameref_t gs, sc_int npc);
extern void gs_set_npc_seen(sc_gameref_t gs, sc_int npc, sc_bool seen);
extern sc_bool gs_npc_seen(sc_gameref_t gs, sc_int npc);
extern sc_int gs_npc_walkstep_count(sc_gameref_t gs, sc_int npc);
extern void gs_set_npc_walkstep(sc_gameref_t gs, sc_int npc,
                                sc_int walk, sc_int walkstep);
extern sc_int gs_npc_walkstep(sc_gameref_t gs, sc_int npc, sc_int walk);
extern void gs_decrement_npc_walkstep(sc_gameref_t gs,
                                      sc_int npc, sc_int walkstep);
extern void gs_clear_npc_references(sc_gameref_t gs);
extern void gs_clear_object_references(sc_gameref_t gs);
extern void gs_set_multiple_references(sc_gameref_t gs);
extern void gs_clear_multiple_references(sc_gameref_t gs);

/* Pattern matching functions. */
extern sc_bool uip_match(const sc_char *pattern,
                         const sc_char *string, sc_gameref_t game);
extern sc_char *uip_replace_pronouns(sc_gameref_t game, const sc_char *string);
extern void uip_assign_pronouns(sc_gameref_t game, const sc_char *string);
extern void uip_debug_trace(sc_bool flag);

/* Library perspective enumeration and functions. */
enum {
	LIB_FIRST_PERSON = 0,
	LIB_SECOND_PERSON = 1,
	LIB_THIRD_PERSON = 2
};

extern void lib_warn_battle_system(void);
extern sc_int lib_random_roomgroup_member(sc_gameref_t game, sc_int roomgroup);
extern const sc_char *lib_get_room_name(sc_gameref_t game, sc_int Room);
extern void lib_print_room_name(sc_gameref_t game, sc_int Room);
extern void lib_print_room_description(sc_gameref_t game, sc_int Room);
extern sc_bool lib_cmd_go_north(sc_gameref_t game);
extern sc_bool lib_cmd_go_east(sc_gameref_t game);
extern sc_bool lib_cmd_go_south(sc_gameref_t game);
extern sc_bool lib_cmd_go_west(sc_gameref_t game);
extern sc_bool lib_cmd_go_up(sc_gameref_t game);
extern sc_bool lib_cmd_go_down(sc_gameref_t game);
extern sc_bool lib_cmd_go_in(sc_gameref_t game);
extern sc_bool lib_cmd_go_out(sc_gameref_t game);
extern sc_bool lib_cmd_go_northeast(sc_gameref_t game);
extern sc_bool lib_cmd_go_southeast(sc_gameref_t game);
extern sc_bool lib_cmd_go_northwest(sc_gameref_t game);
extern sc_bool lib_cmd_go_southwest(sc_gameref_t game);
extern sc_bool lib_cmd_go_room(sc_gameref_t game);
extern sc_bool lib_cmd_verbose(sc_gameref_t game);
extern sc_bool lib_cmd_brief(sc_gameref_t game);
extern sc_bool lib_cmd_notify_on_off(sc_gameref_t game);
extern sc_bool lib_cmd_notify(sc_gameref_t game);
extern sc_bool lib_cmd_time(sc_gameref_t game);
extern sc_bool lib_cmd_date(sc_gameref_t game);
extern sc_bool lib_cmd_quit(sc_gameref_t game);
extern sc_bool lib_cmd_restart(sc_gameref_t game);
extern sc_bool lib_cmd_undo(sc_gameref_t game);
extern sc_bool lib_cmd_history(sc_gameref_t game);
extern sc_bool lib_cmd_history_number(sc_gameref_t game);
extern sc_bool lib_cmd_again(sc_gameref_t game);
extern sc_bool lib_cmd_redo_number(sc_gameref_t game);
extern sc_bool lib_cmd_redo_text(sc_gameref_t game);
extern sc_bool lib_cmd_redo_last(sc_gameref_t game);
extern sc_bool lib_cmd_hints(sc_gameref_t game);
extern sc_bool lib_cmd_help(sc_gameref_t game);
extern sc_bool lib_cmd_license(sc_gameref_t game);
extern sc_bool lib_cmd_information(sc_gameref_t game);
extern sc_bool lib_cmd_clear(sc_gameref_t game);
extern sc_bool lib_cmd_statusline(sc_gameref_t game);
extern sc_bool lib_cmd_version(sc_gameref_t game);
extern sc_bool lib_cmd_look(sc_gameref_t game);
extern sc_bool lib_cmd_print_room_exits(sc_gameref_t game);
extern sc_bool lib_cmd_wait(sc_gameref_t game);
extern sc_bool lib_cmd_wait_number(sc_gameref_t game);
extern sc_bool lib_cmd_examine_self(sc_gameref_t game);
extern sc_bool lib_cmd_examine_npc(sc_gameref_t game);
extern sc_bool lib_cmd_examine_object(sc_gameref_t game);
extern sc_bool lib_cmd_count(sc_gameref_t game);
extern sc_bool lib_cmd_take_all(sc_gameref_t game);
extern sc_bool lib_cmd_take_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_take_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_take_all_from(sc_gameref_t game);
extern sc_bool lib_cmd_take_from_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_take_from_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_take_all_from_npc(sc_gameref_t game);
extern sc_bool lib_cmd_take_from_npc_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_take_from_npc_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_take_npc(sc_gameref_t game);
extern sc_bool lib_cmd_drop_all(sc_gameref_t game);
extern sc_bool lib_cmd_drop_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_drop_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_wear_all(sc_gameref_t game);
extern sc_bool lib_cmd_wear_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_wear_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_remove_all(sc_gameref_t game);
extern sc_bool lib_cmd_remove_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_remove_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_kiss_npc(sc_gameref_t game);
extern sc_bool lib_cmd_kiss_object(sc_gameref_t game);
extern sc_bool lib_cmd_kiss_other(sc_gameref_t game);
extern sc_bool lib_cmd_kill_other(sc_gameref_t game);
extern sc_bool lib_cmd_eat_object(sc_gameref_t game);
extern sc_bool lib_cmd_give_object_npc(sc_gameref_t game);
extern sc_bool lib_cmd_inventory(sc_gameref_t game);
extern sc_bool lib_cmd_open_object(sc_gameref_t game);
extern sc_bool lib_cmd_close_object(sc_gameref_t game);
extern sc_bool lib_cmd_unlock_object_with(sc_gameref_t game);
extern sc_bool lib_cmd_lock_object_with(sc_gameref_t game);
extern sc_bool lib_cmd_unlock_object(sc_gameref_t game);
extern sc_bool lib_cmd_lock_object(sc_gameref_t game);
extern sc_bool lib_cmd_ask_npc_about(sc_gameref_t game);
extern sc_bool lib_cmd_put_all_in(sc_gameref_t game);
extern sc_bool lib_cmd_put_in_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_put_in_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_put_all_on(sc_gameref_t game);
extern sc_bool lib_cmd_put_on_except_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_put_on_multiple(sc_gameref_t game);
extern sc_bool lib_cmd_read_object(sc_gameref_t game);
extern sc_bool lib_cmd_read_other(sc_gameref_t game);
extern sc_bool lib_cmd_stand_on_object(sc_gameref_t game);
extern sc_bool lib_cmd_stand_on_floor(sc_gameref_t game);
extern sc_bool lib_cmd_attack_npc_with(sc_gameref_t game);
extern sc_bool lib_cmd_sit_on_object(sc_gameref_t game);
extern sc_bool lib_cmd_sit_on_floor(sc_gameref_t game);
extern sc_bool lib_cmd_lie_on_object(sc_gameref_t game);
extern sc_bool lib_cmd_lie_on_floor(sc_gameref_t game);
extern sc_bool lib_cmd_get_off_object(sc_gameref_t game);
extern sc_bool lib_cmd_get_off(sc_gameref_t game);
extern sc_bool lib_cmd_save(sc_gameref_t game);
extern sc_bool lib_cmd_restore(sc_gameref_t game);
extern sc_bool lib_cmd_locate_object(sc_gameref_t game);
extern sc_bool lib_cmd_locate_npc(sc_gameref_t game);
extern sc_bool lib_cmd_turns(sc_gameref_t game);
extern sc_bool lib_cmd_score(sc_gameref_t game);
extern sc_bool lib_cmd_get_what(sc_gameref_t game);
extern sc_bool lib_cmd_open_what(sc_gameref_t game);
extern sc_bool lib_cmd_close_other(sc_gameref_t game);
extern sc_bool lib_cmd_lock_other(sc_gameref_t game);
extern sc_bool lib_cmd_lock_what(sc_gameref_t game);
extern sc_bool lib_cmd_unlock_other(sc_gameref_t game);
extern sc_bool lib_cmd_unlock_what(sc_gameref_t game);
extern sc_bool lib_cmd_stand_other(sc_gameref_t game);
extern sc_bool lib_cmd_sit_other(sc_gameref_t game);
extern sc_bool lib_cmd_lie_other(sc_gameref_t game);
extern sc_bool lib_cmd_give_object(sc_gameref_t game);
extern sc_bool lib_cmd_give_what(sc_gameref_t game);
extern sc_bool lib_cmd_remove_what(sc_gameref_t game);
extern sc_bool lib_cmd_drop_what(sc_gameref_t game);
extern sc_bool lib_cmd_wear_what(sc_gameref_t game);
extern sc_bool lib_cmd_profanity(sc_gameref_t game);
extern sc_bool lib_cmd_examine_all(sc_gameref_t game);
extern sc_bool lib_cmd_examine_other(sc_gameref_t game);
extern sc_bool lib_cmd_locate_other(sc_gameref_t game);
extern sc_bool lib_cmd_unix_like(sc_gameref_t game);
extern sc_bool lib_cmd_dos_like(sc_gameref_t game);
extern sc_bool lib_cmd_ask_object(sc_gameref_t game);
extern sc_bool lib_cmd_ask_npc(sc_gameref_t game);
extern sc_bool lib_cmd_ask_other(sc_gameref_t game);
extern sc_bool lib_cmd_block_object(sc_gameref_t game);
extern sc_bool lib_cmd_block_other(sc_gameref_t game);
extern sc_bool lib_cmd_block_what(sc_gameref_t game);
extern sc_bool lib_cmd_break_object(sc_gameref_t game);
extern sc_bool lib_cmd_break_other(sc_gameref_t game);
extern sc_bool lib_cmd_break_what(sc_gameref_t game);
extern sc_bool lib_cmd_destroy_what(sc_gameref_t game);
extern sc_bool lib_cmd_smash_what(sc_gameref_t game);
extern sc_bool lib_cmd_buy_object(sc_gameref_t game);
extern sc_bool lib_cmd_buy_other(sc_gameref_t game);
extern sc_bool lib_cmd_buy_what(sc_gameref_t game);
extern sc_bool lib_cmd_clean_object(sc_gameref_t game);
extern sc_bool lib_cmd_clean_other(sc_gameref_t game);
extern sc_bool lib_cmd_clean_what(sc_gameref_t game);
extern sc_bool lib_cmd_climb_object(sc_gameref_t game);
extern sc_bool lib_cmd_climb_other(sc_gameref_t game);
extern sc_bool lib_cmd_climb_what(sc_gameref_t game);
extern sc_bool lib_cmd_cry(sc_gameref_t game);
extern sc_bool lib_cmd_cut_object(sc_gameref_t game);
extern sc_bool lib_cmd_cut_other(sc_gameref_t game);
extern sc_bool lib_cmd_cut_what(sc_gameref_t game);
extern sc_bool lib_cmd_drink_object(sc_gameref_t game);
extern sc_bool lib_cmd_drink_other(sc_gameref_t game);
extern sc_bool lib_cmd_drink_what(sc_gameref_t game);
extern sc_bool lib_cmd_dance(sc_gameref_t game);
extern sc_bool lib_cmd_eat_other(sc_gameref_t game);
extern sc_bool lib_cmd_feed(sc_gameref_t game);
extern sc_bool lib_cmd_fight(sc_gameref_t game);
extern sc_bool lib_cmd_feel(sc_gameref_t game);
extern sc_bool lib_cmd_fix_object(sc_gameref_t game);
extern sc_bool lib_cmd_fix_other(sc_gameref_t game);
extern sc_bool lib_cmd_fix_what(sc_gameref_t game);
extern sc_bool lib_cmd_fly(sc_gameref_t game);
extern sc_bool lib_cmd_hint(sc_gameref_t game);
extern sc_bool lib_cmd_attack_npc(sc_gameref_t game);
extern sc_bool lib_cmd_hit_object(sc_gameref_t game);
extern sc_bool lib_cmd_hit_other(sc_gameref_t game);
extern sc_bool lib_cmd_hit_what(sc_gameref_t game);
extern sc_bool lib_cmd_hum(sc_gameref_t game);
extern sc_bool lib_cmd_jump(sc_gameref_t game);
extern sc_bool lib_cmd_kick_object(sc_gameref_t game);
extern sc_bool lib_cmd_kick_other(sc_gameref_t game);
extern sc_bool lib_cmd_kick_what(sc_gameref_t game);
extern sc_bool lib_cmd_light_object(sc_gameref_t game);
extern sc_bool lib_cmd_light_other(sc_gameref_t game);
extern sc_bool lib_cmd_light_what(sc_gameref_t game);
extern sc_bool lib_cmd_lift_object(sc_gameref_t game);
extern sc_bool lib_cmd_lift_other(sc_gameref_t game);
extern sc_bool lib_cmd_lift_what(sc_gameref_t game);
extern sc_bool lib_cmd_listen(sc_gameref_t game);
extern sc_bool lib_cmd_mend_object(sc_gameref_t game);
extern sc_bool lib_cmd_mend_other(sc_gameref_t game);
extern sc_bool lib_cmd_mend_what(sc_gameref_t game);
extern sc_bool lib_cmd_move_object(sc_gameref_t game);
extern sc_bool lib_cmd_move_other(sc_gameref_t game);
extern sc_bool lib_cmd_move_what(sc_gameref_t game);
extern sc_bool lib_cmd_please(sc_gameref_t game);
extern sc_bool lib_cmd_press_object(sc_gameref_t game);
extern sc_bool lib_cmd_press_other(sc_gameref_t game);
extern sc_bool lib_cmd_press_what(sc_gameref_t game);
extern sc_bool lib_cmd_pull_object(sc_gameref_t game);
extern sc_bool lib_cmd_pull_other(sc_gameref_t game);
extern sc_bool lib_cmd_pull_what(sc_gameref_t game);
extern sc_bool lib_cmd_punch(sc_gameref_t game);
extern sc_bool lib_cmd_push_object(sc_gameref_t game);
extern sc_bool lib_cmd_push_other(sc_gameref_t game);
extern sc_bool lib_cmd_push_what(sc_gameref_t game);
extern sc_bool lib_cmd_repair_object(sc_gameref_t game);
extern sc_bool lib_cmd_repair_other(sc_gameref_t game);
extern sc_bool lib_cmd_repair_what(sc_gameref_t game);
extern sc_bool lib_cmd_rub_object(sc_gameref_t game);
extern sc_bool lib_cmd_rub_other(sc_gameref_t game);
extern sc_bool lib_cmd_rub_what(sc_gameref_t game);
extern sc_bool lib_cmd_run(sc_gameref_t game);
extern sc_bool lib_cmd_say(sc_gameref_t game);
extern sc_bool lib_cmd_sell_object(sc_gameref_t game);
extern sc_bool lib_cmd_sell_other(sc_gameref_t game);
extern sc_bool lib_cmd_sell_what(sc_gameref_t game);
extern sc_bool lib_cmd_shake_object(sc_gameref_t game);
extern sc_bool lib_cmd_shake_npc(sc_gameref_t game);
extern sc_bool lib_cmd_shake_other(sc_gameref_t game);
extern sc_bool lib_cmd_shake_what(sc_gameref_t game);
extern sc_bool lib_cmd_shout(sc_gameref_t game);
extern sc_bool lib_cmd_sing(sc_gameref_t game);
extern sc_bool lib_cmd_sleep(sc_gameref_t game);
extern sc_bool lib_cmd_smell_object(sc_gameref_t game);
extern sc_bool lib_cmd_smell_other(sc_gameref_t game);
extern sc_bool lib_cmd_stop_object(sc_gameref_t game);
extern sc_bool lib_cmd_stop_other(sc_gameref_t game);
extern sc_bool lib_cmd_stop_what(sc_gameref_t game);
extern sc_bool lib_cmd_suck_object(sc_gameref_t game);
extern sc_bool lib_cmd_suck_other(sc_gameref_t game);
extern sc_bool lib_cmd_suck_what(sc_gameref_t game);
extern sc_bool lib_cmd_talk(sc_gameref_t game);
extern sc_bool lib_cmd_thank(sc_gameref_t game);
extern sc_bool lib_cmd_touch_object(sc_gameref_t game);
extern sc_bool lib_cmd_touch_other(sc_gameref_t game);
extern sc_bool lib_cmd_touch_what(sc_gameref_t game);
extern sc_bool lib_cmd_turn_object(sc_gameref_t game);
extern sc_bool lib_cmd_turn_other(sc_gameref_t game);
extern sc_bool lib_cmd_turn_what(sc_gameref_t game);
extern sc_bool lib_cmd_unblock_object(sc_gameref_t game);
extern sc_bool lib_cmd_unblock_other(sc_gameref_t game);
extern sc_bool lib_cmd_unblock_what(sc_gameref_t game);
extern sc_bool lib_cmd_wash_object(sc_gameref_t game);
extern sc_bool lib_cmd_wash_other(sc_gameref_t game);
extern sc_bool lib_cmd_wash_what(sc_gameref_t game);
extern sc_bool lib_cmd_whistle(sc_gameref_t game);
extern sc_bool lib_cmd_interrogation(sc_gameref_t game);
extern sc_bool lib_cmd_xyzzy(sc_gameref_t game);
extern sc_bool lib_cmd_egotistic(sc_gameref_t game);
extern sc_bool lib_cmd_yes_or_no(sc_gameref_t game);
extern sc_bool lib_cmd_verb_object(sc_gameref_t game);
extern sc_bool lib_cmd_verb_npc(sc_gameref_t game);
extern void lib_debug_trace(sc_bool flag);

/* Resource opaque typedef and control functions. */
typedef struct sc_resource_s *sc_resourceref_t;
extern sc_bool res_has_sound(sc_gameref_t game);
extern sc_bool res_has_graphics(sc_gameref_t game);
extern void res_clear_resource(sc_resourceref_t resource);
extern sc_bool res_compare_resource(sc_resourceref_t from,
                                    sc_resourceref_t with);
extern void res_handle_resource(sc_gameref_t game,
                                const sc_char *partial_format,
                                const sc_vartype_t vt_partial[]);
extern void res_sync_resources(sc_gameref_t game);
extern void res_cancel_resources(sc_gameref_t game);

/* Game runner functions. */
extern sc_bool run_game_task_commands(sc_gameref_t game, const sc_char *string);
extern sc_gameref_t run_create(sc_read_callbackref_t callback, void *opaque);
extern void run_interpret(CONTEXT, sc_gameref_t game);
extern void run_destroy(sc_gameref_t game);
extern void run_restart(CONTEXT, sc_gameref_t game);
extern void run_save(sc_gameref_t game, sc_write_callbackref_t callback, void *opaque);
extern sc_bool run_save_prompted(sc_gameref_t game);
extern sc_bool run_restore(CONTEXT, sc_gameref_t game, sc_read_callbackref_t callback, void *opaque);
extern sc_bool run_restore_prompted(CONTEXT, sc_gameref_t game);
extern sc_bool run_undo(CONTEXT, sc_gameref_t game);
extern void run_quit(CONTEXT, sc_gameref_t game);
extern sc_bool run_is_running(sc_gameref_t game);
extern sc_bool run_has_completed(sc_gameref_t game);
extern sc_bool run_is_undo_available(sc_gameref_t game);
extern void run_get_attributes(sc_gameref_t game,
                               const sc_char **game_name,
                               const sc_char **game_author,
                               const sc_char **game_compile_date,
                               sc_int *turns, sc_int *score,
                               sc_int *max_score,
                               const sc_char **current_room_name,
                               const sc_char **status_line,
                               const sc_char **preferred_font,
                               sc_bool *bold_room_names, sc_bool *verbose,
                               sc_bool *notify_score_change);
extern void run_set_attributes(sc_gameref_t game,
                               sc_bool bold_room_names, sc_bool verbose,
                               sc_bool notify_score_change);
extern sc_hintref_t run_hint_iterate(sc_gameref_t game, sc_hintref_t hint);
extern const sc_char *run_get_hint_question(sc_gameref_t game,
        sc_hintref_t hint);
extern const sc_char *run_get_subtle_hint(sc_gameref_t game,
        sc_hintref_t hint);
extern const sc_char *run_get_unsubtle_hint(sc_gameref_t game,
        sc_hintref_t hint);

/* Event functions. */
extern sc_bool evt_can_see_event(sc_gameref_t game, sc_int event);
extern void evt_tick_events(sc_gameref_t game);
extern void evt_debug_trace(sc_bool flag);

/* Task functions. */
extern sc_bool task_has_hints(sc_gameref_t game, sc_int task);
extern const sc_char *task_get_hint_question(sc_gameref_t game, sc_int task);
extern const sc_char *task_get_hint_subtle(sc_gameref_t game, sc_int task);
extern const sc_char *task_get_hint_unsubtle(sc_gameref_t game, sc_int task);
extern sc_bool task_can_run_task_directional(sc_gameref_t game,
        sc_int task, sc_bool forwards);
extern sc_bool task_can_run_task(sc_gameref_t game, sc_int task);
extern sc_bool task_run_task(sc_gameref_t game, sc_int task, sc_bool forwards);
extern void task_debug_trace(sc_bool flag);

/* Task restriction functions. */
extern sc_bool restr_pass_task_object_state(sc_gameref_t game,
        sc_int var1, sc_int var2);
extern sc_bool restr_eval_task_restrictions(sc_gameref_t game,
        sc_int task, sc_bool *pass,
        const sc_char **fail_message);
extern void restr_debug_trace(sc_bool flag);

/* NPC gender enumeration and functions. */
enum {
	NPC_MALE = 0,
	NPC_FEMALE = 1,
	NPC_NEUTER = 2
};

extern sc_bool npc_in_room(sc_gameref_t game, sc_int npc, sc_int Room);
extern sc_int npc_count_in_room(sc_gameref_t game, sc_int Room);
extern void npc_setup_initial(sc_gameref_t game);
extern void npc_start_npc_walk(sc_gameref_t game, sc_int npc, sc_int walk);
extern void npc_tick_npcs(sc_gameref_t game);
extern void npc_turn_update(sc_gameref_t game);
extern void npc_debug_trace(sc_bool flag);

/* Object open/closed state enumeration and functions. */
enum {
	OBJ_WONTCLOSE = 0,
	OBJ_OPEN = 5,
	OBJ_CLOSED = 6,
	OBJ_LOCKED = 7
};

extern sc_bool obj_is_static(sc_gameref_t game, sc_int object);
extern sc_bool obj_is_container(sc_gameref_t game, sc_int _object);
extern sc_bool obj_is_surface(sc_gameref_t game, sc_int object);
extern sc_int obj_container_object(sc_gameref_t game, sc_int n);
extern sc_int obj_surface_object(sc_gameref_t game, sc_int n);
extern sc_bool obj_indirectly_in_room(sc_gameref_t game,
                                      sc_int _object, sc_int Room);
extern sc_bool obj_indirectly_held_by_player(sc_gameref_t game, sc_int object);
extern sc_bool obj_directly_in_room(sc_gameref_t game,
                                    sc_int _object, sc_int Room);
extern sc_int obj_stateful_object(sc_gameref_t game, sc_int n);
extern sc_int obj_dynamic_object(sc_gameref_t game, sc_int n);
extern sc_int obj_wearable_object(sc_gameref_t game, sc_int n);
extern sc_int obj_standable_object(sc_gameref_t game, sc_int n);
extern sc_int obj_get_size(sc_gameref_t game, sc_int object);
extern sc_int obj_get_weight(sc_gameref_t game, sc_int _object);
extern sc_int obj_get_player_size_limit(sc_gameref_t game);
extern sc_int obj_get_player_weight_limit(sc_gameref_t game);
extern sc_int obj_get_container_maxsize(sc_gameref_t game, sc_int object);
extern sc_int obj_get_container_capacity(sc_gameref_t game, sc_int object);
extern sc_int obj_lieable_object(sc_gameref_t game, sc_int n);
extern sc_bool obj_appears_plural(sc_gameref_t game, sc_int _object);
extern void obj_setup_initial(sc_gameref_t game);
extern sc_int obj_container_index(sc_gameref_t game, sc_int object);
extern sc_int obj_surface_index(sc_gameref_t game, sc_int object);
extern sc_int obj_stateful_index(sc_gameref_t game, sc_int _object);
extern sc_char *obj_state_name(sc_gameref_t game, sc_int object);
extern sc_bool obj_shows_initial_description(sc_gameref_t game, sc_int object);
extern void obj_turn_update(sc_gameref_t game);
extern void obj_debug_trace(sc_bool flag);

/* Locale support, and locale-sensitive functions. */
extern void loc_detect_game_locale(sc_prop_setref_t bundle);
extern sc_bool loc_set_locale(const sc_char *name);
extern const sc_char *loc_get_locale(void);
extern sc_bool sc_isspace(sc_char character);
extern sc_bool sc_isdigit(sc_char character);
extern sc_bool sc_isalpha(sc_char character);
extern sc_char sc_toupper(sc_char character);
extern sc_char sc_tolower(sc_char character);
extern void loc_debug_dump(void);

/* Debugger interface. */
typedef struct sc_debugger_s *sc_debuggerref_t;
extern sc_bool debug_run_command(sc_gameref_t game,
                                 const sc_char *debug_command);
extern sc_bool debug_cmd_debugger(sc_gameref_t game);
extern void debug_set_enabled(sc_gameref_t game, sc_bool enable);
extern sc_bool debug_get_enabled(sc_gameref_t game);
extern void debug_game_started(CONTEXT, sc_gameref_t game);
extern void debug_game_ended(CONTEXT, sc_gameref_t game);
extern void debug_turn_update(CONTEXT, sc_gameref_t game);

/* OS interface functions. */
extern sc_bool if_get_trace_flag(sc_uint bitmask);
extern void if_print_string(const sc_char *string);
extern void if_print_debug(const sc_char *string);
extern void if_print_character(sc_char character);
extern void if_print_debug_character(sc_char character);
extern void if_print_tag(sc_int tag, const sc_char *arg);
extern void if_read_line(sc_char *buffer, sc_int length);
extern void if_read_debug(sc_char *buffer, sc_int length);
extern sc_bool if_confirm(sc_int _type);
extern void *if_open_saved_game(sc_bool is_save);
extern void if_close_saved_game(void *opaque);
extern void if_display_hints(sc_gameref_t game);
extern void if_update_sound(const sc_char *filepath, sc_int sound_offset,
		sc_int sound_length, sc_bool is_looping);
extern void if_update_graphic(const sc_char *filepath, sc_int graphic_offset, sc_int graphic_length);

#endif

} // End of namespace Adrift
} // End of namespace Glk
