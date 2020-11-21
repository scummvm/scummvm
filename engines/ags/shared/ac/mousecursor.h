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

#ifndef AGS_SHARED_AC_MOUSECURSOR_H
#define AGS_SHARED_AC_MOUSECURSOR_H

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

#define MCF_ANIMMOVE 1
#define MCF_DISABLED 2
#define MCF_STANDARD 4
#define MCF_HOTSPOT  8  // only animate when over hotspot
// this struct is also in the plugin header file
struct MouseCursor {
    int   pic;
    short hotx, hoty;
    short view;
    char  name[10];
    char  flags;
    MouseCursor();

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out);
    void ReadFromSavegame(Common::Stream *in);
    void WriteToSavegame(Common::Stream *out) const;
};

#endif
