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

#ifndef XEEN_SOUND_H
#define XEEN_SOUND_H

#include "common/scummsys.h"
#include "common/system.h"
#include "xeen/files.h"

namespace Xeen {

class SoundManager {
private:
	XeenEngine *_vm;
public:
	SoundManager(XeenEngine *vm);

	void proc2(File &f);

	void startMusic(int v1);

	void playSong(const File &f) {}

	void playSample(const Common::SeekableReadStream *stream, int v2) {}
};

} // End of namespace Xeen

#endif /* XEEN_SOUND_H */
