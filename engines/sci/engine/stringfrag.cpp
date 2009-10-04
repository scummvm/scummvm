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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

/*
The following is an email Lars wrote mid march and which explains a little
bit about system strings. I paste it here for future reference. Some of this
could possible be turned into documentation for the strings frags code...
*/

/*
Max Horn wrote:
> Basically, saving the reg_t "array" (?) in the SystemString struct (see
> sync_SystemString in engine/savegame.cpp) seems like a bad hack that will
> result in breakage...

The short explanation is that the casts _will_ go away, but that the
system strings, too, will become stringfrag-based eventually.

The long version requires a bit of background: During the SCI01
timeframe, a Memory() call was added which does not really play well
with the Glutton architecture (as well as having the potential for
nasty game bugs - it allows games to write to any part of the
heap). The same memory call is used for modification of strings and
integer arrays (as is direct array indexing). From a formal point of
view, the use of either of these for string handling is wrong. There
exist string APIs that should be used instead. But things being as
they are, we need to be able to tell the two types apart
somehow. Currently, we have a heuristic in there which doesn't always
work (it breaks LSL5 password protection, for one thing) - the string
frags code is intended to replace this heuristic, but requires
changing the argument parsing of every kernel function that uses
strings. This _will_ cause regressions, and for that reason I only
changed the interface definitions, not the implementation. As I wrote
to Willem the other day, I was trying to commit as much of the
stringfrag code as I could without breaking existing code, and isolate
the breakage to a later set of commits.

Also, a little background on the system strings. In Sierra SCI, the
heap is stored in the interpreter's data segment and is not precisely
64K in size - but rather 64K minus the size of the interpreter's
static data. However, the script code does have read/write access
to this static data. The strings that we store in the system strings
table are stored in static data in Sierra SCI. We have only two at
this point:

1. The savegame directory
2. Parser error handling

2) doesn't actually need to be saved, because not only is the variable
only used during error handling, but the routine in question is called
re-entrantly by the kernel function - and not even the old save system
supports saving in that kind of situation, much less Sierra SCI. On
the other hand, the scripts retain a pointer to 1) which needs to
remain valid across saves.
*/

#define STRINGFRAG_SEGMENT s->string_frag_segment
// #define STRINGFRAG_SEGMENT 0xffff

static int internal_is_valid_stringfrag(EngineState *s, reg_t *buffer) {
	if (buffer == NULL)
		return 0;

	while ((buffer->offset & 0xff00) != 0 &&
	       (buffer->offset & 0x00ff) != 0) {
		if (buffer->segment != STRINGFRAG_SEGMENT)
			return 0;
		buffer++;
	}

	if (buffer->segment != STRINGFRAG_SEGMENT) {
		return 0;
	}

	return 1;
}

int is_valid_stringfrag(EngineState *s, reg_t pos) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	return internal_is_valid_stringfrag(s, buffer);
}

static int internal_stringfrag_length(EngineState *s, reg_t *buffer) {
	int result = 0;

	if (buffer == NULL) {
//	error("Error: Invalid stringfrag handle");
		return 0;
	}

	while ((buffer->offset & 0xff00) != 0 &&
	       (buffer->offset & 0x00ff) != 0) {
		if (buffer->segment != STRINGFRAG_SEGMENT) {
//	    error("Error: Invalid stringfrag handle");
			return 0;
		}

		result += 2;
		buffer++;
	}

	if (buffer->segment != STRINGFRAG_SEGMENT) {
		error("Error: Invalid stringfrag handle");
		return 0;
	}

	if ((buffer->offset & 0xff00) != 0)
		result++;
	return result;
}

int stringfrag_length(EngineState *s, reg_t pos) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	return internal_stringfrag_length(s, buffer);
}

static void internal_stringfrag_to_ascii(EngineState *s, reg_t *buffer) {
	int str_length = internal_stringfrag_length(s, buffer);
	char *temp = (char *)malloc(str_length + 1);
	int i = 0;

	while ((buffer->offset & 0xff00) != 0 &&
	       (buffer->offset & 0x00ff) != 0) {
		temp[i] = (buffer->offset & 0xff00) >> 8;
		if (temp[i] != 0)
			temp[i+1] = buffer->offset & 0xff;
		i += 2;
		buffer++;
	}

	if ((buffer->offset & 0xff00) != 0) {
		temp[i] = (buffer->offset & 0xff00) >> 8;
		temp[i+1] = 0;
	} else {
		temp[i] = 0;
	}

	strcpy((char *)buffer, temp);
	free(temp);
}

void stringfrag_to_ascii(EngineState *s, reg_t pos) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	internal_stringfrag_to_ascii(s, buffer);
}

static void internal_ascii_to_stringfrag(EngineState *s, reg_t *buffer) {
	int str_length = strlen((char *)buffer);
	int words = str_length % 2 ? (str_length+2)/2 : (str_length + 1)/2;
	char *temp = (char *)malloc(str_length + 1);

	strcpy(temp, (char *)buffer);
	for (int i = 0; i < words; i++) {
		buffer[i].segment = STRINGFRAG_SEGMENT;
		buffer[i].offset = temp[i*2];
		if (temp[i*2])
			buffer[i].offset |= temp[i*2+1];
	}

	free(temp);
}

void ascii_to_stringfrag(EngineState *s, reg_t pos) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	internal_ascii_to_stringfrag(s, buffer);
}

static void internal_stringfrag_append_char(EngineState *s, reg_t *buffer, unsigned char c) {
	while ((buffer->offset & 0xff00) != 0 &&
	       (buffer->offset & 0x00ff) != 0)
		buffer++;

	if ((buffer->offset & 0xff00) == 0) {
		buffer->offset = c << 8;
	} else {
		if ((buffer->offset & 0x00ff) == 0) {
			buffer->offset |= c;
			buffer++;
			buffer->segment = STRINGFRAG_SEGMENT;
			buffer->offset = 0;
		}
	}
}

void stringfrag_append_char(EngineState *s, reg_t pos, unsigned char c) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	internal_stringfrag_append_char(s, buffer, c);
}

void stringfrag_setchar(reg_t *buffer, int pos, int offset, unsigned char c) {
	switch (offset) {
	case 0 :
		buffer[pos].offset = (buffer[pos].offset & 0x00ff) | (c << 8);
		break;
	case 1 :
		buffer[pos].offset = (buffer[pos].offset & 0xff00) | c;
		break;
	}
}

unsigned char stringfrag_getchar(reg_t *buffer, int pos, int offset) {
	switch (offset) {
	case 0 :
		return buffer[pos].offset >> 8;
	case 1 :
		return buffer[pos].offset & 0xff;
	default:
		return 0; // FIXME: Is it safe to return "0" here?
	}
}

void stringfrag_memmove(EngineState *s, reg_t *buffer, int sourcepos, int destpos, int count) {
	/* Some of these values are not used yet. There are a couple
	   of cases that we could implement faster if the need arises, in
	   which case we would most certainly need these. */
	int source_begin_pos = sourcepos/2;
	int source_begin_offset = sourcepos%2;
	int source_end_pos = (sourcepos+count-1)/2;
	int source_end_offset = (sourcepos+count-1)%2;

	int dest_begin_pos = destpos/2;
	int dest_begin_offset = destpos%2;
	int dest_end_pos = (destpos+count-1)/2;
	int dest_end_offset = (destpos+count-1)%2;

	if (sourcepos < destpos) {
		for (int n = count-1; n >= 0; n--) {
			buffer[dest_end_pos].segment = STRINGFRAG_SEGMENT;
			stringfrag_setchar(buffer, dest_end_pos, dest_end_offset,
					   stringfrag_getchar(buffer, source_end_pos, source_end_offset));
			if (source_end_offset ^= 1)
				source_end_pos --;
			if (dest_end_offset ^= 1)
				dest_end_pos --;
		}
	} else {
		for (int n = 0; n < count; n++) {
			buffer[dest_begin_pos].segment = STRINGFRAG_SEGMENT;
			stringfrag_setchar(buffer, dest_begin_pos, dest_begin_offset,
					   stringfrag_getchar(buffer, source_begin_pos, source_begin_offset));
			if (!(source_begin_offset ^= 1))
				source_begin_pos ++;
			if (!(dest_begin_offset ^= 1))
				dest_begin_pos ++;
		}
	}
}

static void internal_stringfrag_insert_char(EngineState *s, reg_t *buffer, int p, unsigned char c) {
	reg_t *save = buffer + p/2;
	reg_t *seeker = buffer + p/2;
	int restore_nul_offset;
	int restore_nul_pos;
	int len = internal_stringfrag_length(s, buffer);

	while ((seeker->offset & 0xff00) != 0 &&
	       (seeker->offset & 0x00ff) != 0)
		seeker++;

	/* The variables restore_null_offset and restore_null_pos will
	   indicate where the NUL character should be PUT BACK after
	   inserting c, as this operation might overwrite the NUL. */
	if (seeker->offset & 0xff00) {
		restore_nul_offset = 1;
		restore_nul_pos = 0;
	} else {
		restore_nul_offset = 0;
		restore_nul_pos = 1;
	}

	if (seeker-save == 0) { // So p is at the end, use fast method
		internal_stringfrag_append_char(s, seeker, c);
		return;
	}

	stringfrag_memmove(s, buffer, p, p+1, len-p);
	stringfrag_setchar(buffer, p/2, p%2, c);
	stringfrag_setchar(seeker, restore_nul_pos, restore_nul_offset, 0);
}

void stringfrag_insert_char(EngineState *s, reg_t pos, int p, unsigned char c) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	internal_stringfrag_insert_char(s, buffer, p, c);
}

static void internal_stringfrag_delete_char(EngineState *s, reg_t *buffer, int p) {
	//reg_t *save = buffer + p;
	reg_t *seeker = buffer + p;
	int restore_nul_offset;
	int restore_nul_pos;
	int len = internal_stringfrag_length(s, buffer);

	while ((seeker->offset & 0xff00) != 0 &&
	       (seeker->offset & 0x00ff) != 0)
		seeker++;

	/* The variables restore_null_offset and restore_null_pos will
	   indicate where the NUL character should be PUT BACK after
	   deletion, as this operation might overwrite the NUL. */
	if (seeker->offset & 0xff00) {
		restore_nul_offset = 1;
		restore_nul_pos = -1;
	} else {
		restore_nul_offset = 0;
		restore_nul_pos = 0;
	}

	stringfrag_memmove(s, buffer, p, p-1, len-p);
	stringfrag_setchar(seeker, restore_nul_pos, restore_nul_offset, 0);
}

void stringfrag_delete_char(EngineState *s, reg_t pos, int p) {
	reg_t *buffer = s->_segMan->derefRegPtr(pos, 1);

	internal_stringfrag_delete_char(s, buffer, p);
}

void internal_stringfrag_strcpy(EngineState *s, reg_t *dest, reg_t *src) {
	while ((src->offset & 0xff00) != 0 &&
	       (src->offset & 0x00ff) != 0) {
		*dest = *src;
		dest++;
		src++;
	}

	*dest = *src;
}

void stringfrag_strcpy(EngineState *s, reg_t dest, reg_t src) {
	reg_t *destbuf = s->_segMan->derefRegPtr(dest, 1);
	reg_t *srcbuf = s->_segMan->derefRegPtr(src, 1);

	internal_stringfrag_strcpy(s, destbuf, srcbuf);
}

void internal_stringfrag_strncpy(EngineState *s, reg_t *dest, reg_t *src, int len) {
	while ((src->offset & 0xff00) != 0 &&
	       (src->offset & 0x00ff) != 0 &&
	       (len -= 2) > 1) {
		*dest = *src;
		dest++;
		src++;
	}

	for (; len > 1; len -= 2) {
		dest->segment = STRINGFRAG_SEGMENT;
		dest->offset = 0;
		len -= 2;
	}

	if (len == 1)
		stringfrag_setchar(dest, 0, 0, 0);

	*dest = *src;
}

void stringfrag_strncpy(EngineState *s, reg_t dest, reg_t src, int len) {
	reg_t *destbuf = s->_segMan->derefRegPtr(dest, 1);
	reg_t *srcbuf = s->_segMan->derefRegPtr(src, 1);

	internal_stringfrag_strncpy(s, destbuf, srcbuf, len);
}

int internal_stringfrag_strcmp(EngineState *s, reg_t *s1, reg_t *s2) {
	int c1, c2;
	while (1) {
		c1 = (uint16)(s1->offset & 0xff00);
		c2 = (uint16)(s2->offset & 0xff00);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;

		c1 = (uint16)(s1->offset & 0x00ff);
		c2 = (uint16)(s2->offset & 0x00ff);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;
	}

	return 0;
}

void stringfrag_strcmp(EngineState *s, reg_t s1, reg_t s2) {
	reg_t *s1buf = s->_segMan->derefRegPtr(s1, 1);
	reg_t *s2buf = s->_segMan->derefRegPtr(s2, 1);

	internal_stringfrag_strcmp(s, s1buf, s2buf);
}

int internal_stringfrag_strncmp(EngineState *s, reg_t *s1, reg_t *s2, int len) {
	int c1, c2;
	while (len) {
		if (len--)
			return 0;
		c1 = (uint16)(s1->offset & 0xff00);
		c2 = (uint16)(s2->offset & 0xff00);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;

		if (len--)
			return 0;

		c1 = (uint16)(s1->offset & 0x00ff);
		c2 = (uint16)(s2->offset & 0x00ff);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;
	}

	return 0;
}

void stringfrag_strncmp(EngineState *s, reg_t s1, reg_t s2, int len) {
	reg_t *s1buf = s->_segMan->derefRegPtr(s1, 1);
	reg_t *s2buf = s->_segMan->derefRegPtr(s2, 1);

	internal_stringfrag_strncmp(s, s1buf, s2buf, len);
}

} // End of namespace Sci
