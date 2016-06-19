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

#include "engines/director/lingo/lingo.h"
#include "common/file.h"
#include "audio/decoders/wave.h"

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

int Lingo::exec_mci(Common::String *s) {
	Common::String params[5];
	MCITokenType command = kMCITokenNone;

	s->trim();
	s->toLowercase();

	MCITokenType state = kMCITokenNone;
	Common::String token;
	const char *ptr = s->c_str();
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
				return 0;
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
				return 0;
			}

			uint32 from = strtol(params[1].c_str(), 0, 10);
			uint32 to = strtol(params[2].c_str(), 0, 10);

			_vm->getSoundManager()->playMCI(*_audioAliases[params[0]], from, to);
		}
		break;
	default:
		warning("Unhandled MCI command: %s", s->c_str());
	}

	return 0;
}

void Lingo::exec_mciwait(Common::String *s) {
	warning("MCI wait file: %s", s->c_str());
}

void Lingo::push(Datum d) {
	_stack.push_back(d);
}

Datum Lingo::pop(void) {
	if (_stack.size() == 0)
		error("stack underflow");

	Datum ret = _stack.back();
	_stack.pop_back();

	return ret;
}

void Lingo::func_xpop() {
	g_lingo->pop();
}

void Lingo::func_printtop(void) {
	Datum d = g_lingo->pop();

	warning("%d\n", d.val);
}

void Lingo::func_constpush() {
	Datum d;
	d.val = ((Symbol *)*g_lingo->_pc++)->u.val;
	g_lingo->push(d);
}

void Lingo::func_varpush() {
}

void Lingo::func_assign() {
}

void Lingo::func_eval() {
}

void Lingo::func_add() {
	Datum d1, d2;
	d2 = g_lingo->pop();
	d1 = g_lingo->pop();
	d1.val += d2.val;
	g_lingo->push(d1);
}

void Lingo::func_sub() {
	Datum d1, d2;
	d2 = g_lingo->pop();
	d1 = g_lingo->pop();
	d1.val -= d2.val;
	g_lingo->push(d1);
}

void Lingo::func_mul() {
	Datum d1, d2;
	d2 = g_lingo->pop();
	d1 = g_lingo->pop();
	d1.val *= d2.val;
	g_lingo->push(d1);
}

void Lingo::func_div() {
	Datum d1, d2;
	d2 = g_lingo->pop();

	if (d2.val == 0)
		error("division by zero");

	d1 = g_lingo->pop();
	d1.val /= d2.val;
	g_lingo->push(d1);
}

void Lingo::func_negate() {
}

void Lingo::func_mci() {
}

void Lingo::func_mciwait() {
}

}
