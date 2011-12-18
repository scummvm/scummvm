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
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/func.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/dreamgen.h"

namespace DreamWeb {

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _rnd("dreamweb"), _context(this), _base(_context) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	_vSyncInterrupt = false;

	_console = 0;
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	_inSaveFile = 0;
	_speed = 1;
	_turbo = false;
	_oldMouseState = 0;
	_channel0 = 0;
	_channel1 = 0;

	_icons1 = NULL;
	_icons2 = NULL;
	_tempCharset = NULL;

	_language = gameDesc->desc.language;
}

DreamWebEngine::~DreamWebEngine() {
	assert(_icons1 == NULL);
	assert(_icons2 == NULL);
	assert(_tempCharset == NULL);

	DebugMan.clearAllDebugChannels();
	delete _console;
}

static void vSyncInterrupt(void *refCon) {
	DreamWebEngine *vm = (DreamWebEngine *)refCon;

	if (!vm->isPaused()) {
		vm->setVSyncInterrupt(true);
	}
}

void DreamWebEngine::setVSyncInterrupt(bool flag) {
	_vSyncInterrupt = flag;
}

void DreamWebEngine::waitForVSync() {
	processEvents();

	if (!_turbo) {
		while (!_vSyncInterrupt) {
			_system->delayMillis(10);
		}
		setVSyncInterrupt(false);
	}

	_base.doShake();
	_base.doFade();
	_system->updateScreen();
}

void DreamWebEngine::quit() {
	_base.data.byte(DreamGen::kQuitrequested) = 1;
	_base.data.byte(DreamGen::kLasthardkey) = 1;
}

void DreamWebEngine::processEvents() {
	if (_eventMan->shouldQuit()) {
		quit();
		return;
	}

	soundHandler();
	Common::Event event;
	int softKey, hardKey;
	while (_eventMan->pollEvent(event)) {
		switch(event.type) {
		case Common::EVENT_RTL:
			quit();
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.flags & Common::KBD_CTRL) {
				switch (event.kbd.keycode) {

				case Common::KEYCODE_d:
					_console->attach();
					_console->onFrame();
					break;

				case Common::KEYCODE_f:
					setSpeed(_speed != 20? 20: 1);
					break;

				case Common::KEYCODE_g:
					_turbo = !_turbo;
					break;

				case Common::KEYCODE_c: //skip statue puzzle
					_base.data.byte(DreamGen::kSymbolbotnum) = 3;
					_base.data.byte(DreamGen::kSymboltopnum) = 5;
					break;

				default:
					break;
				}

				return; //do not pass ctrl + key to the engine
			}

			// Some parts of the ASM code uses the hardware key
			// code directly. We don't have that code, so we fake
			// it for the keys where it's needed and assume it's
			// 0 (which is actually an invalid value, as far as I
			// know) otherwise.

			hardKey = 0;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				hardKey = 1;
				break;
			case Common::KEYCODE_SPACE:
				hardKey = 57;
				break;
			default:
				hardKey = 0;
				break;
			}

			_base.data.byte(DreamGen::kLasthardkey) = hardKey;

			// The rest of the keys are converted to ASCII. This
			// is fairly restrictive, and eventually we may want
			// to let through more keys. I think this is mostly to
			// keep weird glyphs out of savegame names.

			softKey = 0;

			if (event.kbd.keycode >= Common::KEYCODE_a && event.kbd.keycode <= Common::KEYCODE_z) {
				softKey = event.kbd.ascii & ~0x20;
			} else if (event.kbd.keycode == Common::KEYCODE_MINUS ||
				event.kbd.keycode == Common::KEYCODE_SPACE ||
				(event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9)) {
				softKey = event.kbd.ascii;
			} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
				softKey = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			} else if (event.kbd.keycode == Common::KEYCODE_KP_MINUS) {
				softKey = '-';
			} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
				event.kbd.keycode == Common::KEYCODE_DELETE) {
				softKey = 8;
			} else if (event.kbd.keycode == Common::KEYCODE_RETURN
				|| event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
				softKey = 13;
			}

			if (softKey)
				keyPressed(softKey);
			break;
		default:
			break;
		}
	}
}

Common::Error DreamWebEngine::run() {
	syncSoundSettings();
	_console = new DreamWebConsole(this);

	ConfMan.registerDefault("dreamweb_originalsaveload", "false");

	_timer->installTimerProc(vSyncInterrupt, 1000000 / 70, this, "dreamwebVSync");
	_context.__start();
	_base.data.byte(DreamGen::kQuitrequested) = 0;

	_timer->removeTimerProc(vSyncInterrupt);

	return Common::kNoError;
}

void DreamWebEngine::setSpeed(uint speed) {
	debug(0, "setting speed %u", speed);
	_speed = speed;
	_timer->removeTimerProc(vSyncInterrupt);
	_timer->installTimerProc(vSyncInterrupt, 1000000 / 70 / speed, this, "dreamwebVSync");
}

void DreamWebEngine::openFile(const Common::String &name) {
	processEvents();
	closeFile();
	if (_file.open(name))
		return;
	// File not found? See if there is a save state with this name
	// FIXME: Is this really needed? If yes, document why; if not,
	// remove all traces of _inSaveFile.
	_inSaveFile = _saveFileMan->openForLoading(name);
	if (_inSaveFile)
		return;
	error("cannot open file %s", name.c_str());
}

uint32 DreamWebEngine::skipBytes(uint32 bytes) {
	if (!_file.seek(bytes, SEEK_CUR))
		error("seek failed");
	return _file.pos();
}

uint32 DreamWebEngine::readFromFile(uint8 *dst, unsigned size) {
	processEvents();
	if (_file.isOpen())
		return _file.read(dst, size);
	if (_inSaveFile)
		return _inSaveFile->read(dst, size);
	error("file was not opened (read before open)");
}

void DreamWebEngine::closeFile() {
	processEvents();
	if (_file.isOpen())
		_file.close();
	delete _inSaveFile;
	_inSaveFile = 0;
}

Common::String DreamWebEngine::getSavegameFilename(int slot) const {
	// TODO: Are saves from all versions of Dreamweb compatible with each other?
	// Then we can can consider keeping the filenames as DREAMWEB.Dnn.
	// Otherwise, this must be changed to be target dependent.
	//Common::String filename = _targetName + Common::String::format(".d%02d", savegameId);
	Common::String filename = Common::String::format("DREAMWEB.D%02d", slot);
	return filename;
}

void DreamWebEngine::keyPressed(uint16 ascii) {
	debug(2, "key pressed = %04x", ascii);
	uint16 in = (_base.data.word(DreamGen::kBufferin) + 1) & 0x0f;
	uint16 out = _base.data.word(DreamGen::kBufferout);
	if (in == out) {
		warning("keyboard buffer is full");
		return;
	}
	_base.data.word(DreamGen::kBufferin) = in;
	DreamGen::g_keyBuffer[in] = ascii;
}

void DreamWebEngine::mouseCall(uint16 *x, uint16 *y, uint16 *state) {
	processEvents();
	Common::Point pos = _eventMan->getMousePos();
	if (pos.x > 298)
		pos.x = 298;
	if (pos.x < 15)
		pos.x = 15;
	if (pos.y < 15)
		pos.y = 15;
	if (pos.y > 184)
		pos.y = 184;
	*x = pos.x;
	*y = pos.y;

	unsigned newState = _eventMan->getButtonState();
	*state = (newState == _oldMouseState? 0 : newState);
	_oldMouseState = newState;
}

void DreamWebEngine::getPalette(uint8 *data, uint start, uint count) {
	_system->getPaletteManager()->grabPalette(data, start, count);
	while (count--)
		*data++ >>= 2;
}

void DreamWebEngine::setPalette(const uint8 *data, uint start, uint count) {
	assert(start + count <= 256);
	uint8 fixed[768];
	for (uint i = 0; i < count * 3; ++i) {
		fixed[i] = data[i] << 2;
	}
	_system->getPaletteManager()->setPalette(fixed, start, count);
}

void DreamWebEngine::blit(const uint8 *src, int pitch, int x, int y, int w, int h) {
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	if (h <= 0 || w <= 0)
		return;
	_system->copyRectToScreen(src, pitch, x, y, w, h);
}

void DreamWebEngine::printUnderMonitor() {
	uint8 *dst = _base.workspace() + DreamGen::kScreenwidth * 43 + 76;

	Graphics::Surface *s = _system->lockScreen();
	if (!s)
		error("lockScreen failed");

	for (uint y = 0; y < 104; ++y) {
		uint8 *src = (uint8 *)s->getBasePtr(76, 43 + 8 + y);
		for (uint x = 0; x < 170; ++x) {
			if (*src < 231)
				*dst++ = *src++;
			else {
				++dst; ++src;
			}
		}
		dst += DreamGen::kScreenwidth - 170;
	}
	_system->unlockScreen();
}

void DreamWebEngine::cls() {
	_system->fillScreen(0);
}

uint8 DreamWebEngine::modifyChar(uint8 c) const {
	if (c < 128)
		return c;

	switch(_language) {
	case Common::DE_DEU:
		switch(c)
		{
		case 129:
			return 'Z' + 3;
		case 132:
			return 'Z' + 1;
		case 142:
			return 'Z' + 4;
		case 154:
			return 'Z' + 6;
		case 225:
			return 'A' - 1;
		case 153:
			return 'Z' + 5;
		case 148:
			return 'Z' + 2;
		default:
			return c;
		}
	case Common::ES_ESP:
		switch(c) {
		case 160:
			return 'Z' + 1;
		case 130:
			return 'Z' + 2;
		case 161:
			return 'Z' + 3;
		case 162:
			return 'Z' + 4;
		case 163:
			return 'Z' + 5;
		case 164:
			return 'Z' + 6;
		case 165:
			return ',' - 1;
		case 168:
			return 'A' - 1;
		case 173:
			return 'A' - 4;
		case 129:
			return 'A' - 5;
		default:
			return c;
		}
	default:
		return c;
	}
}

} // End of namespace DreamWeb


namespace DreamGen {

// FIXME/TODO: Move this to a better place.
DreamBase::DreamBase(DreamWeb::DreamWebEngine *en) : engine(en) {
	_openChangeSize = kInventx+(4*kItempicsize);
}

} // End of namespace DreamGen
