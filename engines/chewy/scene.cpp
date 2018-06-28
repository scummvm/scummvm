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

struct SoundInfo {
	uint16 enable[MAX_SOUNDS];	// flag, 0 = disable, 1 = enable
	int16 index[MAX_SOUNDS];
	uint16 start[MAX_SOUNDS];
	uint16 channel[MAX_SOUNDS];
	uint16 volume[MAX_SOUNDS];
	uint16 repeatCount[MAX_SOUNDS];
	uint16 stereo[MAX_SOUNDS];	// stereo position for the SFX
};

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
	SoundInfo soundInfo;
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
	int16 speechId;
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

/*struct HotspotSpeech {
	int16 look;
	int16 use;
	int16 talk;
};*/

struct SceneInfo {
	uint16 staticDetailsCount;
	uint16 animatedDetailsCount;
	uint32 spritePtr;
	AnimatedDetails animatedDetails[MAX_DETAILS];
	StaticDetails staticDetails[MAX_DETAILS];
	Hotspot hotspot[MAX_HOTSPOTS];
	RoomInfo roomInfo;
	AutoMove autoMove[MAX_AUTOMOVE];
	int16 hotspotSpeech[MAX_DETAILS * MAX_SOUNDS];
	//uint32 hotspotSoundPtr[MAX_DETAILS][MAX_SOUNDS];	// unused
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
				//_vm->_graphics->drawText(_sceneInfo->hotspot[i].desc + Common::String::format(" (%d)", i), coords.x, coords.y - 8);	// debug
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
			int16 sample = -1;

			// TODO: This is still not right
			int16 speechId = _sceneInfo->hotspotSpeech[_sceneInfo->hotspot[i].resource - 4];
			SoundInfo *s = &_sceneInfo->animatedDetails[speechId].soundInfo;

			switch (_vm->_cursor->getCurrentCursor()) {
			case kUse:
				sample = s->index[0];
				break;
			case kLook:
				sample = s->index[1];
				break;
			case kTalk:
				sample = s->index[2];
				break;
			default:
				break;
			}

			if (sample >= 0)
				_vm->_sound->playSpeech(sample);
		}
	}
}

static void readSSoundInfo(Common::File &indexFile, int16 *data) {
	for (int i = 0; i < MAX_SOUNDS; i++)
		data[i] = indexFile.readSint16LE();
}

static void readUSoundInfo(Common::File &indexFile, uint16 *data) {
	for (int i = 0; i < MAX_SOUNDS; i++)
		data[i] = indexFile.readUint16LE();
}

/**
 * Loads scene information from test.rdi
 * Note that the original loads everything with a single read into a structure,
 * which is why there are some pointers saved in the resource file - however,
 * these are set to zero
 */
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
	indexFile.skip(2);	// room count, unused (set to 100)

	indexFile.seek(sceneInfoSize * _curScene, SEEK_CUR);

	_sceneInfo->staticDetailsCount = indexFile.readUint16LE();
	_sceneInfo->animatedDetailsCount = indexFile.readUint16LE();
	indexFile.skip(4);	// pointer to sprites

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

		SoundInfo *s = &_sceneInfo->animatedDetails[i].soundInfo;
		readUSoundInfo(indexFile, s->enable);
		readSSoundInfo(indexFile, s->index);
		readUSoundInfo(indexFile, s->start);
		readUSoundInfo(indexFile, s->channel);
		readUSoundInfo(indexFile, s->volume);
		readUSoundInfo(indexFile, s->repeatCount);
		readUSoundInfo(indexFile, s->stereo);
		//debug("Sound %i: %i, %i, %i", i, s->index[0], s->index[1], s->index[2]);

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
		_sceneInfo->hotspot[i].speechId = -1;

		if (_sceneInfo->hotspot[i].resource < kATSTextMax) {
			TextEntry *entry = text->getText(_curScene + kADSTextMax, _sceneInfo->hotspot[i].resource);
			if (entry) {
				_sceneInfo->hotspot[i].desc = entry->text;
				//_sceneInfo->hotspot[i].speechId = entry->speechId;	// TODO
				//debug("Hotspot %i: '%s', resource %d", i, entry->text.c_str(), _sceneInfo->hotspot[i].resource);
			}
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

	for (int i = 0; i < MAX_DETAILS * MAX_SOUNDS; i++) {
		_sceneInfo->hotspotSpeech[i] = indexFile.readSint16LE();
		/*HotspotSpeech *hs = &_sceneInfo->hotspotSpeech[i];
		hs->look = indexFile.readSint16LE();
		hs->use = indexFile.readSint16LE();
		hs->talk = indexFile.readSint16LE();
		debug("Hotspot %d: (%d, %d, %d)", i, hs->look, hs->use, hs->talk);*/
	}

	// The rest of the scene data is MAX_DETAILS * 3 * 4 bytes full of null or
	// invalid pointers to sound buffers, where sounds for each hotspot are
	// preloaded
	indexFile.skip(MAX_DETAILS * 3 * 4);

	delete text;
	indexFile.close();
}

} // End of namespace Chewy
