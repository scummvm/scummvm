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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/castmember.h"
#include "director/cursor.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/util.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"


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

	if (!_vm->getCurrentMovie())
		return;

	if (movie.type != VOID) {
		Common::String movieFilenameRaw = movie.asString();
		Window *stage = _vm->getCurrentWindow();

		if (!stage->setNextMovie(movieFilenameRaw))
			return;

		stage->getCurrentMovie()->getScore()->_playState = kPlayStopped;

		stage->_nextMovie.frameS.clear();
		stage->_nextMovie.frameI = -1;

		if (frame.type == VOID)
			return;

		if (frame.type == STRING) {
			stage->_nextMovie.frameS = *frame.u.s;
			return;
		}

		stage->_nextMovie.frameI = frame.asInt();

		return;
	}

	if (frame.type == VOID)
		return;

	_vm->_skipFrameAdvance = true;

	if (frame.type == STRING) {
		if (_vm->getCurrentMovie())
			_vm->getCurrentMovie()->getScore()->setStartToLabel(*frame.u.s);
		return;
	}

	if (_vm->getCurrentMovie())
		_vm->getCurrentMovie()->getScore()->setCurrentFrame(frame.asInt());
}

void Lingo::func_gotoloop() {
	if (!_vm->getCurrentMovie())
		return;

	_vm->getCurrentMovie()->getScore()->gotoLoop();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_gotonext() {
	if (!_vm->getCurrentMovie())
		return;

	_vm->getCurrentMovie()->getScore()->gotoNext();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_gotoprevious() {
	if (!_vm->getCurrentMovie())
		return;

	_vm->getCurrentMovie()->getScore()->gotoPrevious();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_play(Datum &frame, Datum &movie) {
	MovieReference ref;
	Window *stage = _vm->getCurrentWindow();


	// play #done
	if (frame.type == SYMBOL) {
		if (!frame.u.s->equals("done")) {
			warning("Lingo::func_play: unknown symbol: #%s", frame.u.s->c_str());
			return;
		}
		if (stage->_movieStack.empty()) {	// No op if no nested movies
			return;
		}
		ref = stage->_movieStack.back();

		stage->_movieStack.pop_back();

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

	if (!_vm->getCurrentMovie()) {
		warning("Lingo::func_play(): no movie");
		return;
	}

	ref.frameI = _vm->getCurrentMovie()->getScore()->getCurrentFrame();

	stage->_movieStack.push_back(ref);

	func_goto(frame, movie);
}

void Lingo::func_cursor(int cursorId, int maskId) {
	Cursor cursor;

	if (maskId == -1) {
		cursor.readFromResource(cursorId);
	} else {
		cursor.readFromCast(cursorId, maskId);
	}

	// TODO: Figure out why there are artifacts here
	_vm->_wm->replaceCursor(cursor._cursorType, ((Graphics::Cursor *)&cursor));
}

void Lingo::func_beep(int repeats) {
	for (int r = 1; r <= repeats; r++) {
		_vm->getSoundManager()->systemBeep();
		if (r < repeats)
			g_system->delayMillis(400);
	}
}

int Lingo::func_marker(int m) 	{
	if (!_vm->getCurrentMovie())
		return 0;

	int labelNumber = _vm->getCurrentMovie()->getScore()->getCurrentLabelNumber();
	if (m != 0) {
		if (m < 0) {
			for (int marker = 0; marker > m; marker--)
				labelNumber = _vm->getCurrentMovie()->getScore()->getPreviousLabelNumber(labelNumber);
		} else {
			for (int marker = 0; marker < m; marker++)
				labelNumber = _vm->getCurrentMovie()->getScore()->getNextLabelNumber(labelNumber);
		}
	}

	return labelNumber;
}

uint16 Lingo::func_label(Datum &label) {
	Score *score = _vm->getCurrentMovie()->getScore();

	if (!score->_labels)
		return 0;

	if (label.type == STRING)
		return score->getLabel(*label.u.s);

	int num = CLIP<int>(label.asInt() - 1, 0, score->_labels->size() - 1);

	uint16 res = score->getNextLabelNumber(0);

	while (--num > 0)
		res = score->getNextLabelNumber(res);

	return res;
}

} // End of namespace Director
