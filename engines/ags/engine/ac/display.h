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

#ifndef AGS_ENGINE_AC_DISPLAY_H
#define AGS_ENGINE_AC_DISPLAY_H

#include "ags/shared/gui/gui_main.h"

namespace AGS3 {

using AGS::Shared::GUIMain;

// options for 'disp_type' parameter
// blocking speech
#define DISPLAYTEXT_SPEECH        0
// super-blocking message box
#define DISPLAYTEXT_MESSAGEBOX    1
// regular non-blocking overlay
#define DISPLAYTEXT_NORMALOVERLAY 2
// also accepts explicit overlay ID >= OVER_CUSTOM

struct ScreenOverlay;
Shared::Bitmap *create_textual_image(const char *text, int asspch, int isThought,
									 int &xx, int &yy, int &adjustedXX, int &adjustedYY, int wii, int usingfont, int allowShrink,
									 bool &alphaChannel);
// Creates a textual overlay using the given parameters;
// Pass yy = -1 to find Y co-ord automatically
// allowShrink = 0 for none, 1 for leftwards, 2 for rightwards
// pass blocking=2 to create permanent overlay
ScreenOverlay *display_main(int xx, int yy, int wii, const char *text, int disp_type, int usingfont,
							int asspch, int isThought, int allowShrink, bool overlayPositionFixed, bool roomlayer = false);
// Displays a standard blocking message box at a given position
void display_at(int xx, int yy, int wii, const char *text);
// Cleans up display message state
void post_display_cleanup();
// Tests the given string for the voice-over tags and plays cue clip for the given character;
// will assign replacement string, which will be blank string if game is in "voice-only" mode
// and clip was started, or string cleaned from voice-over tags which is safe to display on screen.
// Returns whether voice-over clip was started successfully.
bool try_auto_play_speech(const char *text, const char *&replace_text, int charid);
// Calculates meaningful length of the displayed text
int GetTextDisplayLength(const char *text);
// Calculates number of game loops for displaying a text on screen
int GetTextDisplayTime(const char *text, int canberel = 0);
// Draw an outline if requested, then draw the text on top
void wouttext_outline(Shared::Bitmap *ds, int xxp, int yyp, int usingfont, color_t text_color, const char *texx);
void wouttext_aligned(Shared::Bitmap *ds, int usexp, int yy, int oriwid, int usingfont, color_t text_color, const char *text, HorAlignment align);
void do_corner(Shared::Bitmap *ds, int sprn, int xx1, int yy1, int typx, int typy);
// Returns the image of a button control on the GUI under given child index
int get_but_pic(GUIMain *guo, int indx);
void draw_button_background(Shared::Bitmap *ds, int xx1, int yy1, int xx2, int yy2, GUIMain *iep);
// Calculate the width that the left and right border of the textwindow
// GUI take up
int get_textwindow_border_width(int twgui);
// get the hegiht of the text window's top border
int get_textwindow_top_border_height(int twgui);
// draw_text_window: draws the normal or custom text window
// create a new bitmap the size of the window before calling, and
//   point text_window_ds to it
// returns text start x & y pos in parameters
// Warning!: draw_text_window() and draw_text_window_and_bar() can create new text_window_ds
void draw_text_window(Shared::Bitmap **text_window_ds, bool should_free_ds, int *xins, int *yins, int *xx, int *yy, int *wii, color_t *set_text_color, int ovrheight, int ifnum);
void draw_text_window_and_bar(Shared::Bitmap **text_window_ds, bool should_free_ds,
                              int *xins, int *yins, int *xx, int *yy, int *wii, color_t *set_text_color, int ovrheight = 0, int ifnum = -1);
int get_textwindow_padding(int ifnum);

} // namespace AGS3

#endif
