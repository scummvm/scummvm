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

#ifndef DIRECTOR_UTIL_H
#define DIRECTOR_UTIL_H

namespace Common {
class String;
}

namespace Director {

int castNumToNum(const char *str);
char *numToCastNum(int num);

Common::String toLowercaseMac(const Common::String &s);

Common::String convertPath(Common::String &path);

Common::String getPath(Common::String path, Common::String cwd);

Common::String pathMakeRelative(Common::String path, bool recursive = true, bool addexts = true);

Common::String stripMacPath(const char *name);

Common::String convertMacFilename(const char *name);

Common::String dumpScriptName(const char *prefix, int type, int id, const char *ext);

bool processQuitEvent(bool click = false); // events.cpp

} // End of namespace Director

#endif
