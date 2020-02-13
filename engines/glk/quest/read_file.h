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

#ifndef GLK_QUEST_READ_FILE
#define GLK_QUEST_READ_FILE

#include "glk/quest/geas_file.h"
#include "glk/quest/string.h"
#include "common/array.h"

namespace Glk {
namespace Quest {

extern Common::Array<String> tokenize(String s);
extern String next_token(String full, uint &tok_start, uint &tok_end, bool cvt_paren = false);
extern String first_token(String s, uint &t_start, uint &t_end);
extern String nth_token(String s, int n);
extern String get_token(String s, bool cvt_paren = false);
extern bool find_token(String s, String tok, int &tok_start, int &tok_end, bool cvt_paren = false);
extern GeasFile read_geas_file(GeasInterface *gi, const String &filename);

enum trim_modes { TRIM_SPACES, TRIM_UNDERSCORE, TRIM_BRACE };
extern String trim(String, trim_modes mode = TRIM_SPACES);

//Common::WriteStream &operator<< (Common::WriteStream &o, const Common::Array<String> &v);

} // End of namespace Quest
} // End of namespace Glk

#endif
