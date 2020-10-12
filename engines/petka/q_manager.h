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

#ifndef PETKA_QMANAGER_H
#define PETKA_QMANAGER_H

#include "common/hashmap.h"

namespace Common {
	class SeekableReadStream;
}

namespace Graphics {
	struct Surface;
}

namespace Petka {

class PetkaEngine;

class FlicDecoder;

class QManager {
public:
	explicit QManager(PetkaEngine &vm);

	bool init();

	Common::String findResourceName(uint32 id) const;
	Common::String findSoundName(uint32 id) const;

	Graphics::Surface *getSurface(uint32 id, uint16 w, uint16 h);
	Graphics::Surface *getSurface(uint32 id);
	FlicDecoder *getFlic(uint32 id);

	void removeResource(uint32 id);
	void clearUnneeded();
	void clear();

private:
	struct QResource {
		union {
			Graphics::Surface *surface;
			FlicDecoder *flcDecoder;
		};
		enum ResType {
			kSurface,
			kFlic
		} type;

		~QResource();
	};

	Common::SeekableReadStream *loadFileStream(uint32 id) const;

	static Graphics::Surface *loadBitmapSurface(Common::SeekableReadStream &stream);

private:
	PetkaEngine &_vm;
	Common::HashMap<uint32, QResource> _resourceMap;
	Common::HashMap<uint32, Common::String> _nameMap;
	Common::HashMap<uint32, bool> _isAlwaysNeededMap;
};

} // End of namespace Petka

#endif
