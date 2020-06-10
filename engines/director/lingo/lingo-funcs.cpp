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

#include "audio/decoders/wave.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/util.h"

namespace Director {

enum MCITokenType {
	kMCITokenNone,

	kMCITokenOpen,
	kMCITokenWait,
	kMCITokenPlay,

	kMCITokenType,
	kMCITokenAlias,
	kMCITokenBuffer,
	kMCITokenFrom,
	kMCITokenTo,
	kMCITokenRepeat
};

struct MCIToken {
	MCITokenType command; // Command this flag belongs to
	MCITokenType flag;
	const char *token;
	int pos;  // Position of parameter to store. 0 is always filename. Negative parameters mean boolean
} MCITokens[] = {
	{ kMCITokenNone, kMCITokenOpen,   "open", 0 },
	{ kMCITokenOpen, kMCITokenType,   "type", 1 },
	{ kMCITokenOpen, kMCITokenAlias,  "alias", 2 },
	{ kMCITokenOpen, kMCITokenBuffer, "buffer", 3 },

	{ kMCITokenNone, kMCITokenPlay,   "play", 0 },
	{ kMCITokenPlay, kMCITokenFrom,   "from", 1 },
	{ kMCITokenPlay, kMCITokenTo,     "to", 2 },
	{ kMCITokenPlay, kMCITokenRepeat, "repeat", -3 }, // This is boolean parameter

	{ kMCITokenNone, kMCITokenWait,   "wait", 0 },

	{ kMCITokenNone, kMCITokenNone,   0, 0 }
};

void Lingo::func_mci(const Common::String &name) {
	Common::String params[5];
	MCITokenType command = kMCITokenNone;

	Common::String s = name;
	s.trim();
	s.toLowercase();

	MCITokenType state = kMCITokenNone;
	Common::String token;
	const char *ptr = s.c_str();
	int respos = -1;

	while (*ptr) {
		while (*ptr && *ptr == ' ')
			ptr++;

		token.clear();

		while (*ptr && *ptr != ' ')
			token += *ptr++;

		switch (state) {
		case kMCITokenNone:
			{
				MCIToken *f = MCITokens;

				while (f->token) {
					if (command == f->command && token == f->token)
						break;

					f++;
				}

				if (command == kMCITokenNone) { // We caught command
					command = f->flag; // Switching to processing this command parameters
				} else if (f->flag == kMCITokenNone) { // Unmatched token, storing as filename
					if (!params[0].empty())
						warning("Duplicate filename in MCI command: %s -> %s", params[0].c_str(), token.c_str());
					params[0] = token;
				} else { // This is normal parameter, storing next token to designated position
					if (f->pos > 0) { // This is normal parameter
						state = f->flag;
						respos = f->pos;
					} else { // This is boolean
						params[-f->pos] = "true";
						state = kMCITokenNone;
					}
				}
				break;
			}
		default:
			params[respos] = token;
			state = kMCITokenNone;
			break;
		}
	}

	switch (command) {
	case kMCITokenOpen:
		{
			warning("MCI open file: %s, type: %s, alias: %s buffer: %s", params[0].c_str(), params[1].c_str(), params[2].c_str(), params[3].c_str());

			Common::File *file = new Common::File();

			if (!file->open(params[0])) {
				warning("Failed to open %s", params[0].c_str());
				delete file;
				return;
			}

			if (params[1] == "waveaudio") {
				Audio::AudioStream *sound = Audio::makeWAVStream(file, DisposeAfterUse::YES);
				_audioAliases[params[2]] = sound;
			} else {
				warning("Unhandled audio type %s", params[2].c_str());
			}
		}
		break;
	case kMCITokenPlay:
		{
			warning("MCI play file: %s, from: %s, to: %s, repeat: %s", params[0].c_str(), params[1].c_str(), params[2].c_str(), params[3].c_str());

			if (!_audioAliases.contains(params[0])) {
				warning("Unknown alias %s", params[0].c_str());
				return;
			}

			uint32 from = strtol(params[1].c_str(), 0, 10);
			uint32 to = strtol(params[2].c_str(), 0, 10);

			_vm->getSoundManager()->playMCI(*_audioAliases[params[0]], from, to);
		}
		break;
	default:
		warning("Unhandled MCI command: %s", s.c_str());
	}
}

void Lingo::func_mciwait(const Common::String &name) {
	warning("STUB: MCI wait file: %s", name.c_str());
}

void Lingo::func_goto(Datum &frame, Datum &movie) {
	_vm->_playbackPaused = false;

	if (!_vm->getCurrentScore())
		return;

	if (movie.type != VOID) {
		Common::String movieFilenameRaw = movie.asString();
		Common::String movieFilename = pathMakeRelative(movieFilenameRaw);
		Common::String cleanedFilename;

		bool fileExists = false;

		if (_vm->getPlatform() == Common::kPlatformMacintosh) {
			Common::MacResManager resMan;

			for (const byte *p = (const byte *)movieFilename.c_str(); *p; p++)
				if (*p >= 0x20 && *p <= 0x7f)
					cleanedFilename += (char) *p;

			if (resMan.open(movieFilename)) {
				fileExists = true;
				cleanedFilename = movieFilename;
			} else if (!movieFilename.equals(cleanedFilename) && resMan.open(cleanedFilename)) {
				fileExists = true;
			}
		} else {
			Common::File file;
			cleanedFilename = movieFilename + ".MMM";

			if (file.open(movieFilename)) {
				fileExists = true;
				cleanedFilename = movieFilename;
			} else if (!movieFilename.equals(cleanedFilename) && file.open(cleanedFilename)) {
				fileExists = true;
			}
		}

		debug(1, "func_goto: '%s' -> '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(),
				movieFilename.c_str(), cleanedFilename.c_str());

		if (!fileExists) {
			warning("Movie %s does not exist", movieFilename.c_str());
			return;
		}

		_vm->_nextMovie.movie = cleanedFilename;
		_vm->getCurrentScore()->_stopPlay = true;

		_vm->_nextMovie.frameS.clear();
		_vm->_nextMovie.frameI = -1;

		if (frame.type == VOID)
			return;

		if (frame.type == STRING) {
			_vm->_nextMovie.frameS = *frame.u.s;
			return;
		}

		_vm->_nextMovie.frameI = frame.asInt();

		return;
	}

	if (frame.type == VOID)
		return;

	_vm->_skipFrameAdvance = true;

	if (frame.type == STRING) {
		if (_vm->getCurrentScore())
			_vm->getCurrentScore()->setStartToLabel(*frame.u.s);
		return;
	}

	if (_vm->getCurrentScore())
		_vm->getCurrentScore()->setCurrentFrame(frame.asInt());
}

void Lingo::func_gotoloop() {
	if (!_vm->getCurrentScore())
		return;

	_vm->getCurrentScore()->gotoLoop();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_gotonext() {
	if (!_vm->getCurrentScore())
		return;

	_vm->getCurrentScore()->gotoNext();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_gotoprevious() {
	if (!_vm->getCurrentScore())
		return;

	_vm->getCurrentScore()->gotoPrevious();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_play(Datum &frame, Datum &movie) {
	MovieReference ref;

	// play #done
	if (frame.type == SYMBOL) {
		if (!frame.u.s->equals("done")) {
			warning("Lingo::func_play: unknown symbol: #%s", frame.u.s->c_str());
			return;
		}
		if (_vm->_movieStack.empty()) {	// No op if no nested movies
			return;
		}
		ref = _vm->_movieStack.back();

		_vm->_movieStack.pop_back();

		Datum m, f;

		if (ref.movie.empty()) {
			m.type = VOID;
		} else {
			m.type = STRING;
			m.u.s = new Common::String(ref.movie);
		}

		f.type = INT;
		f.u.i = ref.frameI;

		func_goto(f, m);

		return;
	}

	if (!_vm->getCurrentScore()) {
		warning("Lingo::func_play(): no score");
		return;
	}

	ref.frameI = _vm->getCurrentScore()->getCurrentFrame();

	_vm->_movieStack.push_back(ref);

	func_goto(frame, movie);
}

void Lingo::func_cursor(int cursorId, int maskId) {
	if (_cursorOnStack) {
		// pop cursor
		_vm->getMacWindowManager()->popCursor();
	}

	if (maskId != -1) {
		Cast *cursorCast = _vm->getCastMember(cursorId);
		Cast *maskCast = _vm->getCastMember(maskId);
		if (!cursorCast || !maskCast) {
			warning("func_cursor(): non-existent cast reference");
			return;
		}

		if (cursorCast->_type != kCastBitmap || maskCast->_type != kCastBitmap) {
			warning("func_cursor(): wrong cast reference type");
			return;
		}

		byte *assembly = (byte *)malloc(16 * 16);
		byte *dst = assembly;

		for (int y = 0; y < 16; y++) {
			const byte *cursor = nullptr, *mask = nullptr;

			if (y < cursorCast->_widget->getSurface()->h &&
					y < maskCast->_widget->getSurface()->h) {
				cursor = (const byte *)cursorCast->_widget->getSurface()->getBasePtr(0, y);
				mask = (const byte *)maskCast->_widget->getSurface()->getBasePtr(0, y);
			}

			for (int x = 0; x < 16; x++) {
				if (x >= cursorCast->_widget->getSurface()->w ||
						x >= maskCast->_widget->getSurface()->w) {
					cursor = mask = nullptr;
				}

				if (!cursor) {
					*dst = 3;
				} else {
					*dst = *mask ? 3 : (*cursor ? 1 : 0);
					cursor++;
					mask++;
				}
				dst++;
			}
		}

		warning("STUB: func_cursor(): Hotspot is the registration point of the cast member");
		_vm->getMacWindowManager()->pushCustomCursor(assembly, 16, 16, 1, 1, 3);

		free(assembly);

		return;
	}

	// and then push cursor.
	switch (cursorId) {
	case 0:
	case -1:
	default:
		_vm->getMacWindowManager()->pushArrowCursor();
		break;
	case 1:
		_vm->getMacWindowManager()->pushBeamCursor();
		break;
	case 2:
		_vm->getMacWindowManager()->pushCrossHairCursor();
		break;
	case 3:
		_vm->getMacWindowManager()->pushCrossBarCursor();
		break;
	case 4:
		_vm->getMacWindowManager()->pushWatchCursor();
		break;
	}

	_cursorOnStack = true;
}

void Lingo::func_beep(int repeats) {
	for (int r = 1; r <= repeats; r++) {
		_vm->getSoundManager()->systemBeep();
		if (r < repeats)
			g_system->delayMillis(400);
	}
}

int Lingo::func_marker(int m) 	{
	if (!_vm->getCurrentScore())
		return 0;

	int labelNumber = _vm->getCurrentScore()->getCurrentLabelNumber();
	if (m != 0) {
		if (m < 0) {
			for (int marker = 0; marker > m; marker--)
				labelNumber = _vm->getCurrentScore()->getPreviousLabelNumber(labelNumber);
		} else {
			for (int marker = 0; marker < m; marker++)
				labelNumber = _vm->getCurrentScore()->getNextLabelNumber(labelNumber);
		}
	}

	return labelNumber;
}

}
