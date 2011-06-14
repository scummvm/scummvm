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
 * $URL: https://svn.scummvm.org:4444/svn/dreamweb/dreamweb.cpp $
 * $Id: dreamweb.cpp 79 2011-06-05 08:26:54Z eriktorbjorn $
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

#include "engines/util.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/dreamgen.h"

namespace dreamgen {
	void doshake(dreamgen::Context &context);
	void dofade(dreamgen::Context &context);
	void volumeadjust(dreamgen::Context &context);
}

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

	dreamgen::doshake(_context);
	dreamgen::dofade(_context);
	_system->updateScreen();
}

void DreamWebEngine::quit() {
	warning("Engine should quit gracefully (but doesn't yet)");
	g_system->quit();
}

void DreamWebEngine::processEvents() {
	Common::EventManager *event_manager = _system->getEventManager();
	if (event_manager->shouldQuit()) {
		quit();
		return;
	}
	soundHandler();
	Common::Event event;
	while (event_manager->pollEvent(event)) {
		switch(event.type) {
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
					_context.data.byte(dreamgen::kSymbolbotnum) = 3;
					_context.data.byte(dreamgen::kSymboltopnum) = 5;
					break;

				default:
					break;
				}

				return; //do not pass ctrl + key to the engine
			}

			// As far as I can see, the only keys checked
			// for in 'lasthardkey' are 1 (ESC) and 57
			// (space) so add special cases for them and
			// treat everything else as 0.
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_context.data.byte(dreamgen::kLasthardkey) = 1;
			else if (event.kbd.keycode == Common::KEYCODE_SPACE)
				_context.data.byte(dreamgen::kLasthardkey) = 57;
			else
				_context.data.byte(dreamgen::kLasthardkey) = 0;
			if (event.kbd.ascii)
				keyPressed(event.kbd.ascii);
			break;
		default:
			break;
		}
	}
}


Common::Error DreamWebEngine::run() {
	_console = new DreamWebConsole(this);

	getTimerManager()->installTimerProc(vSyncInterrupt, 1000000 / 70, this);
	//http://martin.hinner.info/vga/timing.html

	dreamgen::__start(_context);
	
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
	if (ascii >= 'a' && ascii <= 'z')
		ascii = (ascii - 'a') + 'A';
	debug(2, "key pressed = %04x", ascii);
	uint8* keybuf = _context.data.ptr(5912, 16); //fixme: some hardcoded offsets are not added as consts
	uint16 in = (_context.data.word(dreamgen::kBufferin) + 1) & 0x0f;
	uint16 out = _context.data.word(dreamgen::kBufferout);
	if (in == out) {
		warning("keyboard buffer is full");
		return;
	}
	_context.data.word(dreamgen::kBufferin) = in;
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

void DreamWebEngine::setGraphicsMode() {
	processEvents();
	initGraphics(320, 200, false);
}

void DreamWebEngine::fadeDos() {
	_context.ds = _context.es = _context.data.word(dreamgen::kBuffers);
	return; //fixme later
	waitForVSync();
	//processEvents will be called from vsync
	uint8 *dst = _context.es.ptr(dreamgen::kStartpal, 768);
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
	_context.es = _context.data.word(dreamgen::kWorkspace);
	_context.di = dreamgen::kScreenwidth * 43 + 76;
	_context.si = _context.di + 8 * dreamgen::kScreenwidth;

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
		_context._add(_context.di, dreamgen::kScreenwidth);
		_context._add(_context.si, dreamgen::kScreenwidth);
	}
	_context.cx = 0;
	_system->unlockScreen();
}

void DreamWebEngine::cls() {
	_system->fillScreen(0);
}

void DreamWebEngine::playSound(uint8 channel, uint8 id, uint8 loops) {
	debug(1, "playSound(%u, %u, %u)", channel, id, loops);
	const SoundData &data = _soundData[id >= 12? 1: 0];

	Audio::Mixer::SoundType type;
	bool speech = id == 62; //actually 50
	if (id >= 12) {
		id -= 12;
		type = Audio::Mixer::kSFXSoundType;
	} else if (speech)
		type = Audio::Mixer::kSpeechSoundType;
	else 
		type = Audio::Mixer::kMusicSoundType;

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

bool DreamWebEngine::loadSpeech(const Common::String &filename) {
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
	_context.push(_context.ax);
	volumeadjust(_context);
	_context.ax = _context.pop();

	uint volume = _context.data.byte(dreamgen::kVolume);
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

	uint8 ch0 = _context.data.byte(dreamgen::kCh0playing);
	if (ch0 == 255)
		ch0 = 0;
	uint8 ch1 = _context.data.byte(dreamgen::kCh1playing);
	if (ch1 == 255)
		ch1 = 0;
	uint8 ch0loop = _context.data.byte(dreamgen::kCh0repeat);

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
	if (!_mixer->isSoundHandleActive(_channelHandle[0]))
		_context.data.byte(dreamgen::kCh0playing) = 255;
	if (!_mixer->isSoundHandleActive(_channelHandle[1]))
		_context.data.byte(dreamgen::kCh1playing) = 255;
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


} // End of namespace DreamWeb


namespace dreamgen {

Common::String getFilename(Context &context) {
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	return name;
}

void multiget(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned x = (uint16)context.di, y = (uint16)context.bx;
	unsigned src = x + y * kScreenwidth;
	unsigned dst = (uint16)context.si;
	context.es = context.ds;
	context.ds = context.data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiget %u,%u %ux%u -> segment: %04x->%04x", x, y, w, h, (uint16)context.ds, (uint16)context.es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = context.ds.ptr(src + kScreenwidth * l, w);
		uint8 *dst_p = context.es.ptr(dst + w * l, w);
		memcpy(dst_p, src_p, w);
	}
	context.si += w * h;
	context.di = src + kScreenwidth * h;
	context.cx = 0;
}

void multiput(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned x = (uint16)context.di, y = (uint16)context.bx;
	unsigned src = (uint16)context.si;
	unsigned dst = x + y * kScreenwidth;
	context.es = context.data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiput %ux%u -> segment: %04x->%04x", w, h, (uint16)context.ds, (uint16)context.es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = context.ds.ptr(src + w * l, w);
		uint8 *dst_p = context.es.ptr(dst + kScreenwidth * l, w);
		memcpy(dst_p, src_p, w);
	}
	context.si += w * h;
	context.di = dst + kScreenwidth * h;
	context.cx = 0;
}

void multidump(Context &context) {
	context.ds = context.data.word(kWorkspace);
	int w = (uint8)context.cl, h = (uint8)context.ch;
	int x = (int16)context.di, y = (int16)context.bx;
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multidump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)context.ds, x, y, offset);
	context.engine->blit(context.ds.ptr(offset, w * h), kScreenwidth, x, y, w, h);
	context.si = context.di = offset + h * kScreenwidth;
	context.cx = 0;
}

void worktoscreen(Context &context) {
	context.ds = context.data.word(kWorkspace);
	uint size = 320 * 200;
	context.engine->blit(context.ds.ptr(0, size), 320, 0, 0, 320, 200);
	context.di = context.si = size;
	context.cx = 0;
}

void printundermon(Context &context) {
	context.engine->printUnderMonitor();
}

void cls(Context &context) {
	context.engine->cls();
}

void frameoutnm(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned pitch = (uint16)context.dx;
	unsigned src = (uint16)context.si;
	int x = (uint16)context.di, y = (uint16)context.bx;
	unsigned dst = x + y * pitch;
	//debug(1, "framenm %ux%u[pitch: %u]-> %d,%d, segment: %04x->%04x", w, h, pitch, x, y, (uint16)context.ds, (uint16)context.es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = context.ds.ptr(src + w * l, w);
		uint8 *dst_p = context.es.ptr(dst + pitch * l, w);
		memcpy(dst_p, src_p, w);
	}
	context.di += dst + pitch * h;
	context.si += w * h;
	context.cx = 0;
}

void seecommandtail(Context &context) {
	context.data.word(kSoundbaseadd) = 0x220;
	context.data.byte(kSoundint) = 5;
	context.data.byte(kSounddmachannel) = 1;
	context.data.byte(kBrightness) = 1;
	context.data.word(kHowmuchalloc) = 0x9360;
}

void randomnumber(Context &context) {
	context.al = context.engine->randomNumber();
}

void quickquit(Context &context) {
	context.engine->quit();
}

void quickquit2(Context &context) {
	context.engine->quit();
}

void keyboardread(Context &context) {
	::error("keyboardread"); //this keyboard int handler, must never be called
}

void resetkeyboard(Context &context) {
}

void setkeyboardint(Context &context) {
}

void readfromfile(Context &context) {
	uint16 dst_offset = context.dx;
	uint16 size = context.cx;
	debug(1, "readfromfile(%04x:%u, %u)", (uint16)context.ds, dst_offset, size);
	context.ax = context.engine->readFromFile(context.ds.ptr(dst_offset, size), size);
	context.flags._c = false;
}

void closefile(Context &context) {
	context.engine->closeFile();
	context.data.byte(kHandle) = 0;
}

void openforsave(Context &context) {
	const char *name = (const char *)context.ds.ptr(context.dx, 13);
	debug(1, "openforsave(%s)", name);
	context.engine->openSaveFileForWriting(name);
}

void openfilenocheck(Context &context) {
	const char *name = (const char *)context.ds.ptr(context.dx, 13);
	debug(1, "checksavefile(%s)", name);
	bool ok = context.engine->openSaveFileForReading(name);
	context.flags._c = !ok;
}

void openfilefromc(Context &context) {
	openfilenocheck(context);
}

void openfile(Context &context) {
	Common::String name = getFilename(context);
	debug(1, "opening file: %s", name.c_str());
	context.engine->openFile(name);
	context.cs.word(kHandle) = 1; //only one handle
	context.flags._c = false;
}

void createfile(Context &context) {
	::error("createfile");
}

void dontloadseg(Context &context) {
	context.ax = context.es.word(context.di);
	context._add(context.di, 2);
	context.dx = context.ax;
	context.cx = 0;
	unsigned pos = context.engine->skipBytes(context.dx);
	context.dx = pos >> 16;
	context.ax = pos & 0xffff;
	context.flags._c = false;
}

void mousecall(Context &context) {
	context.engine->mouseCall();
}

void setmouse(Context &context) {
	context.data.word(kOldpointerx) = 0xffff;
	//warning("setmouse: fixme: add range setting");
	//set vertical range to 15-184
	//set horizontal range to 15-298*2
}

void gettime(Context &context) {
	TimeDate t;
	g_system->getTimeAndDate(t);
	debug(1, "\tgettime: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	context.ch = t.tm_hour;
	context.cl = t.tm_min;
	context.dh = t.tm_sec;
	context.data.byte(kSecondcount) = context.dh;
	context.data.byte(kMinutecount) = context.cl;
	context.data.byte(kHourcount) = context.ch;
}

void allocatemem(Context &context) {
	uint size = (context.bx + 2) * 16;
	debug(1, "allocate mem, %u bytes", size);
	context.flags._c = false;
	SegmentRef seg = context.allocateSegment(size);
	context.ax = (uint16)seg;
	debug(1, "\tsegment address -> %04x", (uint16)context.ax);
}

void deallocatemem(Context &context) {
	uint16 id = (uint16)context.es;
	debug(1, "deallocating segment %04x", id);
	context.deallocateSegment(id);

	//fixing invalid entries in the sprite table
	context.es = context.data;
	uint tsize = 16 * 32;
	uint16 bseg = context.data.word(kBuffers);
	if (!bseg)
		return;
	SegmentRef buffers(&context);
	buffers = bseg;
	uint8 *ptr = buffers.ptr(kSpritetable, tsize);
	for(uint i = 0; i < tsize; i += 32) {
		uint16 seg = READ_LE_UINT16(ptr + i + 6);
		//debug(1, "sprite segment = %04x", seg);
		if (seg == id)
			memset(ptr + i, 0xff, 32);
	}
}

void removeemm(Context &context) {
	::error("removeemm");
}

void setupemm(Context &context) {
	//fixme: double check this, but it seems that emm pages used only for sound
}

void pitinterupt(Context &context) {
	::error("pitinterupt");
}

void getridofpit(Context &context) {
	::error("getridofpit");
}

void setuppit(Context &context) {
	::error("setuppit");
}

void startdmablock(Context &context) {
	::error("startdmablock");
}

void dmaend(Context &context) {
	::error("dmaend");
}

void restoreems(Context &context) {
	::error("restoreems");
}

void saveems(Context &context) {
	::error("saveems");
}

void bothchannels(Context &context) {
	::error("bothchannels");
}

void channel1only(Context &context) {
	::error("channel1only");
}

void channel0only(Context &context) {
	::error("channel0only");
}

void out22c(Context &context) {
	::error("out22c");
}

void soundstartup(Context &context) {
}

void soundend(Context &context) {
}

void interupttest(Context &context) {
	::error("interupttest");
}

void disablesoundint(Context &context) {
	warning("disablesoundint: STUB");
}

void enablesoundint(Context &context) {
	warning("enablesoundint: STUB");
}

void checksoundint(Context &context) {
	context.data.byte(kTestresult) = 1;
	warning("checksoundint: STUB");
}

void setsoundoff(Context &context) {
	warning("setsoundoff: STUB");
}

void readheader(Context &context);

void loadsample(Context &context) {
	context.engine->loadSounds(0, (const char *)context.data.ptr(context.dx, 13));
}

void cancelch0(Context &context);
void cancelch1(Context &context);

void loadsecondsample(Context &context) {
	uint8 ch0 = context.data.byte(kCh0playing);
	if (ch0 >= 12 && ch0 != 255)
		cancelch0(context);
	uint8 ch1 = context.data.byte(kCh1playing);
	if (ch1 >= 12)
		cancelch1(context);
	context.engine->loadSounds(1, (const char *)context.data.ptr(context.dx, 13));
}

void createname(Context &context);

void loadspeech(Context &context) {
	cancelch1(context);
	context.data.byte(kSpeechloaded) = 0;
	createname(context);
	const char *name = (const char *)context.data.ptr(context.di, 13);
	//warning("name = %s", name);
	if (context.engine->loadSpeech(name))
		context.data.byte(kSpeechloaded) = 1;
}

void saveseg(Context &context) {
	context.cx = context.es.word(context.di);
	context._add(context.di, 2);
	savefilewrite(context);
}

void savefilewrite(Context &context) {
	context.ax = context.engine->writeToSaveFile(context.ds.ptr(context.dx, context.cx), context.cx);
}

void savefileread(Context &context) {
	context.ax = context.engine->readFromSaveFile(context.ds.ptr(context.dx, context.cx), context.cx);
}

void loadseg(Context &context) {
	context.ax = context.es.word(context.di);
	context.di += 2;

	uint16 dst_offset = context.dx;
	uint16 size = context.ax;

	debug(1, "loadseg(%04x:%u, %u)", (uint16)context.ds, dst_offset, size);
	context.ax = context.engine->readFromFile(context.ds.ptr(dst_offset, size), size);
	context.flags._c = false;
}

void error(Context &context) {
	::error("error");
}

void generalerror(Context &context) {
	::error("generalerror");
}

void commandonly(Context &context);

void dosreturn(Context &context) {
	context._cmp(context.data.byte(kCommandtype), 250);
	if (context.flags.z()) goto alreadydos;
	context.data.byte(kCommandtype) = 250;
	context.al = 46;
	commandonly(context);
alreadydos:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) return;

	quickquit2(context);
	quickquit(context);
}

void set16colpalette(Context &context) {
}

void mode640x480(Context &context) {
	// Video mode 12h: 640x480 pixels, 16 colors, I believe
	context.al = 0x12 + 128;
	context.ah = 0;
	initGraphics(640, 480, true);
}

void showgroup(Context &context) {
	context.engine->setPalette();
}

void fadedos(Context &context) {
	context.engine->fadeDos();
}

void doshake(Context &context) {
	uint8 &counter = context.data.byte(kShakecounter);
	context._cmp(counter, 48);
	if (context.flags.z())
		return;

	context._add(counter, 1);
	static const int shakeTable[] = {
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,

		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,

		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,

		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  0,
	};
	int offset = shakeTable[counter];
	context.engine->setShakePos(offset >= 0? offset: -offset);
}

void vsync(Context &context) {
	context.engine->waitForVSync();
}

void setmode(Context &context) {
	vsync(context);
	context.engine->setGraphicsMode();
}

void readoneblock(Context &context) {
	context.ds = context.data.word(kWorkspace);
	context.cx = 30000;
	context.dx = 0;
	readfromfile(context);
}

void readabyte(Context & context);

void showpcx(Context &context) {
	Common::String name = getFilename(context);
	Common::File pcxFile;

	if (!pcxFile.open(name)) {
		warning("showpcx: Could not open '%s'", name.c_str());
		return;
	}

	uint8 *maingamepal;
	int i, j;

	// Read the 16-color palette into the 'maingamepal' buffer. Note that
	// the color components have to be adjusted from 8 to 6 bits.

	pcxFile.seek(16, SEEK_SET);
	context.es = context.data.word(kBuffers);
	maingamepal = context.es.ptr(4782, 768); //fixme: hardcoded offset
	pcxFile.read(maingamepal, 48);

	memset(maingamepal + 48, 0xff, 720);
	for (i = 0; i < 48; i++) {
		maingamepal[i] >>= 2;
	}

	// Decode the image data.

	Graphics::Surface *s = g_system->lockScreen();
	Common::Rect rect(640, 480);

	s->fillRect(rect, 0);
	pcxFile.seek(128, SEEK_SET);

	for (int y = 0; y < 480; y++) {
		byte *dst = (byte *)s->getBasePtr(0, y);
		int decoded = 0;

		while (decoded < 320) {
			byte col = pcxFile.readByte();
			byte len;

			if ((col & 0xc0) == 0xc0) {
				len = col & 0x3f;
				col = pcxFile.readByte();
			} else {
				len = 1;
			}

			// The image uses 16 colors and is stored as four bit
			// planes, one for each bit of the color, least
			// significant bit plane first.

			for (i = 0; i < len; i++) {
				int plane = decoded / 80;
				int pos = decoded % 80;

				for (j = 0; j < 8; j++) {
					byte bit = (col >> (7 - j)) & 1;
					dst[8 * pos + j] |= (bit << plane);
				}

				decoded++;
			}
		}
	}

	g_system->unlockScreen();
	pcxFile.close();
}

} /*namespace dreamgen */
