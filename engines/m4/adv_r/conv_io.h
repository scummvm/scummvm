
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_ADV_R_CONV_IO_H
#define M4_ADV_R_CONV_IO_H

#include "common/array.h"
#include "common/serializer.h"
#include "m4/m4_types.h"
#include "m4/adv_r/conv.h"
#include "m4/graphics/gr_pal.h"

namespace M4 {

struct Converstation_Globals {
	Common::Array<byte> conv_save_buff;
	int	event = 0;
	int	event_ready = 0;
	char conv_file_name[MAX_FILENAME_SIZE] = { 0 };
	char conv_name[16];
	Conv *globConv = nullptr;
	bool playerCommAllowed = false;
	long myFinalTrigger = 0;
	bool interface_was_visible = false;  // to remember to turn it back on or 
	int restore_conv = 1;
	int32 glob_x = 0, glob_y = 0;

	const int32 conv_font_spacing_h = 0;
	const int32 conv_font_spacing_v = 5;
	const int32 conv_default_h = conv_font_spacing_h;
	const int32 conv_default_v = conv_font_spacing_v;
	const int32 conv_shading = 65;

	const int32 conv_normal_colour = __BLACK;
	const int32 conv_normal_colour_alt1 = __GREEN;
	const int32 conv_normal_colour_alt2 = __GREEN;
	const int32 conv_hilite_colour = __YELLOW;
	const int32 conv_default_hilite_colour = __YELLOW;
	const int32 conv_hilite_colour_alt1 = __YELLOW;
	const int32 conv_hilite_colour_alt2 = __YELLOW;
	const int32 conv_default_normal_colour = __BLACK;


	void syncGame(Common::Serializer &s);

	void conv_reset_all();
	void conv_reset(const char *filename);

	void conv_play(Conv *c);
	void conv_go(Conv *c);
};


} // End of namespace M4

#endif
