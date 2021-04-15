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

#include "snatcher/graphics.h"
#include "snatcher/resource.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"

#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

namespace Snatcher {

SnatcherEngine::SnatcherEngine(OSystem *system, GameDescription &dsc) : Engine(system), _game(dsc), _fio(0), _scene(0), _gfx(0), _snd(0) {

}

SnatcherEngine::~SnatcherEngine() {
	delete _scene;
	delete _gfx;
	delete _snd;
	delete _fio;
}

Common::Error SnatcherEngine::run() {
	registerDefaultSettings();

	if (!initResource())
		return Common::Error(Common::kReadingFailed);

	if (!initSound(_game.platform, _game.soundOptions))
		return Common::Error(Common::kAudioDeviceInitFailed);

	if (!initGfx(_game.platform))
		return Common::Error(Common::kUnknownError);

	return Common::Error(start() ? Common::kNoError : Common::kUnknownError);
}

bool SnatcherEngine::initResource() {
	return (_fio = new FIO(this, _game.isBigEndian));
}

bool SnatcherEngine::initGfx(Common::Platform platform) {
	if (_gfx = new GraphicsEngine(_system, platform)) {
		initGraphics(_gfx->screenWidth(), _gfx->screenHeight());
		return true;
	}
	return false;
}

bool SnatcherEngine::initSound(Common::Platform platform, int soundOptions) {
	return (_snd = new SoundEngine(platform, soundOptions));
}

struct Struct80 {
	uint16 cmd;
	uint8 f2;
	uint8 f3;
	uint8 f4;
	uint8 f5;
	uint32 f6;
	uint32 fa;

	uint16 fe;
	uint8 f10;
	uint8 f11;
	uint16 f12;
	uint8 f14;
	uint8 f15;
	uint8 f16;
	uint8 f1c;

	uint8 f25;
	uint8 f26;
	uint8 f27;
	uint16 f28;
	uint16 f2a;
	uint8 f2c;
	uint8 f2d;

	uint32 offs_2e;
	const uint8 *ptr_32;
	const uint8 *ptr_36;

	const uint8 *ptr_40;
	const uint8 *ptr_44;
	uint16 prev;
	uint16 next;
	uint16 next2;
	uint8 f4e;
	uint8 f4f;
};

Struct80 _s80[64];
//memset(_s80, 0, 64 * sizeof(Struct80));
uint8 _vc1_var_788A_flags;
uint8 _vc1_var_78BC;
uint8 _vc1_var_78BD;
uint16 _vc1_var_78BE;
const uint8 *_cpos_7880;
bool _var_7886;
uint8 _var_7893;

uint16 _word_B6406;

bool SnatcherEngine::start() {
	_scene = _fio->createSceneResource(2);

	const uint8 *scstart = _scene->getData(0x28000);
	const uint8 *sc = _scene->getDataFromMainTable(0);
	//
	memset(_s80, 0, 64 * sizeof(Struct80));
	//
	for (uint8 cmd = *sc++; cmd != 0xFF; cmd = *sc++) {
		uint8 len = *sc++;
		const uint8 *next = sc + len;

		switch (cmd) {
		case 0:
			_gfx->enqueuePaletteEvent(scstart, sc - scstart);
			break;

		case 1:
			_vc1_var_788A_flags &= ~1;
			_vc1_var_78BD = _vc1_var_78BC;
			_vc1_var_78BE = 0;
			_cpos_7880 = sc;
			_var_7886 = true;
			break;

		case 2: {
			while (sc < next) {
				assert(*sc < ARRAYSIZE(_s80));
				Struct80 *s = &_s80[*sc++];
				if (s->cmd)
					return 0;

				s->cmd = 1;
				s->f25 = *sc++;
				s->f6 = READ_BE_UINT16(sc);
				sc += 2;
				s->fa = READ_BE_UINT16(sc);
				sc += 2;
				s->offs_2e = READ_BE_UINT32(sc);
				sc += 4;
				s->ptr_32 = sc + READ_BE_UINT16(sc) + 2;
				sc += 2;
				s->f28 = 0xFFFF;

				uint8 v = *s->ptr_32;
				for (bool l = true; l; ) {
					if (v == 0xA1 || v == 0xA7) {
						s->f16 = s->ptr_32[1];
						v = s->ptr_32[4];
					} else {
						if (!(v & 0x80))
							s->ptr_36 = _scene->getDataFromTable(s->offs_2e, v);
						l = false;
					}
				}
			}
		} break;

		case 3: 
		case 4: {
			while (sc < next) {
				uint16 i1 = *sc++;
				uint16 i2 = *sc++;
				assert(i1 < ARRAYSIZE(_s80) && i2 < ARRAYSIZE(_s80));
				Struct80 *s1 = &_s80[i1];
				Struct80 *s2 = &_s80[i2];
				s2->next2 = s1->next;
				s1->next = i2;
				s2->prev = i1;
			}
		} break;

		case 5:
		case 6:
		case 7:
			_var_7893 = sc[1];
			break;
		case 8:
			_word_B6406 = READ_BE_UINT16(sc);
			break;
		case 9:
			_vc1_var_78BC = sc[1];
			break;
		default:
			error("Unknown opcode 0x%02x", cmd);
		}

		sc = next;
	}

	while (!shouldQuit()) {
		uint32 nextFrame = _system->getMillis() + 16;

		_gfx->nextFrame();

		delayUntil(nextFrame);
	}

	return true;
}

void SnatcherEngine::delayUntil(uint32 end) {
	uint32 cur = _system->getMillis();
	while (!shouldQuit() && cur < end) {
		updateEvents();
		int ms = MIN<int>(end - cur, 4);
		_system->delayMillis(ms);
		cur = _system->getMillis();
	} 
}

void SnatcherEngine::updateEvents() {
	Common::Event evt;
	while (_eventMan->pollEvent(evt)) {
		/*
		*/
	}
}

void SnatcherEngine::registerDefaultSettings() {
	//ConfMan.registerDefault("cdaudio", true);
}

void SnatcherEngine::syncSoundSettings() {

}

void SnatcherEngine::pauseEngineIntern(bool pause) {

}

} // End of namespace Snatcher
