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
#include "common/iff_container.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

#include "engines/util.h"

#include "audio/mixer.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/dreamgen.h"

namespace DreamWeb {

DreamWebEngine *DreamWebEngine::_instance;

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) : 
	Engine(syst), _gameDescription(gameDesc), _rnd("dreamweb") {
	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	_vSyncInterrupt = false;

	_console = 0;
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	_instance = this;
}

DreamWebEngine::~DreamWebEngine() {
	DebugMan.clearAllDebugChannels();
	delete _console;
}

// Let's see if it's a good idea to emulate VSYNC interrupts with a timer like
// this. There's a chance we'll miss interrupts, which could be countered by
// counting them instead of just flagging them, but we'll see...

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
	Graphics::Surface *s = _system->lockScreen();
	if (!s)
		error("lockScreen failed");
	for(int y = 0; y < 200; ++y) {
		uint8 *scanline = (uint8*)s->getBasePtr(0, y);
		uint8 *src = _context.video.ptr(y * 320, 320);
		memcpy(scanline, src, 320);
	}
	_system->unlockScreen();
	_system->updateScreen();
/*
	while (!_vSyncInterrupt) {
		_system->delayMillis(10);
	}
	setVSyncInterrupt(false);
*/
	// doshake
	// dofade
}

void DreamWebEngine::processEvents() {
	Common::EventManager *event_manager = _system->getEventManager();
	Common::Event event;
	while (event_manager->pollEvent(event)) {
		switch(event.type) {
		case Common::EVENT_RTL:
			warning("quit requested");
			return;
		case Common::EVENT_LBUTTONDOWN:
			_mouseState |= 1;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseState &= ~1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseState |= 2;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseState &= ~2;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouse = event.mouse;
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_d:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_console->attach();
					_console->onFrame();
				}
				break;
			default:
				if (event.kbd.ascii)
					keyPressed(event.kbd.ascii);
				break;
			}
			break;
		default:
			break;
		}
	}
}


Common::Error DreamWebEngine::run() {
	_mouseState = 0;
	_console = new DreamWebConsole(this);

	getTimerManager()->installTimerProc(vSyncInterrupt, 1000000 / 60, this);

	dreamgen::__start(_context);
	
	getTimerManager()->removeTimerProc(vSyncInterrupt);

	return Common::kNoError;
}

void DreamWebEngine::openFile(const Common::String &name) {
	processEvents();
	if (_file.isOpen()) {
		_file.close();
	}
	if (!_file.open(name)) {
		error("cannot open file %s", name.c_str());
	}
}

uint32 DreamWebEngine::readFromFile(uint8 *dst, unsigned size) {
	processEvents();
	if (!_file.isOpen())
		error("file was not opened (read before open)");
	return _file.read(dst, size);
}

void DreamWebEngine::closeFile() {
	processEvents();
	_file.close();
}

void DreamWebEngine::keyPressed(uint16 ascii) {
	debug(1, "key pressed = %04x", ascii);
	uint8* keybuf = _context.data.ptr(5715, 16); //fixme: some hardcoded offsets are not added as consts
	uint16 in = (_context.data.word(dreamgen::kBufferin) + 1) % 0x0f;
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
	Common::Point pos = _mouse;
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
	_context.bx = _mouseState;
}

void DreamWebEngine::setGraphicsMode() {
	processEvents();
	initGraphics(320, 200, false);
}

void DreamWebEngine::fadeDos() {
	//processEvents will be called from vsync
	PaletteManager *palette = _system->getPaletteManager();
	_context.ds = _context.es = _context.data.word(dreamgen::kBuffers);
	uint8 *dst = _context.es.ptr(dreamgen::kStartpal, 768);
	palette->grabPalette(dst, 0, 64);
	for(int fade = 0; fade < 64; ++fade) {
		for(int c = 0; c < 768; ++c) { //original sources decrement 768 values -> 256 colors
			if (dst[c]) {
				--dst[c];
			}
		}
		//Common::hexdump(dst, 64 * 3);
		//palette->setPalette(dst, 0, 64);
		//waitForVSync();
	}
}
void DreamWebEngine::setPalette() {
	uint8 colors[768];
	processEvents();
	PaletteManager *palette = _system->getPaletteManager();
	unsigned n = (uint16)_context.cx;
	uint8 *src = _context.ds.ptr(_context.si, n * 3);
	for(int i = 0; i < n * 3; ++i)
		colors[i] = src[i] * 3;
	//Common::hexdump(colors, n * 3);
	palette->setPalette(colors, _context.al, n);
	_context.si += n * 3;
	_context.cx = 0;
}

} // End of namespace DreamWeb


namespace dreamgen {

static inline DreamWeb::DreamWebEngine *engine() {
	return DreamWeb::DreamWebEngine::instance();
}

void multiget(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned pitch = (uint16)context.data.word(kScreenwidth);
	unsigned src = (uint16)context.di + (uint16)context.bx * pitch;
	unsigned dst = (uint16)context.si;
	context.es = context.ds;
	context.ds = context.data.word(kWorkspace);
	//debug(1, "multiget %ux%u -> segment: %04x->%04x", w, h, (uint16)context.ds, (uint16)context.es);
	for(unsigned y = 0; y < h; ++y) {
		uint8 *src_p = context.ds.ptr(src + pitch * y, w);
		uint8 *dst_p = context.es.ptr(dst + w * y, w);
		memcpy(dst_p, src_p, w);
	}
}

void multiput(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned pitch = (uint16)context.data.word(kScreenwidth);
	unsigned src = (uint16)context.si;
	unsigned dst = (uint16)context.di + (uint16)context.bx * pitch;
	context.es = context.data.word(kWorkspace);
	//debug(1, "multiput %ux%u -> segment: %04x->%04x", w, h, (uint16)context.ds, (uint16)context.es);
	for(unsigned y = 0; y < h; ++y) {
		uint8 *src_p = context.ds.ptr(src + w * y, w);
		uint8 *dst_p = context.es.ptr(dst + pitch * y, w);
		memcpy(dst_p, src_p, w);
	}
}

void multidump(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	context.es = 0xa000;
	context.ds = context.data.word(kWorkspace);
	//debug(1, "multidump %ux%u -> segment: %04x->%04x", w, h, (uint16)context.ds, (uint16)context.es);
	unsigned pitch = (uint16)context.data.word(kScreenwidth);
	unsigned offset = (uint16)context.di + (uint16)context.bx * pitch;
	for(unsigned y = 0; y < h; ++y, offset += pitch) {
		uint8 *src_p = context.ds.ptr(offset, w);
		uint8 *dst_p = context.es.ptr(offset, w);
		memcpy(dst_p, src_p, w);
	}
}

void frameoutnm(Context &context) {
	unsigned w = (uint8)context.cl, h = (uint8)context.ch;
	unsigned pitch = (uint16)context.dx;
	unsigned src = (uint16)context.si;
	unsigned dst = (uint16)context.di + (uint16)context.bx * pitch;
	//debug(1, "framenm %ux%u[pitch: %u] -> segment: %04x->%04x", w, h, pitch, (uint16)context.ds, (uint16)context.es);
	for(unsigned y = 0; y < h; ++y) {
		uint8 *src_p = context.ds.ptr(src + w * y, w);
		uint8 *dst_p = context.es.ptr(dst + pitch * y, w);
		memcpy(dst_p, src_p, w);
	}
}

void seecommandtail(Context &context) {
	context.data.word(kSoundbaseadd) = 0x220;
	context.data.byte(kSoundint) = 5;
	context.data.byte(kSounddmachannel) = 1;
	context.data.byte(kBrightness) = 1;
	context.data.word(kHowmuchalloc) = 0x9360;
}

void randomnumber(Context &context) {
	context.al = engine()->randomNumber();
}

void quickquit(Context &context) {
	assert(0);
	::error("quickquit");
}

void quickquit2(Context &context) {
	assert(0);
	::error("quickquit2");
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
	context.ax = engine()->readFromFile(context.ds.ptr(dst_offset, size), size);
	context.flags._c = false;
}

void closefile(Context &context) {
	engine()->closeFile();
	context.data.byte(kHandle) = 0;
}

void openforsave(Context &context) {
	::error("openforsave");
}

void openfilenocheck(Context &context) {
	::error("openfilenocheck");
}

void openfile(Context &context) {
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	debug(1, "opening file: %s", name.c_str());
	engine()->openFile(name);
	context.cs.word(kHandle) = 1; //only one handle
	context.flags._c = false;
}

void createfile(Context &context) {
	::error("createfile");
}

void dontloadseg(Context &context) {
	::error("dontloadseg");
}

void mousecall(Context &context) {
	engine()->mouseCall();
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
	debug(1, "deallocating segment %04x", (uint16)context.es);
	context.deallocateSegment(context.es);
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
	warning("loadsample: STUB");
	openfile(context);
	closefile(context);
}

void loadsecondsample(Context &context) {
	warning("loadsecondsample: STUB");
	openfile(context);
	closefile(context);
}

void loadspeech(Context &context) {
	::error("loadspeech");
}

void scanfornames(Context &context) {
	warning("scanfornames: STUB");
	context.ch = 0;
}

void saveseg(Context &context) {
	::error("saveseg");
}

void loadseg(Context &context) {
	context.ax = context.es.word(context.di);
	context.di += 2;

	uint16 dst_offset = context.dx;
	uint16 size = context.ax;

	debug(1, "loadseg(%04x:%u, %u)", (uint16)context.ds, dst_offset, size);
	context.ax = engine()->readFromFile(context.ds.ptr(dst_offset, size), size);
	context.flags._c = false;
}

void loadposition(Context &context) {
	::error("loadposition");
}

void saveposition(Context &context) {
	::error("saveposition");
}

void error(Context &context) {
	::error("error");
}

void generalerror(Context &context) {
	::error("generalerror");
}

void dosreturn(Context &context) {
	::error("dosreturn");
}

void set16colpalette(Context &context) {
	warning("set16colpalette: STUB");
}

void mode640x480(Context &context) {
	// Video mode 12h: 640x480 pixels, 16 colors, I believe
	context.al = 0x12 + 128;
	context.ah = 0;
	initGraphics(640, 480, true);
}

void showgroup(Context &context) {
	engine()->setPalette();
}

void fadedos(Context &context) {
	vsync(context);
	engine()->fadeDos();
}

void doshake(Context &context) {
	::error("doshake");
}

void vsync(Context &context) {
	engine()->waitForVSync();
}

void setmode(Context &context) {
	engine()->setGraphicsMode();
}

void readoneblock(Context &context) {
	context.ds = context.data.word(kWorkspace);
	context.cx = 30000;
	context.dx = 0;
	readfromfile(context);
}

void readabyte(Context & context);

void showpcx(Context &context) {
	openfile(context);
	context.ds = context.data.word(kWorkspace);
	context.cx = 128;
	context.dx = 0;
	readfromfile(context);

	context.ds = context.data.word(kWorkspace);
	context.si = 16;
	context.cx = 48;
	context.es = context.data.word(kBuffers);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768;
pcxpal:
	context.push(context.cx);
	readabyte(context);
	context._shr(context.al, 1);
	context._shr(context.al, 1);
	context._stosb();
	context.cx = context.pop();
	if (--context.cx) goto pcxpal;
	context.cx = 768 - 48;
	context.ax = 0x0ffff;
	while (--context.cx) context._stosw();

	readoneblock(context);
	context.si = 0;
	context.di = 0;
	context.cx = 480;
convertpcx:
	context.push(context.cx);
	context.push(context.di);
	context.ds = context.data.word(kWorkspace);
	context.es = context.data.word(kBuffers);
	context.di = 0+(228*13)+32+60;
	context.bx = 0;
sameline:
	readabyte(context);
	context.ah = context.al;
	context._and(context.ah, 0xc0);
	context._cmp(context.ah, 0xc0);
	if (!context.flags.z()) goto normal;
	context.cl = context.al;
	context._and(context.cl, 0x3f);
	context.ch = 0;
	context.push(context.cx);
	readabyte(context);
	context.cx = context.pop();
	context._add(context.bx, context.cx);
	if (--context.cx) context._stosb();
	context._cmp(context.bx, 4 * 80);
	if (!context.flags.z()) goto sameline;
	goto endline;
normal:
	context._stosb();
	context._add(context.bx, 1);
	context._cmp(context.bx, 4 * 80);
	if (!context.flags.z()) goto sameline;

endline:
	context.di = context.pop();
	context.push(context.si);
	context.dx = 0xa000;
	context.es = context.dx;
	context.si = 0+(228*13)+32+60;
	context.ds = context.data.word(kBuffers);

	// TODO: There's a bunch of code here which I assume draws data to the
	//       screen or something like that.

	context.si = context.pop();
	context.cx = context.pop();
	if (--context.cx) goto convertpcx;

	closefile(context);
}

} /*namespace dreamgen */
