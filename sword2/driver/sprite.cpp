/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "driver96.h"
#include "d_draw.h"
#include "render.h"
#include "menu.h"
#include "palette.h"
#include "rdwin.h"

static uint8 *lightMask = 0;

/**
 * This function takes a sprite and creates a mirror image of it.
 * @param dst destination buffer
 * @param src source buffer
 * @param w width of the sprite
 * @param h height of the sprite
 */

int32 MirrorSprite(uint8 *dst, uint8 *src, int16 w, int16 h) {
	int16 x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			*dst++ = *(src + w - x - 1);
		}
		src += w;
	}
	
	return RD_OK;
}

/**
 * This function takes a compressed frame of a sprite with up to 256 colours
 * and decompresses it.
 * @param dest destination buffer
 * @param source source buffer
 * @param decompSize the expected size of the decompressed sprite
 */

int32 DecompressRLE256(uint8 *dest, uint8 *source, int32 decompSize) {
	// PARAMETERS:
	// source	points to the start of the sprite data for input
	// decompSize	gives size of decompressed data in bytes
	// dest		points to start of destination buffer for decompressed
	// 		data

	uint8 headerByte;			// block header byte
	uint8 *endDest = dest + decompSize;	// pointer to byte after end of decomp buffer
	int32 rv;

	while(1) {
		// FLAT block
		// read FLAT block header & increment 'scan' to first pixel
		// of block
		headerByte = *source++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dest + headerByte > endDest) {
				rv = 1;
				break;
			}

			// set the next 'headerByte' pixels to the next colour
			// at 'source'
			memset(dest, *source, headerByte);

			// increment destination pointer to just after this
			// block
			dest += headerByte;

			// increment source pointer to just after this colour
			source++;

			// if we've decompressed all of the data
			if (dest == endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}

		// RAW block
		// read RAW block header & increment 'scan' to first pixel of
		// block
		headerByte = *source++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dest + headerByte > endDest) {
				rv = 1;
				break;
			}

			// copy the next 'headerByte' pixels from source to
			// destination
			memcpy(dest,source,headerByte);

			// increment destination pointer to just after this
			// block
			dest += headerByte;

			// increment source pointer to just after this block
			source += headerByte;

			// if we've decompressed all of the data
			if (dest == endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}
	}

	return rv;
}

/**
 * Unwinds a run of 16-colour data into 256-colour palette data.
 */

void UnwindRaw16(uint8 *dest, uint8 *source, uint8 blockSize, uint8 *colTable) {
	// for each pair of pixels
	while (blockSize > 1) {
		// 1st colour = number in table at position given by upper
		// nibble of source byte
		*dest++ = colTable[(*source) >> 4];

		// 2nd colour = number in table at position given by lower
		// nibble of source byte
		*dest++ = colTable[(*source) & 0x0f];

		// point to next source byte
		source++;

		// decrement count of how many pixels left to read
		blockSize -= 2;
	}

	// if there's a final odd pixel
	if (blockSize) {
		// colour = number in table at position given by upper nibble
		// of source byte
		*dest++ = colTable[(*source) >> 4];
	}
}

/**
 * This function takes a compressed frame of a sprite (with up to 16 colours)
 * and decompresses it.
 * @param dest destination buffer
 * @param source source buffer
 * @param decompSize the expected size of the uncompressed sprite
 * @param colTable mapping from the 16 encoded colours to the current palette
 */

int32 DecompressRLE16(uint8 *dest, uint8 *source, int32 decompSize, uint8 *colTable) {
	uint8 headerByte;			// block header byte
	uint8 *endDest = dest + decompSize;	// pointer to byte after end of decomp buffer
	int32 rv;

	while(1) {
		// FLAT block
		// read FLAT block header & increment 'scan' to first pixel
		// of block
		headerByte = *source++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dest + headerByte > endDest) {
				rv = 1;
				break;
			}

			// set the next 'headerByte' pixels to the next
			// colour at 'source'
			memset(dest, *source, headerByte);

			// increment destination pointer to just after this
			// block
			dest += headerByte;

			// increment source pointer to just after this colour
			source++;

			// if we've decompressed all of the data
			if (dest == endDest) {	
				rv = 0;		// return "OK"
				break;
			}
		}

		// RAW block
		// read RAW block header & increment 'scan' to first pixel of
		// block
		headerByte = *source++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dest + headerByte > endDest) {
				rv = 1;
				break;
			}

			// copy the next 'headerByte' pixels from source to
			// destination (NB. 2 pixels per byte)
			UnwindRaw16(dest, source, headerByte, colTable);

			// increment destination pointer to just after this
			// block
			dest += headerByte;

			// increment source pointer to just after this block
			// (NB. headerByte gives pixels, so /2 for bytes)
			source += (headerByte + 1) / 2;

			// if we've decompressed all of the data
			if (dest >= endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}
	}

	return rv;
}

/**
 * Creates a sprite surface. Sprite surfaces are used by the in-game dialogs
 * and for displaying cutscene subtitles, which makes them much easier to draw
 * than standard sprites.
 * @param s information about how to decode the sprite
 * @param sprite the buffer that will be created to store the surface
 * @return RD_OK, or an error code
 */

int32 CreateSurface(_spriteInfo *s, uint8 **sprite) {
	uint8 *newSprite;

	*sprite = (uint8 *) malloc(s->w * s->h);
	if (!*sprite)
		return RDERR_OUTOFMEMORY;

	if (s->type & RDSPR_NOCOMPRESSION) {
		memcpy(*sprite, s->data, s->w * s->h);
	} else {
		if ((s->type >> 8) == (RDSPR_RLE16 >> 8)) {
			if (DecompressRLE16(*sprite, s->data, s->w * s->h, s->colourTable)) {
				free(*sprite);
				return RDERR_DECOMPRESSION;
			}
		} else {
			if (DecompressRLE256(*sprite, s->data, s->w * s->h)) {
				free(*sprite);
				return RDERR_DECOMPRESSION;
			}
		}

		if (s->type & RDSPR_FLIP) {
			newSprite = (uint8 *) malloc(s->w * s->h);
			if (!newSprite) {
				free(*sprite);
				return RDERR_OUTOFMEMORY;
			}
			MirrorSprite(newSprite, *sprite, s->w, s->h);
			free(*sprite);
			*sprite = newSprite;
		}
	}

	return RD_OK;
}

/**
 * Draws the sprite surface created earlier.
 * @param s information about how to place the sprite
 * @param surface pointer to the surface created earlier
 * @param clipRect the clipping rectangle
 */

void DrawSurface(_spriteInfo *s, uint8 *surface, ScummVM::Rect *clipRect) {
	ScummVM::Rect rd, rs;
	uint16 x, y, srcPitch;
	uint8 *src, *dst;

	rs.left = 0;
	rs.right = s->w;
	rs.top = 0;
	rs.bottom = s->h;

	srcPitch = s->w;

	if (s->type & RDSPR_DISPLAYALIGN) {
		rd.top = s->y;
		rd.left = s->x;
	} else {
		rd.top = s->y - scrolly;
		rd.left = s->x - scrollx;
	}

	rd.right = rd.left + rs.right;
	rd.bottom = rd.top + rs.bottom;

	if (clipRect) {
		if (clipRect->left > rd.left) {
			rs.left += (clipRect->left - rd.left);
			rd.left = clipRect->left;
		}

		if (clipRect->top > rd.top) {
			rs.top += (clipRect->top - rd.top);
			rd.top = clipRect->top;
		}

		if (clipRect->right < rd.right) {
			rd.right = clipRect->right;
		}

		if (clipRect->bottom < rd.bottom) {
			rd.bottom = clipRect->bottom;
		}

		if (rd.width() <= 0 || rd.height() <= 0)
			return;
	}

	src = surface + rs.top * srcPitch + rs.left;
	dst = lpBackBuffer + screenWide * rd.top + rd.left;

	// Surfaces are always transparent.

	for (y = 0; y < rd.height(); y++) {
		for (x = 0; x < rd.width(); x++) {
			if (src[x])
				dst[x] = src[x];
		}
		src += srcPitch;
		dst += screenWide;
	}

	UploadRect(&rd);
	SetNeedRedraw();
}

/**
 * Destroys a surface.
 */

void DeleteSurface(uint8 *surface) {
	free(surface);
}

#define SCALE_MAXWIDTH 512
#define SCALE_MAXHEIGHT 512

/**
 * Draws a sprite onto the screen. The type of the sprite can be a combination
 * of the following flags, some of which are mutually exclusive:
 * RDSPR_DISPLAYALIGN	The sprite is drawn relative to the top left corner
 *			of the screen
 * RDSPR_FLIP		The sprite is mirrored
 * RDSPR_TRANS		The sprite has a transparent colour zero
 * RDSPR_BLEND		The sprite is translucent
 * RDSPR_SHADOW		The sprite is affected by the light mask. (Scaled
 *			sprites always are.)
 * RDSPR_NOCOMPRESSION	The sprite data is not compressed
 * RDSPR_RLE16		The sprite data is a 16-colour compressed sprite
 * RDSPR_RLE256		The sprite data is a 256-colour compressed sprite
 * @param s all the information needed to draw the sprite
 * @warning Sprites will only be drawn onto the background, not over menubar
 * areas.
 */

// FIXME: I'm sure this could be optimized. There's plenty of data copying and
// mallocing here.

int32 DrawSprite(_spriteInfo *s) {
	uint8 *src, *dst;
	uint8 *sprite, *newSprite;
	uint8 *backbuf = NULL;
	uint8 red, green, blue;
	uint16 scale;
	int16 i, j;
	uint16 srcPitch;
	bool freeSprite = false;
	bool clipped = false;
	ScummVM::Rect rd, rs;

	// -----------------------------------------------------------------
	// Decompression and mirroring
	// -----------------------------------------------------------------

	if (s->type & RDSPR_NOCOMPRESSION)
		sprite = s->data;
	else {
		sprite = (uint8 *) malloc(s->w * s->h);
		freeSprite = true;
		if (!sprite)
			return RDERR_OUTOFMEMORY;
		if ((s->type >> 8) == (RDSPR_RLE16 >> 8)) {
			if (DecompressRLE16(sprite, s->data, s->w * s->h, s->colourTable)) {
				free(sprite);
				return RDERR_DECOMPRESSION;
			}
		} else {
			if (DecompressRLE256(sprite, s->data, s->w * s->h)) {
				free(sprite);
				return RDERR_DECOMPRESSION;
			}
		}
	}

	if (s->type & RDSPR_FLIP) {
		newSprite = (uint8 *) malloc(s->w * s->h);
		if (newSprite == NULL) {
			if (freeSprite)
				free(sprite);
			return RDERR_OUTOFMEMORY;
		}
		MirrorSprite(newSprite, sprite, s->w, s->h);
		if (freeSprite)
			free(sprite);
		sprite = newSprite;
		freeSprite = true;
	}

	// -----------------------------------------------------------------
	// Positioning and clipping.
	// -----------------------------------------------------------------

	if (!(s->type & RDSPR_DISPLAYALIGN)) {
		s->x += parallaxScrollx;
		s->y += parallaxScrolly;
	}

	s->y += 40;

	// A scale factor 0 or 256 means don't scale. Why do they use two
	// different values to mean the same thing? Normalize it here for
	// convenience.

	scale = (s->scale == 0) ? 256 : s->scale;

	rs.top = 0;
	rs.left = 0;

	if (scale != 256) {
		rs.right = s->scaledWidth;
		rs.bottom = s->scaledHeight;
		srcPitch = s->scaledWidth;
	} else {
		rs.right = s->w;
		rs.bottom = s->h;
		srcPitch = s->w;
	}

	rd.top = s->y;
	rd.left = s->x;

	if (!(s->type & RDSPR_DISPLAYALIGN)) {
		rd.top -= scrolly;
		rd.left -= scrollx;
	}

	rd.right = rd.left + rs.right;
	rd.bottom = rd.top + rs.bottom;

	// Check if the sprite would end up completely outside the screen.

	if (rd.left > 640 || rd.top > 440 || rd.right < 0 || rd.bottom < 40) {
		if (freeSprite)
			free(sprite);
		return RD_OK;
	}

	if (rd.top < 40) {
		rs.top = 40 - rd.top;
		rd.top = 40;
		clipped = true;
	}
	if (rd.bottom > 440) {
		rd.bottom = 440;
		rs.bottom = rs.top + (rd.bottom - rd.top);
		clipped = true;
	}
	if (rd.left < 0) {
		rs.left = -rd.left;
		rd.left = 0;
		clipped = true;
	}
	if (rd.right > 640) {
		rd.right = 640;
		rs.right = rs.left + (rd.right - rd.left);
		clipped = true;
	}

	// -----------------------------------------------------------------
	// Scaling
	// -----------------------------------------------------------------

	if (scale != 256) {
		if ((renderCaps & RDBLTFX_ARITHMETICSTRETCH) && !clipped)
			backbuf = lpBackBuffer + screenWide * rd.top + rd.left;
			

		if (s->scaledWidth > SCALE_MAXWIDTH || s->scaledHeight > SCALE_MAXHEIGHT) {
			if (freeSprite)
				free(sprite);
			return RDERR_NOTIMPLEMENTED;
		}

		newSprite = (uint8 *) malloc(s->scaledWidth * s->scaledHeight);
		if (newSprite == NULL) {
			if (freeSprite)
				free(sprite);
			return RDERR_OUTOFMEMORY;
		}

		if (scale < 256) {
			SquashImage(newSprite, s->scaledWidth, s->scaledWidth, s->scaledHeight, sprite, s->w, s->w, s->h, backbuf);
		} else {
			if (s->scale > 512) {
				if (freeSprite)
					free(sprite);
				return RDERR_INVALIDSCALING;
			}
			StretchImage(newSprite, s->scaledWidth, s->scaledWidth, s->scaledHeight, sprite, s->w, s->w, s->h, backbuf);
		}

		if (freeSprite)
			free(sprite);
		sprite = newSprite;
		freeSprite = true;
	}

	// -----------------------------------------------------------------
	// Light masking
	// -----------------------------------------------------------------

	// The light mask is an optional layer that covers the entire room
	// and which is used to simulate light and shadows. Scaled sprites
	// (actors, presumably) are always affected.

	if ((renderCaps & RDBLTFX_SHADOWBLEND) && lightMask && (scale != 256 || (s->type & RDSPR_SHADOW))) {
		uint8 *lightMap;

		if (!freeSprite) {
			newSprite = (uint8 *) malloc(s->w * s->h);
			memcpy(newSprite, sprite, s->w * s->h);
			sprite = newSprite;
			freeSprite = true;
		}

		src = sprite + rs.top * srcPitch + rs.left;
		lightMap = lightMask + (rd.top + scrolly - 40) * locationWide + rd.left + scrollx;

		for (i = 0; i < rs.height(); i++) {
			for (j = 0; j < rs.width(); j++) {
				if (src[j] && lightMap[j]) {
					uint8 r = ((32 - lightMap[j]) * palCopy[src[j]][0]) >> 5;
					uint8 g = ((32 - lightMap[j]) * palCopy[src[j]][1]) >> 5;
					uint8 b = ((32 - lightMap[j]) * palCopy[src[j]][2]) >> 5;
					src[j] = QuickMatch(r, g, b);
				}
			}
			src += srcPitch;
			lightMap += locationWide;
		}
	}

	// -----------------------------------------------------------------
	// Drawing
	// -----------------------------------------------------------------

	src = sprite + rs.top * srcPitch + rs.left;
	dst = lpBackBuffer + screenWide * rd.top + rd.left;

	if (s->type & RDSPR_BLEND) {
		if (renderCaps & RDBLTFX_ALLHARDWARE) {
			for (i = 0; i < rs.height(); i++) {
				for (j = 0; j < rs.width(); j++) {
					if (src[j] && ((i & 1) == (j & 1)))
						dst[j] = src[j];
				}
				src += srcPitch;
				dst += screenWide;
			}
		} else {
			if (s->blend & 0x01) {
				red = s->blend >> 8;
				for (i = 0; i < rs.height(); i++) {
					for (j = 0; j < rs.width(); j++) {
						if (src[j]) {
							uint8 r = (palCopy[src[j]][0] * red + palCopy[dst[j]][0] * (8 - red)) >> 3;
							uint8 g = (palCopy[src[j]][1] * red + palCopy[dst[j]][1] * (8 - red)) >> 3;
							uint8 b = (palCopy[src[j]][2] * red + palCopy[dst[j]][2] * (8 - red)) >> 3;
							dst[j] = QuickMatch(r, g, b);
						}
					}
					src += srcPitch;
					dst += screenWide;
				}
			} else if (s->blend & 0x02) {
				// FIXME: This case looks bogus to me. The
				// same value for the red, green and blue
				// parameters, and we multiply with the source
				// color's palette index rather than its color
				// component.
				//
				// But as far as I can see, that's how the
				// original
				// code did it.
				//
				// Does anyone know where this case was used
				// anyway?

				red = palCopy[s->blend >> 8][0];
				green = palCopy[s->blend >> 8][0];
				blue = palCopy[s->blend >> 8][0];
				for (i = 0; i < rs.height(); i++) {
					for (j = 0; j < rs.width(); j++) {
						if (src[j]) {
							uint8 r = (src[j] * red + (16 - src[j]) * palCopy[dst[j]][0]) >> 4;
							uint8 g = (src[j] * green + (16 - src[j]) * palCopy[dst[j]][1]) >> 4;
							uint8 b = (src[j] * blue + (16 - src[j]) * palCopy[dst[j]][2]) >> 4;
							dst[j] = QuickMatch(r, g, b);
						}
					}
					src += srcPitch;
					dst += screenWide;
				}
			} else {
				warning("DrawSprite: Invalid blended sprite");
				if (freeSprite)
					free(sprite);
				return RDERR_UNKNOWNTYPE;
			}
		}
	} else {
		if (s->type & RDSPR_TRANS) {
			for (i = 0; i < rs.height(); i++) {
				for (j = 0; j < rs.width(); j++) {
					if (src[j])
						dst[j] = src[j];
				}
				src += srcPitch;
				dst += screenWide;
			}
		} else {
			for (i = 0; i < rs.height(); i++) {
				memcpy(dst, src, rs.width());
				src += srcPitch;
				dst += screenWide;
			}
		}
	}

	if (freeSprite)
		free(sprite);

	// UploadRect(&rd);
	SetNeedRedraw();

	return RD_OK;
}

/**
 * Opens the light masking sprite for a room.
 */

int32 OpenLightMask(_spriteInfo *s) {
	// FIXME: The light mask is only needed on higher graphics detail
	// settings, so to save memory we could simply ignore it on lower
	// settings. But then we need to figure out how to ensure that it
	// is properly loaded if the user changes the settings in mid-game.

	if (lightMask)
		return RDERR_NOTCLOSED;

	lightMask = (uint8 *) malloc(s->w * s->h);
	if (!lightMask)
		return RDERR_OUTOFMEMORY;

	if (DecompressRLE256(lightMask, s->data, s->w * s->h))
		return RDERR_DECOMPRESSION;

	return RD_OK;
}

/**
 * Closes the light masking sprite for a room.
 */

int32 CloseLightMask(void) {
	if (!lightMask)
		return RDERR_NOTOPEN;

	free(lightMask);
	lightMask = 0;
	return RD_OK;
}
