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

#ifndef KYRA_SOUND_MACRES_H
#define KYRA_SOUND_MACRES_H

namespace Common {
	class Archive;
	class MacResManager;
	class String;
	template<class T> class Array;
}

namespace Kyra {

class SoundMacRes {
public:
	SoundMacRes(KyraEngine_v1 *vm);
	~SoundMacRes();
	bool init();
	bool setQuality(bool hi);
	Common::SeekableReadStream *getResource(uint16 id, uint32 type);

private:
	Common::String _kyraMacExe;
	Common::MacResManager *_resMan;
	Common::Archive *_stuffItArchive;
};

} // End of namespace Kyra

#endif
