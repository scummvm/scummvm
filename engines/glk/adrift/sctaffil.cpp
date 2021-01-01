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
#include "glk/adrift/detection.h"
#include "common/algorithm.h"
#include "common/zlib.h"
#include "common/memstream.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Put integer and boolean read functions in here?
 */

/* Assorted definitions and constants. */
static const sc_uint TAF_MAGIC = 0x5bdcfa41;
enum {
	VERSION_HEADER_SIZE = 14,
	V400_HEADER_EXTRA = 8
};
enum {
	OUT_BUFFER_SIZE = 31744,
	IN_BUFFER_SIZE = 16384,
	GROW_INCREMENT = 8
};
static const sc_char NEWLINE = '\n';
static const sc_char CARRIAGE_RETURN = '\r';
static const sc_char NUL = '\0';

/*
 * Game TAF data structure.  The game structure contains the original TAF
 * file header, a growable array of "slab" descriptors, each of which holds
 * metadata for a "slab" (around a decompression buffer full of TAF strings),
 * the length of the descriptor array and elements allocated, and a current
 * location for iteration.
 *
 * Saved game files (.TAS) are just like TAF files except that they lack the
 * header.  So for files of this type, the header is all zeroes.
 */
struct sc_slabdesc_t {
	sc_byte *data;
	sc_int size;
};
typedef sc_slabdesc_t *sc_slabdescref_t;
struct sc_taf_s {
	sc_uint magic;
	sc_byte header[VERSION_HEADER_SIZE + V400_HEADER_EXTRA];
	sc_int version;
	sc_int total_in_bytes;
	sc_slabdescref_t slabs;
	sc_int slab_count;
	sc_int slabs_allocated;
	sc_bool is_unterminated;
	sc_int current_slab;
	sc_int current_offset;
};
typedef sc_taf_s sc_taf_t;


/* Microsoft Visual Basic PRNG magic numbers, initial and current state. */
static const sc_int PRNG_CST1 = 0x43fd43fd,
                    PRNG_CST2 = 0x00c39ec3,
                    PRNG_CST3 = 0x00ffffff,
                    PRNG_INITIAL_STATE = 0x00a09e86;
static sc_int taf_random_state = 0x00a09e86;

/*
 * taf_random()
 * taf_random_reset()
 *
 * Version 3.9 and version 3.8 games are obfuscated by xor'ing each character
 * with the PRNG in Visual Basic.  So here we have to emulate that, to unob-
 * fuscate data from such game files.  The PRNG generates 0..prng_cst3, which
 * we multiply by 255 and then divide by prng_cst3 + 1 to get output in the
 * range 0..254.  Thanks to Rik Snel for uncovering this obfuscation.
 */
static sc_byte taf_random(void) {
	/* Generate and return the next pseudo-random number. */
	taf_random_state = (taf_random_state * PRNG_CST1 + PRNG_CST2) & PRNG_CST3;
	return (BYTE_MAX * (sc_uint) taf_random_state) / (sc_uint)(PRNG_CST3 + 1);
}

static void taf_random_reset(void) {
	/* Reset PRNG to initial conditions. */
	taf_random_state = PRNG_INITIAL_STATE;
}


/*
 * taf_is_valid()
 *
 * Return TRUE if pointer is a valid TAF structure, FALSE otherwise.
 */
static sc_bool taf_is_valid(sc_tafref_t taf) {
	return taf && taf->magic == TAF_MAGIC;
}


/*
 * taf_create_empty()
 *
 * Allocate and return a new, empty TAF structure.
 */
static sc_tafref_t taf_create_empty(void) {
	sc_tafref_t taf;

	/* Create an empty TAF structure. */
	taf = (sc_tafref_t)sc_malloc(sizeof(*taf));
	taf->magic = TAF_MAGIC;
	memset(taf->header, 0, sizeof(taf->header));
	taf->version = TAF_VERSION_NONE;
	taf->total_in_bytes = 0;
	taf->slabs = NULL;
	taf->slab_count = 0;
	taf->slabs_allocated = 0;
	taf->is_unterminated = FALSE;
	taf->current_slab = 0;
	taf->current_offset = 0;

	/* Return the new TAF structure. */
	return taf;
}


/*
 * taf_destroy()
 *
 * Free TAF memory, and destroy a TAF structure.
 */
void taf_destroy(sc_tafref_t taf) {
	sc_int index_;
	assert(taf_is_valid(taf));

	/* First free each slab in the slabs array,... */
	for (index_ = 0; index_ < taf->slab_count; index_++)
		sc_free(taf->slabs[index_].data);

	/*
	 * ...then free slabs growable array, and poison and free the TAF structure
	 * itself.
	 */
	sc_free(taf->slabs);
	memset(taf, 0xaa, sizeof(*taf));
	sc_free(taf);
}


/*
 * taf_finalize_last_slab()
 *
 * Insert nul's into slab data so that it turns into a series of nul-terminated
 * strings.  Nul's are used to replace carriage return and newline pairs.
 */
static void taf_finalize_last_slab(sc_tafref_t taf) {
	sc_slabdescref_t slab;
	sc_int index_;

	/* Locate the final slab in the slab descriptors array. */
	assert(taf->slab_count > 0);
	slab = taf->slabs + taf->slab_count - 1;

	/*
	 * Replace carriage return and newline pairs with nuls, and individual
	 * carriage returns with a single newline.
	 */
	for (index_ = 0; index_ < slab->size; index_++) {
		if (slab->data[index_] == CARRIAGE_RETURN) {
			if (index_ < slab->size - 1 && slab->data[index_ + 1] == NEWLINE) {
				slab->data[index_] = NUL;
				slab->data[index_ + 1] = NUL;
				index_++;
			} else
				slab->data[index_] = NEWLINE;
		}

		/* Also protect against unlikely incoming nul characters. */
		else if (slab->data[index_] == NUL)
			slab->data[index_] = NEWLINE;
	}
}


/*
 * taf_find_buffer_extent()
 *
 * Search backwards from the buffer end for a terminating carriage return and
 * line feed.  If none, found, return length and set is_unterminated to TRUE.
 * Otherwise, return the count of usable bytes found in the buffer.
 */
static sc_int taf_find_buffer_extent(const sc_byte *buffer, sc_int length, sc_bool *is_unterminated) {
	sc_int bytes;

	/* Search backwards from the buffer end for the final line feed. */
	for (bytes = length; bytes > 1; bytes--) {
		if (buffer[bytes - 2] == CARRIAGE_RETURN && buffer[bytes - 1] == NEWLINE)
			break;
	}
	if (bytes < 2) {
		/* No carriage return and newline termination found. */
		*is_unterminated = TRUE;
		return length;
	}

	*is_unterminated = FALSE;
	return bytes;
}


/*
 * taf_append_buffer()
 *
 * Append a buffer of TAF lines to an existing TAF structure.  Returns the
 * number of characters consumed from the buffer.
 */
static sc_int taf_append_buffer(sc_tafref_t taf, const sc_byte *buffer, sc_int length) {
	sc_int bytes;
	sc_bool is_unterminated;

	/* Locate the extent of appendable data in the buffer. */
	bytes = taf_find_buffer_extent(buffer, length, &is_unterminated);

	/* See if the last buffer handled contained at least one data line. */
	if (!taf->is_unterminated) {
		sc_slabdescref_t slab;

		/* Extend the slabs array if we've reached the current allocation. */
		if (taf->slab_count == taf->slabs_allocated) {
			taf->slabs_allocated += GROW_INCREMENT;
			taf->slabs = (sc_slabdescref_t)sc_realloc(taf->slabs,
			             taf->slabs_allocated * sizeof(*taf->slabs));
		}

		/* Advance to the next unused slab in the slab descriptors array. */
		slab = taf->slabs + taf->slab_count;
		taf->slab_count++;

		/* Copy the input buffer into the new slab. */
		slab->data = (sc_byte *)sc_malloc(bytes);
		memcpy(slab->data, buffer, bytes);
		slab->size = bytes;
	} else {
		sc_slabdescref_t slab;

		/* Locate the final slab in the slab descriptors array. */
		assert(taf->slab_count > 0);
		slab = taf->slabs + taf->slab_count - 1;

		/*
		 * The last buffer we saw had no line endings in it.  In this case,
		 * append the input buffer to the end of the last slab's data, rather
		 * than creating a new slab.  This may cause allocation to overflow
		 * the system limits on single allocated areas on some platforms.
		 */
		slab->data = (sc_byte *)sc_realloc(slab->data, slab->size + bytes);
		memcpy(slab->data + slab->size, buffer, bytes);
		slab->size += bytes;

		/*
		 * Use a special case for the final carriage return and newline pairing
		 * that are split over two buffers; force correct termination of this
		 * slab.
		 */
		if (slab->size > 1
		        && slab->data[slab->size - 2] == CARRIAGE_RETURN
		        && slab->data[slab->size - 1] == NEWLINE)
			is_unterminated = FALSE;
	}

	/*
	 * Note if this buffer requires that the next be coalesced with it.  If it
	 * doesn't, finalize the last slab by breaking it into separate lines.
	 */
	taf->is_unterminated = is_unterminated;
	if (!is_unterminated)
		taf_finalize_last_slab(taf);

	/* Return count of buffer bytes consumed. */
	return bytes;
}


/*
 * taf_unobfuscate()
 *
 * Unobfuscate a version 3.9 and version 3.8 TAF file from data read by
 * repeated calls to the callback() function.  Callback() should return the
 * count of bytes placed in the buffer, or 0 if no more (end of file).
 * Assumes that the file has been read past the header.
 */
static sc_bool taf_unobfuscate(sc_tafref_t taf, sc_read_callbackref_t callback,
		void *opaque, sc_bool is_gamefile) {
	sc_byte *buffer;
	sc_int bytes, used_bytes, total_bytes, index_;

	/* Reset the PRNG, and synchronize with the header already read. */
	taf_random_reset();
	for (index_ = 0; index_ < VERSION_HEADER_SIZE; index_++)
		taf_random();

	/*
	 * Malloc buffer, done to help systems with limited stacks, and initialize
	 * count of bytes read and used in the buffer to zero.
	 */
	buffer = (sc_byte *)sc_malloc(IN_BUFFER_SIZE);
	used_bytes = 0;
	total_bytes = 0;

	/* Unobfuscate in buffer sized chunks. */
	do {
		/* Try to obtain more data. */
		bytes = callback(opaque,
		                 buffer + used_bytes, IN_BUFFER_SIZE - used_bytes);

		/* Unobfuscate data read in. */
		for (index_ = 0; index_ < bytes; index_++)
			buffer[used_bytes + index_] ^= taf_random();

		/*
		 * Add data read in and unobfuscated to buffer used data, and if
		 * unobfuscated data is available, add it to the TAF.
		 */
		used_bytes += bytes;
		if (used_bytes > 0) {
			sc_int consumed;

			/* Add lines from this buffer to the TAF. */
			consumed = taf_append_buffer(taf, buffer, used_bytes);

			/* Move unused buffer data to buffer start. */
			memmove(buffer, buffer + consumed, IN_BUFFER_SIZE - consumed);

			/* Note counts of bytes consumed and remaining in the buffer. */
			used_bytes -= consumed;
			total_bytes += consumed;
		}
	} while (bytes > 0);

	/*
	 * Unobfuscation completed, note the total bytes read.  This value is
	 * actually not used for version 3.9 and version 3.8 games, but we maintain
	 * it just in case.
	 */
	taf->total_in_bytes = total_bytes;
	if (is_gamefile)
		taf->total_in_bytes += VERSION_HEADER_SIZE;

	/* Check that we found the end of the input file as expected. */
	if (used_bytes > 0) {
		sc_error("taf_unobfuscate:"
		         " warning: %ld unhandled bytes in the buffer\n", used_bytes);
	}

	if (taf->is_unterminated)
		sc_fatal("taf_unobfuscate: unterminated final data slab\n");

	/* Return successfully. */
	sc_free(buffer);
	return TRUE;
}

#define BUFFER_SIZE 16384

/*
 * taf_decompress()
 *
 * Decompress a version 4.0 TAF
 */
static sc_bool taf_decompress(sc_tafref_t taf, sc_read_callbackref_t callback,
		void *opaque, sc_bool is_gamefile) {
#if defined(USE_ZLIB)
	Common::SeekableReadStream *src = (Common::SeekableReadStream *)opaque;
	assert(src);
	Common::MemoryWriteStreamDynamic dest(DisposeAfterUse::YES);
	size_t startingPos = src->pos();

	if (!Common::inflateZlibHeaderless(&dest, src))
		return false;

	// Iterate through pushing data out to the taf file
	const byte *pTemp = dest.getData();
	int bytesRemaining = dest.size();

	while (bytesRemaining > 0) {
		int consumed = taf_append_buffer(taf, pTemp, bytesRemaining);
		bytesRemaining -= consumed;
	}

	taf->total_in_bytes = src->pos() - startingPos;

	return true;
#else
	return false;
#endif
}

/*
 * taf_read_raw()
 *
 * Read an uncompressed version 4.0 TAF save chunk used by ScummVM
 */
static sc_bool taf_read_raw(sc_tafref_t taf, sc_read_callbackref_t callback,
	void *opaque, sc_bool is_gamefile) {
	byte *buffer = new byte[BUFFER_SIZE];
	size_t bytesRead, bytesLeft = 0;
	size_t totalBytes, bytesWritten;

	for (;;) {
		bytesRead = callback(opaque, buffer + bytesLeft, BUFFER_SIZE - bytesLeft);
		if ((bytesLeft + bytesRead) == 0)
			break;

		totalBytes = bytesLeft + bytesRead;
		bytesWritten = taf_append_buffer(taf, buffer, totalBytes);

		bytesLeft = totalBytes - bytesWritten;
		if (bytesLeft)
			Common::copy(buffer + bytesWritten, buffer + totalBytes, buffer);
	}

	delete[] buffer;
	return true;
}

/*
 * taf_create_from_callback()
 *
 * Create a TAF structure from data read in by repeated calls to the
 * callback() function.  Callback() should return the count of bytes placed
 * in the buffer, or 0 if no more (end of file).
 */
static sc_tafref_t taf_create_from_callback(sc_read_callbackref_t callback,
		void *opaque, sc_bool is_gamefile) {
	sc_tafref_t taf;
	sc_bool status = FALSE;
	assert(callback);

	/* Create an empty TAF structure. */
	taf = taf_create_empty();

	/*
	 * Determine the TAF file version in use.  For saved games, we always use
	 * version 4.0 format.  For others, it's determined from the header.
	 */
	if (is_gamefile) {
		sc_int in_bytes;

		/*
		 * Read in the ADRIFT header for game files.  Start by reading in the
		 * shorter header common to all.
		 */
		in_bytes = callback(opaque, taf->header, VERSION_HEADER_SIZE);
		if (in_bytes != VERSION_HEADER_SIZE) {
			sc_error("taf_create: not enough data for standard TAF header\n");
			taf_destroy(taf);
			return NULL;
		}

		/* Handle different TAF versions */
		int version = AdriftMetaEngine::detectGameVersion(taf->header);

		if (version == TAF_VERSION_500 || version == TAF_VERSION_390 ||
				version == TAF_VERSION_380) {
			taf->version = TAF_VERSION_500;

		} else if (version == TAF_VERSION_400) {
			/* Read in the version 4.0 header extension. */
			in_bytes = callback(opaque,
			                    taf->header + VERSION_HEADER_SIZE,
			                    V400_HEADER_EXTRA);
			if (in_bytes != V400_HEADER_EXTRA) {
				sc_error("taf_create:"
				         " not enough data for extended TAF header\n");
				taf_destroy(taf);
				return NULL;
			}

			taf->version = TAF_VERSION_400;

		} else {
			taf_destroy(taf);
			return NULL;
		}
	} else {
		/* Saved games are always considered to be for ScummVM, version 5.0. */
		taf->version = TAF_VERSION_SAVE;
	}

	/*
	 * Call the appropriate game file reader function.  For version 4.0 games,
	 * data is compressed with Zlib.  For version 3.9 and version 3.8 games,
	 * it's obfuscated with the Visual Basic PRNG.
	 */
	switch (taf->version) {
	case TAF_VERSION_SAVE:
		status = taf_read_raw(taf, callback, opaque, is_gamefile);
		break;

	case TAF_VERSION_500:
		sc_error("taf_create: ADRIFT 5 games are not yet supported");
		break;

	case TAF_VERSION_400:
		status = taf_decompress(taf, callback, opaque, is_gamefile);
		break;

	case TAF_VERSION_390:
	case TAF_VERSION_380:
		status = taf_unobfuscate(taf, callback, opaque, is_gamefile);
		break;

	default:
		sc_fatal("taf_create: invalid version\n");
	}
	if (!status) {
		taf_destroy(taf);
		return NULL;
	}

	/* Return successfully. */
	return taf;
}


/*
 * taf_create()
 * taf_create_tas()
 *
 * Public entry points for taf_create_from_callback().  Return a taf object
 * constructed from either *.TAF (game) or *.TAS (saved game state) file data.
 */
sc_tafref_t taf_create(sc_read_callbackref_t callback, void *opaque) {
	return taf_create_from_callback(callback, opaque, TRUE);
}

sc_tafref_t taf_create_tas(sc_read_callbackref_t callback, void *opaque) {
	return taf_create_from_callback(callback, opaque, FALSE);
}


/*
 * taf_first_line()
 *
 * Iterator rewind function, reset current slab location to TAF data start.
 */
void taf_first_line(sc_tafref_t taf) {
	assert(taf_is_valid(taf));

	/* Set current locations to TAF start. */
	taf->current_slab = 0;
	taf->current_offset = 0;
}


/*
 * taf_next_line()
 *
 * Iterator function, return the next line of data from a TAF, or NULL
 * if no more lines.
 */
const sc_char *taf_next_line(sc_tafref_t taf) {
	assert(taf_is_valid(taf));

	/* If there is a next line, return it and advance current. */
	if (taf->current_slab < taf->slab_count) {
		sc_char *line;

		/* Get the effective address of the current line. */
		line = (sc_char *) taf->slabs[taf->current_slab].data;
		line += taf->current_offset;

		/*
		 * Advance to the next line.  The + 2 skips the NULs used to replace the
		 * carriage return and line feed.
		 */
		taf->current_offset += strlen(line) + 2;
		if (taf->current_offset >= taf->slabs[taf->current_slab].size) {
			taf->current_slab++;
			taf->current_offset = 0;
		}

		return line;
	}

	/* No more lines, so return NULL. */
	return NULL;
}


/*
 * taf_more_lines()
 *
 * Iterator end function, returns TRUE if more TAF lines are readable.
 */
sc_bool taf_more_lines(sc_tafref_t taf) {
	assert(taf_is_valid(taf));

	/* Return TRUE if not at TAF data end. */
	return taf->current_slab < taf->slab_count;
}


/*
 * taf_get_game_data_length()
 *
 * Returns the number of bytes read to decompress the game.  Resources are
 * appended to the TAF file after the game, so this value allows them to
 * be located.
 */
sc_int taf_get_game_data_length(sc_tafref_t taf) {
	assert(taf_is_valid(taf));

	/*
	 * Return the count of bytes inflated; this includes the TAF header length
	 * for TAF, rather than TAS, files.  For TAS files, the count of file bytes
	 * read is irrelevant, and is never used.
	 */
	return taf->total_in_bytes;
}


/*
 * taf_get_version()
 *
 * Return the version number of the TAF file, 400, 390, or 380.
 */
sc_int taf_get_version(sc_tafref_t taf) {
	assert(taf_is_valid(taf));

	assert(taf->version != TAF_VERSION_NONE);
	return taf->version;
}


/*
 * taf_debug_is_taf_string()
 * taf_debug_dump()
 *
 * Print out a complete TAF structure.  The first function is a helper for
 * properties debugging, indicating if a given address is a string in a TAF
 * slab, and therefore safe to print.
 */
sc_bool taf_debug_is_taf_string(sc_tafref_t taf, const void *addr) {
	const sc_byte *const addr_ = (const sc_byte *)addr;
	sc_int index_;

	/*
	 * Compare pointer, by address directly, against all memory contained in
	 * the TAF slabs.  Return TRUE if in range.
	 */
	for (index_ = 0; index_ < taf->slab_count; index_++) {
		if (addr_ >= taf->slabs[index_].data
		        && addr_ < taf->slabs[index_].data + taf->slabs[index_].size)
			return TRUE;
	}

	return FALSE;
}

void taf_debug_dump(sc_tafref_t taf) {
	sc_int index_, current_slab, current_offset;
	assert(taf_is_valid(taf));

	/* Dump complete structure. */
	sc_trace("TAFfile: debug dump follows...\n");
	sc_trace("taf->header =");
	for (index_ = 0; index_ < (sc_int) sizeof(taf->header); index_++)
		sc_trace(" %02x", taf->header[index_]);
	sc_trace("\n");

	sc_trace("taf->version = %s\n",
	         taf->version == TAF_VERSION_400 ? "4.00" :
	         taf->version == TAF_VERSION_390 ? "3.90" :
	         taf->version == TAF_VERSION_380 ? "3.80" : "[Unknown]");

	sc_trace("taf->slabs = \n");
	for (index_ = 0; index_ < taf->slab_count; index_++) {
		sc_trace("%3ld : %p, %ld bytes\n", index_,
		         (void *)taf->slabs[index_].data, taf->slabs[index_].size);
	}

	sc_trace("taf->slab_count = %ld\n", taf->slab_count);
	sc_trace("taf->slabs_allocated = %ld\n", taf->slabs_allocated);
	sc_trace("taf->current_slab = %ld\n", taf->current_slab);
	sc_trace("taf->current_offset = %ld\n", taf->current_offset);

	/* Save current location. */
	current_slab = taf->current_slab;
	current_offset = taf->current_offset;

	/* Print out taf lines using taf iterators. */
	sc_trace("\ntaf iterators:\n");
	taf_first_line(taf);
	for (index_ = 0; taf_more_lines(taf); index_++)
		sc_trace("%5ld %s\n", index_, taf_next_line(taf));

	/* Restore current location. */
	taf->current_slab = current_slab;
	taf->current_offset = current_offset;
}

} // End of namespace Adrift
} // End of namespace Glk
