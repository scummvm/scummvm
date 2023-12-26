
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

#ifndef M4_ADV_R_ADV_FILE_H
#define M4_ADV_R_ADV_FILE_H

#include "common/stream.h"
#include "m4/adv_r/adv.h"
#include "m4/fileio/sys_file.h"
#include "m4/graphics/gr_buff.h"
#include "m4/gui/gui.h"
#include "m4/m4_types.h"

namespace M4 {

M4sprite *kernel_CreateThumbnail(int32 *spriteSize);
bool kernel_CreateSSFromGrBuff(GrBuff *myBuff, RGB8 *myPalette, bool completePal, const char *ssName);

bool kernel_load_room(int minPalEntry, int maxPalEntry, SceneDef *rdef, GrBuff **scr_orig_data, GrBuff **scr_orig);
void kernel_unload_room(SceneDef *rdef, GrBuff **code_data, GrBuff **loadBuffer);

int kernel_save_game(int slot, const char *desc, int32 sizeofDesc, M4sprite *thumbNail, int32 sizeofThumbData);
bool kernel_load_game(int slot);
bool kernel_save_game_exists(int32 slot);
int32 extract_room_num(const Common::String &name);

bool kernel_load_variant(char *variant);
GrBuff *load_codes(SysFile *code_file);
bool load_background(SysFile *pic_file, GrBuff **loadBuffer, RGB8 *palette);

bool load_picture_and_codes(SysFile *pic_file, SysFile *code_file, GrBuff **loadBuf, GrBuff **code_data, uint8 minPalEntry, uint8 maxPalEntry);

bool kernel_load_code_variant(SceneDef *rdef, char *variant, GrBuff **codeData);

void kernel_current_background_name(char *result);
void kernel_current_codes_name(char *result);
void screen_capture(RGB8 *masterPalette);

} // End of namespace M4

#endif
