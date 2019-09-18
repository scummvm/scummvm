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
#include "glk/adrift/scprotos.h"
#include "glk/adrift/serialization.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_uint MEMENTO_MAGIC = 0x9fd33d1d;
enum { MEMO_ALLOCATION_BLOCK = 32 };

/*
 * Game memo structure, saves a serialized game.  Allocation is preserved so
 * that structures can be reused without requiring reallocation.
 */
struct sc_memo_s {
	sc_byte *serialized_game;
	sc_int allocation;
	sc_int length;
};
typedef sc_memo_s sc_memo_t;
typedef sc_memo_t *sc_memoref_t;

/*
 * Game command history structure, similar to a memo.  Saves a player input
 * command to create a history, reusing allocation where possible.
 */
struct sc_history_s {
	sc_char *command;
	sc_int sequence;
	sc_int timestamp;
	sc_int turns;
	sc_int allocation;
	sc_int length;
};
typedef sc_history_s sc_history_t;
typedef sc_history_t *sc_historyref_t;

/*
 * Memo set structure.  This reserves space for a predetermined number of
 * serialized games, and an indicator cursor showing where additions are
 * placed.  The structure is a ring, with old elements being overwritten by
 * newer arrivals.  Also tacked onto this structure is a set of strings
 * used to hold a command history that operates in a somewhat csh-like way,
 * also a ring with limited capacity.
 */
enum { MEMO_UNDO_TABLE_SIZE = 16, MEMO_HISTORY_TABLE_SIZE = 64 };
struct sc_memo_set_s {
	sc_uint magic;
	sc_memo_t memo[MEMO_UNDO_TABLE_SIZE];
	sc_int memo_cursor;

	sc_history_t history[MEMO_HISTORY_TABLE_SIZE];
	sc_int history_count;
	sc_int current_history;
	sc_bool is_at_start;
};
typedef sc_memo_set_s sc_memo_set_t;


/*
 * memo_is_valid()
 *
 * Return TRUE if pointer is a valid memo set, FALSE otherwise.
 */
static sc_bool memo_is_valid(sc_memo_setref_t memento) {
	return memento && memento->magic == MEMENTO_MAGIC;
}


/*
 * memo_round_up()
 *
 * Round up an allocation in bytes to the next allocation block.
 */
static sc_int memo_round_up(sc_int allocation) {
	sc_int extended;

	extended = allocation + MEMO_ALLOCATION_BLOCK - 1;
	return (extended / MEMO_ALLOCATION_BLOCK) * MEMO_ALLOCATION_BLOCK;
}


/*
 * memo_create()
 *
 * Create and return a new set of memos.
 */
sc_memo_setref_t memo_create(void) {
	sc_memo_setref_t memento;

	/* Create and initialize a clean set of memos. */
	memento = (sc_memo_setref_t)sc_malloc(sizeof(*memento));
	memento->magic = MEMENTO_MAGIC;

	memset(memento->memo, 0, sizeof(memento->memo));
	memento->memo_cursor = 0;

	memset(memento->history, 0, sizeof(memento->history));
	memento->history_count = 0;
	memento->current_history = 0;
	memento->is_at_start = FALSE;

	return memento;
}


/*
 * memo_destroy()
 *
 * Destroy a memo set, and free its heap memory.
 */
void memo_destroy(sc_memo_setref_t memento) {
	sc_int index_;
	assert(memo_is_valid(memento));

	/* Free the content of any used memo and any used history. */
	for (index_ = 0; index_ < MEMO_UNDO_TABLE_SIZE; index_++) {
		sc_memoref_t memo;

		memo = memento->memo + index_;
		sc_free(memo->serialized_game);
	}
	for (index_ = 0; index_ < MEMO_HISTORY_TABLE_SIZE; index_++) {
		sc_historyref_t history;

		history = memento->history + index_;
		sc_free(history->command);
	}

	/* Poison and free the memo set itself. */
	memset(memento, 0xaa, sizeof(*memento));
	sc_free(memento);
}


/*
 * memo_save_game_callback()
 *
 * Callback function for game serialization.  Appends the data passed in to
 * that already stored in the memo.
 */
static void memo_save_game_callback(void *opaque, const sc_byte *buffer, sc_int length) {
	sc_memoref_t memo = (sc_memoref_t)opaque;
	sc_int required;
	assert(opaque && buffer && length > 0);

	/*
	 * If necessary, increase the allocation for this memo.  Serialized games
	 * tend to grow slightly as the game progresses, so we add a bit of extra
	 * to the actual allocation.
	 */
	required = memo->length + length;
	if (required > memo->allocation) {
		required = memo_round_up(required + 2 * MEMO_ALLOCATION_BLOCK);
		memo->serialized_game = (sc_byte *)sc_realloc(memo->serialized_game, required);
		memo->allocation = required;
	}

	/* Add this block of data to the buffer. */
	memcpy(memo->serialized_game + memo->length, buffer, length);
	memo->length += length;
}


/*
 * memo_save_game()
 *
 * Store a game in the next memo slot.
 */
void memo_save_game(sc_memo_setref_t memento, sc_gameref_t game) {
	sc_memoref_t memo;
	assert(memo_is_valid(memento));

	/*
	 * If the current slot is in use, we can re-use its allocation.  Saved
	 * games will tend to be of roughly equal sizes, so it's worth doing.
	 */
	memo = memento->memo + memento->memo_cursor;
	memo->length = 0;

	/* Serialize the given game into this memo. */
	SaveSerializer ser(game, memo_save_game_callback, memo);
	ser.save();

	/*
	 * If serialization worked (failure would be a surprise), advance the
	 * current memo cursor.
	 */
	if (memo->length > 0) {
		memento->memo_cursor++;
		memento->memo_cursor %= MEMO_UNDO_TABLE_SIZE;
	} else
		sc_error("memo_save_game: warning: game save failed\n");
}


/*
 * memo_load_game_callback()
 *
 * Callback function for game deserialization.  Returns data from the memo
 * until it's drained.
 */
static sc_int memo_load_game_callback(void *opaque, sc_byte *buffer, sc_int length) {
	sc_memoref_t memo = (sc_memoref_t)opaque;
	sc_int bytes;
	assert(opaque && buffer && length > 0);

	/* Send back either all the bytes, or as many as the buffer allows. */
	bytes = (memo->length < length) ? memo->length : length;

	/* Read and remove the first block of data (or all if less than length). */
	memcpy(buffer, memo->serialized_game, bytes);
	memmove(memo->serialized_game,
	        memo->serialized_game + bytes, memo->length - bytes);
	memo->length -= bytes;

	/* Return the count of bytes placed in the buffer. */
	return bytes;
}


/*
 * memo_load_game()
 *
 * Restore a game from the last memo slot used, if possible.
 */
sc_bool memo_load_game(sc_memo_setref_t memento, sc_gameref_t game) {
	sc_int cursor;
	sc_memoref_t memo;
	assert(memo_is_valid(memento));

	/* Look back one from the current memo cursor. */
	cursor = (memento->memo_cursor == 0)
	         ? MEMO_UNDO_TABLE_SIZE - 1 : memento->memo_cursor - 1;
	memo = memento->memo + cursor;

	/* If this slot is not empty, restore the serialized game held in it. */
	if (memo->length > 0) {
		sc_bool status;

		/*
		 * Deserialize the given game from this memo; failure would be somewhat
		 * of a surprise here.
		 */
		LoadSerializer ser(game, memo_load_game_callback, memo);
		status = ser.load();
		if (!status)
			sc_error("memo_load_game: warning: game load failed\n");

		/*
		 * This should have drained the memo of all data, but to be sure that
		 * there's no chance of trying to restore from this slot again, we'll
		 * force it anyway.
		 */
		if (memo->length > 0) {
			sc_error("memo_load_game: warning: data remains after loading\n");
			memo->length = 0;
		}

		/* Regress current memo, and return TRUE if we restored a memo. */
		memento->memo_cursor = cursor;
		return status;
	}

	/* There are no more memos to restore. */
	return FALSE;
}


/*
 * memo_is_load_available()
 *
 * Returns TRUE if a memo restore is likely to succeed if called, FALSE
 * otherwise.
 */
sc_bool memo_is_load_available(sc_memo_setref_t memento) {
	sc_int cursor;
	sc_memoref_t memo;
	assert(memo_is_valid(memento));

	/*
	 * Look back one from the current memo cursor.  Return TRUE if this slot
	 * contains a serialized game.
	 */
	cursor = (memento->memo_cursor == 0)
	         ? MEMO_UNDO_TABLE_SIZE - 1 : memento->memo_cursor - 1;
	memo = memento->memo + cursor;
	return memo->length > 0;
}


/*
 * memo_clear_games()
 *
 * Forget the memos of saved games.
 */
void memo_clear_games(sc_memo_setref_t memento) {
	sc_int index_;
	assert(memo_is_valid(memento));

	/* Deallocate every entry. */
	for (index_ = 0; index_ < MEMO_UNDO_TABLE_SIZE; index_++) {
		sc_memoref_t memo;

		memo = memento->memo + index_;
		sc_free(memo->serialized_game);
	}

	/* Reset all entries and the cursor. */
	memset(memento->memo, 0, sizeof(memento->memo));
	memento->memo_cursor = 0;
}


/*
 * memo_save_command()
 *
 * Store a player command in the command history, evicting any least recently
 * used item if necessary.
 */
void memo_save_command(sc_memo_setref_t memento, const sc_char *command, sc_int timestamp, sc_int turns) {
	sc_historyref_t history;
	sc_int length;
	assert(memo_is_valid(memento));

	/* As with memos, reuse the allocation of the next slot if it has one. */
	history = memento->history
	          + memento->history_count % MEMO_HISTORY_TABLE_SIZE;

	/*
	 * Resize the allocation for this slot if required.  Strings tend to be
	 * short, so round up to a block to avoid too many reallocs.
	 */
	length = strlen(command) + 1;
	if (history->allocation < length) {
		sc_int required;

		required = memo_round_up(length);
		history->command = (sc_char *)sc_realloc(history->command, required);
		history->allocation = required;
	}

	/* Save the string into this slot, and normalize it for neatness. */
	strcpy(history->command, command);
	sc_normalize_string(history->command);
	history->sequence = memento->history_count + 1;
	history->timestamp = timestamp;
	history->turns = turns;
	history->length = length;

	/* Increment the count of histories handled. */
	memento->history_count++;
}


/*
 * memo_unsave_command()
 *
 * Remove the last saved command.  This is special functionality for the
 * history lister.  To keep synchronized with the runner main loop, it needs
 * to "invent" a history item at the end of the list before listing, then
 * remove it again as the main runner loop will add the real thing.
 */
void memo_unsave_command(sc_memo_setref_t memento) {
	assert(memo_is_valid(memento));

	/* Do nothing if for some reason there's no history to unsave. */
	if (memento->history_count > 0) {
		sc_historyref_t history;

		/* Decrement the count of histories handled, erase the prior entry. */
		memento->history_count--;
		history = memento->history
		          + memento->history_count % MEMO_HISTORY_TABLE_SIZE;
		history->sequence = 0;
		history->timestamp = 0;
		history->turns = 0;
		history->length = 0;
	}
}


/*
 * memo_get_command_count()
 *
 * Return a count of available saved commands.
 */
sc_int memo_get_command_count(sc_memo_setref_t memento) {
	assert(memo_is_valid(memento));

	/* Return the lesser of the history count and the history table size. */
	if (memento->history_count < MEMO_HISTORY_TABLE_SIZE)
		return memento->history_count;
	else
		return MEMO_HISTORY_TABLE_SIZE;
}


/*
 * memo_first_command()
 *
 * Iterator rewind function, reset current location to the first command.
 */
void memo_first_command(sc_memo_setref_t memento) {
	sc_int cursor;
	sc_historyref_t history;
	assert(memo_is_valid(memento));

	/*
	 * If the buffer has cycled, we have the full complement of saved commands,
	 * so start iterating at the current cursor.  Otherwise, start from index 0.
	 * Detect cycling by looking at the current slot; if it's filled, we've
	 * been here before.  Set at_start flag to indicate the special case for
	 * circular buffers.
	 */
	cursor = memento->history_count % MEMO_HISTORY_TABLE_SIZE;
	history = memento->history + cursor;
	memento->current_history = (history->length > 0) ? cursor : 0;
	memento->is_at_start = TRUE;
}


/*
 * memo_next_command()
 *
 * Iterator function, return the next saved command and its sequence id
 * starting at 1, and the timestamp and turns when the command was saved.
 */
void memo_next_command(sc_memo_setref_t memento, const sc_char **command,
			sc_int *sequence, sc_int *timestamp, sc_int *turns) {
	assert(memo_is_valid(memento));

	/* If valid, return the current command and advance. */
	if (memo_more_commands(memento)) {
		sc_historyref_t history;

		/* Note the current history, and advance its index. */
		history = memento->history + memento->current_history;
		memento->current_history++;
		memento->current_history %= MEMO_HISTORY_TABLE_SIZE;
		memento->is_at_start = FALSE;

		/* Return details from the history noted above. */
		*command = history->command;
		*sequence = history->sequence;
		*timestamp = history->timestamp;
		*turns = history->turns;
	} else {
		/* Return NULL and zeroes if no more commands available. */
		*command = NULL;
		*sequence = 0;
		*timestamp = 0;
		*turns = 0;
	}
}


/*
 * memo_more_commands()
 *
 * Iterator end function, returns TRUE if more commands are readable.
 */
sc_bool memo_more_commands(sc_memo_setref_t memento) {
	sc_int cursor;
	sc_historyref_t history;
	assert(memo_is_valid(memento));

	/* Get the current effective write position, and the current history. */
	cursor = memento->history_count % MEMO_HISTORY_TABLE_SIZE;
	history = memento->history + memento->current_history;

	/*
	 * More data if the current history is behind the write position and is
	 * occupied, or if it matches and is occupied and we're at the start of
	 * iteration (circular buffer special case).
	 */
	if (memento->current_history == cursor)
		return (memento->is_at_start) ? history->length > 0 : FALSE;
	else
		return history->length > 0;
}


/*
 * memo_find_command()
 *
 * Find and return the command string for the given sequence number (-ve
 * indicates an offset from the last defined), or NULL if not found.
 */
const sc_char *memo_find_command(sc_memo_setref_t memento, sc_int sequence) {
	sc_int target, index_;
	sc_historyref_t matched;
	assert(memo_is_valid(memento));

	/* Decide on a search target, depending on the sign of sequence. */
	target = (sequence < 0) ? memento->history_count + sequence + 1 : sequence;

	/*
	 * A backwards search starting at the write position would probably be more
	 * efficient here, but this is a rarely called function so we'll do it the
	 * simpler way.
	 */
	matched = NULL;
	for (index_ = 0; index_ < MEMO_HISTORY_TABLE_SIZE; index_++) {
		sc_historyref_t history;

		history = memento->history + index_;
		if (history->sequence == target) {
			matched = history;
			break;
		}
	}

	/*
	 * Return the command or NULL.  If sequence passed in was zero, and the
	 * history was not full, this will still return NULL as it should, since
	 * this unused history's command found by the search above will be NULL.
	 */
	return matched ? matched->command : NULL;
}


/*
 * memo_clear_commands()
 *
 * Forget all saved commands.
 */
void memo_clear_commands(sc_memo_setref_t memento) {
	sc_int index_;
	assert(memo_is_valid(memento));

	/* Deallocate every entry. */
	for (index_ = 0; index_ < MEMO_HISTORY_TABLE_SIZE; index_++) {
		sc_historyref_t history;

		history = memento->history + index_;
		sc_free(history->command);
	}

	/* Reset all entries, the count, and the iteration variables. */
	memset(memento->history, 0, sizeof(memento->history));
	memento->history_count = 0;
	memento->current_history = 0;
	memento->is_at_start = FALSE;
}

} // End of namespace Adrift
} // End of namespace Glk
