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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/scalpel/scalpel_debugger.h"
#include "sherlock/sherlock.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/aiff.h"

namespace Sherlock {

namespace Scalpel {

ScalpelDebugger::ScalpelDebugger(SherlockEngine *vm) : Debugger(vm) {
	registerCmd("3do_playmovie", WRAP_METHOD(ScalpelDebugger, cmd3DO_PlayMovie));
	registerCmd("3do_playaudio", WRAP_METHOD(ScalpelDebugger, cmd3DO_PlayAudio));
}

bool ScalpelDebugger::cmd3DO_PlayMovie(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: 3do_playmovie <3do-movie-file>\n");
		return true;
	}

	// play gets postponed until debugger is closed
	Common::String filename = argv[1];
	_3doPlayMovieFile = filename;

	return cmdExit(0, 0);
}

bool ScalpelDebugger::cmd3DO_PlayAudio(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: 3do_playaudio <3do-audio-file>\n");
		return true;
	}

	Common::File *file = new Common::File();
	if (!file->open(argv[1])) {
		debugPrintf("can not open specified audio file\n");
		return true;
	}

	Audio::AudioStream *testStream;
	Audio::SoundHandle testHandle;

	// Try to load the given file as AIFF/AIFC
	testStream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);

	if (testStream) {
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &testHandle, testStream);
		_vm->_events->clearEvents();

		while ((!_vm->shouldQuit()) && g_system->getMixer()->isSoundHandleActive(testHandle)) {
			_vm->_events->pollEvents();
			g_system->delayMillis(10);
			if (_vm->_events->kbHit()) {
				break;
			}
		}

		debugPrintf("playing completed\n");
		g_system->getMixer()->stopHandle(testHandle);
	}

	return true;
}

} // End of namespace Scalpel

} // End of namespace Sherlock
