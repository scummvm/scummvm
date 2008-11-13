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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/str.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/surface.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/animation.h"
#include "picture/menu.h"
#include "picture/movie.h"
#include "picture/palette.h"
#include "picture/render.h"
#include "picture/resource.h"
#include "picture/script.h"
#include "picture/screen.h"
#include "picture/segmap.h"
#include "picture/sound.h"
#include "picture/microtiles.h"

namespace Picture {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

PictureEngine::PictureEngine(OSystem *syst, const PictureGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource();
	syst->getEventManager()->registerRandomSource(*_rnd, "picture");

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

}

PictureEngine::~PictureEngine() {
	delete _rnd;
}

Common::Error PictureEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(true);
	_system->initSize(640, 400);
	_system->endGFXTransaction();
	return Common::kNoError;
}

void PictureEngine::syncSoundSettings() {
	/*
	_music->setVolume(ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	*/
}

Common::Error PictureEngine::go() {

	_isSaveAllowed = true;

 	_quitGame = false;
	_counter01 = 0;
	_counter02 = 0;
	_movieSceneFlag = false;
	_flag01 = 0;
	
	_cameraX = 0;
	_cameraY = 0;
	_newCameraX = 0;
	_newCameraY = 0;
	_cameraHeight = 0;

	_guiHeight = 26;

	_sceneWidth = 0;
	_sceneHeight = 0;
	
	_doSpeech = true;
	_doText = true;

	_walkSpeedY = 5;
	_walkSpeedX = 1;

	_mouseX = 0;
	_mouseY = 0;
	_mouseCounter = 0;
	_mouseButtonPressedFlag = false;
	_mouseButton = 0;
	_mouseDisabled = 0;
	_leftButtonDown = false;
	_rightButtonDown = false;

	_arc = new ArchiveReader();
	_arc->openArchive("WESTERN");

	_res = new ResourceCache(this);

	_screen = new Screen(this);

	_script = new ScriptInterpreter(this);
	_anim = new AnimationPlayer(this);
	_palette = new Palette(this);
	_segmap = new SegmentMap(this);
	_moviePlayer = new MoviePlayer(this);
	_menuSystem = new MenuSystem(this);
	
	_sound = new Sound(this);

	_system->showMouse(true);

//#define TEST_MENU
#ifdef TEST_MENU
	_screen->registerFont(0, 0x0D);
	_screen->registerFont(1, 0x0E);
	_screen->loadMouseCursor(12);
	_palette->loadAddPalette(9, 224);
	_palette->setDeltaPalette(_palette->getMainPalette(), 7, 0, 31, 224);
	_screen->finishTalkTextItems();
	_screen->clearSprites();
	while (1) {
		updateInput();
		_menuSystem->update();
		updateScreen();
	}
#endif

#if 1
	_script->loadScript(0, 0);
	_script->runScript(0);
#endif

	delete _arc;
	delete _res;
	delete _screen;
	delete _script;
	delete _anim;
	delete _palette;
	delete _segmap;
	delete _moviePlayer;
	delete _menuSystem;
	
	delete _sound;

	return Common::kNoError;
}

void PictureEngine::loadScene(uint resIndex) {
	// TODO

	byte *scene = _res->load(resIndex);

	uint32 imageSize = READ_LE_UINT32(scene);
	_sceneResIndex = resIndex;
	_sceneHeight = READ_LE_UINT16(scene + 4);
	_sceneWidth = READ_LE_UINT16(scene + 6);

	// Load scene palette
	_palette->loadAddPaletteFrom(scene + 8, 0, 128);

	// Load scene background
	byte *source = scene + 392;
	byte *destp = _screen->_backScreen;
	byte *destEnd = destp + _sceneWidth * _sceneHeight;
 	while (destp < destEnd) {
		int count = 1;
		byte pixel = *source++;
		if (pixel & 0x80) {
			pixel &= 0x7F;
			count = *source++;
			count += 2;
		}
		memset(destp, pixel, count);
		destp += count;
	}

	debug(0, "_sceneWidth = %d; _sceneHeight = %d", _sceneWidth, _sceneHeight);

	// Load scene segmap
 	_segmap->load(scene + imageSize + 4);

	_screen->_fullRefresh = true;
	_screen->_renderQueue->clear();

}

void PictureEngine::updateScreen() {

	// FIXME: Quick hack, sometimes cameraY was negative (the code in updateCamera was at fault)
	if (_cameraY < 0) _cameraY = 0;

	_segmap->addMasksToRenderQueue();
	_screen->addTalkTextItemsToRenderQueue();

	_screen->_renderQueue->update();

	//printf("_guiHeight = %d\n", _guiHeight); fflush(stdout);

	if (_screen->_guiRefresh && _guiHeight > 0 && _cameraHeight > 0) {
		_system->copyRectToScreen((const byte *)_screen->_frontScreen + _cameraHeight * 640,
			640, 0, _cameraHeight, 640, _guiHeight);
		_screen->_guiRefresh = false;
	}

	_system->updateScreen();

	updateCamera();
}

void PictureEngine::updateInput() {

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:

			// FIXME: This is just for debugging
			switch (event.kbd.keycode) {
			case Common::KEYCODE_F7:
				savegame("toltecs.001", "Quicksave");
				break;
			case Common::KEYCODE_F9:
				loadgame("toltecs.001");
				break;
			default:
				break;
			}

			break;
		case Common::EVENT_QUIT:
			// FIXME: Find a nicer way to quit
			_system->quit();
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = false;
			break;
		default:
			break;
		}
	}

	if (_mouseDisabled == 0) {

		if (_mouseCounter > 0)
			_mouseCounter--;

		byte mouseButtons = 0;
		if (_leftButtonDown)
			mouseButtons |= 1;
		if (_rightButtonDown)
			mouseButtons |= 2;

		if (mouseButtons != 0) {
			if (!_mouseButtonPressedFlag) {
				_mouseButton = mouseButtons;
				if (_mouseCounter != 0)
					_mouseButton |= 0x80;
				_mouseCounter = 30; // maybe TODO
				_mouseButtonPressedFlag = true;
			} else {
				_mouseButton = 0;
			}
		} else {
			_mouseButtonPressedFlag = false;
			_mouseButton = 0;
		}

	}

}

void PictureEngine::setGuiHeight(int16 guiHeight) {
	if (guiHeight != _guiHeight) {
		_guiHeight = guiHeight;
		_cameraHeight = 400 - _guiHeight;
		debug(0, "PictureEngine::setGuiHeight() _guiHeight = %d; _cameraHeight = %d", _guiHeight, _cameraHeight);
		// TODO: clearScreen();
	}
}

void PictureEngine::setCamera(int16 x, int16 y) {

	_screen->finishTalkTextItems();

	_screen->clearSprites();
	
	_cameraX = x;
	_newCameraX = x;

	_cameraY = y;
	_newCameraY = y;

}

void PictureEngine::scrollCameraUp(int16 delta) {
	if (_newCameraY > 0) {
		if (_newCameraY < delta)
			_newCameraY = 0;
		else
			_newCameraY -= delta;
	}
}

void PictureEngine::scrollCameraDown(int16 delta) {
	debug(0, "PictureEngine::scrollCameraDown(%d)", delta);
	if (_newCameraY != _sceneHeight - _cameraHeight) {
		if (_sceneHeight - _cameraHeight < _newCameraY + delta)
			delta += (_sceneHeight - _cameraHeight) - (delta + _newCameraY);
		_newCameraY += delta;
		debug(0, "PictureEngine::scrollCameraDown() _newCameraY = %d; delta = %d", _newCameraY, delta);
	}
}

void PictureEngine::scrollCameraLeft(int16 delta) {
	if (_newCameraX > 0) {
		if (_newCameraX < delta)
			_newCameraX = 0;
		else
			_newCameraX -= delta;
	}
}

void PictureEngine::scrollCameraRight(int16 delta) {
	debug(0, "PictureEngine::scrollCameraRight(%d)", delta);
	if (_newCameraX != _sceneWidth - 640) {
		if (_sceneWidth - 640 < delta + _newCameraX)
			delta += (_sceneWidth - 640) - (delta + _newCameraX);
		_newCameraX += delta;
		debug(0, "PictureEngine::scrollCameraRight() _newCameraX = %d; delta = %d", _newCameraY, delta);
	}
}

void PictureEngine::updateCamera() {

	if (_cameraX != _newCameraX) {
		//dirtyFullRefresh = -1;
		_cameraX = _newCameraX;
		_screen->_fullRefresh = true;
		_screen->finishTalkTextItems();
	}

	if (_cameraY != _newCameraY) {
		//dirtyFullRefresh = -1;
		_cameraY = _newCameraY;
		_screen->_fullRefresh = true;
		_screen->finishTalkTextItems();
	}

	debug(0, "PictureEngine::updateCamera() _cameraX = %d; _cameraY = %d", _cameraX, _cameraY);

}

void PictureEngine::talk(int16 slotIndex, int16 slotOffset) {
	
	byte *scanData = _script->getSlotData(slotIndex) + slotOffset;
	
	while (*scanData < 0xF0) {
	
		if (*scanData == 0x19) {
			scanData++;
		} else if (*scanData == 0x14) {
			scanData++;
		} else if (*scanData == 0x0A) {
			scanData += 4;
		} else if (*scanData < 0x0A) {
			scanData++;
		}
	
		scanData++;
	}
	
	if (*scanData == 0xFE) {
		if (_doSpeech) {
			int16 resIndex = READ_LE_UINT16(scanData + 1);
			debug(0, "PictureEngine::talk() playSound(resIndex: %d)", resIndex);
			_sound->playSpeech(resIndex);
		}
		if (_doText) {
			_screen->updateTalkText(slotIndex, slotOffset);
		} else {
			// TODO: font_sub_4B3E2
		}
	} else {
		_screen->updateTalkText(slotIndex, slotOffset);
	}

}

void PictureEngine::playText(int16 slotIndex, int16 slotOffset) {

	byte *textData = _script->getSlotData(slotIndex) + slotOffset;
	
	debug(0, "PictureEngine::playText() [textData = %s]", (char*)textData);

	Common::String str;
	while (*textData < 0xF0) {
		if (*textData >= 32)
			str += (char)*textData;
		textData++;
	}
	
	debug(0, "PictureEngine::playText() [%s]", str.c_str());

}

void PictureEngine::walk(byte *walkData) {

	int16 xdelta, ydelta, v8, v10, v11;
	int16 xstep, ystep;
	ScriptWalk walkInfo;

	walkInfo.y = READ_LE_UINT16(walkData + 0);
	walkInfo.x = READ_LE_UINT16(walkData + 2);
	walkInfo.y1 = READ_LE_UINT16(walkData + 4);
	walkInfo.x1 = READ_LE_UINT16(walkData + 6);
	walkInfo.y2 = READ_LE_UINT16(walkData + 8);
	walkInfo.x2 = READ_LE_UINT16(walkData + 10);
	walkInfo.yerror = READ_LE_UINT16(walkData + 12);
	walkInfo.xerror = READ_LE_UINT16(walkData + 14);
	walkInfo.mulValue = READ_LE_UINT16(walkData + 16);
	walkInfo.scaling = READ_LE_UINT16(walkData + 18);
	
	walkInfo.scaling = -_segmap->getScalingAtPoint(walkInfo.x, walkInfo.y);

	if (walkInfo.y1 < walkInfo.y2)
		ystep = -1;
	else
		ystep = 1;
	ydelta = ABS(walkInfo.y1 - walkInfo.y2) * _walkSpeedY;
	
	if (walkInfo.x1 < walkInfo.x2)
		xstep = -1;
	else
		xstep = 1;
	xdelta = ABS(walkInfo.x1 - walkInfo.x2) * _walkSpeedX;

	debug(0, "PictureEngine::walk() xdelta = %d; ydelta = %d", xdelta, ydelta);

	if (xdelta > ydelta)
		SWAP(xdelta, ydelta);

	v8 = 100 * xdelta;
	if (v8 != 0) {
		if (walkInfo.scaling > 0)
			v8 -= v8 * ABS(walkInfo.scaling) / 100;
		else
			v8 += v8 * ABS(walkInfo.scaling) / 100;
		if (ydelta != 0)
			v8 /= ydelta;
	}

	if (ydelta > ABS(walkInfo.x1 - walkInfo.x2) * _walkSpeedX) {
		v10 = 100 - walkInfo.scaling;
		v11 = v8;
  	} else {
		v10 = v8;
  		v11 = 100 - walkInfo.scaling;
	}

	walkInfo.yerror += walkInfo.mulValue * v10;
	while (walkInfo.yerror >= 100 * _walkSpeedY) {
		walkInfo.yerror -= 100 * _walkSpeedY;
		if (walkInfo.y == walkInfo.y1) {
			walkInfo.x = walkInfo.x1;
			break;
		}
		walkInfo.y += ystep;
	}

	walkInfo.xerror += walkInfo.mulValue * v11;
	while (walkInfo.xerror >= 100 * _walkSpeedX) {
		walkInfo.xerror -= 100 * _walkSpeedX;
		if (walkInfo.x == walkInfo.x1) {
			walkInfo.y = walkInfo.y1;
			break;
		}
		walkInfo.x += xstep;
	}

	WRITE_LE_UINT16(walkData + 0, walkInfo.y);
	WRITE_LE_UINT16(walkData + 2, walkInfo.x);
	WRITE_LE_UINT16(walkData + 4, walkInfo.y1);
	WRITE_LE_UINT16(walkData + 6, walkInfo.x1);
	WRITE_LE_UINT16(walkData + 8, walkInfo.y2);
	WRITE_LE_UINT16(walkData + 10, walkInfo.x2);
	WRITE_LE_UINT16(walkData + 12, walkInfo.yerror);
	WRITE_LE_UINT16(walkData + 14, walkInfo.xerror);
	WRITE_LE_UINT16(walkData + 16, walkInfo.mulValue);
	WRITE_LE_UINT16(walkData + 18, walkInfo.scaling);

}

int16 PictureEngine::findRectAtPoint(byte *rectData, int16 x, int16 y, int16 index, int16 itemSize) {

	rectData += index * itemSize;
	
	while (1) {
		int16 rectY = READ_LE_UINT16(rectData);
		if (rectY == -10)
			break;
		int16 rectX = READ_LE_UINT16(rectData + 2);
		int16 rectH = READ_LE_UINT16(rectData + 4);
		int16 rectW = READ_LE_UINT16(rectData + 6);

		debug(0, "x = %d; y = %d; x1 = %d; y2 = %d; w = %d; h = %d",
			x, y, rectX, rectY, rectW, rectH);

		if (x >= rectX && x <= rectX + rectW && y >= rectY && y <= rectY + rectH) {
			return index;
		}
		index++;
		rectData += itemSize;
	}
	
	return -1;

}

} // End of namespace Picture
