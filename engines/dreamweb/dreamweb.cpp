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

#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/dreamgen.h"

namespace DreamWeb {

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _rnd("dreamweb") {

	_context.engine = this;
	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	_vSyncInterrupt = false;

	_console = 0;
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	_outSaveFile = 0;
	_inSaveFile = 0;
	_speed = 1;
	_turbo = false;
	_oldMouseState = 0;
	_channel0 = 0;
	_channel1 = 0;

	_language = gameDesc->desc.language;
}

DreamWebEngine::~DreamWebEngine() {
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

	_context.doshake();
	_context.dofade();
	_system->updateScreen();
}

void DreamWebEngine::quit() {
	_context.data.byte(DreamGen::DreamGenContext::kQuitrequested) = 1;
	_context.data.byte(DreamGen::DreamGenContext::kLasthardkey) = 1;
}

void DreamWebEngine::processEvents() {
	Common::EventManager *event_manager = _system->getEventManager();
	if (event_manager->shouldQuit()) {
		quit();
		return;
	}

	if (_enableSavingOrLoading && _loadSavefile >= 0 && _loadSavefile <= 6) {
		debug(1, "loading save state %d", _loadSavefile);
		_context.data.byte(_context.kCurrentslot) = _loadSavefile;
		_loadSavefile = -1;
		_context.loadposition();
		_context.data.byte(_context.kGetback) = 1;
	}

	soundHandler();
	Common::Event event;
	int softKey, hardKey;
	while (event_manager->pollEvent(event)) {
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
					_context.data.byte(DreamGen::DreamGenContext::kSymbolbotnum) = 3;
					_context.data.byte(DreamGen::DreamGenContext::kSymboltopnum) = 5;
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

			_context.data.byte(DreamGen::DreamGenContext::kLasthardkey) = hardKey;

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

	if (ConfMan.hasKey("save_slot")) {
		_enableSavingOrLoading = true;
		_loadSavefile = ConfMan.getInt("save_slot");
	} else {
		_enableSavingOrLoading = false;
		_loadSavefile = -1;
	}

	getTimerManager()->installTimerProc(vSyncInterrupt, 1000000 / 70, this);
	_context.__start();
	_context.data.byte(DreamGen::DreamGenContext::kQuitrequested) = 0;

	getTimerManager()->removeTimerProc(vSyncInterrupt);

	return Common::kNoError;
}

void DreamWebEngine::setSpeed(uint speed) {
	debug(0, "setting speed %u", speed);
	_speed = speed;
	getTimerManager()->removeTimerProc(vSyncInterrupt);
	getTimerManager()->installTimerProc(vSyncInterrupt, 1000000 / 70 / speed, this);
}

void DreamWebEngine::openFile(const Common::String &name) {
	processEvents();
	closeFile();
	if (_file.open(name))
		return;
	_inSaveFile = _system->getSavefileManager()->openForLoading(name);
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
	delete _outSaveFile;
	_outSaveFile = 0;
}

void DreamWebEngine::openSaveFileForWriting(const Common::String &name) {
	processEvents();
	delete _outSaveFile;
	_outSaveFile = _system->getSavefileManager()->openForSaving(name);
}

bool DreamWebEngine::openSaveFileForReading(const Common::String &name) {
	processEvents();
	delete _inSaveFile;
	_inSaveFile = _system->getSavefileManager()->openForLoading(name);
	return _inSaveFile != 0;
}

uint DreamWebEngine::writeToSaveFile(const uint8 *data, uint size) {
	processEvents();
	if (!_outSaveFile)
		error("save file was not opened for writing");
	return _outSaveFile->write(data, size);
}

uint DreamWebEngine::readFromSaveFile(uint8 *data, uint size) {
	processEvents();
	if (!_inSaveFile)
		error("save file was not opened for reading");
	return _inSaveFile->read(data, size);
}


void DreamWebEngine::keyPressed(uint16 ascii) {
	debug(2, "key pressed = %04x", ascii);
	uint8* keybuf = _context.data.ptr(5912, 16); //fixme: some hardcoded offsets are not added as consts
	uint16 in = (_context.data.word(DreamGen::DreamGenContext::kBufferin) + 1) & 0x0f;
	uint16 out = _context.data.word(DreamGen::DreamGenContext::kBufferout);
	if (in == out) {
		warning("keyboard buffer is full");
		return;
	}
	_context.data.word(DreamGen::DreamGenContext::kBufferin) = in;
	keybuf[in] = ascii;
}

void DreamWebEngine::mouseCall() {
	processEvents();
	Common::EventManager *eventMan = _system->getEventManager();
	Common::Point pos = eventMan->getMousePos();
	if (pos.x > 298)
		pos.x = 298;
	if (pos.x < 15)
		pos.x = 15;
	if (pos.y < 15)
		pos.y = 15;
	if (pos.y > 184)
		pos.y = 184;
	_context.cx = pos.x;
	_context.dx = pos.y;

	unsigned state = eventMan->getButtonState();
	_context.bx = state == _oldMouseState? 0: state;
	_oldMouseState = state;
	_context.flags._c = false;
}

void DreamWebEngine::fadeDos() {
	_context.ds = _context.es = _context.data.word(DreamGen::DreamGenContext::kBuffers);
	return; //fixme later
	waitForVSync();
	//processEvents will be called from vsync
	uint8 *dst = _context.es.ptr(DreamGen::DreamGenContext::kStartpal, 768);
	getPalette(dst, 0, 64);
	for(int fade = 0; fade < 64; ++fade) {
		for(int c = 0; c < 768; ++c) { //original sources decrement 768 values -> 256 colors
			if (dst[c]) {
				--dst[c];
			}
		}
		setPalette(dst, 0, 64);
		waitForVSync();
	}
}

void DreamWebEngine::setPalette() {
	processEvents();
	unsigned n = (uint16)_context.cx;
	uint8 *src = _context.ds.ptr(_context.si, n * 3);
	setPalette(src, _context.al, n);
	_context.si += n * 3;
	_context.cx = 0;
}

void DreamWebEngine::getPalette(uint8 *data, uint start, uint count) {
	_system->getPaletteManager()->grabPalette(data, start, count);
	while(count--)
		*data++ >>= 2;
}

void DreamWebEngine::setPalette(const uint8 *data, uint start, uint count) {
	assert(start + count <= 256);
	uint8 fixed[768];
	for(uint i = 0; i < count * 3; ++i) {
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
	_context.es = _context.data.word(DreamGen::DreamGenContext::kWorkspace);
	_context.di = DreamGen::DreamGenContext::kScreenwidth * 43 + 76;
	_context.si = _context.di + 8 * DreamGen::DreamGenContext::kScreenwidth;

	Graphics::Surface *s = _system->lockScreen();
	if (!s)
		error("lockScreen failed");

	for(uint y = 0; y < 104; ++y) {
		uint8 *src = (uint8 *)s->getBasePtr(76, 43 + 8 + y);
		uint8 *dst = _context.es.ptr(_context.di, 170);
		for(uint x = 0; x < 170; ++x) {
			if (*src < 231)
				*dst++ = *src++;
			else {
				++dst; ++src;
			}
		}
		_context._add(_context.di, DreamGen::DreamGenContext::kScreenwidth);
		_context._add(_context.si, DreamGen::DreamGenContext::kScreenwidth);
	}
	_context.cx = 0;
	_system->unlockScreen();
}

void DreamWebEngine::cls() {
	_system->fillScreen(0);
}

void DreamWebEngine::playSound(uint8 channel, uint8 id, uint8 loops) {
	debug(1, "playSound(%u, %u, %u)", channel, id, loops);

	int bank = 0;
	bool speech = false;
	Audio::Mixer::SoundType type = channel == 0?
		Audio::Mixer::kMusicSoundType: Audio::Mixer::kSFXSoundType;

	if (id >= 12) {
		id -= 12;
		bank = 1;
		if (id == 50) {
			speech = true;
			type = Audio::Mixer::kSpeechSoundType;
		}
	}
	const SoundData &data = _soundData[bank];

	Audio::SeekableAudioStream *raw;
	if (!speech) {
		if (id >= data.samples.size() || data.samples[id].size == 0) {
			warning("invalid sample #%u played", id);
			return;
		}

		const Sample &sample = data.samples[id];
		uint8 *buffer = (uint8 *)malloc(sample.size);
		if (!buffer)
			error("out of memory: cannot allocate memory for sound(%u bytes)", sample.size);
		memcpy(buffer, data.data.begin() + sample.offset, sample.size);

		raw = Audio::makeRawStream(
			buffer,
			sample.size, 22050, Audio::FLAG_UNSIGNED);
	} else {
		uint8 *buffer = (uint8 *)malloc(_speechData.size());
		memcpy(buffer, _speechData.begin(), _speechData.size());
		if (!buffer)
			error("out of memory: cannot allocate memory for sound(%u bytes)", _speechData.size());
		raw = Audio::makeRawStream(
			buffer,
			_speechData.size(), 22050, Audio::FLAG_UNSIGNED);

	}

	Audio::AudioStream *stream;
	if (loops > 1) {
		stream = new Audio::LoopingAudioStream(raw, loops < 255? loops: 0);
	} else
		stream = raw;

	if (_mixer->isSoundHandleActive(_channelHandle[channel]))
		_mixer->stopHandle(_channelHandle[channel]);
	_mixer->playStream(type, &_channelHandle[channel], stream);
}

void DreamWebEngine::stopSound(uint8 channel) {
	debug(1, "stopSound(%u)", channel);
	assert(channel == 0 || channel == 1);
	_mixer->stopHandle(_channelHandle[channel]);
	if (channel == 0)
		_channel0 = 0;
	else
		_channel1 = 0;
}

bool DreamWebEngine::loadSpeech(const Common::String &filename) {
	if (ConfMan.getBool("speech_mute"))
		return false;

	Common::File file;
	if (!file.open("speech/" + filename))
		return false;

	debug(1, "loadSpeech(%s)", filename.c_str());

	uint size = file.size();
	_speechData.resize(size);
	file.read(_speechData.begin(), size);
	file.close();
	return true;
}


void DreamWebEngine::soundHandler() {
	_context.data.byte(_context.kSubtitles) = ConfMan.getBool("subtitles");
	_context.push(_context.ax);
	_context.volumeadjust();
	_context.ax = _context.pop();

	uint volume = _context.data.byte(DreamGen::DreamGenContext::kVolume);
	//.vol file loaded into soundbuf:0x4000
	//volume table at (volume * 0x100 + 0x3f00)
	//volume value could be from 1 to 7
	//1 - 0x10-0xff
	//2 - 0x1f-0xdf
	//3 - 0x2f-0xd0
	//4 - 0x3e-0xc1
	//5 - 0x4d-0xb2
	//6 - 0x5d-0xa2
	//7 - 0x6f-0x91
	if (volume >= 8)
		volume = 7;
	volume = (8 - volume) * Audio::Mixer::kMaxChannelVolume / 8;
	_mixer->setChannelVolume(_channelHandle[0], volume);

	uint8 ch0 = _context.data.byte(DreamGen::DreamGenContext::kCh0playing);
	if (ch0 == 255)
		ch0 = 0;
	uint8 ch1 = _context.data.byte(DreamGen::DreamGenContext::kCh1playing);
	if (ch1 == 255)
		ch1 = 0;
	uint8 ch0loop = _context.data.byte(DreamGen::DreamGenContext::kCh0repeat);

	if (_channel0 != ch0) {
		_channel0 = ch0;
		if (ch0) {
			playSound(0, ch0, ch0loop);
		}
	}
	if (_channel1 != ch1) {
		_channel1 = ch1;
		if (ch1) {
			playSound(1, ch1, 1);
		}
	}
	if (!_mixer->isSoundHandleActive(_channelHandle[0])) {
		_context.data.byte(DreamGen::DreamGenContext::kCh0playing) = 255;
		_channel0 = 0;
	}
	if (!_mixer->isSoundHandleActive(_channelHandle[1])) {
		_context.data.byte(DreamGen::DreamGenContext::kCh1playing) = 255;
		_channel1 = 0;
	}

}

void DreamWebEngine::loadSounds(uint bank, const Common::String &filename) {
	debug(1, "loadSounds(%u, %s)", bank, filename.c_str());
	Common::File file;
	if (!file.open(filename)) {
		warning("cannot open %s", filename.c_str());
		return;
	}

	uint8 header[0x60];
	file.read(header, sizeof(header));
	uint tablesize = READ_LE_UINT16(header + 0x32);
	debug(1, "table size = %u", tablesize);

	SoundData &soundData = _soundData[bank];
	soundData.samples.resize(tablesize / 6);
	uint total = 0;
	for(uint i = 0; i < tablesize / 6; ++i) {
		uint8 entry[6];
		Sample &sample = soundData.samples[i];
		file.read(entry, sizeof(entry));
		sample.offset = entry[0] * 0x4000 + READ_LE_UINT16(entry + 1);
		sample.size = READ_LE_UINT16(entry + 3) * 0x800;
		total += sample.size;
		debug(1, "offset: %08x, size: %u", sample.offset, sample.size);
	}
	soundData.data.resize(total);
	file.read(soundData.data.begin(), total);
	file.close();
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


