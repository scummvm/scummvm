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
#include "snatcher/util.h"

#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

namespace Snatcher {

SnatcherEngine::SnatcherEngine(OSystem *system, GameDescription &dsc) : Engine(system), _game(dsc), _fio(nullptr), _module(nullptr), _gfx(nullptr), _snd(nullptr), _commandsFromMain(0), _gfxInfo() {
}

SnatcherEngine::~SnatcherEngine() {
	delete _module;
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
	if (_gfx = new GraphicsEngine(_system, platform, _gfxInfo)) {
		initGraphics(_gfx->screenWidth(), _gfx->screenHeight());
		return true;
	}
	return false;
}

bool SnatcherEngine::initSound(Common::Platform platform, int soundOptions) {
	return (_snd = new SoundEngine(platform, soundOptions));
}

bool SnatcherEngine::start() {
	Util::rngReset();
	GameState state;
	memset(&state, 0, sizeof(GameState));

	const uint32 frameLen = (1000 << 16) / 60;
	uint32 frameTimer = 0;

	int16 countTo5 = 0;
	int16 runspeed = 0;

	while (!shouldQuit()) {
		frameTimer += frameLen;
		uint32 nextFrame = _system->getMillis() + (frameTimer >> 16);
		frameTimer &= 0xFFFF;

		int _sub_doMainScript = 0;

		if (runInitSequence(state))
			continue;

		_gfxInfo.audioSync = _snd->musicIsPlaying() ? _snd->musicGetTime() : 0;
		Util::rngMakeNumber();

		_gfx->setVar(3, 0);
		_gfx->setVar(10, 0);
		countTo5 += runspeed;
		int numLoops = (countTo5 == 5) ? 2 : 1;

		for (int i = 0; i < numLoops; ++i) {
			if (!_gfx->busy(0)) {
				//subVintSub_2();
				bool blockedUpdt = _sub_doMainScript;
				if (_sub_doMainScript) {
					//subVint_mainScriptRun__36Cases();
					blockedUpdt = _gfx->busy(0);
				}
				if (!blockedUpdt) {
					updateModuleState(state);
					_gfx->updateAnimations();
				}
			}

			if (numLoops == 2) {
				if (_gfx->busy(1)) {
					--countTo5;
					numLoops = 1;
				} else {
					countTo5 = 0;
					_gfx->setVar(10, 1);
				}
			}
		}

		_gfx->nextFrame();
		updateEvents();
		delayUntil(nextFrame);
	}

	return true;
}

void SnatcherEngine::delayUntil(uint32 end) {
	const uint32 frameLen = (1000 << 16) / 60;
	uint32 cur = _system->getMillis();
	_gfxInfo.dropFrames = /*cur > end ? ((cur - end) << 16) / frameLen :*/ 0;
	if (cur < end)
		_system->delayMillis(end - cur);
}

void SnatcherEngine::updateEvents() {
	Common::Event evt;
	while (_eventMan->pollEvent(evt)) {
		switch (evt.type) {
		case Common::EVENT_LBUTTONDOWN:
			_commandsFromMain |= (1 << 7);
			break;
		default:
			_commandsFromMain = 0;
			break;
		}
	}
}

bool SnatcherEngine::runInitSequence(GameState &gameState) {
	if (gameState.initState == -1)
		return false;

	gameState.initState = -1;

	return true;
}

void SnatcherEngine::updateModuleState(GameState &state) {
	int _main_switch_1_0_orM1_postIntr = 0;
	uint16 _word_unk_Flags = 0;
	int _word_unk_11 = 0;
	int _sub_val_2 = 0;
	int _unlCDREadSeekWord = 0;
	uint8 _triggerPalFlag7Etc = 0;
	bool _sub_bool_5 = false;

	uint8 _wordRAM__TABLE48__word_B6400[48];
	memset(_wordRAM__TABLE48__word_B6400, 0, 48);

	switch (state.progressMain) {
	case -1:
		_snd->musicStop();
		state.progressMain = 0;
		break;
	case 0:
		switch (state.progressSub) {
		case 0:
			++state.progressSub;
			break;
		case 1:
			if (!_snd->musicIsPlaying()) {
				delete _module;
				_module = _fio->loadModule(4);
				assert(_module);
				++state.progressSub;
			}
			break;
		case 2:
			++state.progressSub;
			break;
		case 3:
			//if (*(uint16*)(&_wordRAM__TABLE48__word_B6400[0])) {
				state.progressSub = 0;
				state.finish = 0;
				++state.progressMain;
			//}
			break;
		default:
			break;
		}
		break;
	case 1:
		switch (state.progressSub) {
		case 0:
			state.frameNo = 0;
			++state.progressSub;
			_snd->pcmPlayEffect(30);
			break;
		case 1:
			if (!(_word_unk_Flags & 0x0F))
				++state.progressSub;
			break;
		case 2:
			if (_module)
				_module->run(state);
			if (state.finish) {
				state.progressSub = 0;
				state.finish = 0;
				state.progressMain = _word_unk_11 ? 7 : state.progressMain + 1;
				state.modIndex = 0;
				_main_switch_1_0_orM1_postIntr = 1;
			}
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (state.progressSub) {
		case 0:
			if (!(_sub_val_2 & 0x1F)) {
				if (!_snd->musicIsPlaying()) {
					uint8 scids[] = { 3, 2 };
					delete _module;
					assert(state.modIndex < ARRAYSIZE(scids));
					_module = _fio->loadModule(scids[state.modIndex]);
					assert(_module);
					++state.progressSub;
				} else {
					_snd->musicStop();
				}
			}
			break;
		case 1:
			if (!(_sub_val_2 & 0x1F)) {
				// startup__runWithFileFunc(2)
				_unlCDREadSeekWord = 0;
				state.frameNo = -1;
				++state.progressSub;
			}
			break;
		case 2:
			if (_module)
				_module->run(state);
			if (state.finish < 0) {
				state.finish = 0;
				state.counter = 10;
				++state.progressSub;
				static byte data[] = { 0x83, 0x00, 0x00, 0x3F };
				_gfx->enqueuePaletteEvent(ResourcePointer(data, 0));
			} else if (state.finish) {
				if (state.modIndex == 0) {
					++state.modIndex;
					state.finish = 0;
				}
				_gfx->reset(GraphicsEngine::kResetSprites);
				state.progressSub = 0;
			}
			break;
		case 3:
			if (--state.counter == 1) {
				static const byte cmd[] = {
					0x01, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xFF, 0xFF,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};
				_gfx->enqueueCopyCommands(ResourcePointer(cmd, 0));
			} else if (state.counter == 0) {
				state.finish = -1;
				state.progressSub = 0;
				_gfx->reset(GraphicsEngine::kResetSetDefaultsExt);
			}
			break;
		default:
			break;
		}
		if (state.finish) {
			state.finish = 0;
			state.progressMain = 7;
			state.progressSub = 0;
			_gfx->scrollCommand(0xFF);
			_gfx->setVar(9, 1);
			_gfx->reset(GraphicsEngine::kResetPalEvents | GraphicsEngine::kResetSprites);
		}
		break;
	case 3:
		_wordRAM__TABLE48__word_B6400[2] = 1;
		state.finish = 0;
		state.progressMain = 0;
		state.progressSub = 0;
		_gfx->reset(GraphicsEngine::kResetScrollState);
		break;
	case 5:
		if (state.finish) {
			++state.progressMain;
			state.progressSub = 0;
			state.finish = 0;
		}
		break;
	case 6:
		switch (state.progressSub) {
		case 0:
			if (!_gfx->isAnimEnabled(31)) {
				_gfx->reset(GraphicsEngine::kResetSetDefaults | GraphicsEngine::kResetSprites);
				_triggerPalFlag7Etc = 0xFF;
				++state.progressSub;
				state.frameNo = -1;
				state.frameState = 0;
			}
			break;
		case 1:
			if (!_snd->musicIsPlaying()) {
				delete _module;
				_module = _fio->loadModule(52);
				assert(_module);
				++state.progressSub;
			}
			break;
		case 2:
			// startup__runWithFileFunc(2)
			_unlCDREadSeekWord = 0;
			++state.progressSub;
			break;
		case 3:
			if (_module)
				_module->run(state);
			if (state.finish) {
				_sub_bool_5 = true;
				++state.progressSub;
			}
			break;
		default:
			break;
		}
	case 7:
		switch (state.progressSub) {
		case 0:
			_snd->musicStop();
			//loadFile56 PCMLT_01.BIN
			++state.progressSub;
			break;
		case 1:
		case 3:
			// startup__runWithFileFunc(2)
			_unlCDREadSeekWord = 0;
			++state.progressSub;
			break;
		case 2:
			//loadFile54 PCMDRMDT.BIN
			++state.progressSub;
			break;
		case 4:
			state.progressMain = 5;
			state.progressSub = 0;
			_main_switch_1_0_orM1_postIntr = -1;
			break;
		default:
			break;
		}
	default:
		break;
	}
}

void SnatcherEngine::registerDefaultSettings() {
	//ConfMan.registerDefault("cdaudio", true);
}

void SnatcherEngine::syncSoundSettings() {

}

void SnatcherEngine::pauseEngineIntern(bool pause) {
	_snd->pause(pause);
}

} // End of namespace Snatcher
