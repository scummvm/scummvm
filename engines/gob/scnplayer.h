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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_SCNPLAYER_H
#define GOB_SCNPLAYER_H

#include "common/file.h"
#include "common/str.h"
#include "common/hashmap.h"

namespace Gob {

class GobEngine;

class SCNPlayer {
public:
	SCNPlayer(GobEngine *vm);
	~SCNPlayer();

	bool play(const char *fileName);

private:
	typedef Common::HashMap<Common::String, int32, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> LabelMap;

	GobEngine *_vm;

	bool play(Common::File &scn);
	bool readLabels(Common::File &scn, LabelMap &labels);

	inline bool lineStartsWith(const Common::String &line, const char *start);

	void gotoLabel(Common::File &scn, const LabelMap &labels, const char *label);
	void clearScreen();
	void playVideo(const char *fileName);
};

} // End of namespace Gob

#endif // GOB_SCNPLAYER_H
