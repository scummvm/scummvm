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

#include "common/system.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "chewy/cursor.h"
#include "chewy/graphics.h"
#include "chewy/scene.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "chewy/text.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

#define MAX_DETAILS 32
#define MAX_HOTSPOTS 50
#define MAX_AUTOMOVE 20
#define MAX_SOUNDS 3

// Animated details - scene animations
struct AnimatedDetails {
	int16 x;
	int16 y;
	byte startFlag;     // 0: no animation
	byte repeat;
	int16 startSprite;
	int16 endSprite;
	int16 spriteCount;
	uint16 delay;
	uint16 delayCount;
	uint16 reverse;     // 0: play normally, 1: play in reverse
	uint16 timerStart;  // seconds until detail is started (0: no timer)
	uint16 zIndex;
	byte loadFlag;      // 0: load animation in memory immediately, 1: load animation in memory when it is played
	byte zoom;
	// 2 * 3 * 7 = 42 bytes sound data - TODO
	byte showOneFrame;  // show a sprite, 0: none, 1: before animation, 2: after animation
	byte currentFrame;
};

// Static details - scene sprites and props
struct StaticDetails {
	int16 x;
	int16 y;
	int16 spriteNum;
	uint16 zIndex;
	byte hide;
	// 1 byte dummy
};

struct Hotspot {
	Common::Rect rect;
	uint16 resource;
	Common::String desc;
};

struct RoomInfo {
	byte roomNum;
	byte picNum;
	byte autoMoveCount;
	byte loadTaf;
	Common::String tafName;	// 14 bytes
	byte zoomFactor;
	// 1 byte dummy
};

struct AutoMove {
	int16 x;
	int16 y;
	byte spriteNum;	// sprite number to draw when the end point is reached
	// 1 byte dummy
};

struct HotspotSpeech {
	int16 look;
	int16 use;
	int16 talk;
};

struct SceneInfo {
	uint16 staticDetailsCount;
	uint16 animatedDetailsCount;
	uint32 spritePtr;
	AnimatedDetails animatedDetails[MAX_DETAILS];
	StaticDetails staticDetails[MAX_DETAILS];
	Hotspot hotspot[MAX_HOTSPOTS];
	RoomInfo roomInfo;
	AutoMove autoMove[MAX_AUTOMOVE];
	HotspotSpeech hotspotSpeech[MAX_DETAILS];
	byte hotspotSound[MAX_DETAILS][MAX_SOUNDS];
};

Scene::Scene(ChewyEngine *vm) : _vm(vm) {
	_sceneInfo = new SceneInfo();
	_vm->_graphics->setDescSurface(Common::Point(-1, -1));
}

Scene::~Scene() {
	delete _sceneInfo;
}

void Scene::change(uint scene) {
	_curScene = scene;
	_vm->_cursor->setCursor(0);
	_vm->_cursor->showCursor();
	
	loadSceneInfo();
	draw();
}

void Scene::draw() {
	// Background
	_vm->_graphics->drawImage("episode1.tgp", _curScene);

	for (uint16 i = 0; i < MAX_DETAILS; i++) {
		// Static details
		StaticDetails s = _sceneInfo->staticDetails[i];
		if (s.spriteNum >= 0 && s.x >= 0 && s.y >= 0 && !s.hide)
			_vm->_graphics->drawSprite(Common::String::format("det%d.taf", _curScene), s.spriteNum, s.x, s.y);
	}

	// TODO: These are all hardcoded for now
	_vm->_graphics->drawSprite("det1.taf", 0, 200, 100);
	_vm->_graphics->loadFont("6x8.tff");
	//_vm->_graphics->drawText("This is a test", 200, 80);

	_vm->_graphics->setDescSurface(Common::Point(-1, -1));
}

void Scene::updateMouse(Common::Point coords) {
	_vm->_graphics->restoreDescSurface();

	// Static details
	for (uint16 i = 0; i < MAX_HOTSPOTS; i++) {
		//_vm->_graphics->drawRect(_sceneInfo->hotspot[i].rect, 0);	// debug
		if (_sceneInfo->hotspot[i].rect.contains(coords) && _sceneInfo->hotspot[i].resource < kATSTextMax) {
			if (coords.y >= 8) {
				_vm->_graphics->setDescSurface(Common::Point(coords.x, coords.y - 8));
				_vm->_graphics->drawText(_sceneInfo->hotspot[i].desc, coords.x, coords.y - 8);
			}
			break;
		}
	}
}

void Scene::mouseClick(Common::Point coords) {
	// Static details
	for (uint16 i = 0; i < MAX_HOTSPOTS; i++) {
		//_vm->_graphics->drawRect(_sceneInfo->hotspot[i].rect, 0);	// debug
		if (_sceneInfo->hotspot[i].rect.contains(coords)) {
			int sample = -1;

			switch (_vm->_cursor->getCurrentCursor()) {
			case kLook:
				sample = _sceneInfo->hotspotSpeech[i].look;
				break;
			case kUse:
				sample = _sceneInfo->hotspotSpeech[i].use;
				break;
			case kTalk:
				sample = _sceneInfo->hotspotSpeech[i].talk;
				break;
			default:
				break;
			}

			if (sample >= 0)
				_vm->_sound->playSpeech(sample);
		}
	}
}

void Scene::loadSceneInfo() {
	const uint32 sceneInfoSize = 3784;
	const uint32 headerRDI = MKTAG('R', 'D', 'I', '\0');
	const char *sceneIndexFileName = "test.rdi";
	Common::File indexFile;
	if (!Common::File::exists(sceneIndexFileName))
		error("File %s not found", sceneIndexFileName);
	Text *text = new Text();

	indexFile.open(sceneIndexFileName);

	uint32 header = indexFile.readUint32BE();
	if (header != headerRDI)
		error("Invalid resource - %s", sceneIndexFileName);

	indexFile.seek(sceneInfoSize * _curScene, SEEK_CUR);

	// TODO: These can be set to larger numbers than MAX_DETAILS
	_sceneInfo->staticDetailsCount = indexFile.readUint16LE();
	_sceneInfo->animatedDetailsCount = indexFile.readUint16LE();
	indexFile.skip(6);

	// Animated details
	for (int i = 0; i < MAX_DETAILS; i++) {
		_sceneInfo->animatedDetails[i].x = indexFile.readSint16LE();
		_sceneInfo->animatedDetails[i].y = indexFile.readSint16LE();
		_sceneInfo->animatedDetails[i].startFlag = indexFile.readByte();
		_sceneInfo->animatedDetails[i].repeat = indexFile.readByte();
		_sceneInfo->animatedDetails[i].startSprite = indexFile.readSint16LE();
		_sceneInfo->animatedDetails[i].endSprite = indexFile.readSint16LE();
		_sceneInfo->animatedDetails[i].spriteCount = indexFile.readSint16LE();
		_sceneInfo->animatedDetails[i].delay = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].delayCount = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].reverse = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].timerStart = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].zIndex = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].loadFlag = indexFile.readByte();
		_sceneInfo->animatedDetails[i].zoom = indexFile.readByte();
		indexFile.skip(42);	// 2 * 3 * 7 = 42 bytes sound data - TODO
		_sceneInfo->animatedDetails[i].showOneFrame = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].currentFrame = indexFile.readUint16LE();
	}

	// Static details
	for (int i = 0; i < MAX_DETAILS; i++) {
		_sceneInfo->staticDetails[i].x = indexFile.readSint16LE();
		_sceneInfo->staticDetails[i].y = indexFile.readSint16LE();
		_sceneInfo->staticDetails[i].spriteNum = indexFile.readSint16LE();
		_sceneInfo->staticDetails[i].zIndex = indexFile.readUint16LE();
		_sceneInfo->staticDetails[i].hide = indexFile.readByte();
		indexFile.readByte();	// padding
	}

	// Hotspots
	for (int i = 0; i < MAX_HOTSPOTS; i++) {
		_sceneInfo->hotspot[i].rect.left = indexFile.readUint16LE();
		_sceneInfo->hotspot[i].rect.top = indexFile.readUint16LE();
		_sceneInfo->hotspot[i].rect.right = indexFile.readUint16LE();
		_sceneInfo->hotspot[i].rect.bottom = indexFile.readUint16LE();
		if (!_sceneInfo->hotspot[i].rect.isValidRect())
			warning("Hotspot %d has an invalid rect", i);
	}

	// Hotspot descriptions
	for (int i = 0; i < MAX_HOTSPOTS; i++) {
		_sceneInfo->hotspot[i].resource = indexFile.readUint16LE() + 4;
		_sceneInfo->hotspot[i].desc = "";

		if (_sceneInfo->hotspot[i].resource < kATSTextMax) {
			TextEntry *entry = text->getText(_curScene + kADSTextMax, _sceneInfo->hotspot[i].resource);
			if (entry)
				_sceneInfo->hotspot[i].desc = entry->text;
		}
	}

	// Room info
	_sceneInfo->roomInfo.roomNum = indexFile.readByte();
	_sceneInfo->roomInfo.picNum = indexFile.readByte();
	_sceneInfo->roomInfo.autoMoveCount = indexFile.readByte();
	_sceneInfo->roomInfo.loadTaf = indexFile.readByte();
	
	_sceneInfo->roomInfo.tafName = "";
	for (int i = 0; i < 14; i++)
		_sceneInfo->roomInfo.tafName += indexFile.readByte();

	_sceneInfo->roomInfo.zoomFactor = indexFile.readByte();
	indexFile.readByte();	// padding
	
	for (int i = 0; i < MAX_AUTOMOVE; i++) {
		_sceneInfo->autoMove[i].x = indexFile.readSint16LE();
		_sceneInfo->autoMove[i].y = indexFile.readSint16LE();
		_sceneInfo->autoMove[i].spriteNum = indexFile.readByte();
		indexFile.readByte();	// padding
		if (i > _sceneInfo->roomInfo.autoMoveCount && !(_sceneInfo->autoMove[i].x <= 0 || _sceneInfo->autoMove[i].y <= 0))
			warning("Auto move %d should be unused, but it isn't (max auto move items are %d)", i, _sceneInfo->roomInfo.autoMoveCount);
	}

	for (int i = 0; i < MAX_DETAILS; i++) {
		// FIXME: These are all wrong... investigate why
		_sceneInfo->hotspotSpeech[i].look = indexFile.readSint16LE();
		_sceneInfo->hotspotSpeech[i].use = indexFile.readSint16LE();
		_sceneInfo->hotspotSpeech[i].talk = indexFile.readSint16LE();
	}

	for (int i = 0; i < MAX_DETAILS; i++) {
		_sceneInfo->hotspotSound[i][0] = indexFile.readSint16LE();
		_sceneInfo->hotspotSound[i][1] = indexFile.readSint16LE();
		_sceneInfo->hotspotSound[i][2] = indexFile.readSint16LE();
	}

	// TODO: We seem to be missing a chunk of data (186 bytes) from the end of
	// the room info structure

	delete text;
	indexFile.close();
}

} // End of namespace Chewy
