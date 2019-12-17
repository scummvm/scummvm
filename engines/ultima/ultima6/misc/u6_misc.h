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

#ifndef ULTIMA6_MISC_U6_MISC_H
#define ULTIMA6_MISC_U6_MISC_H

/*
 *  U6misc.h
 *  Nuvie
 *
 *  Created by Eric Fry on Sat Jun 14 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */
#include "ultima/shared/std/string.h"
#include "ultima/ultima6/core/nuvie_defs.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;

typedef enum {
	BLOCKED,
	CAN_MOVE,
	FORCE_MOVE
} MovementStatus;

std::string config_get_game_key(Configuration *config);
const char *get_game_tag(int game_type);
void config_get_path(Configuration *config, std::string filename, std::string &path);
uint8 get_game_type(const char *string);
nuvie_game_t get_game_type(Configuration *config);
void build_path(std::string path, std::string filename, std::string &full_path);
bool directory_exists(const char *directory);
bool file_exists(const char *path);
void print_b(DebugLevelType level, uint8 num);
void print_b16(DebugLevelType level, uint16 num);
void print_indent(DebugLevelType level, uint8 indent);
void print_bool(DebugLevelType level, bool state, const char *yes = "true", const char *no = "false");
void print_flags(DebugLevelType level, uint8 num, const char *f[8]);
bool subtract_rect(SDL_Rect *rect1, SDL_Rect *rect2, SDL_Rect *sub_rect);
uint8 get_nuvie_dir_code(uint8 original_dir_code);
sint8 get_original_dir_code(uint8 nuvie_dir_code);
uint8 get_direction_code(sint16 rel_x, sint16 rel_y);
uint8 get_reverse_direction(uint8 dir);
void get_relative_dir(uint8 dir, sint16 *rel_x, sint16 *rel_y);
const char *get_direction_name(uint8 dir);
const char *get_direction_name(sint16 rel_x, sint16 rel_y);
int str_bsearch(const char *str[], int max, const char *value);
void stringToLower(std::string &str);
/* Is point x,y within rect?
 */
inline bool point_in_rect(uint16 x, uint16 y, SDL_Rect *rect) {
	uint16 rx2 = rect->x + rect->w,
	       ry2 = rect->y + rect->h;
	return ((x >= rect->x && x <= rx2 && y >= rect->y && y <= ry2));
}


/* Does line xy->x2y2 cross rect, to any extent?
 */
inline bool line_in_rect(uint16 x1, uint16 y1, uint16 x2, uint16 y2, SDL_Rect *rect) {
	uint16 rx2 = rect->x + rect->w,
	       ry2 = rect->y + rect->h;
	return (((y1 >= rect->y && y1 <= ry2 && x1 <= rx2 && x2 >= rect->x)
	         || (x1 >= rect->x && x1 <= rx2 && y1 <= ry2 && y2 >= rect->y)));
}


/* Measure a timeslice for a single function-call. (last_time must be static)
 * Returns fraction of a second between this_time and last_time.
 */
inline uint32 divide_time(uint32 this_time, uint32 &last_time, uint32 *passed_time = NULL) {
	uint32 ms_passed = (this_time - last_time) > 0 ? (this_time - last_time) : 1;
	uint32 fraction = 1000 / ms_passed; // % of second
	last_time = this_time;
	if (passed_time)
		*passed_time = ms_passed;
	return (fraction);
}

int mkdir_recursive(std::string path, int mode);

void draw_line_8bit(int sx, int sy, int ex, int ey, uint8 col, uint8 *pixels, uint16 w, uint16 h);

bool string_i_compare(const std::string &s1, const std::string &s2);

void *nuvie_realloc(void *ptr, size_t size);

Uint32 sdl_getpixel(SDL_Surface *surface, int x, int y);

void scale_rect_8bit(unsigned char *Source, unsigned char *Target, int SrcWidth, int SrcHeight, int TgtWidth, int TgtHeight);

bool has_file_extension(const char *filename, const char *extension);

bool has_fmtowns_support(Configuration *config);

uint16 wrap_signed_coord(sint16 coord, uint8 level);
sint8 get_wrapped_rel_dir(sint16 p1, sint16 p2, uint8 level);

std::string encode_xml_entity(const std::string &s);

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
