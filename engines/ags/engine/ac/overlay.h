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

#ifndef AGS_ENGINE_AC_OVERLAY_H
#define AGS_ENGINE_AC_OVERLAY_H

#include "common/std/vector.h"
#include "ags/shared/util/geometry.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/dynobj/script_overlay.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

void Overlay_Remove(ScriptOverlay *sco);
void Overlay_SetText(ScriptOverlay *scover, int width, int fontid, int text_color, const char *text);
void Overlay_SetText(ScreenOverlay &over, int x, int y, int width, int fontid, int text_color, const char *text);
int  Overlay_GetX(ScriptOverlay *scover);
void Overlay_SetX(ScriptOverlay *scover, int newx);
int  Overlay_GetY(ScriptOverlay *scover);
void Overlay_SetY(ScriptOverlay *scover, int newy);
int  Overlay_GetValid(ScriptOverlay *scover);
ScriptOverlay *Overlay_CreateGraphical(int x, int y, int slot, bool transparent = true, bool clone = false);
ScriptOverlay *Overlay_CreateTextual(int x, int y, int width, int font, int colour, const char *text);
ScreenOverlay *Overlay_CreateGraphicCore(bool room_layer, int x, int y, int slot, bool transparent = true, bool clone = false);
ScreenOverlay *Overlay_CreateTextCore(bool room_layer, int x, int y, int width, int font, int text_color,
									  const char *text, int disp_type, int allow_shrink);

ScreenOverlay *get_overlay(int type);
// Calculates overlay position in its respective layer (screen or room)
Point get_overlay_position(const ScreenOverlay &over);
size_t add_screen_overlay(bool roomlayer, int x, int y, int type, int sprnum);
size_t add_screen_overlay(bool roomlayer, int x, int y, int type, Shared::Bitmap *piccy, bool has_alpha);
size_t add_screen_overlay(bool roomlayer, int x, int y, int type, Shared::Bitmap *piccy, int pic_offx, int pic_offy, bool has_alpha);
void remove_screen_overlay(int type);
void remove_all_overlays();
// Creates and registers a managed script object for // Creates and registers a managed script object for existing overlay object;
// optionally adds an internal engine reference to prevent object's disposal
ScriptOverlay *create_scriptoverlay(ScreenOverlay &over, bool internal_ref = false);
// Restores overlays, e.g. after restoring a game save
void restore_overlays();

std::vector<ScreenOverlay> &get_overlays();

} // namespace AGS3

#endif
