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

#include "sherlock/debugger.h"
#include "sherlock/sherlock.h"
#include "sherlock/music.h"

#include "sherlock/scalpel/3do/movie_decoder.h"

#include "audio/mixer.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/3do.h"

namespace Sherlock {

Debugger::Debugger(SherlockEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("continue",	     WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scene",         WRAP_METHOD(Debugger, cmdScene));
	registerCmd("3do_playmovie", WRAP_METHOD(Debugger, cmd3DO_PlayMovie));
	registerCmd("3do_playaudio", WRAP_METHOD(Debugger, cmd3DO_PlayAudio));
	registerCmd("song",          WRAP_METHOD(Debugger, cmdSong));
	registerCmd("dumpfile",      WRAP_METHOD(Debugger, cmdDumpFile));
}

void Debugger::postEnter() {
	if (!_3doPlayMovieFile.empty()) {
		Scalpel3DOMoviePlay(_3doPlayMovieFile.c_str(), Common::Point(0, 0));

		_3doPlayMovieFile.clear();
	}

	_vm->pauseEngine(false);
}

int Debugger::strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Debugger::cmdScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: scene <room>\n");
		return true;
	} else {
		_vm->_scene->_goToScene = strToInt(argv[1]);
		return false;
	}
}

bool Debugger::cmd3DO_PlayMovie(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: 3do_playmovie <3do-movie-file>\n");
		return true;
	}

	// play gets postboned until debugger is closed
	Common::String filename = argv[1];
	_3doPlayMovieFile = filename;

	return cmdExit(0, 0);
}

bool Debugger::cmd3DO_PlayAudio(int argc, const char **argv) {
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

bool Debugger::cmdSong(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: song <room>\n");
		return true;
	}

	if (!_vm->_music->loadSong(strToInt(argv[1]))) {
		debugPrintf("Invalid song number.\n");
		return true;
	}
	return false;
}

bool Debugger::cmdDumpFile(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: dumpfile <resource name>\n");
		return true;
	}

	Common::SeekableReadStream *s = _vm->_res->load(argv[1]);
	if (!s) {
		debugPrintf("Invalid resource.\n");
		return true;
	}

	byte *buffer = new byte[s->size()];
	s->read(buffer, s->size());

	Common::DumpFile dumpFile;
	dumpFile.open(argv[1]);

	dumpFile.write(buffer, s->size());
	dumpFile.flush();
	dumpFile.close();

	delete[] buffer;

	debugPrintf("Resource %s has been dumped to disk.\n", argv[1]);

	return true;
}

} // End of namespace Sherlock
