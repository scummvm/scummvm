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

#ifndef GLK_ZCODE_PROCESSOR
#define GLK_ZCODE_PROCESSOR

#include "glk/zcode/mem.h"
#include "glk/zcode/glk_interface.h"
#include "glk/zcode/frotz_types.h"
#include "common/stack.h"

namespace Glk {
namespace ZCode {

#define TEXT_BUFFER_SIZE 200

#define CODE_BYTE(v)	   v = codeByte()
#define CODE_WORD(v)       v = codeWord()
#define CODE_IDX_WORD(v,i) v = codeWordIdx(i)
#define GET_PC(v)          v = getPC()
#define SET_PC(v)          setPC(v)

enum string_type {
	LOW_STRING, ABBREVIATION, HIGH_STRING, EMBEDDED_STRING, VOCABULARY
};

class Processor;
class Quetzal;
typedef void (Processor::*Opcode)();

/**
 * Zcode processor
 */
class Processor : public GlkInterface, public virtual Mem {
	friend class Quetzal;
private:
	static const char *const ERR_MESSAGES[ERR_NUM_ERRORS];
	static Opcode var_opcodes[64];
	static Opcode ext_opcodes[64];
	Common::Array<Opcode> op0_opcodes;
	Common::Array<Opcode> op1_opcodes;

	int _finished;
	zword zargs[8];
	int zargc;
	uint _randomInterval;
	uint _randomCtr;
	bool first_restart;
	bool script_valid;

	// Stack data
	zword _stack[STACK_SIZE];
	zword *_sp;
	zword *_fp;
	zword _frameCount;

	// Text related fields
	static zchar ZSCII_TO_LATIN1[];
	zchar *_decoded, *_encoded;
	int _resolution;
	int _errorCount[ERR_NUM_ERRORS];

	// Buffer related fields
	bool _locked;
	zchar _prevC;
	zchar _buffer[TEXT_BUFFER_SIZE];
	size_t _bufPos;

	// Stream related fields
	int script_width;
	strid_t sfp, rfp, pfp;
	bool ostream_screen;
	bool ostream_script;
	bool ostream_memory;
	bool ostream_record;
	bool istream_replay;
	bool message;
	Common::FixedStack<Redirect, MAX_NESTING> _redirect;
protected:
	/**
	 * \defgroup General support methods
	 * @{
	 */

	/**
	 * Load an operand, either a variable or a constant.
	 */
	void load_operand(zbyte type);

	/**
	 * Given the operand specifier byte, load all (up to four) operands
	 * for a VAR or EXT opcode.
	 */
	void load_all_operands(zbyte specifier);

	/**
	 * Call a subroutine. Save PC and FP then load new PC and initialise
	 * new stack frame. Note that the caller may legally provide less or
	 * more arguments than the function actually has. The call type "ct"
	 * can be 0 (z_call_s), 1 (z_call_n) or 2 (direct call).
	 */
	void call(zword routine, int argc, zword *args, int ct);

	/**
	 * Return from the current subroutine and restore the previous _stack
	 * frame. The result may be stored (0), thrown away (1) or pushed on
	 * the stack (2). In the latter case a direct call has been finished
	 * and we must exit the interpreter loop.
	 */
	void ret(zword value);

	/**
	 * Take a jump after an instruction based on the flag, either true or
	 * false. The branch can be short or long; it is encoded in one or two
	 * bytes respectively. When bit 7 of the first byte is set, the jump
	 * takes place if the flag is true; otherwise it is taken if the flag
	 * is false. When bit 6 of the first byte is set, the branch is short;
	 * otherwise it is long. The offset occupies the bottom 6 bits of the
	 * first byte plus all the bits in the second byte for long branches.
	 * Uniquely, an offset of 0 means return false, and an offset of 1 is
	 * return true.
	 */
	void branch(bool flag);

	/**
	 * Store an operand, either as a variable or pushed on the stack.
	 */
	void store(zword value);

	/*
	 * Call the interpreter loop directly. This is necessary when
	 *
	 * - a sound effect has been finished
	 * - a read instruction has timed out
	 * - a newline countdown has hit zero
	 *
	 * The interpreter returns the result value on the stack.
	 */
	int direct_call(zword addr);

	/**
	 * Set the seed value for the random number generator.
	 */
	void seed_random(int value);

	/**@}*/

	/**
	 * \defgroup Input support methods
	 * @{
	 */

	 /**
	  * High level output function.
	  */
	void print_char(zchar c);

	/**
	 * Print a string of ASCII characters.
	 */
	void print_string(const char *s);

	/*
	 * Print a unicode string.
	 */
	void print_string_uni(const uint32 *s);

	/**
	 * Print an unsigned 32bit number in decimal or hex.
	 */
	void print_long(uint value, int base);

	 /**
	  * High level newline function.
	  */
	void new_line();

	/**
	 * Copy the contents of the text buffer to the output streams.
	 */
	void flush_buffer();

	/**
	 * Returns true if the buffer is empty
	 */
	bool bufferEmpty() const { return !_bufPos; }

	/**
	 * An error has occurred. Ignore it, pass it to os_fatal or report
	 * it according to err_report_mode.
	 * @param errNum		Numeric code for error (1 to ERR_NUM_ERRORS)
	 */
	void runtimeError(ErrorCode errNum) override;

	/**@}*/

	/**
	 * \defgroup Input support methods
	 * @{
	 */

	/**
	 * Check if the given key is an input terminator.
	 */
	bool is_terminator(zchar key);

	/**
	 * Ask the user a question; return true if the answer is yes.
	 */
	bool read_yes_or_no(const char *s);

	/**
	 * Read a string from the current input stream.
	 */
	void read_string(int max, zchar *buffer);

	/**
	 * Ask the user to type in a number and return it.
	 */
	int read_number();

	/**@}*/

	/**
	 * \defgroup Memory support methods
	 * @{
	 */

	/**
	 * Called when the H_FLAGS field of the header has changed
	 */
	void flagsChanged(zbyte value) override;

	/**
	 * This function does the dirty work for z_save_undo.
	 */
	int save_undo();

	/**
	 * This function does the dirty work for z_restore_undo.
	 */
	int restore_undo();

	/**
	 * Begin output redirection to the memory of the Z-machine.
	 */
	void memory_open(zword table, zword xsize, bool buffering);

	/**
	 * End of output redirection.
	 */
	void memory_close();

	/**
	 * Redirect a newline to the memory of the Z-machine.
	 */
	void memory_new_line();

	/**
	 * Redirect a string of characters to the memory of the Z-machine.
	 */
	void memory_word(const zchar *s);

	/**@}*/

	/**
	 * \defgroup Object support methods
	 * @{
	 */

	/**
	 * Calculate the address of an object.
	 */
	zword object_address(zword obj);

	/**
	 * Return the address of the given object's name.
	 */
	zword object_name(zword object);

	/**
	 * Calculate the start address of the property list associated with an object.
	 */
	zword first_property(zword obj);

	/**
	 * Calculate the address of the next property in a property list.
	 */
	zword next_property(zword prop_addr);

	/**
	 * Unlink an object from its parent and siblings.
	 */
	void unlink_object(zword object);

	/**@}*/

	/**
	 * \defgroup Screen support methods
	 * @{
	 */

	/**
	 * Start printing a so-called debugging message. The contents of the
	 * message are passed to the message stream, a Frotz specific output
	 * stream with maximum priority.
	 */
	void screen_mssg_on();

	/**
	 * Stop printing a "debugging" message
	 */
	void screen_mssg_off();

	/**
	 * Map a runic character to its Unicode equivalent, if there is one
	 */
	uint32 zchar_to_unicode_rune(zchar c);

	/**
	 * Display a single character on the screen.
	 */
	void screen_char(zchar c);

	/**
	 * Print a newline to the screen.
	 */
	void screen_new_line();

	/**
	 * Print a newline to the screen.
	 */
	void screen_word(const zchar *s);

	/**
	 * Erase the entire screen to background colour.
	 */
	void erase_screen(zword win);

	/**
	 * Erase a window to background colour.
	 */
	void erase_window(zword win);

	/**@}*/

	/**
	 * \defgroup Stream support methods
	 * @{
	 */

	/**
	 * Waits for the user to type an input line
	 */
	zchar console_read_input(int max, zchar *buf, zword timeout, bool continued);

	/**
	 * Waits for a keypress
	 */
	zchar console_read_key(zword timeout);

	/**
	 * Write a single character to the scrollback buffer.
	 *
	 */
	void scrollback_char(zchar c);

	/**
	 * Write a string to the scrollback buffer.
	 */
	void scrollback_word(const zchar *s);

	/**
	 * Send an input line to the scrollback buffer.
	 */
	void scrollback_write_input(const zchar *buf, zchar key);

	/**
	 * Remove an input line from the scrollback buffer.
	 */
	void scrollback_erase_input(const zchar *buf);

	/**
	 * Start printing a "debugging" message.
	 */
	void stream_mssg_on();

	/**
	 * Stop printing a "debugging" message.
	 */
	void stream_mssg_off();

	/**
	 * Send a single character to the output stream.
	 */
	void stream_char(zchar c);

	/**
	 * Send a string of characters to the output streams.
	 */
	void stream_word(const zchar *s);

	/**
	 * Send a newline to the output streams.
	 */
	void stream_new_line();

	/**
	 * Read a single keystroke from the current input stream.
	 */
	zchar stream_read_key(zword timeout, zword routine, bool hot_keys);

	/**
	 * Read a line of input from the current input stream.
	 */
	zchar stream_read_input(int max, zchar *buf, zword timeout, zword routine,
		bool hot_keys, bool no_scripting);

	/*
	 * script_open
	 *
	 * Open the transscript file. 'AMFV' makes this more complicated as it
	 * turns transscription on/off several times to exclude some text from
	 * the transscription file. This wasn't a problem for the original V4
	 * interpreters which always sent transscription to the printer, but it
	 * means a problem to modern interpreters that offer to open a new file
	 * every time transscription is turned on. Our solution is to append to
	 * the old transscription file in V1 to V4, and to ask for a new file
	 * name in V5+.
	 *
	 */
	void script_open();

	/*
	 * Stop transscription.
	 */
	void script_close();

	/**
	 * Write a newline to the transscript file.
	 */
	void script_new_line();

	/**
	 * Write a single character to the transscript file.
	 */
	void script_char(zchar c);

	/**
	 * Write a string to the transscript file.
	 */
	void script_word(const zchar *s);

	/**
	 * Send an input line to the transscript file.
	 */
	void script_write_input(const zchar *buf, zchar key);

	/**
	 * Remove an input line from the transscript file.
	 */
	void script_erase_input(const zchar *buf);

	/**
	 * Start sending a "debugging" message to the transscript file.
	 */
	void script_mssg_on();

	/**
	 * Stop writing a "debugging" message.
	 */
	void script_mssg_off();

	/**
	 * Open a file to record the player's input.
	 */
	void record_open();

	/**
	 * Stop recording the player's input.
	 */
	void record_close();

	/**
	 * Helper function for record_char.
	 */
	void record_code(int c, bool force_encoding);

	/**
	 * Write a character to the command file.
	 */
	void record_char(zchar c);

	/**
	 * Copy a keystroke to the command file.
	 */
	void record_write_key(zchar key);

	/**
	 * Copy a line of input to a command file.
	 */
	void record_write_input(const zchar *buf, zchar key);

	/**
	 * Open a file of commands for playback.
	 */
	void replay_open();

	/**
	 * Stop playback of commands.
	 */
	void replay_close();

	/*
	 * Helper function for replay_key and replay_line.
	 */
	int replay_code();

	/**
	 * Read a character from the command file.
	 */
	zchar replay_char();

	/**
	 * Read a keystroke from a command file.
	 */
	zchar replay_read_key();

	/*
	 * Read a line of input from a command file.
	 */
	zchar replay_read_input(zchar *buf);

	/**@}*/

	/**
	 * \defgroup Text support methods
	 * @{
	 */

	/**
	 * Map a ZSCII character into Unicode.
	 */
	zchar translate_from_zscii(zbyte c);

	/**
	 * Convert a Unicode character to ZSCII, returning 0 on failure.
	 */
	zbyte unicode_to_zscii(zchar c);

	/**
	 * Map a Unicode character onto the ZSCII alphabet.
	 */
	zbyte translate_to_zscii(zchar c);

	/**
	 * Return a character from one of the three character sets.
	 */
	zchar alphabet(int set, int index);

	/**
	 * Find the number of bytes used for dictionary resolution.
	 */
	void find_resolution();

	/**
	 * Copy a ZSCII string from the memory to the global "decoded" string.
	 */
	void load_string(zword addr, zword length);

	/**
	 * Encode the Unicode text in the global "decoded" string then write
	 * the result to the global "encoded" array. (This is used to look up
	 * words in the dictionary.) Up to V3 the vocabulary resolution is
	 * two, from V4 it is three, and from V9 it is any number of words.
	 * Because each word contains three Z-characters, that makes six or
	 * nine Z-characters respectively. Longer words are chopped to the
	 * proper size, shorter words are are padded out with 5's. For word
	 * completion we pad with 0s and 31s, the minimum and maximum
	 * Z-characters.
	 */
	void encode_text(int padding);

	/**
	 * Convert _encoded text to Unicode. The _encoded text consists of 16bit
	 * words. Every word holds 3 Z-characters (5 bits each) plus a spare
	 * bit to mark the last word. The Z-characters translate to ZSCII by
	 * looking at the current current character set. Some select another
	 * character set, others refer to abbreviations.
	 *
	 * There are several different string types:
	 *
	 *    LOW_STRING - from the lower 64KB (byte address)
	 *    ABBREVIATION - from the abbreviations table (word address)
	 *    HIGH_STRING - from the end of the memory map (packed address)
	 *    EMBEDDED_STRING - from the instruction stream (at PC)
	 *    VOCABULARY - from the dictionary (byte address)
	 *
	 * The last type is only used for word completion.
	 */
	void decode_text(string_type st, zword addr);

	/**
	 * Print a signed 16bit number.
	 */
	void print_num(zword value);

	/**
	 * print_object
	 *
	 * Print an object description.
	 *
	 */
	void print_object(zword object);

	/**
	 * Scan a dictionary searching for the given word. The first argument
	 * can be
	 *
	 * 0x00 - find the first word which is >= the given one
	 * 0x05 - find the word which exactly matches the given one
	 * 0x1f - find the last word which is <= the given one
	 *
	 * The return value is 0 if the search fails.
	 */
	zword lookup_text(int padding, zword dct);

	/**
	 * Handles converting abbreviations that weren't handled by early Infocom games
	 * into their expanded versions
	 */
	void handleAbbreviations();

	/**
	 * Translate a single word to a token and append it to the token
	 * buffer. Every token consists of the address of the dictionary
	 * entry, the length of the word and the offset of the word from
	 * the start of the text buffer. Unknown words cause empty slots
	 * if the flag is set (such that the text can be scanned several
	 * times with different dictionaries); otherwise they are zero.
	 *
	 */
	void tokenise_text(zword text, zword length, zword from, zword parse, zword dct, bool flag);

	/**
	 * Split an input line into words and translate the words to tokens.
	 */
	void tokenise_line(zword text, zword token, zword dct, bool flag);

	/**
	 * Scan the vocabulary to complete the last word on the input line
	 * (similar to "tcsh" under Unix). The return value is
	 *
	 *    2 ==> completion is impossible
	 *    1 ==> completion is ambiguous
	 *    0 ==> completion is successful
	 *
	 * The function also returns a string in its second argument. In case
	 * of 2, the string is empty; in case of 1, the string is the longest
	 * extension of the last word on the input line that is common to all
	 * possible completions (for instance, if the last word on the input
	 * is "fo" and its only possible completions are "follow" and "folly"
	 * then the string is "ll"); in case of 0, the string is an extension
	 * to the last word that results in the only possible completion.
	 */
	int completion(const zchar *buffer, zchar *result);

	 /**
	  * Convert a Unicode character to lowercase.
	  * Taken from Zip2000 by Kevin Bracey.
	  */
	zchar unicode_tolower(zchar c);

	/**@}*/

	/**
	 * \defgroup Window/V6 Opcode methods
	 * @{
	 */

	/**
	 * Return the window number in zargs[0]. In V6 only, -3 refers to the
	 * current window.
	 */
	zword winarg0();

	/**
	 * Return the (optional) window number in zargs[2]. -3 refers to the
	 * current window. This optional window number was only used by some
	 * V6 opcodes: set_cursor, set_margins, set_colour.
	 */
	zword winarg2();

	 /**@}*/
protected:
	/**
	 * \defgroup General Opcode methods
	 * @{
	 */

	/*
	 * Load and execute an extended opcode.
	 */
	void __extended__();

	/*
	 * Exit game because an unknown opcode has been hit.
	 */
	void __illegal__();

	/*
	 * Store the current _stack frame for later use with z_throw.
	 *
	 *	no zargs used
	 */
	void z_catch();

	/**
	 * Go back to the given _stack frame and return the given value.
	 *
	 *	zargs[0] = value to return
	 *	zargs[1] = _stack frame
	 */
	void z_throw();

	/*
	 * Call a subroutine and discard its result.
	 *
	 * 	zargs[0] = packed address of subroutine
	 *	zargs[1] = first argument (optional)
	 *	...
	 *	zargs[7] = seventh argument (optional)
	 */
	void z_call_n();

	/**
	 * Call a subroutine and store its result.
	 *
	 * 	zargs[0] = packed address of subroutine
	 *	zargs[1] = first argument (optional)
	 *	...
	 *	zargs[7] = seventh argument (optional)
	 */
	void z_call_s();

	/**
	 * Branch if subroutine was called with >= n arg's.
	 *
	 * 	zargs[0] = number of arguments
	 */
	void z_check_arg_count();

	/**
	 * Jump unconditionally to the given address.
	 *
	 *	zargs[0] = PC relative address
	 */
	void z_jump();

	/*
	 * No operation.
	 *
	 *	no zargs used
	 */
	void z_nop();

	/*
	 * Stop game and exit interpreter.
	 *
	 *	no zargs used
	 */
	void z_quit();

	/*
	 * Return from a subroutine with the given value.
	 *
	 *	zargs[0] = value to return
	 */
	void z_ret();

	/*
	 * Return from a subroutine with a value popped off the stack.
	 *
	 *	no zargs used
	 */
	void z_ret_popped();

	/*
	 * Return from a subroutine with false (0).
	 *
	 * 	no zargs used
	 */
	void z_rfalse();

	/*
	 * Return from a subroutine with true (1).
	 *
	 * 	no zargs used
	 */
	void z_rtrue();

	/**
	 * Store a random number or set the random number seed.
	 *
	 *	zargs[0] = range (positive) or seed value (negative)
	 */
	void z_random();

	/**
	 * Load / play / stop / discard a sound effect.
	 *
	 *	zargs[0] = number of bleep (1 or 2) or sample
	 *	zargs[1] = operation to perform (samples only)
	 *	zargs[2] = repeats and volume (play sample only)
	 *	zargs[3] = end-of-sound routine (play sample only, optional)
	 *
	 * Note: Volumes range from 1 to 8, volume 255 is the default volume.
	 *	 Repeats are stored in the high byte, 255 is infinite loop.
	 *
	 */
	void z_sound_effect();

	/**
	 * Branch if the story file is a legal copy
	 */
	void z_piracy();

	/**
	 * Save the current Z-machine state for a future undo.
	 *
	 *	no zargs used
	 */
	void z_save_undo();

	/**
	 * Restore a Z-machine state from memory.
	 *
	 *	no zargs used
	 */
	void z_restore_undo();

	/**@}*/

	/**
	 * \defgroup Input Opcode methods
	 * @{
	 */

	 /**
	  * Add or remove a menu and branch if successful.
	  *
	  * 	zargs[0] = number of menu
	  *	zargs[1] = table of menu entries or 0 to remove menu
	  */
	void z_make_menu();

	/**
	 * Read a line of input and (in V5+) store the terminating key.
	 *
	 *	zargs[0] = address of text buffer
	 *	zargs[1] = address of token buffer
	 *	zargs[2] = timeout in tenths of a second (optional)
	 *	zargs[3] = packed address of routine to be called on timeout
	 */
	void z_read();

	/**
	 * Read and store a key.
	 *
	 *	zargs[0] = input device (must be 1)
	 *	zargs[1] = timeout in tenths of a second (optional)
	 *	zargs[2] = packed address of routine to be called on timeout
	 */
	void z_read_char();

	/**
	 * z_read_mouse, write the current mouse status into a table.
	 *
	 *	zargs[0] = address of table
	 */
	void z_read_mouse();

	/**@}*/

	/**
	 * \defgroup Math Opcode methods
	 * @{
	 */

	/**
	 * 16 bit addition.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_add();

	/**
	 * Bitwise AND operation.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_and();

	/**
	 * Arithmetic SHIFT operation.
	 *
	 *	zargs[0] = value
	 *	zargs[1] = #positions to shift left (positive) or right
	 */
	void z_art_shift();

	/**
	 * Signed 16bit division.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_div();

	/**
	 * B ranch if the first value equals any of the following.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value (optional)
	 *	...
	 *	zargs[3] = fourth value (optional)
	 */
	void z_je();

	/**
	 * Branch if the first value is greater than the second.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_jg();

	/**
	 * Branch if the first value is less than the second.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_jl();

	/**
	 * Branch if value is zero.
	 *
	 * 	zargs[0] = value
	 */
	void z_jz();

	/**
	 * Logical SHIFT operation.
	 *
	 * 	zargs[0] = value
	 *	zargs[1] = #positions to shift left (positive) or right (negative)
	 */
	void z_log_shift();

	/*
	 * Remainder after signed 16bit division.
	 *
	 * 	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_mod();

	/**
	 * 16 bit multiplication.
	 *
	 * 	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_mul();

	/**
	 * Bitwise NOT operation.
	 *
	 * 	zargs[0] = value
	 */
	void z_not();

	/**
	 * Bitwise OR operation.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_or();

	/**
	 * 16 bit substraction.
	 *
	 *	zargs[0] = first value
	 *	zargs[1] = second value
	 */
	void z_sub();

	/**
	 * Branch if all the flags of a bit mask are set in a value.
	 *
	 *	zargs[0] = value to be examined
	 *	zargs[1] = bit mask
	 */
	void z_test();

	/**@}*/

	/**
	 * \defgroup Object Opcode methods
	 * @{
	 */

	/**
	 * Branch if the first object is inside the second.
	 *
	 *        zargs[0] = first object
	 *        zargs[1] = second object
	 */
	void z_jin();

	/**
	 * Store the child of an object.
	 *
	 *        zargs[0] = object
	 */
	void z_get_child();

	/**
	 * Store the number of the first or next property.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = address of current property (0 gets the first property)
	 */
	void z_get_next_prop();

	/**
	 * Store the parent of an object.
	 *
	 *        zargs[0] = object
	 */
	void z_get_parent();

	/**
	 * Store the value of an object property.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = number of property to be examined
	 */
	void z_get_prop();

	/**
	 * Store the address of an object property.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = number of property to be examined
	 */
	void z_get_prop_addr();

	/**
	 * Store the length of an object property.
	 *
	 *         zargs[0] = address of property to be examined
	 */
	void z_get_prop_len();

	/**
	 * Store the sibling of an object.
	 *
	 *        zargs[0] = object
	 */
	void z_get_sibling();

	/**
	 * Make an object the first child of another object.
	 *
	 *        zargs[0] = object to be moved
	 *        zargs[1] = destination object
	 */
	void z_insert_obj();

	/**
	 * Set the value of an object property.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = number of property to set
	 *        zargs[2] = value to set property to
	 */
	void z_put_prop();

	/**
	 * Unlink an object from its parent and siblings.
	 *
	 *        zargs[0] = object
	 */
	void z_remove_obj();

	/**
	 * Set an object attribute.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = number of attribute to set
	 */
	void z_set_attr();

	/**
	 * Branch if an object attribute is set.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = number of attribute to test
	 */
	void z_test_attr();

	/**
	 * Clear an object attribute.
	 *
	 *        zargs[0] = object
	 *        zargs[1] = number of attribute to be cleared
	 */
	void z_clear_attr();

	/**@}*/

	/**
	 * \defgroup Screen Opcode methods
	 * @{
	 */

	/**
	 * Turn text buffering on/off.
	 *
	 *		zargs[0] = new text buffering flag (0 or 1)
	 */
	void z_buffer_mode();

	/**
	 * Set the screen buffering mode.
	 *
	 *	zargs[0] = mode
	 */
	void z_buffer_screen();

	/**
	 * Erase the line starting at the cursor position.
	 *
	 *		zargs[0] = 1 + #units to erase (1 clears to the end of the line)
	 */
	void z_erase_line();

	/**
	 * Erase a window or the screen to background colour.
	 *
	 *		zargs[0] = window (-3 current, -2 screen, -1 screen & unsplit)
	 */
	void z_erase_window();

	/**
	 * Write the cursor coordinates into a table.
	 *
	 *		zargs[0] = address to write information to
	 */
	void z_get_cursor();

	/**
	 * Print ASCII text in a rectangular area.
	 *
	 *		zargs[0] = address of text to be printed
	 *		zargs[1] = width of rectangular area
	 *		zargs[2] = height of rectangular area (optional)
	 *		zargs[3] = number of char's to skip between lines (optional)
	 */
	void z_print_table();

	/**
	 * Set the foreground and background colours
	 * to specific RGB colour values.
	 *
	 *	zargs[0] = foreground colour
	 *	zargs[1] = background colour
	 *	zargs[2] = window (-3 is the current one, optional)
	 */
	void z_set_true_colour();

	/**
	 * Set the foreground and background colours.
	 *
	 *		zargs[0] = foreground colour
	 *		zargs[1] = background colour
	 *		zargs[2] = window (-3 is the current one, optional)
	 */
	void z_set_colour();

	/**
	 * Set the font for text output and store the previous font.
	 *
	 *		 zargs[0] = number of font or 0 to keep current font
	 */
	void z_set_font();

	/**
	 * Set the cursor position or turn the cursor on/off.
	 *
	 *		zargs[0] = y-coordinate or -2/-1 for cursor on/off
	 *		zargs[1] = x-coordinate
	 *		zargs[2] = window (-3 is the current one, optional)
	 */
	void z_set_cursor();

	/**
	 * z_set_text_style, set the style for text output.
	 *
	 *		 zargs[0] = style flags to set or 0 to reset text style
	 */
	void z_set_text_style();

	/**
	 * Select the current window.
	 *
	 *		zargs[0] = window to be selected (-3 is the current one)
	 */
	void z_set_window();

	/**
	 * Display the status line for V1 to V3 games.
	 *
	 *		no zargs used
	 */
	void pad_status_line(int column);

	/**
	 * Display the status line for V1 to V3 games.
	 *
	 *		no zargs used
	 */
	void z_show_status();

	/**
	 * Split the screen into an upper (1) and lower (0) window.
	 *
	 *		zargs[0] = height of upper window in screen units (V6) or #lines
	 */
	void z_split_window();

	/**@}*/

	/**
	 * \defgroup Stream Opcode methods
	 * @{
	 */

	/**
	 * Select an input stream.
	 *
	 *	zargs[0] = input stream to be selected
	 */
	void z_input_stream();

	/**
	 * Open or close an output stream.
	 *
	 *	zargs[0] = stream to open (positive) or close (negative)
	 *	zargs[1] = address to redirect output to (stream 3 only)
	 *	zargs[2] = width of redirected output (stream 3 only, optional)
	 */
	void z_output_stream();

	/**
	 * Re-load dynamic area, clear the stack and set the PC.
	 *
	 * 	no zargs used
	 */
	void z_restart();

	/**
	 * Save [a part of] the Z-machine state to disk.
	 *
	 *	zargs[0] = address of memory area to save (optional)
	 *	zargs[1] = number of bytes to save
	 *	zargs[2] = address of suggested file name
	 */
	void z_save();

	/**
	 * Restore [a part of] a Z-machine state from disk
	 *
	 *	zargs[0] = address of area to restore (optional)
	 *	zargs[1] = number of bytes to restore
	 *	zargs[2] = address of suggested file name
	 */
	void z_restore();

	/**
	 * Check the story file integrity.
	 *
	 *	no zargs used
	 */
	void z_verify();

	/**@}*/

	/**
	 * \defgroup Table Opcode methods
	 * @{
	 */

	/**
	 * Copy a table or fill it with zeroes.
	 *
	 *	zargs[0] = address of table
	 * 	zargs[1] = destination address or 0 for fill
	 *	zargs[2] = size of table
	 *
	 * Note: Copying is safe even when source and destination overlap; but
	 *       if zargs[1] is negative the table _must_ be copied forwards.
	 */
	void z_copy_table();

	/**
	 * Store a value from a table of bytes.
	 *
	 *	zargs[0] = address of table
	 *	zargs[1] = index of table entry to store
	 */
	void z_loadb();

	/**
	 * Store a value from a table of words.
	 *
	 *	zargs[0] = address of table
	 *	zargs[1] = index of table entry to store
	 */
	void z_loadw();

	/**
	 * Find and store the address of a target within a table.
	 *
	 *	zargs[0] = target value to be searched for
	 *	zargs[1] = address of table
	 *	zargs[2] = number of table entries to check value against
	 *	zargs[3] = type of table (optional, defaults to 0x82)
	 *
	 * Note: The table is a word array if bit 7 of zargs[3] is set; otherwise
	 *       it's a byte array. The lower bits hold the address step.
	 */
	void z_scan_table();

	/**
	 * Write a byte into a table of bytes.
	 *
	 *	zargs[0] = address of table
	 *	zargs[1] = index of table entry
	 *	zargs[2] = value to be written
	 */
	void z_storeb();

	/**
	 * Write a word into a table of words.
	 *
	 *	zargs[0] = address of table
	 *	zargs[1] = index of table entry
	 *	zargs[2] = value to be written
	 */
	void z_storew();

	/**@}*/

	/**
	 * \defgroup Text Opcode methods
	 * @{
	 */

	/**
	 * Test if a unicode character can be printed (bit 0) and read (bit 1).
	 *
	 * 	zargs[0] = Unicode
	 */
	void z_check_unicode();

	/**
	 * Encode a ZSCII string for use in a dictionary.
	 *
	 *	zargs[0] = address of text buffer
	 *	zargs[1] = length of ASCII string
	 *	zargs[2] = offset of ASCII string within the text buffer
	 *	zargs[3] = address to store encoded text in
	 *
	 * This is a V5+ opcode and therefore the dictionary resolution must be
	 * three 16bit words.
	 */
	void z_encode_text();

	/**
	 * Print a new line.
	 *
	 * 	no zargs used
	 *
	 */
	void z_new_line();

	/**
	 * Print a string embedded in the instruction stream.
	 *
	 *	no zargs used
	 */
	void z_print();

	/**
	 * Print a string from the lower 64KB.
	 *
	 *	zargs[0] = address of string to print
	 */
	void z_print_addr();

	/**
	 * Print a single ZSCII character.
	 *
	 *	zargs[0] = ZSCII character to be printed
	 */
	void z_print_char();

	/**
	 * Print a formatted table.
	 *
	 *	zargs[0] = address of formatted table to be printed
	 */
	void z_print_form();

	/**
	 * Print a signed number.
	 *
	 * 	zargs[0] = number to print
	 */
	void z_print_num();

	/**
	 * Print an object description.
	 *
	 * 	zargs[0] = number of object to be printed
	 */
	void z_print_obj();

	/**
	 * Print the string at the given packed address.
	 *
	 * 	zargs[0] = packed address of string to be printed
	 */
	void z_print_paddr();

	/*
	 * Print the string at PC, print newline then return true.
	 *
	 * 	no zargs used
	 */
	void z_print_ret();

	/**
	 * Print unicode character
	 *
	 * 	zargs[0] = Unicode
	 */
	void z_print_unicode();

	/**
	 * Make a lexical analysis of a ZSCII string.
	 *
	 *	zargs[0] = address of string to analyze
	 *	zargs[1] = address of token buffer
	 *	zargs[2] = address of dictionary (optional)
	 *	zargs[3] = set when unknown words cause empty slots (optional)
	 */
	void z_tokenise();

	 /**@}*/

	/**
	 * \defgroup Variable Opcode methods
	 * @{
	 */

	/**
	 * Decrement a variable.
	 *
	 * 	zargs[0] = variable to decrement
	 */
	void z_dec();

	/**
	 * Decrement a variable and branch if now less than value.
	 *
	 * 	zargs[0] = variable to decrement
	 * 	zargs[1] = value to check variable against
	 */
	void z_dec_chk();

	/**
	 * Increment a variable.
	 *
	 * 	zargs[0] = variable to increment
	 */
	void z_inc();

	/**
	 * Increment a variable and branch if now greater than value.
	 *
	 * 	zargs[0] = variable to increment
	 * 	zargs[1] = value to check variable against
	 */
	void z_inc_chk();

	/**
	 * Store the value of a variable.
	 *
	 *	zargs[0] = variable to store
	 */
	void z_load();

	/**
	 * Pop a value off the game stack and discard it.
	 *
	 *	no zargs used
	 */
	void z_pop();

	/**
	 * Pop n values off the game or user stack and discard them.
	 *
	 *	zargs[0] = number of values to discard
	 *	zargs[1] = address of user stack (optional)
	 */
	void z_pop_stack();

	/**
	 * Pop a value off...
	 *
	 * a) ...the game or a user stack and store it (V6)
	 *
	 *	zargs[0] = address of user stack (optional)
	 *
	 * b) ...the game stack and write it to a variable (other than V6)
	 *
	 *	zargs[0] = variable to write value to
	 */
	void z_pull();

	/**
	 * Push a value onto the game stack.
	 *
	 *	zargs[0] = value to push onto the stack
	 */
	void z_push();

	/**
	 * Push a value onto a user stack then branch if successful.
	 *
	 *	zargs[0] = value to push onto the stack
	 *	zargs[1] = address of user stack
	 */
	void z_push_stack();

	/**
	 * Write a value to a variable.
	 *
	 * 	zargs[0] = variable to be written to
	 *      zargs[1] = value to write
	 */
	void z_store();

	/**@}*/

	/**
	 * \defgroup Window/V6 Opcode methods
	 * @{
	 */

	/**
	 * z_draw_picture, draw a picture.
	 *
	 *	zargs[0] = number of picture to draw
	 *	zargs[1] = y-coordinate of top left corner
	 *	zargs[2] = x-coordinate of top left corner
	 */
	void z_draw_picture();

	/**
	 * Get information on a picture or the graphics file.
	 *
	 *	zargs[0] = number of picture or 0 for the graphics file
	 *	zargs[1] = address to write information to
	 */
	void z_picture_data();

	/**
	 * Erase a picture with background colour.
	 *
	 *	zargs[0] = number of picture to erase
	 *	zargs[1] = y-coordinate of top left corner (optional)
	 *	zargs[2] = x-coordinate of top left corner (optional)
	 */
	void z_erase_picture();

	/**
	 * Set the left and right margins of a window.
	 *
	 *	zargs[0] = left margin in pixels
	 *	zargs[1] = right margin in pixels
	 *	zargs[2] = window (-3 is the current one, optional)
	 */
	void z_set_margins();


	/**
	 * Place a window on the screen.
	 *
	 *	zargs[0] = window (-3 is the current one)
	 *	zargs[1] = y-coordinate
	 *	zargs[2] = x-coordinate
	 *
	 */
	void z_move_window();

	/**
	 * Change the width and height of a window.
	 *
	 *	zargs[0] = window (-3 is the current one)
	 *	zargs[1] = new height in screen units
	 *	zargs[2] = new width in screen units
	 */
	void z_window_size();

	/**
	 * Set / clear / toggle window attributes.
	 *
	 *	zargs[0] = window (-3 is the current one)
	 *	zargs[1] = window attribute flags
	 *	zargs[2] = operation to perform (optional, defaults to 0)
	 */
	void z_window_style();

	/**
	 * Store the value of a window property.
	 *
	 *	zargs[0] = window (-3 is the current one)
	 *	zargs[1] = number of window property to be stored
	 */
	void z_get_wind_prop();

	/**
	 * Set the value of a window property.
	 *
	 *	zargs[0] = window (-3 is the current one)
	 *	zargs[1] = number of window property to set
	 *	zargs[2] = value to set window property to
	 */
	void z_put_wind_prop();

	/**
	 * Scroll a window up or down.
	 *
	 *	zargs[0] = window (-3 is the current one)
	 *	zargs[1] = #screen units to scroll up (positive) or down (negative)
	 */
	void z_scroll_window();

	/**
	 * Select a window as mouse window.
	 *
	 *	zargs[0] = window number (-3 is the current) or -1 for the screen
	 */
	void z_mouse_window();

	/**
	 * Prepare a group of pictures for faster display.
	 *
	 *	zargs[0] = address of table holding the picture numbers
	 */
	void z_picture_table();

	 /**@}*/
public:
	/**
	 * Constructor
	 */
	Processor(OSystem *syst, const GlkGameDescription &gameDesc);
	~Processor() override {}

	/**
	 * Initialization
	 */
	void initialize();

	/**
	 * Z-code interpreter main loop
	 */
	void interpret();

	/**
	 * \defgroup Memory access methods
	 * @{
	 */

	/**
	 * Square brackets operator
	 */
	zbyte &operator[](uint addr) { return zmp[addr]; }

	/**
	 * Read a code byte
	 */
	zbyte codeByte() { return *pcp++; }

	/**
	 * Read a code word
	 */
	zword codeWord() {
		zword v = READ_BE_UINT16(pcp);
		pcp += 2;
		return v;
	}

	/**
	 * Return a code word at a given address
	 */
	zword codeWordIdx(uint addr) const {
		return READ_BE_UINT16(pcp + addr);
	}

	/**
	 * Return the current program execution offset
	 */
	uint getPC() const {
		assert(pcp);
		return pcp - zmp;
	}

	/**
	 * Set the program execution offset
	 */
	void setPC(uint addr) { pcp = zmp + addr; }

	 /**@}*/
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
