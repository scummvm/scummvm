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

/* Based on Glulx intrepreter version 0.5.4 */

#ifndef GLK_GLULXE
#define GLK_GLULXE

#include "common/scummsys.h"
#include "common/random.h"
#include "glk/glk_api.h"
#include "glk/glulx/glulx_types.h"

namespace Glk {
namespace Glulx {

class Glulx;
typedef void (Glulx::*CharHandler)(unsigned char);
typedef void (Glulx::*UnicharHandler)(uint32);

/**
 * Glulx game interpreter
 */
class Glulx : public GlkAPI {
private:
	/**
	 * \defgroup vm fields
	 * @{
	 */

	bool vm_exited_cleanly;
	uint gamefile_start, gamefile_len;
	UnicharHandler glkio_unichar_han_ptr;
	char *init_err, *init_err2;
	CharHandler stream_char_handler;

	byte *memmap;
	byte *stack;
	UnicharHandler stream_unichar_handler;

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

	/**@}*/

	/**
	 * \defgroup main fields
	 * @{
	 */

	/**
	 * The library_autorestore_hook is called right after the VM's initial setup. This is an appropriate time
	 * to autorestore an initial game state, if the library has that capability. (Currently, only iosglk does.)
	 */
	void(*library_autorestore_hook)(void);

	/**@}*/

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

	/**
	 * \defgroup heap fields
	 * @{
	 */

	uint heap_start;    ///< zero for inactive heap
	int alloc_count;

	/* The heap_head/heap_tail is a doubly-linked list of blocks, both
	   free and allocated. It is kept in address order. It should be
	   complete -- that is, the first block starts at heap_start, and each
	   block ends at the beginning of the next block, until the last one,
	   which ends at endmem.

	   (Heap_start is never the same as end_mem; if there is no heap space,
	   then the heap is inactive and heap_start is zero.)

	   Adjacent free blocks may be merged at heap_alloc() time.

	   ### To make alloc more efficient, we could keep a separate
	   free-list. To make free more efficient, we could keep a hash
	   table of allocations.
	 */
	heapblock_t *heap_head;
	heapblock_t *heap_tail;

	/**@}*/

	/**
	 * \defgroup operand fields
	 * @{
	 */

	/**
	 * This is a handy array in which to look up operandlists quickly. It stores the operandlists
	 * for the first 128 opcodes, which are the ones used most frequently.
	 */
	const operandlist_t *fast_operandlist[0x80];

	/**@}*/

	/**
	 * \defgroup serial fields
	 * @{
	 */

	/**
	 * This can be adjusted before startup by platform-specific startup code -- that is, preference code.
	 */
	int max_undo_level;

	int undo_chain_size;
	int undo_chain_num;
	byte **undo_chain;

	/**
	 * This will contain a copy of RAM (ramstate to endmem) as it exists in the game file.
	 */
	byte *ramcache;

	/**@}*/

	/**
	 * \defgroup string fields
	 * @{
	 */

	uint iosys_mode;
	uint iosys_rock;

	/**
	 * The current string-decoding tables, broken out into a fast and easy-to-use form.
	 */
	bool tablecache_valid;
	cacheblock_t tablecache;

	/* This misbehaves if a Glk function has more than one S argument. */
#define STATIC_TEMP_BUFSIZE (127)
	char temp_buf[STATIC_TEMP_BUFSIZE + 1];

	/**@}*/

	Common::String _savegameDescription;
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

	uint ReadMemory(uint addr);
	void WriteMemory(uint addr, uint val);
	char *CaptureCArray(uint addr, uint len, int passin);
	void ReleaseCArray(char *ptr, uint addr, uint len, int passout);
	uint *CaptureIArray(uint addr, uint len, int passin);
	void ReleaseIArray(uint *ptr, uint addr, uint len, int passout);
	void **CapturePtrArray(uint addr, uint len, int objclass, int passin);
	void ReleasePtrArray(void **ptr, uint addr, uint len, int objclass, int passout);
	uint ReadStructField(uint addr, uint fieldnum);
	void WriteStructField(uint addr, uint fieldnum, uint val);
	char *DecodeVMString(uint addr);
	void ReleaseVMString(char *ptr);
	uint32 *DecodeVMUstring(uint addr);
	void ReleaseVMUstring(uint32 *ptr);

	/**@}*/

	/**
	 * \defgroup search support methods
	 * @{
	 */

	/**
	 * This massages the key into a form that's easier to handle. When it returns, the key will
	 * be stored in keybuf if keysize <= 4; otherwise, it will be in memory.
	 */
	void fetchkey(unsigned char *keybuf, uint key, uint keysize, uint options);

	/**@}*/

	/**
	 * \defgroup serial support methods
	 * @{
	 */

	uint write_memstate(dest_t *dest);
	uint write_heapstate(dest_t *dest, int portable);
	uint write_stackstate(dest_t *dest, int portable);
	uint read_memstate(dest_t *dest, uint chunklen);
	uint read_heapstate(dest_t *dest, uint chunklen, int portable, uint *sumlen, uint **summary);
	uint read_stackstate(dest_t *dest, uint chunklen, int portable);
	uint write_heapstate_sub(uint sumlen, uint *sumarray, dest_t *dest, int portable);
	static int sort_heap_summary(const void *p1, const void *p2);

	int read_byte(dest_t *dest, byte *val);
	int read_short(dest_t *dest, uint16 *val);
	int read_long(dest_t *dest, uint *val);

	int write_byte(dest_t *dest, byte val);
	int write_short(dest_t *dest, uint16 val);
	int write_long(dest_t *dest, uint val);

	int read_buffer(dest_t *dest, byte *ptr, uint len);
	int reposition_write(dest_t *dest, uint pos);
	int write_buffer(dest_t *dest, const byte *ptr, uint len);

	/**@}*/

	/**
	 * \defgroup string support methods
	 * @{
	 */

	void stream_setup_unichar();

	void nopio_char_han(unsigned char ch);
	void filio_char_han(unsigned char ch);
	void nopio_unichar_han(uint32 ch);
	void filio_unichar_han(uint32 ch);
	void glkio_unichar_nouni_han(uint32 val);

	void dropcache(cacheblock_t *cablist);
	void buildcache(cacheblock_t *cablist, uint nodeaddr, int depth, int mask);
	void dumpcache(cacheblock_t *cablist, int count, int indent);

	/**@}*/
public:
	/**
	 * Constructor
	 */
	Glulx(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override {
		return INTERPRETER_GLULX;
	}

	/**
	 * Loads Quetzal chunks from the passed savegame
	 */
	Common::Error loadGameChunks(QuetzalReader &quetzal) override;

	/**
	 * Writes out the Quetzal chunks within a savegame
	 */
	Common::Error saveGameChunks(QuetzalWriter &quetzal) override;

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;

	/**
	 * \defgroup Main access methods
	 * @{
	 */

	/**
	 * Display an error in the error window, and then exit.
	 */
	void NORETURN_PRE fatal_error_handler(const char *str, const char *arg, bool useVal, int val);

	/**
	 * Display a warning in the error window, and then continue.
	 */
	void nonfatal_warning_handler(const char *str, const char *arg, bool useVal, int val);

	/**
	 * \defgroup Files access methods
	 * @{
	 */

	/**
	 * Validates the game file, and if it's invalid, displays an error dialog
	 */
	bool is_gamefile_valid();

	/**@}*/

	/**
	 * \defgroup Vm access methods
	 * @{
	 */

	/**
	 * Read in the game file and build the machine, allocating all the memory necessary.
	*/
	void setup_vm();

	/**
	 * Deallocate all the memory and shut down the machine.
	 */
	void finalize_vm();

	/**
	 * Put the VM into a state where it's ready to begin executing the game. This is called
	 * both at startup time, and when the machine performs a "restart" opcode.
	 */
	void vm_restart();

	/**
	 * Change the size of the memory map. This may not be available at all; #define FIXED_MEMSIZE
	 * if you want the interpreter to unconditionally refuse. The internal flag should be true only
	 * when the heap-allocation system is calling. Returns 0 for success; otherwise, the operation failed.
	 */
	uint change_memsize(uint newlen, bool internal);

	/**
	 * If addr is 0, pop N arguments off the stack, and put them in an array. If non-0, take N arguments
	 * from that main memory address instead. This has to dynamically allocate if there are more than
	 * 32 arguments, but that shouldn't be a problem.
	 */
	uint *pop_arguments(uint count, uint addr);

	/**
	 * Make sure that count bytes beginning with addr all fall within the current memory map.
	 * This is called at every memory (read) access if VERIFY_MEMORY_ACCESS is defined in the header file.
	*/
	void verify_address(uint addr, uint count);

	/**
	 * Make sure that count bytes beginning with addr all fall within RAM. This is called at every memory
	 * write if VERIFY_MEMORY_ACCESS is defined in the header file.
	*/
	void verify_address_write(uint addr, uint count);

	/**
	 * Make sure that an array of count elements (size bytes each), starting at addr, does not fall
	 * outside the memory map. This goes to some trouble that verify_address() does not, because we need
	 * to be wary of lengths near -- or beyond -- 0x7FFFFFFF.
	 */
	void verify_array_addresses(uint addr, uint count, uint size);

	/**@}*/

	/**
	 * \defgroup Exec access methods
	 * @{
	 */

	/**
	 * The main interpreter loop. This repeats until the program is done
	 */
	void execute_loop();

	/**@}*/

	/**
	 * \defgroup Operand access methods
	 * @{
	 */

	/**
	 * Set up the fast-lookup array of operandlists. This is called just once, when the terp starts up.
	 */
	void init_operands();

	/**
	 * Return the operandlist for a given opcode. For opcodes in the range 00..7F, it's faster
	 * to use the array fast_operandlist[].
	*/
	const operandlist_t *lookup_operandlist(uint opcode);

	/**
	 * Read the list of operands of an instruction, and put the values in args. This assumes
	 * that the PC is at the beginning of the operand mode list (right after an opcode number.)
	 * Upon return, the PC will be at the beginning of the next instruction.
	 *
	 * This also assumes that args points at an allocated array of MAX_OPERANDS oparg_t structures.
	*/
	void parse_operands(oparg_t *opargs, const operandlist_t *oplist);

	/**
	 * Store a result value, according to the desttype and destaddress given. This is usually used to store
	 * the result of an opcode, but it's also used by any code that pulls a call-stub off the stack.
	 */
	void store_operand(uint desttype, uint destaddr, uint storeval);

	void store_operand_s(uint desttype, uint destaddr, uint storeval);
	void store_operand_b(uint desttype, uint destaddr, uint storeval);

	/**@}*/

	/**
	 * \defgroup Func access methods
	 * @{
	 */

	/**
	 * This writes a new call frame onto the stack, at stackptr. It leaves frameptr pointing
	 * to the frame (ie, the original stackptr value.) argc and argv are an array of arguments.
	 * Note that if argc is zero, argv may be nullptr.
	 */
	void enter_function(uint addr, uint argc, uint *argv);

	/**
	 * Pop the current call frame off the stack. This is very simple.
	*/
	void leave_function();

	/**
	 * Push the magic four values on the stack: result destination, PC, and frameptr.
	 */
	void push_callstub(uint desttype, uint destaddr);

	/**
	 * Remove the magic four values from the stack, and use them. The returnvalue, whatever it is,
	 * is put at the result destination; the PC and frameptr registers are set.
	*/
	void pop_callstub(uint returnvalue);

	/**
	 * Remove the magic four values, but interpret them as a string restart state.
	 * Returns zero if it's a termination stub, or returns the restart address. The bitnum is extra.
	 */
	uint pop_callstub_string(int *bitnum);

	/**@}*/

	/**
	 * \defgroup Heap access methods
	 * @{
	 */

	/**
	 * Set the heap state to inactive, and free the block lists. This is called when the game
	 * starts or restarts.
	 */
	void heap_clear();

	/**
	 * Returns whether the heap is active.
	 */
	int heap_is_active() const;

	/**
	 * Returns the start address of the heap, or 0 if the heap is not active.
	 */
	uint heap_get_start() const;

	/**
	 * Allocate a block. If necessary, activate the heap and/or extend memory. This may not be
	 * available at all; #define FIXED_MEMSIZE if you want the interpreter to unconditionally refuse.
	 * Returns the memory address of the block, or 0 if the operation failed.
	 */
	uint heap_alloc(uint len);

	/**
	 * Free a heap block. If necessary, deactivate the heap.
	 */
	void heap_free(uint addr);

	/**
	 * Create an array of words, in the VM serialization format:
	 *
	 *   heap_start
	 *   alloc_count
	 *   addr of first block
	 *   len of first block
	 *   ...
	 *
	 * (Note that these are uint values -- native byte ordering. Also, the blocks will be in address order,
	 * which is a stricter guarantee than the VM specifies; that'll help in heap_apply_summary().)
	 *
	 * If the heap is inactive, store nullptr. Return 0 for success; otherwise, the operation failed.
	 *
	 * The array returned in summary must be freed with glulx_free() after the caller uses it.
	 */
	int heap_get_summary(uint *valcount, uint **summary);

	/**
	 * Given an array of words in the above format, set up the heap to contain it. As noted above,
	 * the caller must ensure that the blocks are in address order. When this is called, the heap
	 * must be inactive.
	 *
	 * Return 0 for success. Otherwise the operation failed (and, most likely, caused a fatal error).
	*/
	int heap_apply_summary(uint valcount, uint *summary);

	/**@}*/

	/**
	 * \defgroup Serial access methods
	 * @{
	 */

	/**@}*/

	/**
	 * \defgroup Search access methods
	 * @{
	 */


	/**
	 * An array of data structures is stored in memory, beginning at start, each structure being structsize bytes.
	 * Within each struct, there is a key value keysize bytes long, starting at position keyoffset (from
	 * the start of the structure.) Search through these in order. If one is found whose key matches, return it.
	 * If numstructs are searched with no result, return nullptr.
	 *
	 * numstructs may be -1 (0xFFFFFFFF) to indicate no upper limit to the number of structures to search.
	 * The search will continue until a match is found, or (if ZeroKeyTerminates is set) a zero key.
	 *
	 * The KeyIndirect, ZeroKeyTerminates, and ReturnIndex options may be used.
	 */
	uint linear_search(uint key, uint keysize, uint start, uint structsize, uint numstructs,
	                   uint keyoffset, uint options);

	/**
	 * An array of data structures is in memory, as above. However, the structs must be stored in forward
	 * order of their keys (taking each key to be a multibyte unsigned integer.) There can be no duplicate keys.
	 * numstructs must indicate the exact length of the array; it cannot be -1.
	 *
	 * The KeyIndirect and ReturnIndex options may be used.
	 */
	uint binary_search(uint key, uint keysize, uint start, uint structsize, uint numstructs,
	                   uint keyoffset, uint options);

	/**
	 * The structures may be anywhere in memory, in any order. They are linked by a four-byte address field,
	 * which is found in each struct at position nextoffset. If this field contains zero, it indicates
	 * the end of the linked list.
	 *
	 * The KeyIndirect and ZeroKeyTerminates options may be used.
	 */
	uint linked_search(uint key, uint keysize, uint start, uint keyoffset, uint nextoffset, uint options);

	/**@}*/

	/**
	 * \defgroup Osdepend access methods
	 * @{
	 */

	inline void *glulx_malloc(uint len) {
		return malloc(len);
	}
	inline void *glulx_realloc(void *ptr, uint len) {
		return realloc(ptr, len);
	}
	inline void glulx_free(void *ptr) {
		free(ptr);
	}
	inline void glulx_setrandom(uint32 seed) {
		_random.setSeed(seed);
	}
	inline uint glulx_random() {
		return _random.getRandomNumber(0xfffffff);
	}

	void glulx_sort(void *addr, int count, int size, int(*comparefunc)(const void *p1, const void *p2));

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

	/**
	 * Set up the class hash tables and other startup-time stuff.
	 */
	bool init_dispatch();

	/**
	 * The object registration/unregistration callbacks that the library calls
	 * to keep the hash tables up to date.
	 */
	gidispatch_rock_t glulxe_classtable_register(void *obj, uint objclass);

	gidispatch_rock_t glulxe_classtable_register_existing(void *obj, uint objclass, uint dispid);

	void glulxe_classtable_unregister(void *obj, uint objclass, gidispatch_rock_t objrock);

	gidispatch_rock_t glulxe_retained_register(void *array, uint len, const char *typecode);
	void glulxe_retained_unregister(void *array, uint len, const char *typecode, gidispatch_rock_t objrock);

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
	int init_profile();
	void profile_set_call_counts(int flag);

	#if VM_PROFILING
	uint profile_opcount;
	#define profile_tick() (profile_opcount++)
	int profile_profiling_active();
	void profile_in(uint addr, uint stackuse, int accel);
	void profile_out(uint stackuse);
	void profile_fail(const char *reason);
	void profile_quit();
	#else /* VM_PROFILING */
	void profile_tick() {}
	void profile_profiling_active() {}
	void profile_in(uint addr, uint stackuse, int accel) {}
	void profile_out(uint stackuse)  {}
	void profile_fail(const char *reason) {}
	void profile_quit() {}
	#endif /* VM_PROFILING */

#if VM_DEBUGGER
	unsigned long debugger_opcount;
	void debugger_tick() { debugger_opcount++ }
	int debugger_load_info_stream(strid_t stream);
	int debugger_load_info_chunk(strid_t stream, uint pos, uint len);
	void debugger_track_cpu(int flag);
	void debugger_set_start_trap(int flag);
	void debugger_set_quit_trap(int flag);
	void debugger_set_crash_trap(int flag);
	void debugger_check_story_file();
	void debugger_setup_start_state();
	int debugger_ever_invoked();
	int debugger_cmd_handler(char *cmd);
	void debugger_cycle_handler(int cycle);
	void debugger_check_func_breakpoint(uint addr);
	void debugger_block_and_debug(char *msg);
	void debugger_handle_crash(char *msg);
	void debugger_handle_quit();
#else /* VM_DEBUGGER */
	void debugger_tick() {}
	void debugger_check_story_file() {}
	void debugger_setup_start_state() {}
	void debugger_check_func_breakpoint(uint addr) {}
	void debugger_handle_crash(const char *msg) {}
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

	/* Uncomment this definition if your gfloat32 type is not a standard
	   IEEE-754 single-precision (32-bit) format. Normally, Glulx assumes
	   that it can reinterpret-cast IEEE-754 int values into gfloat32
	   values. If you uncomment this, Glulx switches to lengthier
	   (but safer) encoding and decoding functions. */
	/* #define FLOAT_NOT_NATIVE (1) */

	int init_float() {
		return true;
	}

	/**
	 * Encode floats by a lot of annoying bit manipulation.
	 * The function is adapted from code in Python  (Objects/floatobject.c)
	 */
	static uint encode_float(gfloat32 val);

	/**
	 * Decode floats by a lot of annoying bit manipulation.
	 * The function is adapted from code in Python  (Objects/floatobject.c)
	 */
	static gfloat32 decode_float(uint val);

	/* Uncomment this definition if your powf() function does not support
	   all the corner cases specified by C99. If you uncomment this,
	   osdepend.c will provide a safer implementation of glulx_powf(). */
	/* #define FLOAT_COMPILE_SAFER_POWF (1) */

	inline gfloat32 glulx_powf(gfloat32 val1, gfloat32 val2) const {
		return powf(val1, val2);
	}

#endif /* FLOAT_SUPPORT */
	/**@}*/

	/**
	 * \defgroup serial access methods
	 * @{
	 */

	/**
	 * Set up the undo chain and anything else that needs to be set up.
	 */
	bool init_serial();

	/**
	 * Clean up memory when the VM shuts down.
	 */
	void final_serial();

	/**
	 * Add a state pointer to the undo chain. This returns 0 on success, 1 on failure.
	 */
	uint perform_saveundo();

	/**
	 * Pull a state pointer from the undo chain. This returns 0 on success, 1 on failure.
	 * Note that if it succeeds, the frameptr, localsbase, and valstackbase registers are invalid;
	 * they must be rebuilt from the stack.
	 */
	uint perform_restoreundo();

	uint perform_verify();

	/**@}*/

	/**
	 * \defgroup Strings access methods
	 * @{
	 */

	/**
	 * Write a signed integer to the current output stream.
	 */
	void stream_num(int val, int inmiddle, int charnum);

	/**
	 * Write a Glulx string object to the current output stream. inmiddle is zero if we are beginning
	 * a new string, or nonzero if restarting one (E0/E1/E2, as appropriate for the string type).
	 */
	void stream_string(uint addr, int inmiddle, int bitnum);

	/**
	 * Get the current table address.
	 */
	uint stream_get_table();

	/**
	 * Set the current table address, and rebuild decoding cache.
	 */
	void stream_set_table(uint addr);

	void stream_get_iosys(uint *mode, uint *rock);
	void stream_set_iosys(uint mode, uint rock);
	char *make_temp_string(uint addr);
	uint32 *make_temp_ustring(uint addr);
	void free_temp_string(char *str);
	void free_temp_ustring(uint32 *str);

	/**@}*/
};

extern Glulx *g_vm;

#define fatal_error(s)  (fatal_error_handler((s), nullptr, false, 0))
#define fatal_error_2(s1, s2)  (fatal_error_handler((s1), (s2), false, 0))
#define fatal_error_i(s, v)  (fatal_error_handler((s), nullptr, true, (v)))
#define nonfatal_warning(s) (nonfatal_warning_handler((s), nullptr, false, 0))
#define nonfatal_warning_2(s1, s2) (nonfatal_warning_handler((s1), (s2), false, 0))
#define nonfatal_warning_i(s, v) (nonfatal_warning_handler((s), nullptr, true, (v)))

} // End of namespace Glulx
} // End of namespace Glk

#endif
