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
#include "snatcher/script.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"
#include "snatcher/text.h"
#include "snatcher/ui.h"
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

SnatcherEngine::SnatcherEngine(OSystem *system, GameDescription &dsc) : Engine(system), _game(dsc), _fio(nullptr), _module(nullptr), _scd(nullptr), _gfx(nullptr), _snd(nullptr), _input(),
	_scriptEngine(nullptr), _cmdQueue(nullptr), _script(nullptr), _lastKeys(0), _releaseKeys(0), _keyRepeat(false), _enableLightGun(false), _ui(nullptr), _gfxInfo(),
		_frameLen((100000 << 14) / (6000000 / 1001)), _realLightGunPos() {
	assert(system);
}

SnatcherEngine::~SnatcherEngine() {
	delete _module;
	delete _gfx;
	delete _snd;
	delete _fio;
	delete _scd;
	delete _scriptEngine;
	delete _cmdQueue;
	delete _script;
	delete _ui;
}

Common::Error SnatcherEngine::run() {
	registerDefaultSettings();

	if (!initResource())
		return Common::Error(Common::kReadingFailed);

	if (!initSound(_system->getMixer(),	_game.platform, _game.soundOptions))
		return Common::Error(Common::kAudioDeviceInitFailed);

	if (!initGfx(_game.platform, SNATCHER_GFXMODE_8BIT))
		return Common::Error(Common::kUnsupportedColorMode);

	if (!initScriptEngine())
		return Common::Error(Common::kUnknownError);

	return Common::Error(start() ? Common::kNoError : Common::kUnknownError);
}

bool SnatcherEngine::initResource() {
	_fio = new FIO(this, _game.isBigEndian);
	if (!_fio)
		return false;

	uint32 size = 0;
	uint8 *data = _fio->fileData(96, &size);
	if (!data)
		return false;
	_scd = new ResourcePointer(data, 0, 0xD400, true);
	if (!_scd)
		return false;

	return true;
}

bool SnatcherEngine::initGfx(Common::Platform platform, bool use8BitColorMode) {
	Graphics::PixelFormat pxf;

	// Currently, the animation code needs access to the sound engine. Maybe this can be improved later...
	assert(_snd);

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

	_gfx = new GraphicsEngine(&pxf, _system, platform, _gfxInfo, _snd);

	if (_gfx) {
		initGraphics(_gfx->screenWidth(), _gfx->screenHeight(), &pxf);
		assert(_scd);
		_gfx->setTextFont(_scd->makePtr(0x14B7C)(), 816, _scd->makePtr(0x14EAC)(), 102);
		return true;
	}

	return false;
}

bool SnatcherEngine::initSound(Audio::Mixer *mixer, Common::Platform platform, int soundOptions) {
	assert(_mixer);
	assert(_fio);

	_snd = new SoundEngine(_fio, platform, soundOptions);
	if (!_snd || !_snd->init(mixer))
		return false;

	syncSoundSettings();

	return true;
}

bool SnatcherEngine::initScriptEngine() {
	_cmdQueue = new CmdQueue(this);
	if (!_cmdQueue)
		return false;

	assert(_scd);
	_ui = new UI(_gfx, _cmdQueue, _scd);
	if (!_ui)
		return false;

	_scriptEngine = new ScriptEngine(_cmdQueue, _ui, _scd);
	if (!_scriptEngine)
		return false;

	_script = new Script(0x3800);
	if (!_script)
		return false;

	return true;
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

	//playBootLogoAnimation(state);
	state.prologue = -1;
	state.modProcessTop = 7;
	state.modProcessSub = 4;

	uint32 frameTimer = 0;

	int16 countTo5 = 0;
	int16 runspeed = 0;

	while (!shouldQuit()) {
		frameTimer += _frameLen;
		uint32 nextFrame = _system->getMillis() + (frameTimer >> 14);
		frameTimer &= 0x3FFF;

		updateChapter(state);

		_gfxInfo.audioSync = _snd->cdaIsPlaying() ? _snd->cdaGetTime() : 0;
		Util::rngMakeNumber();

		_gfx->setVar(3, 0);
		_gfx->setVar(10, 0);
		countTo5 += runspeed;
		int numLoops = (countTo5 == 5) ? 2 : 1;

		for (int i = 0; i < numLoops; ++i) {
			if (!_gfx->busy(0)) {
				bool blockedMod = _cmdQueue->enabled();
				if (_cmdQueue->enabled()) {
					_cmdQueue->run();
					blockedMod = _gfx->busy(0);
				}
				if (!blockedMod) {
					updateModuleState(state);
					_gfx->updateAnimations();
				}
			}

			_gfx->updateText();

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

		checkEvents(state);

		_gfx->nextFrame();
		_snd->update();
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
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP8, 0x00, 0x01, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_DOWN, 0x00, 0x02, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP2, 0x00, 0x02, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_LEFT, 0x00, 0x04, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP4, 0x00, 0x04, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_RIGHT, 0x00, 0x08, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP6, 0x00, 0x08, false},
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP7, 0x00, 0x05, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP9, 0x00, 0x09, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP3, 0x00, 0x0A, false },
	{ Common::EVENT_KEYDOWN, Common::EVENT_KEYUP, Common::KEYCODE_KP1, 0x00, 0x06, false },
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

static const uint8 _scdKeyConfigs[6][3] = {
	{ 6, 4, 5 }, { 6, 5, 4 },
	{ 4, 6, 5 }, { 4, 5, 6 },
	{ 5, 4, 6 }, { 5, 6, 4 }
};

void SnatcherEngine::checkEvents(const GameState &state) {
	Common::Event evt;
	_lastKeys &= ~_releaseKeys;
	_releaseKeys = 0;
	_input.controllerFlags = _input.controllerFlagsRemapped = 0;
	Common::Point mouse;

	while (_eventMan->pollEvent(evt)) {
		for (const InputEvent &k : _defaultKeyEvents) {
			if (evt.type == k.pressType && (k.kc == Common::KEYCODE_INVALID || evt.kbd.keycode == k.kc) && (k.kFlag == 0 || (evt.kbd.flags & k.kFlag))) {
				if (!(_lastKeys & k.internalEvent)) {
					_input.controllerFlags |= k.internalEvent;
					// The A, B and C buttons can be reconfigured in the main menu. This applies the setting.
					for (int f = 0; f < 3; ++f) {
						if (k.internalEvent & (f << 5)) 
							_input.controllerFlagsRemapped |= (1 << _scdKeyConfigs[state.conf.controllerSetup][f]);
					}
					_input.controllerFlagsRemapped |= (k.internalEvent & ~0x70);
					_lastKeys |= k.internalEvent;
					if (_keyRepeat || k.releaseType == Common::EVENT_INVALID)
						_releaseKeys |= k.internalEvent;
					if (k.updateCoords) {
						_realLightGunPos = evt.mouse;
						// The lightgun coordinates are supposed to be based on a 256 x 256 system, with 128 being
						// the screen center. I solve this by always adding the diff to the y-bias.
						_input.lightGunPos.x = CLIP<int>(_realLightGunPos.x - state.conf.lightGunBias.x, 0, 255);
						_input.lightGunPos.y = CLIP<int>(_realLightGunPos.y - state.conf.lightGunBias.y, 0, 255);
					}
				}
			} else if ((evt.type == k.releaseType) && (k.kc == Common::KEYCODE_INVALID || evt.kbd.keycode == k.kc) && (k.kFlag == 0 || (evt.kbd.flags & k.kFlag))) {
				_lastKeys &= ~k.internalEvent;
			}
		}
	}

	if (_enableLightGun && _input.controllerFlags & 0x100)
		_gfx->setVar(7, 4);
}

void SnatcherEngine::updateChapter(GameState &state) {
	// The chapterSub variable is an invention to emulate the interaction between the main thread and the
	// vertical interrupt handlers in the original code. It may complicate things a bit (e. g. I have to
	// provide means to drop out of several involved functions and continue with them later), but I still
	// like this better than making the code multi-threaded, using a timer proc.

	if (state.chapterSub & 0x80) {
		if (!(_cmdQueue->enabled() || (_snd->pcmGetStatus().statusBits & 7)))
			state.chapterSub &= ~0x80;
		else
			return;
	}

	switch (state.chapter) {
	case 0:
		// Intro and main menu
		if (state.prologue != -1)
			break;

		if (state.menuSelect == 0)
			_gfx->setVar(1, _gfx->getVar(1) | 2);
		else
			_gfx->setVar(5, 1);

		_cmdQueue->writeUInt16(0x16);
		_cmdQueue->writeUInt16(0x03);
		_cmdQueue->writeUInt32(0xECB6);
		_cmdQueue->writeUInt16(0x03);
		_cmdQueue->writeUInt32(0xECC4);
		_cmdQueue->writeUInt16(0x01);
		_cmdQueue->writeUInt32(0xED0E);
		_cmdQueue->start();

		++state.chapter;
		state.chapterSub |= 0x80;

		break;

	case 1:
		// Init first scene or load savegame
		_scriptEngine->resetArrays();
		if (state.menuSelect == 0) {
			_cmdQueue->writeUInt16(0x11);
			_cmdQueue->writeUInt32(0x1A800);
			_cmdQueue->writeUInt16(0x3F);
			_cmdQueue->writeUInt16(15);
		} else {
			/* load savegame */
		}

		_ui->setControllerConfig(state.conf.controllerSetup);

		_cmdQueue->start();
		++state.chapter;
		state.chapterSub |= 0x80;
		break;

	case 2:
		// Act I
		if (state.chapterSub & 0x40) {
			if (_scriptEngine->postProcess(_script)) {
				for (int i = 5; i < 15; ++i)
					_gfx->setAnimParameter(i, GraphicsEngine::kAnimParaEnable, 0);
				_cmdQueue->writeUInt16(0x22);
				_cmdQueue->writeUInt32(0x139D0);
				_cmdQueue->start();
				state.chapterSub |= 0xA0;
			}				
			state.chapterSub &= ~0x40;
		} else if (state.chapterSub & 0x20) {
			_gfx->setVar(11, 0xFF);
			if (!_ui->drawVerbs() || !_ui->verbsTabInputPrompt(_input.controllerFlagsRemapped)) {
				state.chapterSub |= 0x80;
			} else {
				_scriptEngine->processInput();
				state.chapterSub &= ~0x20;
			}
		} else if (state.chapterSub == 0) {
			_scriptEngine->run(_script);
			_cmdQueue->start();
			state.chapterSub |= 0xC0;
		}
		break;

	case 3:
		break;

	case 4:
		break;

	case 5:
		break;

	default:
		break;
	}
}

void SnatcherEngine::updateModuleState(GameState &state) {
	bool _sub_bool_5 = false;
	++_gfxInfo.frameCounter;

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
			if (!(_snd->pcmGetStatus().statusBits & 0x0F))
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
				state.prologue = 1;
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
				// original: check whether file is loaded
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
				_gfx->enqueuePaletteEvent(_scd->makePtr(0x14B1C));
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
				_gfx->enqueueDrawCommands(_scd->makePtr(0x14B4E));
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
			if (!_gfx->getAnimParameter(31, GraphicsEngine::kAnimParaEnable)) {
				_gfx->reset(GraphicsEngine::kResetSetDefaults | GraphicsEngine::kResetAnimations);
				_gfx->setVar(11, 0xFF);
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
			//_unlCDREadSeekWord = 0;
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
			// original: start load PCMLT_01.BIN
			++state.modProcessSub;
			break;
		case 1:
		case 3:
			// original: check if file loaded
			++state.modProcessSub;
			break;
		case 2:
			// original: start load PCMDRMDT.BIN
			++state.modProcessSub;
			break;
		case 4:
			state.modProcessTop = 5;
			state.modProcessSub = 0;
			state.prologue = -1;
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
	// the screen center. I just add the diff to the y-bias...
	state.conf.lightGunBias.y -= ((256 - _gfx->screenHeight()) / 2);
}

} // End of namespace Snatcher
