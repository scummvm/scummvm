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

#include "engines/util.h"

#include "audio/mixer.h"

#include "graphics/palette.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/dreamgen.h"

namespace DreamWeb {

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	_rnd = new Common::RandomSource("dreamweb");

	_vSyncInterrupt = false;

	_console = 0;
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
}

DreamWebEngine::~DreamWebEngine() {
	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
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
	while (!_vSyncInterrupt) {
		_system->delayMillis(10);
	}
	setVSyncInterrupt(false);
	// doshake
	// dofade
}

Common::Error DreamWebEngine::run() {
	_console = new DreamWebConsole(this);
	
	dreamgen::Context context;
	dreamgen::__start(context);

	Common::EventManager *event_manager = _system->getEventManager();
	getTimerManager()->installTimerProc(vSyncInterrupt, 1000000 / 60, this);

	do {
		uint32 frame_time = _system->getMillis();
		Common::Event event;
		while (event_manager->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_RTL:
				return Common::kNoError;
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
					break;
				}
				break;
			default:
				debug(0, "skipped event type %d", event.type);
			}
		}
	} while (!shouldQuit());

	getTimerManager()->removeTimerProc(vSyncInterrupt);

	return Common::kNoError;
}

} // End of namespace DreamWeb


namespace dreamgen {
void seecommandtail(Context &context) {
	context.ds.word(kSoundbaseadd) = 0x220;
	context.ds.byte(kSoundint) = 5;
	context.ds.byte(kSounddmachannel) = 1;
	context.ds.byte(kBrightness) = 1;
	context.ds.word(kHowmuchalloc) = 0x9360;
}

void randomnumber(Context &context) {
	::error("randomnumber");
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
	::error("keyboardread");
}

void resetkeyboard(Context &context) {
}

void setkeyboardint(Context &context) {
}

void readfromfile(Context &context) {
	::error("readfromfile");
}

void closefile(Context &context) {
	::error("closefile");
}

void openforsave(Context &context) {
	::error("openforsave");
}

void openfilenocheck(Context &context) {
	::error("openfilenocheck");
}

void openfile(Context &context) {
	::error("openfile");
}

void createfile(Context &context) {
	::error("createfile");
}

void dontloadseg(Context &context) {
	::error("dontloadseg");
}

void mousecall(Context &context) {
	::error("mousecall");
}

void setmouse(Context &context) {
	::error("setmouse");
}

void gettime(Context &context) {
	::error("gettime");
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
	::error("deallocatemem");
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
	::error("disablesoundint");
}

void enablesoundint(Context &context) {
	::error("enablesoundint");
}

void checksoundint(Context &context) {
	::error("checksoundint");
}

void setsoundoff(Context &context) {
	::error("setsoundoff");
}


void loadsecondsample(Context &context) {
	::error("loadsecondsample");
}

void loadsample(Context &context) {
	::error("loadsample");
}

void loadspeech(Context &context) {
	::error("loadspeech");
}

void scanfornames(Context &context) {
	::error("scanfornames");
}

void saveseg(Context &context) {
	::error("saveseg");
}

void loadseg(Context &context) {
	::error("loadseg");
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
	::error("set16colpalette");
}

void mode640x480(Context &context) {
	::error("mode640x480");
}

void showgroup(Context &context) {
	::error("showgroup");
}

void fadedos(Context &context) {
	::error("fadedos");
}

void doshake(Context &context) {
	::error("doshake");
}

void vsync(Context &context) {
	::error("vsync");
}

void setmode(Context &context) {
	::error("setmode");
}

void readoneblock(Context &context) {
	::error("readoneblock");
}

void showpcx(Context &context) {
	::error("showpcx");
}

} /*namespace dreamgen */
