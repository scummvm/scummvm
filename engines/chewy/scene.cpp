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
#include "chewy/text.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

#define MAX_DETAILS 32
#define MAX_HOTSPOTS 50

// Animated details - scene animations
struct AnimatedDetails {
	uint16 x;
	uint16 y;
	// 66 bytes animated details - TODO
};

// Static details - scene sprites and props
struct StaticDetails {
	int16 x;
	int16 y;
	int16 spriteNum;
	uint16 z;
	byte hide;
	// 1 byte dummy
};

struct Hotspot {
	Common::Rect rect;
	uint16 resource;
	Common::String desc;
};

struct SceneInfo {
	uint16 staticDetailsCount;
	uint16 animatedDetailsCount;
	uint32 spritePtr;
	AnimatedDetails animatedDetails[MAX_DETAILS];
	StaticDetails staticDetails[MAX_DETAILS];
	Hotspot hotspot[MAX_HOTSPOTS];
	byte roomNum;
	byte picNum;
	byte autoMoveCount;
	byte loadTaf;
	Common::String tafName;	// 14 bytes
	byte zoomFactor;
	// 1 byte dummy
	// 6 * 20 = 120 bytes automove coordinates - TODO
	// MAX_DETAILS * 3 * 2 = 192 bytes voc - TODO
	// MAX_DETAILS * 3 = 96 bytes samples - TODO
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

	// Static details
	for (uint16 i = 0; i < MAX_DETAILS; i++) {
		StaticDetails s = _sceneInfo->staticDetails[i];
		if (s.spriteNum >= 0 && s.x >= 0 && s.y >= 0 && !s.hide)
			_vm->_graphics->drawSprite(Common::String::format("det%d.taf", _curScene), s.spriteNum, s.x, s.y);
	}

	// TODO: These are all hardcoded for now
	_vm->_graphics->drawSprite("det1.taf", 0, 200, 100);
	_vm->_graphics->loadFont("6x8.tff");
	_vm->_graphics->drawText("This is a test", 200, 80);

	_vm->_graphics->setDescSurface(Common::Point(-1, -1));
}

void Scene::updateMouse(Common::Point coords) {
	_vm->_graphics->restoreDescSurface();

	// Animated details
	// TODO: handle these

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
		_sceneInfo->animatedDetails[i].x = indexFile.readUint16LE();
		_sceneInfo->animatedDetails[i].y = indexFile.readUint16LE();
		indexFile.skip(66);	// animated details info - TODO: read these
	}

	// Static details
	for (int i = 0; i < MAX_DETAILS; i++) {
		_sceneInfo->staticDetails[i].x = indexFile.readSint16LE();
		_sceneInfo->staticDetails[i].y = indexFile.readSint16LE();
		_sceneInfo->staticDetails[i].spriteNum = indexFile.readSint16LE();
		_sceneInfo->staticDetails[i].z = indexFile.readUint16LE();
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
		
		if (_sceneInfo->hotspot[i].resource < kATSTextMax) {
			TextEntry *entry = text->getText(_curScene + kADSTextMax, _sceneInfo->hotspot[i].resource);
			if (entry)
				_sceneInfo->hotspot[i].desc = entry->text;
			else
				_sceneInfo->hotspot[i].desc = Common::String::format("Hotspot %d", _sceneInfo->hotspot[i].resource);
		} else {
			// TODO: Handle these types of hotspot descriptions
			_sceneInfo->hotspot[i].desc = "";
			//warning("Hotspot %d has an invalid description resource (%d)", i, _sceneInfo->hotspot[i].resource);
			//_sceneInfo->hotspot[i].desc = Common::String::format("Hotspot %d", _sceneInfo->hotspot[i].resource);
		}
	}

	_sceneInfo->roomNum = indexFile.readByte();
	_sceneInfo->picNum = indexFile.readByte();
	_sceneInfo->autoMoveCount = indexFile.readByte();
	_sceneInfo->loadTaf = indexFile.readByte();
	
	for (int i = 0; i < 14; i++)
		_sceneInfo->tafName += indexFile.readByte();

	_sceneInfo->zoomFactor = indexFile.readByte();
	indexFile.readByte();	// padding
	
	// 6 * 20 = 120 bytes automove coordinates - TODO: read these
	// MAX_DETAILS * 3 * 2 = 192 bytes voc - TODO: read these
	// MAX_DETAILS * 3 = 96 bytes samples - TODO: read these

	delete text;
	indexFile.close();
}

} // End of namespace Chewy
