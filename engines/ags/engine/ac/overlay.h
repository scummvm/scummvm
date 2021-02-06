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

#ifndef AGS_ENGINE_AC_OVERLAY_H
#define AGS_ENGINE_AC_OVERLAY_H

#include "ags/lib/std/vector.h"
#include "ags/engine/ac/screenoverlay.h"
#include "ags/engine/ac/dynobj/scriptoverlay.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

void Overlay_Remove(ScriptOverlay *sco);
void Overlay_SetText(ScriptOverlay *scover, int wii, int fontid, int clr, const char *text);
int  Overlay_GetX(ScriptOverlay *scover);
void Overlay_SetX(ScriptOverlay *scover, int newx);
int  Overlay_GetY(ScriptOverlay *scover);
void Overlay_SetY(ScriptOverlay *scover, int newy);
int  Overlay_GetValid(ScriptOverlay *scover);
ScriptOverlay *Overlay_CreateGraphical(int x, int y, int slot, int transparent);
ScriptOverlay *Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char *text);

int  find_overlay_of_type(int type);
void remove_screen_overlay(int type);
// Calculates overlay position in screen coordinates
void get_overlay_position(const ScreenOverlay &over, int *x, int *y);
size_t add_screen_overlay(int x, int y, int type, Shared::Bitmap *piccy, bool alphaChannel = false);
size_t add_screen_overlay(int x, int y, int type, Shared::Bitmap *piccy, int pic_offx, int pic_offy, bool alphaChannel = false);
void remove_screen_overlay_index(size_t over_idx);
void recreate_overlay_ddbs();

extern int is_complete_overlay;
extern int is_text_overlay; // blocking text overlay on screen

extern std::vector<ScreenOverlay> screenover;

} // namespace AGS3

#endif
