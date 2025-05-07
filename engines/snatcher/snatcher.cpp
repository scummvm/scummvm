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
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/list.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "engines/util.h"

// Snatcher makes use of horizontal interrupts. These can be invoked after any given count of rendered scan lines.
// These interrupts are mostly used for changing vertical scroll offsets, but some effects also change the colors.
// In ScummVM, this can only work if we use at least 16bit color depth. 8 bit rendering will not allow the already
// rendered pixels to keep their colors. So the following setting may cause color glitches and is just there for
// backends that can't handle the required color depth.
#define			SNATCHER_GFXMODE_8BIT			false

namespace Snatcher {

SnatcherEngine::SnatcherEngine(OSystem *system, GameDescription &dsc) : Engine(system), _game(dsc), _fio(nullptr), _module(nullptr), _gfx(nullptr), _snd(nullptr), _input(), _lastKeys(0),
	_releaseKeys(0), _keyRepeat(false), _gfxInfo(), _frameLen((100000 << 14) / (6000000 / 1001)), _realLightGunPos() {
	assert(system);
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

	if (!initSound(_system->getMixer(),	_game.platform, _game.soundOptions))
		return Common::Error(Common::kAudioDeviceInitFailed);

	if (!initGfx(_game.platform, SNATCHER_GFXMODE_8BIT))
		return Common::Error(Common::kUnknownError);

	return Common::Error(start() ? Common::kNoError : Common::kUnknownError);
}

bool SnatcherEngine::initResource() {
	return (_fio = new FIO(this, _game.isBigEndian));
}

bool SnatcherEngine::initGfx(Common::Platform platform, bool use8BitColorMode) {
	Graphics::PixelFormat pxf;

	if (use8BitColorMode) {
		pxf = Graphics::PixelFormat::createFormatCLUT8();
	} else {
		Graphics::PixelFormat pxf16bit;
		Common::List<Graphics::PixelFormat> modes = _system->getSupportedFormats();
		for (Common::List<Graphics::PixelFormat>::const_iterator g = modes.begin(); g != modes.end() && pxf.bytesPerPixel == 0; ++g) {
			if (g->aBits())
				continue;
			if (g->bytesPerPixel == 4)
				pxf = *g;
			else if (g->bytesPerPixel == 2)
				pxf16bit = *g;
		}
		if (pxf.bytesPerPixel == 0)
			pxf = pxf16bit;
		if (pxf.bytesPerPixel == 0)
			error("%s(): No suitable color mode found", __FUNCTION__);
	}

	_gfx = new GraphicsEngine(&pxf, _system, platform, _gfxInfo);
	if (_gfx) {
		initGraphics(_gfx->screenWidth(), _gfx->screenHeight(), &pxf);
		return true;
	}

	return false;
}

bool SnatcherEngine::initSound(Audio::Mixer *mixer, Common::Platform platform, int soundOptions) {
	assert(_mixer);
	assert(_fio);
	_snd = new SoundEngine(_fio, platform, soundOptions);
	return (_snd && _snd->init(mixer));
}

void SnatcherEngine::playBootLogoAnimation(const GameState &state) {
	uint32 frameTimer = 0;
	int curSeqState = 0;
	_snd->fmSendCommand(242, -1);

	while (curSeqState != -1 && !shouldQuit()) {
		frameTimer += _frameLen;
		uint32 nextFrame = _system->getMillis() + (frameTimer >> 14);
		frameTimer &= 0x3FFF;

		int nextState = _gfx->displayBootLogoFrame(curSeqState);
		if (curSeqState == 6 && nextState == 7)
			_snd->fmSendCommand(63, -1);
		curSeqState = nextState;

		checkEvents(state);
		_snd->update();
		delayUntil(nextFrame);
	}
}

bool SnatcherEngine::start() {
	Util::rngReset();
	GameState state;

	playBootLogoAnimation(state);

	uint32 frameTimer = 0;

	int16 countTo5 = 0;
	int16 runspeed = 0;

	while (!shouldQuit()) {
		frameTimer += _frameLen;
		uint32 nextFrame = _system->getMillis() + (frameTimer >> 14);
		frameTimer &= 0x3FFF;

		int _sub_doMainScript = 0;

		updateTopLevelState(state);

		_gfxInfo.audioSync = _snd->cdaIsPlaying() ? _snd->cdaGetTime() : 0;
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
		_snd->update();
		checkEvents(state);
		delayUntil(nextFrame);
	}

	return true;
}

void SnatcherEngine::delayUntil(uint32 end) {
	uint32 cur = _system->getMillis();
	_gfxInfo.dropFrames = /*cur > end ? ((cur - end) << 14) / _frameLen :*/ 0;
	if (cur < end)
		_system->delayMillis(end - cur);
}

struct InputEvent {
	Common::EventType pressType;
	Common::EventType releaseType;
	Common::KeyCode kc;
	uint16 kFlag;
	uint16 internalEvent;
	bool updateCoords;
};

static const InputEvent _defaultKeyEvents[] = {
	// Arrow buttons
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_UP, 0x00, 0x01, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP8, Common::KBD_NUM, 0x01, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_DOWN, 0x00, 0x02, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP2, Common::KBD_NUM, 0x02, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_LEFT, 0x00, 0x04, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP4, Common::KBD_NUM, 0x04, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_RIGHT, 0x00, 0x08, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP6, Common::KBD_NUM, 0x08, false},
	{ Common::EVENT_WHEELUP, Common::EVENT_INVALID, Common::KEYCODE_INVALID, 0x00, 0x01, false },
	{ Common::EVENT_WHEELDOWN, Common::EVENT_INVALID, Common::KEYCODE_INVALID, 0x00, 0x02, false },

	// A, B, C buttons
	{Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_a, 0x00, 0x10, false},
	{Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_b, 0x00, 0x20, false},
	{Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_c, 0x00, 0x40, false},

	{ Common::EVENT_MBUTTONDOWN, Common::EVENT_MBUTTONUP, Common::KEYCODE_INVALID, 0x00, 0x20, false },

	// Start button
	{Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_RETURN, 0x00, 0x80, false},
	{Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_SPACE, 0x00, 0x80, false},

	// Lightgun
	{ Common::EVENT_LBUTTONDOWN, Common::EVENT_LBUTTONUP, Common::KEYCODE_INVALID, 0x00, 0x100, true },

	// Lightgun Start button
	{ Common::EVENT_RBUTTONDOWN, Common::EVENT_RBUTTONUP, Common::KEYCODE_INVALID, 0x00, 0x200, false },
};

void SnatcherEngine::checkEvents(const GameState &state) {
	Common::Event evt;
	_lastKeys &= ~_releaseKeys;
	_releaseKeys = 0;
	_input.controllerFlags = 0;
	Common::Point mouse;

	while (_eventMan->pollEvent(evt)) {
		for (const InputEvent &k : _defaultKeyEvents) {
			if (evt.type == k.pressType && (k.kc == Common::KEYCODE_INVALID || evt.kbd.keycode == k.kc) && (k.kFlag == 0 || (evt.kbd.flags & k.kFlag))) {
				if (!(_lastKeys & k.internalEvent)) {
					_input.controllerFlags |= k.internalEvent;
					_lastKeys |= k.internalEvent;
					if (_keyRepeat || k.releaseType == Common::EVENT_INVALID)
						_releaseKeys |= k.internalEvent;
					if (k.updateCoords) {
						_realLightGunPos = evt.mouse;
						// The lightgun coordinates are supposed to be based on a 256 x 256 system, with 128 being
						// the screen center. I solve this by always adding -16 to the y-bias.
						_input.lightGunPos.x = CLIP<int>(_realLightGunPos.x - state.conf.lightGunBias.x, 0, 255);
						_input.lightGunPos.y = CLIP<int>(_realLightGunPos.y - state.conf.lightGunBias.y, 0, 255);
					}
				}
			} else if ((evt.type == k.releaseType) && (k.kc == Common::KEYCODE_INVALID || evt.kbd.keycode == k.kc) && (k.kFlag == 0 || (evt.kbd.flags & k.kFlag))) {
				_lastKeys &= ~k.internalEvent;
			}
		}
	}
}

void SnatcherEngine::updateTopLevelState(GameState &state) {
	switch (state.topLevelState) {
	case 0:
		if (state.main_switch_1_0_orM1_postIntr == -1) {
			++state.topLevelState;
		}
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	default:
		error("%s(): Invalid state %d", __FUNCTION__, state.topLevelState);
		break;
	}
}

void SnatcherEngine::updateModuleState(GameState &state) {
	int _unlCDREadSeekWord = 0;
	uint8 _triggerPalFlag7Etc = 0;
	bool _sub_bool_5 = false;

	switch (state.modProcessTop) {
	case -1:
		_snd->cdaStop();
		state.modProcessTop = 0;
		break;
	case 0:
		switch (state.modProcessSub) {
		case 0:
			++state.modProcessSub;
			break;
		case 1:
			if (!_snd->cdaIsPlaying()) {
				delete _module;
				_module = _fio->loadModule(4);
				assert(_module);
				++state.modProcessSub;
			}
			break;
		case 2:
			++state.modProcessSub;
			break;
		case 3:
			//if (*(uint16*)(&_wordRAM__TABLE48__word_B6400[0])) {
				state.modProcessSub = 0;
				state.finish = 0;
				++state.modProcessTop;
			//}
			break;
		default:
			break;
		}
		break;
	case 1:
		switch (state.modProcessSub) {
		case 0:
			state.frameNo = 0;
			++state.modProcessSub;
			_snd->pcmInitSound(30);
			break;
		case 1:
			if (!(_snd->pcmGetStatus() & 0x0F))
				++state.modProcessSub;
			break;
		case 2:
			if (_module)
				_module->run(state);
			if (state.finish) {
				state.modProcessSub = 0;
				state.finish = 0;
				state.modProcessTop = state.menuSelect ? 7 : state.modProcessTop + 1;
				state.modIndex = 0;
				state.main_switch_1_0_orM1_postIntr = 1;
			}
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (state.modProcessSub) {
		case 0:
			if (!(_gfx->frameCount() & 0x1F)) {
				if (!_snd->cdaIsPlaying()) {
					static const uint8 scids[] = { 3, 2 };
					delete _module;
					assert(state.modIndex < ARRAYSIZE(scids));
					_module = _fio->loadModule(scids[state.modIndex]);
					assert(_module);
					++state.modProcessSub;
				} else {
					_snd->cdaStop();
				}
			}
			break;
		case 1:
			if (!(_gfx->frameCount() & 0x1F)) {
				// startup__runWithFileFunc(2)
				_unlCDREadSeekWord = 0;
				state.frameNo = -1;
				++state.modProcessSub;
			}
			break;
		case 2:
			if (_module)
				_module->run(state);
			if (state.finish < 0) {
				state.finish = 0;
				state.counter = 10;
				++state.modProcessSub;
				static const uint8 data[] = { 0x83, 0x00, 0x00, 0x3F };
				_gfx->enqueuePaletteEvent(ResourcePointer(data, 0));
			} else if (state.finish) {
				if (state.modIndex == 0) {
					++state.modIndex;
					state.finish = 0;
				}
				_gfx->reset(GraphicsEngine::kResetAnimations);
				state.modProcessSub = 0;
			}
			break;
		case 3:
			if (--state.counter == 1) {
				static const uint8 cmd[] = {
					0x01, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xFF, 0xFF,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};
				_gfx->enqueueDrawCommands(ResourcePointer(cmd, 0));
			} else if (state.counter == 0) {
				state.finish = -1;
				state.modProcessSub = 0;
				_gfx->reset(GraphicsEngine::kResetSetDefaultsExt);
			}
			break;
		default:
			break;
		}
		if (state.finish) {
			state.finish = 0;
			state.modProcessTop = 7;
			state.modProcessSub = 0;
			_gfx->transitionCommand(0xFF);
			_gfx->setVar(9, 1);
			_gfx->reset(GraphicsEngine::kResetPalEvents | GraphicsEngine::kResetAnimations);
		}
		break;
	case 3:
		//_wordRAM__TABLE48__word_B6400[2] = 1;
		state.finish = 0;
		state.modProcessTop = 0;
		state.modProcessSub = 0;
		_gfx->reset(GraphicsEngine::kResetScrollState);
		break;
	case 5:
		// bset    #3,(GA_SUB__COMMUNICATION_FLAG+1).
		if (state.finish) {
			// bclr    #3,(GA_SUB__COMMUNICATION_FLAG+1).l
			++state.modProcessTop;
			state.modProcessSub = 0;
			state.finish = 0;
		}
		break;
	case 6:
		switch (state.modProcessSub) {
		case 0:
			if (!_gfx->isAnimEnabled(31)) {
				_gfx->reset(GraphicsEngine::kResetSetDefaults | GraphicsEngine::kResetAnimations);
				_triggerPalFlag7Etc = 0xFF;
				++state.modProcessSub;
				state.frameNo = -1;
				state.frameState = 0;
			}
			break;
		case 1:
			if (!_snd->cdaIsPlaying()) {
				delete _module;
				_module = _fio->loadModule(52);
				assert(_module);
				++state.modProcessSub;
			}
			break;
		case 2:
			// startup__runWithFileFunc(2)
			_unlCDREadSeekWord = 0;
			++state.modProcessSub;
			break;
		case 3:
			if (_module)
				_module->run(state);
			if (state.finish) {
				_sub_bool_5 = true;
				++state.modProcessSub;
			}
			break;
		default:
			break;
		}
		break;
	case 7:
		switch (state.modProcessSub) {
		case 0:
			_snd->cdaStop();
			//loadFile56 PCMLT_01.BIN
			++state.modProcessSub;
			break;
		case 1:
		case 3:
			// startup__runWithFileFunc(2)
			_unlCDREadSeekWord = 0;
			++state.modProcessSub;
			break;
		case 2:
			//loadFile54 PCMDRMDT.BIN
			++state.modProcessSub;
			break;
		case 4:
			state.modProcessTop = 5;
			state.modProcessSub = 0;
			state.main_switch_1_0_orM1_postIntr = -1;
			break;
		default:
			break;
		}
	default:
		break;
	}
}

void SnatcherEngine::registerDefaultSettings() {
}

void SnatcherEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (!_snd)
		return;

	int volMusic = 192;
	int volSFX = 192;
	bool mute = false;

	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");
	if (!mute) {
	if (ConfMan.hasKey("music_volume"))
		volMusic = ConfMan.getInt("music_volume");
	if (ConfMan.hasKey("sfx_volume"))
		volSFX = ConfMan.getInt("sfx_volume");
	} else {
		volMusic = 0;
		volSFX = 0;
	}

	_snd->setMusicVolume(volMusic);
	_snd->setSoundEffectVolume(volSFX);
}

void SnatcherEngine::pauseEngineIntern(bool pause) {
	_snd->pause(pause);
}

void SnatcherEngine::calibrateLightGun(GameState &state) {
	state.conf.lightGunBias.x = _realLightGunPos.x - (_gfx->screenWidth() / 2);
	state.conf.lightGunBias.y = _realLightGunPos.y - (_gfx->screenHeight() / 2);
	// The lightgun coordinates are supposed to be based on a 256 x 256 system, with 128 being
	// the screen center. I solve this by always adding -16 to the y-bias.
	state.conf.lightGunBias.y -= ((256 - _gfx->screenHeight()) / 2);
}

} // End of namespace Snatcher
