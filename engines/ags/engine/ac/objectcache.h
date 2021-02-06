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

#ifndef AGS_ENGINE_AC_OBJECTCACHE_H
#define AGS_ENGINE_AC_OBJECTCACHE_H

namespace AGS3 {

// stores cached object info
struct ObjectCache {
	Shared::Bitmap *image;
	int   sppic;
	short tintredwas, tintgrnwas, tintbluwas, tintamntwas, tintlightwas;
	short lightlevwas, mirroredWas, zoomWas;
	// The following are used to determine if the character has moved
	int   xwas, ywas;
};

} // namespace AGS3

#endif
