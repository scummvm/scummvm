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

#ifndef TESORO_SCRIPTS_H
#define TESORO_SCRIPTS_H

#include "common/textconsole.h"
#include "tesoro/core/archive.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/cameraframe.h"
#include "tesoro/core/cursor.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/scriptblock.h"
#include "tesoro/world.h"
#include "tesoro/tesoro.h"

namespace Tesoro {

/** The scripts: the compiled blocks, the runner, and everything a running block asks of the game. */
class Scripts {
public:
	enum { kMaxChoices = 20 };

	explicit Scripts(World &world) : _w(world) {}
	~Scripts() {
	}

	bool load() {
		static const char *const kFiles[] = { "ANIMS.DLG", "CONVERS.DLG", "ICONS.DLG", "OBJETOS.DLG" };
		for (uint i = 0; i < 4; ++i) {
			const uint32 offset = _w._engine._archive.findEntry(kFiles[i]);
			if (!offset) {
				warning("Tesoro: %s is not in the archive", kFiles[i]);
				return false;
			}
			Common::SeekableReadStream &dat = *_w._engine._archive.seekEntry(offset - 4);
			const uint32 size = dat.readUint32LE();
			_text[i].resize(size);
			if (dat.read(_text[i].data(), size) != size || !_library.load(_text[i].data(), size, _w._speech)) {
				warning("Tesoro: %s could not be compiled", kFiles[i]);
				return false;
			}
		}
		_library.resolve();
		return true;
	}

	const ScriptLibrary &library() const { return _library; }

private:
	World &_w;
	ScriptLibrary _library;
	Common::Array<byte> _text[4];
};

} // End of namespace Tesoro

#endif
