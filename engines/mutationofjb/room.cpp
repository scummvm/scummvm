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

#include "mutationofjb/room.h"
#include "mutationofjb/animationdecoder.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/util.h"
#include "common/str.h"
#include "common/translation.h"
#include "graphics/screen.h"

namespace MutationOfJB {

class RoomAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	RoomAnimationDecoderCallback(Room &room) : _room(room) {}
	virtual void onFrame(int frameNo, Graphics::Surface &surface) override;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	Room &_room;
};

void RoomAnimationDecoderCallback::onPaletteUpdated(byte palette[PALETTE_SIZE]) {
	_room._screen->setPalette(palette, 0x00, 0xC0); // Load only 0xC0 colors.
}

void RoomAnimationDecoderCallback::onFrame(int frameNo, Graphics::Surface &surface) {
	if (frameNo == 0) {
		Common::Rect rect(0, 0, 320, 139);
		if (_room._game->isCurrentSceneMap()) {
			rect = Common::Rect(0, 0, 320, 200);
		}
		_room._screen->blitFrom(surface, rect, Common::Point(0, 0));
	}

	const int frameNo1 = frameNo + 1;

	Scene *scene = _room._game->getGameData().getCurrentScene();
	if (scene) {
		const uint8 noObjects = scene->getNoObjects();
		for (int i = 0; i < noObjects; ++i) {
			Object &object = scene->_objects[i];
			const uint16 startFrame = (object._WY << 8) + object._FS;
			if (frameNo1 >= startFrame && frameNo1 < startFrame + object._NA) {
				const int x = object._x;
				const int y = object._y;
				const int w = (object._XL + 3) / 4 * 4; // Original code uses this to round up width to a multiple of 4.
				const int h = object._YL;
				Common::Rect rect(x, y, x + w, y + h);

				const Graphics::Surface sharedSurface = surface.getSubArea(rect);
				Graphics::Surface outSurface;
				outSurface.copyFrom(sharedSurface);
				_room._surfaces[_room._objectsStart[i] + frameNo1 - startFrame] = outSurface;
			}
		}
	}
}

Room::Room(Game *game, Graphics::Screen *screen) : _game(game), _screen(screen) {}

bool Room::load(uint8 roomNumber, bool roomB) {
	_objectsStart.clear();

	Scene *const scene = _game->getGameData().getCurrentScene();
	if (scene) {
		const uint8 noObjects = scene->getNoObjects();
		for (int i = 0; i < noObjects; ++i) {
			uint8 firstIndex = 0;
			if (i != 0) {
				firstIndex = _objectsStart[i - 1] + scene->_objects[i - 1]._NA;
			}
			_objectsStart.push_back(firstIndex);

			uint8 numAnims = scene->_objects[i]._NA;
			while (numAnims--) {
				_surfaces.push_back(Graphics::Surface());
			}
		}
	}

	const Common::String fileName = Common::String::format("room%d%s.dat", roomNumber, roomB ? "b" : "");
	AnimationDecoder decoder(fileName);
	RoomAnimationDecoderCallback callback(*this);
	return decoder.decode(&callback);
}

void Room::drawObjectAnimation(uint8 objectId, int animOffset) {
	Scene *const scene = _game->getGameData().getCurrentScene();
	if (!scene) {
		return;
	}
	Object *const object = scene->getObject(objectId);
	if (!object) {
		return;
	}

	const int startFrame = _objectsStart[objectId - 1];
	const int animFrame = startFrame + animOffset;
	_screen->blitFrom(_surfaces[animFrame], Common::Point(object->_x, object->_y));
}

}
