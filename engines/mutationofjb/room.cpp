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

#include "common/rect.h"
#include "common/str.h"

#include "graphics/screen.h"

namespace MutationOfJB {

enum {
	GAME_AREA_WIDTH = 320,
	GAME_AREA_HEIGHT = 139
};

class RoomAnimationDecoderCallback : public AnimationDecoderCallback {
public:
	RoomAnimationDecoderCallback(Room &room) : _room(room) {}
	void onFrame(int frameNo, Graphics::Surface &surface) override;
	void onPaletteUpdated(byte palette[PALETTE_SIZE]) override;
private:
	Room &_room;
};

void RoomAnimationDecoderCallback::onPaletteUpdated(byte palette[PALETTE_SIZE]) {
	_room._screen->setPalette(palette, 0x00, 0xC0); // Load only 0xC0 colors.
}

void RoomAnimationDecoderCallback::onFrame(int frameNo, Graphics::Surface &surface) {
	if (frameNo == 0) {
		Common::Rect rect(0, 0, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
		if (_room._game->isCurrentSceneMap()) {
			rect = Common::Rect(0, 0, 320, 200);
		} else {
			_room._background.blitFrom(surface, rect, Common::Point(0, 0));
		}
		_room._screen->blitFrom(surface, rect, Common::Point(0, 0));
	}

	const int frameNo1 = frameNo + 1;

	Scene *scene = _room._game->getGameData().getCurrentScene();
	if (scene) {
		const uint8 noObjects = scene->getNoObjects();
		for (int i = 0; i < noObjects; ++i) {
			Object &object = scene->_objects[i];
			const uint16 startFrame = (object._roomFrameMSB << 8) + object._roomFrameLSB;
			if (frameNo1 >= startFrame && frameNo1 < startFrame + object._numFrames) {
				const int x = object._x;
				const int y = object._y;
				const int w = (object._width + 3) / 4 * 4; // Original code uses this to round up width to a multiple of 4.
				const int h = object._height;
				Common::Rect rect(x, y, x + w, y + h);

				const Graphics::Surface sharedSurface = surface.getSubArea(rect);
				Graphics::Surface outSurface;
				outSurface.copyFrom(sharedSurface);
				_room._surfaces[_room._objectsStart[i] + frameNo1 - startFrame] = outSurface;
			}
		}
	}
}

Room::Room(Game *game, Graphics::Screen *screen) : _game(game), _screen(screen), _background(GAME_AREA_WIDTH, GAME_AREA_HEIGHT) {}

bool Room::load(uint8 roomNumber, bool roomB) {
	_objectsStart.clear();
	_surfaces.clear(); // TODO: Fix memory leak.

	Scene *const scene = _game->getGameData().getCurrentScene();
	if (scene) {
		const uint8 noObjects = scene->getNoObjects();
		for (int i = 0; i < noObjects; ++i) {
			uint8 firstIndex = 0;
			if (i != 0) {
				firstIndex = _objectsStart[i - 1] + scene->_objects[i - 1]._numFrames;
			}
			_objectsStart.push_back(firstIndex);

			uint8 numAnims = scene->_objects[i]._numFrames;
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

// TODO: Take the threshold value from ATN data.
struct ThresholdBlitOperation {
	byte operator()(const byte srcColor, const byte destColor) {
		if (destColor <= 0xBF) {
			// Within threshold - replace destination with source color.
			return srcColor;
		}

		// Outside of threshold - keep destination color.
		return destColor;
	}
};

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

	blit_if(_surfaces[animFrame], *_screen, Common::Point(object->_x, object->_y), ThresholdBlitOperation());
	if (!_game->isCurrentSceneMap())
		blit_if(_surfaces[animFrame], _background, Common::Point(object->_x, object->_y), ThresholdBlitOperation());
}

void Room::drawObject(uint8 objectId, uint8 overrideFrame) {
	Scene *const currentScene = _game->getGameData().getCurrentScene();
	Object *const object = currentScene->getObject(objectId);

	drawObjectAnimation(objectId, (overrideFrame ? overrideFrame : object->_currentFrame) - _objectsStart[objectId - 1] - 1);
}

void Room::drawBitmap(uint8 bitmapId) {
	GameData &gameData = _game->getGameData();

	Scene *const scene = gameData.getCurrentScene();
	if (!scene) {
		return;
	}
	Bitmap *const bitmap = scene->getBitmap(bitmapId);
	if (!bitmap) {
		return;
	}

	Common::Rect bitmapArea(bitmap->_x1, bitmap->_y1, bitmap->_x2 + 1, bitmap->_y2 + 1);
	drawFrames(bitmap->_roomFrame - 1, bitmap->_roomFrame - 1, bitmapArea, 0xC0);
}

void Room::drawStatic(Static *const stat) {
	if (!stat || !stat->allowsImplicitPickup()) {
		return;
	}

	const uint8 frame = stat->_active ? 1 : 2; // Hardcoded values. Active is taken from frame 1 and inactive from frame 2.
	const Common::Rect staticArea(stat->_x, stat->_y, stat->_x + stat->_width, stat->_y + stat->_height);
	drawFrames(frame, frame, staticArea, 0xC0); // Hardcoded threshold.
}

void Room::drawFrames(int fromFrame, int toFrame, const Common::Rect &area, uint8 threshold) {
	GameData &gameData = _game->getGameData();

	Scene *const scene = gameData.getCurrentScene();
	if (!scene) {
		return;
	}

	const Common::String fileName = Common::String::format("room%d%s.dat", gameData._currentScene, gameData._partB ? "b" : "");

	{
		AnimationDecoder decoder(fileName, *_screen);
		decoder.setPartialMode(fromFrame, toFrame, area, threshold);
		decoder.decode(nullptr);
		if (!area.isEmpty())
			_screen->getSubArea(area); // Add dirty rect.
		else
			_screen->makeAllDirty();
	}

	if (!_game->isCurrentSceneMap()) {
		AnimationDecoder decoder(fileName, _background);
		decoder.setPartialMode(fromFrame, toFrame, area, threshold);
		decoder.decode(nullptr);
	}
}

void Room::initialDraw() {
	Scene *const currentScene = _game->getGameData().getCurrentScene();

	for (uint8 i = 0; i < currentScene->getNoStatics(); ++i) {
		Static *const stat = currentScene->getStatic(i + 1);
		if (stat->_active && stat->allowsImplicitPickup()) {
			drawStatic(stat);
		}
	}

	for (uint8 i = 0; i < currentScene->getNoObjects(); ++i) {
		Object *const obj = currentScene->getObject(i + 1);
		if (obj->_active) {
			drawObjectAnimation(i + 1, obj->_currentFrame - _objectsStart[i] - 1);
		}
	}

	for (uint8 i = 0; i < currentScene->getNoBitmaps(); ++i) {
		Bitmap *const bitmap = currentScene->getBitmap(i + 1);
		if (bitmap->_isVisible && bitmap->_roomFrame > 0) {
			drawBitmap(i + 1);
		}
	}
}

void Room::redraw(bool useBackgroundBuffer) {
	if (useBackgroundBuffer && !_game->isCurrentSceneMap()) {
		Common::Rect rect(0, 0, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
		_screen->blitFrom(_background.rawSurface(), rect, Common::Point(0, 0));
	} else {
		initialDraw();
	}
}

}
