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
#ifndef SLUDGE_SAVEDATA_H
#define SLUDGE_SAVEDATA_H

namespace Sludge {

struct StackHandler;
struct Variable;

class CustomSaveHelper {
public:
	static bool fileToStack(const Common::String &filename, StackHandler *sH);
	static bool stackToFile(const Common::String &filename, const Variable &from);

	static uint16 _saveEncoding;

private:
	static const char UTF8_CHECKER[];
	static char _encode1;
	static char _encode2;

	static void writeStringEncoded(const Common::String checker, Common::WriteStream *stream);
	static Common::String readStringEncoded(Common::SeekableReadStream *fp);
	static char *readTextPlain(Common::SeekableReadStream *fp);

};

} // End of namespace Sludge

#endif
