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
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

	return internal_is_valid_stringfrag(s, buffer);
}
	
static int internal_stringfrag_length(EngineState *s, reg_t *buffer) {
	int result = 0;

	if (buffer == NULL) {
//	SCIkwarn(SCIkERROR, "Error: Invalid stringfrag handle");
		return 0;
	}

	while ((buffer->offset & 0xff00) != 0 &&
	       (buffer->offset & 0x00ff) != 0) {
		if (buffer->segment != STRINGFRAG_SEGMENT) {
//	    SCIkwarn(SCIkERROR, "Error: Invalid stringfrag handle");
			return 0;
		}

		result += 2;
		buffer++;
	}

	if (buffer->segment != STRINGFRAG_SEGMENT) {
		SCIkwarn(SCIkERROR, "Error: Invalid stringfrag handle");
		return 0;
	}

	if ((buffer->offset & 0xff00) != 0)
		result++;
	return result;
}

int stringfrag_length(EngineState *s, reg_t pos) {
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

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
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

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
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

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
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

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
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

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
	reg_t *buffer = kernel_dereference_reg_pointer(s, pos, 1);

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
	reg_t *destbuf = kernel_dereference_reg_pointer(s, dest, 1);
	reg_t *srcbuf = kernel_dereference_reg_pointer(s, src, 1);

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
	reg_t *destbuf = kernel_dereference_reg_pointer(s, dest, 1);
	reg_t *srcbuf = kernel_dereference_reg_pointer(s, src, 1);

	internal_stringfrag_strncpy(s, destbuf, srcbuf, len);
}

int internal_stringfrag_strcmp(EngineState *s, reg_t *s1, reg_t *s2) {
	int c1, c2;
	while (1) {
		c1 = (byte)(s1->offset & 0xff00);
		c2 = (byte)(s2->offset & 0xff00);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;

		c1 = (byte)(s1->offset & 0x00ff);
		c2 = (byte)(s2->offset & 0x00ff);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;
	}

	return 0;
}

void stringfrag_strcmp(EngineState *s, reg_t s1, reg_t s2) {
	reg_t *s1buf = kernel_dereference_reg_pointer(s, s1, 1);
	reg_t *s2buf = kernel_dereference_reg_pointer(s, s2, 1);

	internal_stringfrag_strcmp(s, s1buf, s2buf);
}

int internal_stringfrag_strncmp(EngineState *s, reg_t *s1, reg_t *s2, int len) {
	int c1, c2;
	while (len) {
		if (len--)
			return 0;
		c1 = (byte)(s1->offset & 0xff00);
		c2 = (byte)(s2->offset & 0xff00);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;

		if (len--)
			return 0;

		c1 = (byte)(s1->offset & 0x00ff);
		c2 = (byte)(s2->offset & 0x00ff);
		if (c1 != c2)		// We found a difference
			return c1 - c2;
		else if (c1 == 0)	// Both strings ended
			return 0;
	}

	return 0;
}

void stringfrag_strncmp(EngineState *s, reg_t s1, reg_t s2, int len) {
	reg_t *s1buf = kernel_dereference_reg_pointer(s, s1, 1);
	reg_t *s2buf = kernel_dereference_reg_pointer(s, s2, 1);

	internal_stringfrag_strncmp(s, s1buf, s2buf, len);
}

} // end of namespace Sci
