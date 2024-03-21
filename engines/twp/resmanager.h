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

#ifndef TWP_RESMANAGER_H
#define TWP_RESMANAGER_H

#include "common/str.h"
#include "common/hashmap.h"
#include "twp/gfx.h"
#include "twp/spritesheet.h"

namespace Twp {

class Font;

class ResManager {
private:
	enum {
		START_ACTORID = 1000,
		END_ACTORID = 2000,
		START_ROOMID = 2000,
		END_ROOMID = 3000,
		START_OBJECTID = 3000,
		END_OBJECTID = 100000,
		START_LIGHTID = 100000,
		END_LIGHTID = 200000,
		START_SOUNDDEFID = 200000,
		END_SOUNDDEFID = 250000,
		START_SOUNDID = 250000,
		END_SOUNDID = 300000,
		START_THREADID = 300000,
		END_THREADID = 8000000,
		START_CALLBACKID = 8000000,
		END_CALLBACKID = 10000000,
	};

public:
	static Common::String getKey(const Common::String &path);
	Texture *texture(const Common::String &name);
	SpriteSheet *spriteSheet(const Common::String &name);
	Common::SharedPtr<Font> font(const Common::String &name);
	void resetSaylineFont();

	bool isThread(int id) const;
	bool isRoom(int id) const;
	bool isActor(int id) const;
	bool isObject(int id) const;
	bool isSound(int id) const;
	bool isLight(int id) const;
	bool isCallback(int id) const;
	int getCallbackId() const;

	int newRoomId();
	int newObjId();
	int newActorId();
	int newSoundDefId();
	int newSoundId();
	int newThreadId();
	int newCallbackId();
	int newLightId();

	void resetIds(int callbackId);

private:
	void loadTexture(const Common::String &name);
	void loadSpriteSheet(const Common::String &name);
	void loadFont(const Common::String &name);

public:
	Common::HashMap<Common::String, Texture> _textures;
	Common::HashMap<Common::String, SpriteSheet> _spriteSheets;
	Common::HashMap<Common::String, Common::SharedPtr<Font> > _fonts;
	Common::SharedPtr<Object> _allObjects[100000];

private:
	int _roomId = START_ROOMID;
	int _actorId = START_ACTORID;
	int _objId = START_OBJECTID;
	int _soundDefId = START_SOUNDDEFID;
	int _soundId = START_SOUNDID;
	int _threadId = START_THREADID;
	int _callbackId = START_CALLBACKID;
	int _lightId = START_LIGHTID;
};
} // namespace Twp

#endif
