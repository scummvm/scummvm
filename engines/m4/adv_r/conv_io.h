
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
	bool interface_was_visible = false;  // to remember to turn it back on
	int restore_conv = 1;
	int ent = 0;

	int32 conv_font_spacing_h = 0;
	int32 conv_font_spacing_v = 5;
	int32 conv_default_h = conv_font_spacing_h;
	int32 conv_default_v = conv_font_spacing_v;
	int32 conv_shading = 65;

	int32 conv_normal_colour = __BLACK;
	int32 conv_normal_colour_alt1 = __GREEN;
	int32 conv_normal_colour_alt2 = __GREEN;
	int32 conv_hilite_colour = __YELLOW;
	int32 conv_default_hilite_colour = __YELLOW;
	int32 conv_hilite_colour_alt1 = __YELLOW;
	int32 conv_hilite_colour_alt2 = __YELLOW;
	int32 conv_default_normal_colour = __BLACK;

	int32 width = 0, height = 0;
	int32 glob_x = 0, glob_y = 0;
	int32 r_x1 = 0, r_y1 = 0, r_x2 = 0, r_y2 = 0;

	void syncGame(Common::Serializer &s);

	void conv_reset_all();
	void conv_reset(const char *filename);

	void conv_play(Conv *c);
	void conv_go(Conv *c);
};

extern Conv *conv_load(const char *filename, int x1, int y1, int32 myTrigger, bool want_box = true);
extern void conv_load_and_prepare(const char *filename, int trigger, bool ignoreIt = false);

extern void conv_unload(Conv *c);
extern void conv_shutdown();

extern Conv *conv_get_handle();
extern void conv_set_handle(Conv *c);

extern void conv_resume(Conv *c);
extern void conv_resume();
extern void conv_reset(char *filename);

extern void conv_reset_all();
extern void conv_play(Conv *c);

extern void set_conv_name(const char *s);
extern const char *get_conv_name();
extern const char *conv_sound_to_play();
extern int32 conv_whos_talking();

extern long conv_get_decl_val(decl_chunk *decl);
extern void conv_set_decl_val(decl_chunk *decl, long val);
extern void conv_export_value(Conv *c, long val, int index);
extern void conv_export_value_curr(long val, int index);
extern void conv_export_pointer(Conv *c, long *val, int index);
extern void conv_export_pointer_curr(long *val, int index);

extern void conv_set_font_spacing(int32 h, int32 v);
extern void conv_set_text_colour(int32 norm_colour, int32 hi_colour);

extern void conv_set_text_colours(int32 norm_colour, int32 norm_colour_alt1, int32 norm_colour_alt2,
	int32 hi_colour, int32 hi_colour_alt1, int32 hi_colour_alt2);

extern void conv_set_shading(int32 shade);
extern void conv_set_box_xy(int32 x, int32 y);
extern void conv_get_dlg_coords(int32 *x1, int32 *y1, int32 *x2, int32 *y2);
extern void conv_set_dlg_coords(int32 x1, int32 y1, int32 x2, int32 y2);
extern void conv_set_default_text_colour(int32 norm_colour, int32 hi_colour);
extern void conv_set_default_hv(int32 h, int32 v);

extern int conv_get_event();
extern void conv_set_event(int e);
extern int conv_is_event_ready();

extern void conv_swap_words(Conv *c);
extern long conv_current_node();
extern int32 conv_current_entry();

} // End of namespace M4

#endif
