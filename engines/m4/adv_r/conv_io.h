
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
#include "m4/gui/gui_dialog.h"

namespace M4 {

#define HIDDEN			0x00000004
#define DESTROYED       0x00000008

#define INITIAL         1
#define PERSISTENT      2

#define CONV_OK          0
#define CONV_QUIT       -1
#define CONV_NEW        -2
#define CONV_BAIL       -3

#define CONV_UNKNOWN_MODE   0
#define CONV_GET_TEXT_MODE  1
#define CONV_SET_TEXT_MODE  2
#define CONV_GET_MESG_MODE  3
#define CONV_UPDATE_MODE    4

#define DECL_POINTER	1

struct Converstation_Globals {
	Common::Array<byte> conv_save_buff;
	Common::Array<byte> convSave;
	int	event = 0;
	int	event_ready = 0;
	char conv_name[16];
	Conv *globConv = nullptr;
	bool playerCommAllowed = false;
	int32 myFinalTrigger = 0;
	bool interface_was_visible = false;  // to remember to turn it back on
	int restore_conv = 1;
	int ent = 0;
	bool swap = false;

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

	TextScrn *myTextScrn = nullptr;
	int32 width = 0, height = 0;
	int32 glob_x = 0, glob_y = 0;
	int32 r_x1 = 0, r_y1 = 0, r_x2 = 0, r_y2 = 0;
	int n_t_e = 0;

	void syncGame(Common::Serializer &s);

	void conv_reset_all();
};

Conv *conv_load(const char *filename, int x1, int y1, int32 myTrigger, bool want_box = true);
void conv_load_and_prepare(const char *filename, int trigger, bool ignoreIt = false);

void conv_unload(Conv *c);
void conv_unload();
void conv_shutdown();

Conv *conv_get_handle();
void conv_set_handle(Conv *c);

void conv_resume(Conv *c);
void conv_resume();
void conv_reset(char *filename);

void conv_reset_all();
void conv_play(Conv *c);
void conv_play();

const char *conv_sound_to_play();
int32 conv_whos_talking();

int32 conv_get_decl_val(Conv *c, decl_chunk *decl);
void conv_set_decl_val(Conv *c, decl_chunk *decl, int32 val);
void conv_export_value(Conv *c, int32 val, int index);
void conv_export_value_curr(int32 val, int index);
void conv_export_pointer(Conv *c, int32 *val, int index);
void conv_export_pointer_curr(int32 *val, int index);

void conv_set_font_spacing(int32 h, int32 v);
void conv_set_text_colour(int32 norm_colour, int32 hi_colour);

void conv_set_text_colours(int32 norm_colour, int32 norm_colour_alt1, int32 norm_colour_alt2,
	int32 hi_colour, int32 hi_colour_alt1, int32 hi_colour_alt2);

void conv_set_shading(int32 shade);
void conv_set_box_xy(int32 x, int32 y);
void conv_get_dlg_coords(int32 *x1, int32 *y1, int32 *x2, int32 *y2);
void conv_set_dlg_coords(int32 x1, int32 y1, int32 x2, int32 y2);
void conv_set_default_text_colour(int32 norm_colour, int32 hi_colour);
void conv_set_default_hv(int32 h, int32 v);

int conv_get_event();
void conv_set_event(int e);
int conv_is_event_ready();

void conv_swap_words(Conv *c);
int32 conv_current_node();
int32 conv_current_entry();

int conv_toggle_flags(entry_chunk *entry);
int ok_status(entry_chunk *entry);
int conv_get_text(int32 offset, int32 size, Conv *c);
void cdd_init();

} // End of namespace M4

#endif
