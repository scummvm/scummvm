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

// SCI 1 view resource defrobnicator

#include "common/endian.h"

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

#define V0_LOOPS_NR_OFFSET 0
#define V0_FIRST_LOOP_OFFSET 8
#define V0_MIRROR_LIST_OFFSET 2

#define V1_LOOPS_NR_OFFSET 0
#define V1_MIRROR_MASK 2
#define V1_PALETTE_OFFSET 6
#define V1_FIRST_LOOP_OFFSET 8

#define V1_RLE 0x80 // run-length encode?
#define V1_RLE_BG 0x40 // background fill

#define V2_HEADER_SIZE 0
#define V2_BYTES_PER_LOOP 12
#define V2_BYTES_PER_CEL 13

#define V2_COPY_OF_LOOP 2
#define V2_CELS_NUM 4
#define V2_LOOP_OFFSET 14

gfx_pixmap_t *gfxr_draw_cel0(int id, int loop, int cel, byte *resource, int size, gfxr_view_t *view, int mirrored) {
	int xl = READ_LE_UINT16(resource);
	int yl = READ_LE_UINT16(resource + 2);
	int pixmap_size = xl * yl;
	int xdisplace = ((signed char *)resource)[4];
	int ydisplace = ((signed char *)resource)[5];
	int color_key = resource[6];
	int pos = 7;
	int writepos = mirrored ? xl : 0;
	int line_base = 0;
	gfx_pixmap_t *retval = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xl, yl, id, loop, cel));
	byte *dest = retval->index_data;

	retval->color_key = 255; // Pick something larger than 15

	retval->xoffset = mirrored ? xdisplace : -xdisplace;
	retval->yoffset = -ydisplace;

	retval->palette = (view && view->palette) ? view->palette->getref() : 
						gfx_sci0_image_pal[sci0_palette]->getref();

	if (xl <= 0 || yl <= 0) {
		gfx_free_pixmap(retval);
		GFXERROR("View %02x:(%d/%d) has invalid xl=%d or yl=%d\n", id, loop, cel, xl, yl);
		return NULL;
	}

	if (mirrored) {
		while (yl && pos < size) {
			int op = resource[pos++];
			int count = op >> 4;
			int color = op & 0xf;

			if (view->flags & GFX_PIXMAP_FLAG_PALETTIZED)
				color = view->translation[color];

			if (color == color_key)
				color = retval->color_key;

			while (count) {
				int pixels = writepos - line_base;

				if (pixels > count)
					pixels = count;

				writepos -= pixels;
				memset(dest + writepos, color, pixels);
				count -= pixels;

				if (writepos == line_base) {
					yl--;
					writepos += (xl << 1);
					line_base += xl;
				}
			}
		}
	} else {

		while (writepos < pixmap_size && pos < size) {
			int op = resource[pos++];
			int count = op >> 4;
			int color = op & 0xf;

			if (view && (view->flags & GFX_PIXMAP_FLAG_PALETTIZED))
				color = view->translation[color];

			if (color == color_key)
				color = retval->color_key;

			if (writepos + count > pixmap_size) {
				GFXERROR("View %02x:(%d/%d) writes RLE data over its designated end at rel. offset 0x%04x\n", id, loop, cel, pos);
				return NULL;
			}

			memset(dest + writepos, color, count);
			writepos += count;
		}
	}

	return retval;
}

gfxr_view_t *gfxr_draw_view0(int id, byte *resource, int size, int palette) {
	int i;
	gfxr_view_t *view;
	int mirror_bitpos = 1;
	int mirror_bytepos = V0_MIRROR_LIST_OFFSET;
	int palette_ofs = READ_LE_UINT16(resource + 6);

	if (size < V0_FIRST_LOOP_OFFSET + 8) {
		GFXERROR("Attempt to draw empty view %04x\n", id);
		return NULL;
	}

	view = (gfxr_view_t *)malloc(sizeof(gfxr_view_t));
	view->ID = id;

	view->loops_nr = resource[V0_LOOPS_NR_OFFSET];

	// Set palette
	view->flags = 0;
	view->palette = gfx_sci0_image_pal[sci0_palette]->getref();

	if ((palette_ofs) && (palette >= 0)) {
		byte *paldata = resource + palette_ofs + (palette * GFX_SCI0_IMAGE_COLORS_NR);

		for (i = 0; i < GFX_SCI0_IMAGE_COLORS_NR; i++)
			view->translation[i] = *(paldata++);

		view->flags |= GFX_PIXMAP_FLAG_PALETTIZED;
	}

	if (view->loops_nr * 2 + V0_FIRST_LOOP_OFFSET > size) {
		GFXERROR("View %04x: Not enough space in resource to accomodate for the claimed %d loops\n", id, view->loops_nr);
		free(view);
		return NULL;
	}

	view->loops = (gfxr_loop_t*)malloc(sizeof(gfxr_loop_t) * ((view->loops_nr) ? view->loops_nr : 1)); /* Alloc 1 if no loop */

	for (i = 0; i < view->loops_nr; i++) {
		int loop_offset = READ_LE_UINT16(resource + V0_FIRST_LOOP_OFFSET + (i << 1));
		int mirrored = resource[mirror_bytepos] & mirror_bitpos;

		if ((mirror_bitpos <<= 1) == 0x100) {
			mirror_bytepos++;
			mirror_bitpos = 1;
		}

		view->loops[i].cels_nr = READ_LE_UINT16(resource + loop_offset);
		view->loops[i].cels = (gfx_pixmap_t**)calloc(view->loops[i].cels_nr, sizeof(gfx_pixmap_t *));

		for (int j = 0; j < view->loops[i].cels_nr; j++) {
			int cel_offset = READ_LE_UINT16(resource + loop_offset + 4 + (j << 1));
			view->loops[i].cels[j] = gfxr_draw_cel0(id, i, j, resource + cel_offset, size - cel_offset, view, mirrored);
		}
	}

	return view;
}

#define NEXT_LITERAL_BYTE(n) \
	if (literal_pos == runlength_pos) \
		runlength_pos += n; \
	literal_pos += n;

static int decompress_sci_view(int id, int loop, int cel, byte *resource, byte *dest, int mirrored, int pixmap_size, int size,
	int runlength_pos, int literal_pos, int xl, int yl, int color_key) {
	int writepos = mirrored ? xl : 0;
	int linebase = 0;

	// For some cels the RLE data ends at the last non-transparent pixel,
	// so we initialize the whole pixmap to transparency first
	memset(dest, color_key, pixmap_size);

	while ((mirrored ? linebase < pixmap_size : writepos < pixmap_size) && literal_pos < size && runlength_pos < size) {
		int op = resource[runlength_pos];
		int bytes;
		int readbytes = 0;
		int color = 0;

		if (literal_pos == runlength_pos)
			literal_pos += 1;
	
		runlength_pos += 1;

		if (op & V1_RLE) {
			bytes = op & 0x3f;
			op &= (V1_RLE | V1_RLE_BG);
			readbytes = (op & V1_RLE_BG) ? 0 : 1;
		} else {
			readbytes = bytes = op & 0x3f;
			op = 0;
		}

		assert(runlength_pos + readbytes <= size);

		/*
		if (writepos - bytes < 0) {
			warning("[GFX] View %02x:(%d/%d) describes more bytes than needed: %d/%d bytes at rel. offset 0x%04x",
					id, loop, cel, writepos - bytes, pixmap_size, pos - 1);
			bytes = pixmap_size - writepos;
		}
		*/

		if (mirrored && op == V1_RLE) {
			color = resource[literal_pos];
			NEXT_LITERAL_BYTE(1);
		}

		assert(op || literal_pos + bytes <= size);

		if (!mirrored && (writepos + bytes > pixmap_size)) {
			warning("[GFX] Writing out of bounds: %d bytes at %d > size %d", bytes, writepos, pixmap_size);
		}

		if (mirrored) {
			while (bytes--) {
				writepos--;
				if (op) {
					*(dest + writepos) = (op & V1_RLE_BG) ? color_key : color;
				} else {
					*(dest + writepos) = *(resource + literal_pos);
					NEXT_LITERAL_BYTE(1);
				}
				if (writepos == linebase) {
					writepos += 2 * xl;
					linebase += xl;
				}
			}
		} else {
			if (op) {
				if (op & V1_RLE_BG)
					memset(dest + writepos, color_key, bytes);
				else {
					color = resource[literal_pos];

					NEXT_LITERAL_BYTE(1);
					memset(dest + writepos, color, bytes);
				}
			} else {
				memcpy(dest + writepos, resource + literal_pos, bytes);
				NEXT_LITERAL_BYTE(bytes);
			}
			writepos += bytes;
		}
	}

	return 0;
}

static int decompress_sci_view_amiga(int id, int loop, int cel, byte *resource, byte *dest, int mirrored, int pixmap_size, int size,
	int pos, int xl, int yl, int color_key) {
	int writepos = mirrored ? xl - 1 : 0;

	while (writepos < pixmap_size && pos < size) {
		int op = resource[pos++];
		int bytes = (op & 0x07) ? op & 0x07 : op >> 3;
		int color = (op & 0x07) ? op >> 3 : color_key;

		if (mirrored) {
			while (bytes--) {
				dest[writepos--] = color;
				// If we've just written the first pixel of a line...
				if (!((writepos + 1) % xl)) {
					// Then move to the end of next line
					writepos += 2 * xl;

					if (writepos >= pixmap_size && bytes) {
						warning("[GFX] View %02x:(%d/%d) writing out of bounds", id, loop, cel);
						break;
					}
				}
			}
		} else {
			if (writepos + bytes > pixmap_size) {
				warning("[GFX] View %02x:(%d/%d) describes more bytes than needed: %d/%d bytes at rel. offset 0x%04x",
				        id, loop, cel, writepos - bytes, pixmap_size, pos - 1);
				bytes = pixmap_size - writepos;
			}
			memset(dest + writepos, color, bytes);
			writepos += bytes;
		}
	}

	if (writepos < pixmap_size) {
		warning("[GFX] View %02x:(%d/%d) not enough pixel data in view", id, loop, cel);
		return 1;
	}

	return 0;
}

gfx_pixmap_t *gfxr_draw_cel1(int id, int loop, int cel, int mirrored, byte *resource, byte *cel_base, int size, gfxr_view_t *view, bool isAmiga, bool isSci11) {
	int xl = READ_LE_UINT16(cel_base);
	int yl = READ_LE_UINT16(cel_base + 2);
	int pixmap_size = xl * yl;
	int xdisplace = isSci11 ? READ_LE_UINT16(cel_base + 4) : (int8) cel_base[4];
	int ydisplace = isSci11 ? READ_LE_UINT16(cel_base + 6) : (int8) cel_base[5];
	int runlength_offset = isSci11 ? READ_LE_UINT16(cel_base + 24) : 8;
	int literal_offset = isSci11 ? READ_LE_UINT16(cel_base + 28) : 8;
	gfx_pixmap_t *retval = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xl, yl, id, loop, cel));
	byte *dest = retval->index_data;
	int decompress_failed;

	retval->color_key = cel_base[isSci11 ? 8 : 6];
	retval->xoffset = mirrored ? xdisplace : -xdisplace;
	retval->yoffset = -ydisplace;
	// FIXME: In LSL5, it seems that the inventory has views without palettes (or we don't load palettes properly)
	retval->palette = (view && view->palette) ? view->palette->getref() : NULL;

	if (xl <= 0 || yl <= 0) {
		gfx_free_pixmap(retval);
		GFXERROR("View %02x:(%d/%d) has invalid xl=%d or yl=%d\n", id, loop, cel, xl, yl);
		return NULL;
	}

	if (!isAmiga)
		decompress_failed = decompress_sci_view(id, loop, cel, resource, dest, mirrored, pixmap_size, size, runlength_offset,
		                                        literal_offset, xl, yl, retval->color_key);
	else
		decompress_failed = decompress_sci_view_amiga(id, loop, cel, resource, dest, mirrored, pixmap_size, size, runlength_offset,
		                    xl, yl, retval->color_key);

	if (decompress_failed) {
		gfx_free_pixmap(retval);
		return NULL;
	}

	return retval;
}

gfxr_view_t *gfxr_draw_view1(int id, byte *resource, int size, Palette *static_pal, bool isSci11) {
	uint16 palOffset = READ_LE_UINT16(resource + V1_PALETTE_OFFSET + (isSci11 ? 2 : 0));
	uint16 headerSize = isSci11 ? READ_LE_UINT16(resource + V2_HEADER_SIZE) : 0;
	byte* seeker = resource + headerSize;
	uint16 loopOffset = 0;
	int amiga_game = 0;
	gfxr_view_t *view = (gfxr_view_t *)malloc(sizeof(gfxr_view_t));

	view->ID = id;
	view->flags = 0;
	view->loops_nr = READ_LE_UINT16(resource + V1_LOOPS_NR_OFFSET + (isSci11 ? 2 : 0)) & 0xFF;

	if (palOffset > 0) {
		if (!isSci11)
			view->palette = gfxr_read_pal1(id, resource + palOffset, size - palOffset);
		else
			view->palette = gfxr_read_pal11(id, resource + palOffset, size - palOffset);
	} else if (static_pal && static_pal->size() == GFX_SCI1_AMIGA_COLORS_NR) {
		// Assume we're running an amiga game.
		amiga_game = 1;
		view->palette = static_pal->getref();
	} else {
		view->palette = NULL;
	}

	view->loops = (gfxr_loop_t *)calloc(view->loops_nr, sizeof(gfxr_loop_t));

	for (int i = 0; i < view->loops_nr; i++) {
		if (!isSci11) {
			bool mirrored = READ_LE_UINT16(resource + V1_MIRROR_MASK) & (1 << i);
			loopOffset = READ_LE_UINT16(resource + V1_FIRST_LOOP_OFFSET + (i << 1));
			view->loops[i].cels_nr = READ_LE_UINT16(resource + loopOffset);
			view->loops[i].cels = (gfx_pixmap_t**)calloc(view->loops[i].cels_nr, sizeof(gfx_pixmap_t *));

			for (int j = 0; j < view->loops[i].cels_nr; j++) {
				int cel_offset = READ_LE_UINT16(resource + loopOffset + 4 + (j << 1));
				view->loops[i].cels[j] = gfxr_draw_cel1(id, i, j, mirrored, 
														resource + cel_offset, 
														resource + cel_offset, 
														size - cel_offset, 
														view, amiga_game, false);
			}
		} else {
			byte copy_entry = seeker[V2_COPY_OF_LOOP];
			bool mirrored = (copy_entry != 255);
			byte *buf = !mirrored ? seeker : resource + headerSize + copy_entry * resource[V2_BYTES_PER_LOOP];
			loopOffset = READ_LE_UINT16(buf + V2_LOOP_OFFSET);
			view->loops[i].cels_nr = buf[V2_CELS_NUM];
			view->loops[i].cels = (gfx_pixmap_t**)calloc(view->loops[i].cels_nr, sizeof(gfx_pixmap_t *));

			byte* cellSeeker = resource + loopOffset;
			for (int j = 0; j < view->loops[i].cels_nr; j++) {
				view->loops[i].cels[j] = gfxr_draw_cel1(id, i, j, mirrored, resource, cellSeeker, size, view, 0, true);
				cellSeeker += resource[V2_BYTES_PER_CEL];
			}

			seeker += resource[V2_BYTES_PER_LOOP];
		}
	}

	return view;
}

} // End of namespace Sci
