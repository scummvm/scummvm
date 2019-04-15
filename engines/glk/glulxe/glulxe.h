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

#ifndef GLK_GLULXE
#define GLK_GLULXE

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/glulxe/glulxe_types.h"

namespace Glk {
namespace Glulxe {

/**
 * Glulxe game interpreter
 */
class Glulxe : public GlkAPI {
public:
	bool vm_exited_cleanly;
	strid_t gamefile;
	uint gamefile_start, gamefile_len;
	char *init_err, *init_err2;

	unsigned char *memmap;
	unsigned char *stack;

	uint ramstart;
	uint endgamefile;
	uint origendmem;
	uint stacksize;
	uint startfuncaddr;
	uint checksum;
	uint stackptr;
	uint frameptr;
	uint pc;
	uint origstringtable;
	uint stringtable;
	uint valstackbase;
	uint localsbase;
	uint endmem;
	uint protectstart, protectend;
	uint prevpc;

	/**
	 * \defgroup accel fields
	 * @{
	 */

	uint classes_table;     ///< class object array
	uint indiv_prop_start;  ///< first individual prop ID
	uint class_metaclass;   ///< "Class" class object
	uint object_metaclass;  ///< "Object" class object
	uint routine_metaclass; ///< "Routine" class object
	uint string_metaclass;  ///< "String" class object
	uint self;              ///< address of global "self"
	uint num_attr_bytes;    ///< number of attributes / 8
	uint cpv__start;        ///< array of common prop defaults
	accelentry_t **accelentries;

	/**@}*/
protected:
	/**
	 * \defgroup glkop fields
	 * @{
	 */

	/**
	 * The library_select_hook is called every time the VM blocks for input.
	 * The app might take this opportunity to autosave, for example.
	 */
	void (*library_select_hook)(uint);

	arrayref_t *arrays;

	/**
	 * The list of hash tables, for the classes.
	 */
	int num_classes;
	classtable_t **classes;

	/**@}*/
protected:
	/**
	 * \defgroup accel support methods
	 * @{
	 */

	void accel_error(const char *msg);
	uint func_1_z__region(uint argc, uint *argv);

	/**
	 * The old set of accel functions (2 through 7) are deprecated; they behave badly if the Inform 6
	 * NUM_ATTR_BYTES option (parameter 7) is changed from its default value (7). They will not be removed,
	 * but new games should use functions 8 through 13 instead
	 */
	uint func_2_cp__tab(uint argc, uint *argv);
	uint func_3_ra__pr(uint argc, uint *argv);
	uint func_4_rl__pr(uint argc, uint *argv);
	uint func_5_oc__cl(uint argc, uint *argv);
	uint func_6_rv__pr(uint argc, uint *argv);
	uint func_7_op__pr(uint argc, uint *argv);

	/**
	 * Here are the newer functions, which support changing NUM_ATTR_BYTES.
	   These call get_prop_new() instead of get_prop()
	 */
	uint func_8_cp__tab(uint argc, uint *argv);
	uint func_9_ra__pr(uint argc, uint *argv);
	uint func_10_rl__pr(uint argc, uint *argv);
	uint func_11_oc__cl(uint argc, uint *argv);
	uint func_12_rv__pr(uint argc, uint *argv);
	uint func_13_op__pr(uint argc, uint *argv);
	int obj_in_class(uint obj);

	/**
	 * Look up a property entry.
	 */
	uint get_prop(uint obj, uint id);

	/**
	 * Look up a property entry. This is part of the newer set of accel functions (8 through 13),
	 * which support increasing NUM_ATTR_BYTES. It is identical to get_prop() except that it calls
	 * the new versions of func_5 and func_2
	 */
	uint get_prop_new(uint obj, uint id);

	 /**@}*/

	/**
	 * \defgroup glkop support methods
	 * @{
	 */

	/**
	 * Build a hash table to hold a set of Glk objects.
	 */
	classtable_t *new_classtable(uint firstid);

	/**
	 * Find a Glk object in the appropriate hash table.
	 */
	void *classes_get(int classid, uint objid);

	/**
	 * Put a Glk object in the appropriate hash table. If origid is zero, invent a new
	 * unique ID for it.
	 */
	classref_t *classes_put(int classid, void *obj, uint origid);

	/**
	 * Delete a Glk object from the appropriate hash table.
	 */
	void classes_remove(int classid, void *obj);

	long glulxe_array_locate(void *array, uint len, char *typecode, gidispatch_rock_t objrock, int *elemsizeref);
	gidispatch_rock_t glulxe_array_restore(long bufkey, uint len, char *typecode, void **arrayref);

	char *grab_temp_c_array(uint addr, uint len, int passin);
	void release_temp_c_array(char *arr, uint addr, uint len, int passout);
	uint *grab_temp_i_array(uint addr, uint len, int passin);
	void release_temp_i_array(uint *arr, uint addr, uint len, int passout);
	void **grab_temp_ptr_array(uint addr, uint len, int objclass, int passin);
	void release_temp_ptr_array(void **arr, uint addr, uint len, int objclass, int passout);

	/**
	 * This reads through the prototype string, and pulls Floo objects off the stack. It also works out the maximal number
	 * of gluniversal_t objects which could be used by the Glk call in question. It then allocates space for them.
	 */
	void prepare_glk_args(const char *proto, dispatch_splot_t *splot);

	/**
	 * This long and unpleasant function translates a set of Floo objects into a gluniversal_t array. It's recursive, too,
	 * to deal with structures.
	 */
	void parse_glk_args(dispatch_splot_t *splot, const char **proto, int depth, int *argnumptr, uint subaddress, int subpassin);
		
	/**
	 * This is about the reverse of parse_glk_args().
	 */
	void unparse_glk_args(dispatch_splot_t *splot, const char **proto, int depth,
		int *argnumptr, uint subaddress, int subpassout);

	/**
	 * Create a string identifying this game. We use the first 64 bytes of the memory map, encoded as hex,
	 */
	char *get_game_id();

	/**@}*/
public:
	/**
	 * Constructor
	 */
	Glulxe(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame();

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_GLULXE; }

	/**
	 * Load a savegame from the passed stream
	 */
	virtual Common::Error loadGameData(strid_t file) override;

	/**
	 * Save the game to the passed stream
	 */
	virtual Common::Error saveGameData(strid_t file, const Common::String &desc) override;

	/**
	 * \defgroup Main access methods
	 * @{
	 */
	void set_library_start_hook(void(*)(void));
	void set_library_autorestore_hook(void(*)(void));

	/**
	 * Display an error in the error window, and then exit.
	 */
	void fatal_error_handler(const char *str, const char *arg, bool useVal, int val);

	/**
	 * Display a warning in the error window, and then continue.
	 */
	void nonfatal_warning_handler(const char *str, const char *arg, bool useVal, int val);
#define fatal_error(s)  (fatal_error_handler((s), nullptr, false, 0))
#define fatal_error_2(s1, s2)  (fatal_error_handler((s1), (s2), false, 0))
#define fatal_error_i(s, v)  (fatal_error_handler((s), nullptr, true, (v)))
#define nonfatal_warning(s) (nonfatal_warning_handler((s), nullptr, false, 0))
#define nonfatal_warning_2(s1, s2) (nonfatal_warning_handler((s1), (s2), false, 0))
#define nonfatal_warning_i(s, v) (nonfatal_warning_handler((s), nullptr, true, (v)))

	/**
	 * \defgroup Files access methods
	 * @{
	 */

	 /**
	  * Validates the game file, and if it's invalid, displays an error dialog
	  */
	bool is_gamefile_valid();

	int locate_gamefile(int isblorb);
	
	/**@}*/

	/**
	 * \defgroup Vm access methods
	 * @{
	 */

	void setup_vm(void);
	void finalize_vm(void);
	void vm_restart(void);
	uint change_memsize(uint newlen, int internal);
	uint *pop_arguments(uint count, uint addr);
	void verify_address(uint addr, uint count);
	void verify_address_write(uint addr, uint count);
	void verify_array_addresses(uint addr, uint count, uint size);

	/**@}*/

	/**
	 * \defgroup Exec access methods
	 * @{
	 */
	void execute_loop(void);

	/**@}*/

	/**
	 * \defgroup Operand access methods
	 * @{
	 */	operandlist_t *fast_operandlist[0x80];
	void init_operands(void);
	operandlist_t *lookup_operandlist(uint opcode);
	void parse_operands(oparg_t *opargs, operandlist_t *oplist);
	void store_operand(uint desttype, uint destaddr, uint storeval);
	void store_operand_s(uint desttype, uint destaddr, uint storeval);
	void store_operand_b(uint desttype, uint destaddr, uint storeval);

	/**@}*/

	/**
	 * \defgroup Func access methods
	 * @{
	 */

	void enter_function(uint addr, uint argc, uint *argv);
	void leave_function(void);
	void push_callstub(uint desttype, uint destaddr);
	void pop_callstub(uint returnvalue);
	uint pop_callstub_string(int *bitnum);

	/**@}*/

	/**
	 * \defgroup Strings access methods
	 * @{
	 */

	void stream_num(int val, int inmiddle, int charnum);
	void stream_string(uint addr, int inmiddle, int bitnum);
	uint stream_get_table(void);
	void stream_set_table(uint addr);
	void stream_get_iosys(uint *mode, uint *rock);
	void stream_set_iosys(uint mode, uint rock);
	char *make_temp_string(uint addr);
	uint *make_temp_ustring(uint addr);
	void free_temp_string(const char *str);
	void free_temp_ustring(const uint *str);

	/**@}*/

	/**
	 * \defgroup Heap access methods
	 * @{
	 */
	void heap_clear(void);
	int heap_is_active(void);
	uint heap_get_start(void);
	uint heap_alloc(uint len);
	void heap_free(uint addr);
	int heap_get_summary(uint *valcount, uint **summary);
	int heap_apply_summary(uint valcount, uint *summary);
	void heap_sanity_check(void);

	/**@}*/

	/**
	 * \defgroup Serial access methods
	 * @{
	 */

	int max_undo_level;
	int init_serial(void);
	void final_serial(void);
	uint perform_save(strid_t str);
	uint perform_restore(strid_t str, int fromshell);
	uint perform_saveundo(void);
	uint perform_restoreundo(void);
	uint perform_verify(void);

	/**@}*/

	/**
	 * \defgroup Search access methods
	 * @{
	 */

	uint linear_search(uint key, uint keysize,
		uint start, uint structsize, uint numstructs,
		uint keyoffset, uint options);
	uint binary_search(uint key, uint keysize,
		uint start, uint structsize, uint numstructs,
		uint keyoffset, uint options);
	uint linked_search(uint key, uint keysize,
		uint start, uint keyoffset, uint nextoffset,
		uint options);

	/**@}*/

	/**
	 * \defgroup Osdepend access methods
	 * @{
	 */

	void *glulx_malloc(uint len);
	void *glulx_realloc(void *ptr, uint len);
	void glulx_free(void *ptr);
	void glulx_setrandom(uint seed);
	uint glulx_random(void);
	void glulx_sort(void *addr, int count, int size,
		int(*comparefunc)(void *p1, void *p2));

	/**@}*/

	/**
	 * \defgroup Gestalt access methods
	 * @{
	 */

	uint do_gestalt(uint val, uint val2);

	/**@}*/

	/**
	 * \defgroup Glkop access methods
	 * @{
	 */

	 /**
	  * glkop section initialization
	  */
	void glkopInit();

	void set_library_select_hook(void(*func)(uint));

	bool init_dispatch();

	/**
	 * The object registration/unregistration callbacks that the library calls
	 * to keep the hash tables up to date.
	 */
	gidispatch_rock_t glulxe_classtable_register(void *obj, uint objclass);

	gidispatch_rock_t glulxe_classtable_register_existing(void *obj, uint objclass, uint dispid);

	void glulxe_classtable_unregister(void *obj, uint objclass, gidispatch_rock_t objrock);

	gidispatch_rock_t glulxe_retained_register(void *array, uint len, char *typecode);
	void glulxe_retained_unregister(void *array, uint len, char *typecode, gidispatch_rock_t objrock);

	/**
	 * Turn a list of Glulx arguments into a list of Glk arguments, dispatch the function call, and return the result.
	 */
	uint perform_glk(uint funcnum, uint numargs, uint *arglist);

	/**
	 * Read the prefixes of an argument string -- the "<>&+:#!" chars.
	 */
	const char *read_prefix(const char *cx, int *isref, int *isarray, int *passin, int *passout,
		int *nullok, int *isretained, int *isreturn);

	/**
	 * This is used by some interpreter code which has to, well, find a Glk stream given its ID.
	 */
	strid_t find_stream_by_id(uint objid);

	/**
	 * Return the ID of a given Glk window.
	 */
	uint find_id_for_window(winid_t win);

	/**
	 * Return the ID of a given Glk stream.
	 */
	uint find_id_for_stream(strid_t str);

	/**
	 * Return the ID of a given Glk fileref.
	 */
	uint find_id_for_fileref(frefid_t fref);

	/**
	 * Return the ID of a given Glk schannel.
	 */
	uint find_id_for_schannel(schanid_t schan);

	/**@}*/

	/**
	 * \defgroup Profile access methods
	 * @{
	 */

	void setup_profile(strid_t stream, char *filename);
	int init_profile(void);
	void profile_set_call_counts(int flag);
#if VM_PROFILING
	uint profile_opcount;
#define profile_tick() (profile_opcount++)
	int profile_profiling_active(void);
	void profile_in(uint addr, uint stackuse, int accel);
	void profile_out(uint stackuse);
	void profile_fail(char *reason);
	void profile_quit(void);
#else /* VM_PROFILING */
#define profile_tick()         (0)
#define profile_profiling_active()         (0)
#define profile_in(addr, stackuse, accel)  (0)
#define profile_out(stackuse)  (0)
#define profile_fail(reason)   (0)
#define profile_quit()         (0)
#endif /* VM_PROFILING */

#if VM_DEBUGGER
	unsigned long debugger_opcount;
#define debugger_tick() (debugger_opcount++)
	int debugger_load_info_stream(strid_t stream);
	int debugger_load_info_chunk(strid_t stream, uint pos, uint len);
	void debugger_track_cpu(int flag);
	void debugger_set_start_trap(int flag);
	void debugger_set_quit_trap(int flag);
	void debugger_set_crash_trap(int flag);
	void debugger_check_story_file(void);
	void debugger_setup_start_state(void);
	int debugger_ever_invoked(void);
	int debugger_cmd_handler(char *cmd);
	void debugger_cycle_handler(int cycle);
	void debugger_check_func_breakpoint(uint addr);
	void debugger_block_and_debug(char *msg);
	void debugger_handle_crash(char *msg);
	void debugger_handle_quit(void);
#else /* VM_DEBUGGER */
#define debugger_tick()              (0)
#define debugger_check_story_file()  (0)
#define debugger_setup_start_state() (0)
#define debugger_check_func_breakpoint(addr)  (0)
#define debugger_handle_crash(msg)   (0)
#endif /* VM_DEBUGGER */

	/**@}*/

	/**
	 * \defgroup Accel access methods
	 * @{
	 */

	acceleration_func accel_find_func(uint index);
	acceleration_func accel_get_func(uint addr);
	void accel_set_func(uint index, uint addr);
	void accel_set_param(uint index, uint val);

	uint accel_get_param_count() const;
	uint accel_get_param(uint index) const;

	/**
	 * Iterate the entire acceleration table, calling the callback for each (non-nullptr) entry.
	 * This is used only for autosave.
	 */
	void accel_iterate_funcs(void(*func)(uint index, uint addr));

	/**@}*/

	/**
	 * \defgroup Float access methods
	 * @{
	 */
#ifdef FLOAT_SUPPORT

	/* You may have to edit the definition of gfloat32 to make sure it's really
	   a 32-bit floating-point type. */
	typedef float gfloat32;

	/* Uncomment this definition if your gfloat32 type is not a standard
	   IEEE-754 single-precision (32-bit) format. Normally, Glulxe assumes
	   that it can reinterpret-cast IEEE-754 int values into gfloat32
	   values. If you uncomment this, Glulxe switches to lengthier
	   (but safer) encoding and decoding functions. */
	   /* #define FLOAT_NOT_NATIVE (1) */

	   /* float.c */
	int init_float(void);
	uint encode_float(gfloat32 val);
	gfloat32 decode_float(uint val);

	/* Uncomment this definition if your powf() function does not support
	   all the corner cases specified by C99. If you uncomment this,
	   osdepend.c will provide a safer implementation of glulx_powf(). */
	   /* #define FLOAT_COMPILE_SAFER_POWF (1) */

	gfloat32 glulx_powf(gfloat32 val1, gfloat32 val2);

#endif /* FLOAT_SUPPORT */
	/**@}*/
};

extern Glulxe *g_vm;

} // End of namespace Glulxe
} // End of namespace Glk

#endif
