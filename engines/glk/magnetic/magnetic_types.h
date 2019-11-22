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

#ifndef GLK_MAGNETIC_TYPES
#define GLK_MAGNETIC_TYPES

#include "common/scummsys.h"
#include "glk/magnetic/magnetic_defs.h"

namespace Glk {
namespace Magnetic {

struct lookup {
	int16 flag;
	int16 count;

	lookup() : flag(0), count(0) {}
};

struct picture {
	byte *data;
	uint32 data_size;
	uint16 width;
	uint16 height;
	uint16 wbytes;
	uint16 plane_step;
	byte *mask;

	picture() : data(nullptr), data_size(0), width(0), height(0), wbytes(0), plane_step(0),
		mask(nullptr) {}
};

/**
 * Magnetic animated pictures support
 *
 * Note: Some of the pictures for Wonderland and the Collection Volume 1 games
 * are animations. To detect these, pass a pointer to a type8 as the is_anim
 * argument to ms_extract().
 *
 * There are two types of animated images, however almost all images are type1.
 * A type1 image consists of four main elements:
 * 1) A static picture which is loaded straight at the beginning
 * 2) A set of frames with a mask. These frames are just "small pictures", which
 *    are coded like the normal static pictures. The image mask determines
 *    how the frame is removed after it has been displayed. A mask is exactly
 *    1/8 the size of the image and holds 1 bit per pixel, saying "remove pixel"
 *    or leave pixel set when frame gets removed. It might be a good idea to check
 *    your system documentation for masking operations as your system might be
 *    able to use this mask data directly.
 * 3) Positioning tables. These hold animation sequences consisting of commands
 *    like "Draw frame 12 at (123,456)"
 * 4) A playback script, which determines how to use the positioning tables.
 *    These scripts are handled inside Magnetic, so no need to worry about.
 *    However, details can be found in the ms_animate() function.
 *
 * A type2 image is like a type1 image, but it does not have a static
 * picture, nor does it have frame masking. It just consists of frames.
 *
 * How to support animations?
 * After getting is_anim == 1 you should call ms_animate() immediately, and at
 * regular intervals until ms_animate() returns 0. An appropriate interval
 * between calls is about 100 milliseconds.
 * Each call to ms_animate() will fill in the arguments with the address
 * and size of an array of ms_position structures (see below), each of
 * which holds an an animation frame number and x and y co-ordinates. To
 * display the animation, decode all the animation frames (discussed below)
 * from a single call to ms_animate() and display each one over the main picture.
 * If your port does not support animations, define NO_ANIMATION.
 */
struct ms_position {
	int16 x, y;
	int16 number;

	ms_position() : x(0), y(0), number(0) {}
};

/**
 * Magnetic Windows hint support
 *
 * The windowed Magnetic Scolls games included online hints. To add support
 * for the hints to your magnetic port, you should implement the ms_showhints
 * function. It retrieves a pointer to an array of ms_hint structs
 * The root element is always hints[0]. The elcount determines the number
 * of items in this topic. You probably want to display those in some kind
 * of list interface. The content pointer points to the actual description of
 * the items, separated by '\0' terminators. The nodetype is 1 if the items are
 * "folders" and 2 if the items are hints. Hints should be displayed one after
 * another. For "folder" items, the links array holds the index of the hint in
 * the array which is to be displayed on selection. One hint block has exactly
 * one type. The parent element determines the "back" target.
 */
struct ms_hint {
	uint16 elcount;
	uint16 nodetype;
	const char *content;
	uint16 links[MAX_HITEMS];
	uint16 parent;

	ms_hint() : elcount(0), nodetype(0), content(nullptr), parent(0) {
		Common::fill(&links[0], &links[MAX_HITEMS], 0);
	}
};

struct gms_gamma_t {
	const char *const level;       ///< Gamma correction level
	const unsigned char table[8];  ///< Color lookup table
	const bool is_corrected;       ///< Flag if non-linear
};
typedef const gms_gamma_t *gms_gammaref_t;

/* R,G,B color triple definition. */
struct gms_rgb_t {
	int red, green, blue;

	gms_rgb_t() : red(0), green(0), blue(0) {}
	gms_rgb_t(int r, int g, int b) : red(r), green(b), blue(b) {}
};
typedef gms_rgb_t *gms_rgbref_t;

#ifndef GARGLK
struct gms_layering_t {
	long complexity;  /* Count of vertices for this color. */
	long usage;       /* Color usage count. */
	int color;        /* Color index into palette. */
};
#endif

/* Table of single-character command abbreviations. */
struct gms_abbreviation_t {
	const char abbreviation;       /* Abbreviation character. */
	const char *const expansion;   /* Expansion string. */
};
typedef gms_abbreviation_t *gms_abbreviationref_t;

} // End of namespace Magnetic
} // End of namespace Glk

#endif
