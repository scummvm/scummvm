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

#include "glk/scott/scott.h"
#include "glk/quetzal.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/ustr.h"

namespace Glk {
namespace Scott {

Scott::Scott(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		_currentCounter(0), _savedRoom(0), _options(0), _width(0), _topHeight(0), _splitScreen(true),
		_bottomWindow(0), _topWindow(0), _bitFlags(0), _saveSlot(-1) {
	Common::fill(&_nounText[0], &_nounText[16], '\0');
	Common::fill(&_counters[0], &_counters[16], 0);
	Common::fill(&_roomSaved[0], &_roomSaved[16], 0);
}

void Scott::runGame() {
	int vb, no;
	initialize();

	_bottomWindow = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	if (_bottomWindow == nullptr) {
		glk_exit();
		return;
	}
	glk_set_window(_bottomWindow);

	if (_options & TRS80_STYLE) {
		_width = 64;
		_topHeight = 11;
	} else {
		_width = 80;
		_topHeight = 10;
	}

	if (_splitScreen) {
		_topWindow = glk_window_open(_bottomWindow, winmethod_Above | winmethod_Fixed, _topHeight, wintype_TextGrid, 0);
		if (_topWindow == nullptr) {
			_splitScreen = 0;
			_topWindow = _bottomWindow;
		}
	} else {
		_topWindow = _bottomWindow;
	}

	output("ScummVM support adapted from Scott Free, A Scott Adams game driver in C.\n\n");

	// Check for savegame
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	// Load the game
	loadDatabase(&_gameFile, (_options & DEBUGGING) ? 1 : 0);

	// Main game loop
	while (!shouldQuit()) {
		glk_tick();

		performActions(0, 0);
		if (shouldQuit())
			break;

		if (_saveSlot >= 0) {
			// Load any savegame during startup
			loadGameState(_saveSlot);
			_saveSlot = -1;
		}

		look();

		if (getInput(&vb, &no) == -1)
			continue;
		if (g_vm->shouldQuit())
			break;

		switch (performActions(vb, no)) {
		case -1:
			output(_("I don't understand your command. "));
			break;
		case -2:
			output(_("I can't do that yet. "));
			break;
		default:
			break;
		}
		if (shouldQuit())
			return;

		// Brian Howarth games seem to use -1 for forever
		if (_items[LIGHT_SOURCE]._location != DESTROYED && _gameHeader._lightTime != -1) {
			_gameHeader._lightTime--;
			if (_gameHeader._lightTime < 1) {
				_bitFlags |= (1 << LIGHTOUTBIT);
				if (_items[LIGHT_SOURCE]._location == CARRIED ||
						_items[LIGHT_SOURCE]._location == MY_LOC) {
					if (_options & SCOTTLIGHT)
						output(_("Light has run out! "));
					else
						output(_("Your light has run out. "));
				}
				if (_options & PREHISTORIC_LAMP)
					_items[LIGHT_SOURCE]._location = DESTROYED;
			} else if (_gameHeader._lightTime < 25) {
				if (_items[LIGHT_SOURCE]._location == CARRIED ||
						_items[LIGHT_SOURCE]._location == MY_LOC) {

					if (_options & SCOTTLIGHT) {
						output(_("Light runs out in "));
						outputNumber(_gameHeader._lightTime);
						output(_(" turns. "));
					} else {
						if (_gameHeader._lightTime % 5 == 0)
							output(_("Your light is growing dim. "));
					}
				}
			}
		}
	}
}

void Scott::initialize() {
	if (ConfMan.hasKey("YOUARE")) {
		if (ConfMan.getBool("YOUARE"))
			_options |= YOUARE;
		else
			_options &= ~YOUARE;
	}
	if (gDebugLevel > 0)
		_options |= DEBUGGING;
	if (ConfMan.hasKey("SCOTTLIGHT") && ConfMan.getBool("SCOTTLIGHT"))
		_options |= SCOTTLIGHT;
	if (ConfMan.hasKey("TRS80_STYLE") && ConfMan.getBool("TRS80_STYLE"))
		_options |= TRS80_STYLE;
	if (ConfMan.hasKey("PREHISTORIC_LAMP") && ConfMan.getBool("PREHISTORIC_LAMP"))
		_options |= PREHISTORIC_LAMP;
}

void Scott::display(winid_t w, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	Common::String msg = Common::String::vformat(fmt, ap);
	va_end(ap);

	glk_put_string_stream(glk_window_get_stream(w), msg.c_str());
}

void Scott::display(winid_t w, const Common::U32String fmt, ...) {
	Common::U32String msg;

	va_list ap;

	va_start(ap, fmt);
	Common::U32String::vformat(fmt.begin(), fmt.end(), msg, ap);
	va_end(ap);

	glk_put_string_stream_uni(glk_window_get_stream(w), msg.u32_str());
}

void Scott::delay(int seconds) {
	event_t ev;

	if (!glk_gestalt(gestalt_Timer, 0))
		return;

	glk_request_timer_events(1000 * seconds);

	do {
		glk_select(&ev);
	} while (ev.type != evtype_Timer && ev.type != evtype_Quit);

	glk_request_timer_events(0);
}

void Scott::fatal(const char *x) {
	error("%s", x);
}

void Scott::clearScreen(void) {
	glk_window_clear(_bottomWindow);
}

bool Scott::randomPercent(uint n) {
	return _random.getRandomNumber(99) < n;
}

int Scott::countCarried(void) {
	int ct = 0;
	int n = 0;
	while (ct <= _gameHeader._numItems) {
		if (_items[ct]._location == CARRIED)
			n++;
		ct++;
	}
	return n;
}

const char *Scott::mapSynonym(const char *word) {
	int n = 1;
	const char *tp;
	static char lastword[16];   // Last non synonym
	while (n <= _gameHeader._numWords) {
		tp = _nouns[n].c_str();
		if (*tp == '*')
			tp++;
		else
			strcpy(lastword, tp);
		if (scumm_strnicmp(word, tp, _gameHeader._wordLength) == 0)
			return lastword;
		n++;
	}
	return nullptr;
}

int Scott::matchUpItem(const char *text, int loc) {
	const char *word = mapSynonym(text);
	int ct = 0;

	if (word == nullptr)
		word = text;

	while (ct <= _gameHeader._numItems) {
		if (!_items[ct]._autoGet.empty() && _items[ct]._location == loc &&
				scumm_strnicmp(_items[ct]._autoGet.c_str(), word, _gameHeader._wordLength) == 0)
			return ct;
		ct++;
	}

	return -1;
}

Common::String Scott::readString(Common::SeekableReadStream *f) {
	char tmp[1024];
	int c, nc;
	int ct = 0;
	do {
		c = f->readByte();
	} while (f->pos() < f->size() && Common::isSpace(c));
	if (c != '"') {
		fatal("Initial quote expected");
	}

	for (;;) {
		if (f->pos() >= f->size())
			fatal("EOF in string");

		c = f->readByte();
		if (c == '"') {
			nc = f->readByte();
			if (nc != '"') {
				f->seek(-1, SEEK_CUR);
				break;
			}
		}
		if (c == '`')
			c = '"'; // pdd

		// Ensure a valid Glk newline is sent.
		if (c == '\n')
			tmp[ct++] = 10;
		// Special case: assume CR is part of CRLF in a DOS-formatted file, and ignore it.
		else if (c == 13)
			;
		// Pass only ASCII to Glk; the other reasonable option would be to pass Latin-1,
		// but it's probably safe to assume that Scott Adams games are ASCII only.
		else if ((c >= 32 && c <= 126))
			tmp[ct++] = c;
		else
			tmp[ct++] = '?';
	}

	tmp[ct] = 0;
	return Common::String(tmp);
}

void Scott::loadDatabase(Common::SeekableReadStream *f, bool loud) {
	int unused, ni, na, nw, nr, mc, pr, tr, wl, lt, mn, trm;
	int ct;
	int lo;

	// Load the header
	readInts(f, 12, &unused, &ni, &na, &nw, &nr, &mc, &pr, &tr, &wl, &lt, &mn, &trm);

	_gameHeader._numItems = ni;
	_items.resize(ni + 1);
	_gameHeader._numActions = na;
	_actions.resize(na + 1);
	_gameHeader._numWords = nw;
	_gameHeader._wordLength = wl;
	_verbs.resize(nw + 1);
	_nouns.resize(nw + 1);
	_gameHeader._numRooms = nr;
	_rooms.resize(nr + 1);
	_gameHeader._maxCarry = mc;
	_gameHeader._playerRoom = pr;
	_gameHeader._treasures = tr;
	_gameHeader._lightTime = lt;
	_lightRefill = lt;
	_gameHeader._numMessages = mn;
	_messages.resize(mn + 1);
	_gameHeader._treasureRoom = trm;

	// Load the actions
	if (loud)
		debug("Reading %d actions.", na);

	for (int idx = 0; idx < na + 1; ++idx) {
		Action &a = _actions[idx];
		readInts(f, 8,
			&a._vocab, &a._condition[0], &a._condition[1], &a._condition[2],
			&a._condition[3], &a._condition[4], &a._action[0], &a._action[1]);
	}

	if (loud)
		debug("Reading %d word pairs.", nw);
	for (int idx = 0; idx < nw + 1; ++idx) {
		_verbs[idx] = readString(f);
		_nouns[idx] = readString(f);
	}

	if (loud)
		debug("Reading %d rooms.", nr);
	for (int idx = 0; idx < nr + 1; ++idx) {
		Room &r = _rooms[idx];
		readInts(f, 6, &r._exits[0], &r._exits[1], &r._exits[2],
				 &r._exits[3], &r._exits[4], &r._exits[5]);
		r._text =  readString(f);
	}

	if (loud)
		debug("Reading %d messages.", mn);
	for (int idx = 0; idx < mn + 1; ++idx)
		_messages[idx] = readString(f);

	if (loud)
		debug("Reading %d items.", ni);
	for (int idx = 0; idx < ni + 1; ++idx) {
		Item &i = _items[idx];
		i._text = readString(f);

		const char *p = strchr(i._text.c_str(), '/');
		if (p) {
			i._autoGet = Common::String(p);

			// Some games use // to mean no auto get/drop word!
			if (!i._autoGet.hasPrefix("//") && !i._autoGet.hasPrefix("/*")) {
				i._text = Common::String(i._text.c_str(), p);
				i._autoGet.deleteChar(0);

				const char *t = strchr(i._autoGet.c_str(), '/');
				if (t)
					i._autoGet = Common::String(i._autoGet.c_str(), t);
			}
		}

		readInts(f, 1, &lo);
		i._location = (unsigned char)lo;
		i._initialLoc = i._location;
	}

	// Skip Comment Strings
	for (int idx = 0; idx < na + 1; ++idx)
		readString(f);

	readInts(f, 1, &ct);
	if (loud)
		debug("Version %d.%02d of Adventure ", ct / 100, ct % 100);
	readInts(f, 1, &ct);

	if (loud)
		debug("%d.\nLoad Complete.\n", ct);
}

void Scott::output(const Common::String &a) {
	if (_saveSlot == -1)
		display(_bottomWindow, "%s", a.c_str());
}

void Scott::output(const Common::U32String &a) {
	if (_saveSlot == -1)
		display(_bottomWindow, Common::U32String("%S"), a.c_str());
}

void Scott::outputNumber(int a) {
	display(_bottomWindow, "%d", a);
}

void Scott::look(void) {
	const char *const ExitNames[6] = {
		_s("North"), _s("South"), _s("East"), _s("West"), _s("Up"), _s("Down")
	};
	Room *r;
	int ct, f;
	int pos;

	if (_splitScreen)
		glk_window_clear(_topWindow);

	if ((_bitFlags & (1 << DARKBIT)) && _items[LIGHT_SOURCE]._location != CARRIED
			&& _items[LIGHT_SOURCE]._location != MY_LOC) {
		if (_options & YOUARE)
			display(_topWindow, _("You can't see. It is too dark!\n"));
		else
			display(_topWindow, _("I can't see. It is too dark!\n"));
		if (_options & TRS80_STYLE)
			display(_topWindow, TRS80_LINE);
		return;
	}
	r = &_rooms[MY_LOC];
	if (r->_text.hasPrefix("*"))
		display(_topWindow, "%s\n", r->_text.c_str() + 1);
	else {
		if (_options & YOUARE)
			display(_topWindow, _("You are in a %s\n"), r->_text.c_str());
		else
			display(_topWindow, _("I'm in a %s\n"), r->_text.c_str());
	}

	ct = 0;
	f = 0;
	display(_topWindow, _("\nObvious exits: "));
	while (ct < 6) {
		if (r->_exits[ct] != 0) {
			if (f == 0)
				f = 1;
			else
				display(_topWindow, ", ");
			display(_topWindow, Common::U32String("%S"), _(ExitNames[ct]).c_str());
		}
		ct++;
	}

	if (f == 0)
		display(_topWindow, _("none"));
	display(_topWindow, ".\n");
	ct = 0;
	f = 0;
	pos = 0;
	while (ct <= _gameHeader._numItems) {
		if (_items[ct]._location == MY_LOC) {
			if (f == 0) {
				if (_options & YOUARE) {
					display(_topWindow, _("\nYou can also see: "));
					pos = 18;
				} else {
					display(_topWindow, _("\nI can also see: "));
					pos = 16;
				}
				f++;
			} else if (!(_options & TRS80_STYLE)) {
				display(_topWindow, " - ");
				pos += 3;
			}
			if (pos + (int)_items[ct]._text.size() > (_width - 10)) {
				pos = 0;
				display(_topWindow, "\n");
			}
			display(_topWindow, "%s", _items[ct]._text.c_str());
			pos += _items[ct]._text.size();
			if (_options & TRS80_STYLE) {
				display(_topWindow, ". ");
				pos += 2;
			}
		}
		ct++;
	}

	display(_topWindow, "\n");
	if (_options & TRS80_STYLE)
		display(_topWindow, TRS80_LINE);
}

int Scott::whichWord(const char *word, const Common::StringArray &list) {
	int n = 1;
	int ne = 1;
	const char *tp;
	while (ne <= _gameHeader._numWords) {
		tp = list[ne].c_str();
		if (*tp == '*')
			tp++;
		else
			n = ne;
		if (scumm_strnicmp(word, tp, _gameHeader._wordLength) == 0)
			return n;
		ne++;
	}
	return -1;
}

void Scott::lineInput(char *buf, size_t n) {
	event_t ev;

	glk_request_line_event(_bottomWindow, buf, n - 1, 0);

	do {
		glk_select(&ev);
		if (ev.type == evtype_Quit)
			return;
		else if (ev.type == evtype_LineInput)
			break;
		else if (ev.type == evtype_Arrange && _splitScreen)
			look();
	} while (ev.type != evtype_Quit);

	buf[ev.val1] = 0;
}

Common::Error Scott::writeGameData(Common::WriteStream *ws) {
	Common::String msg;

	for (int ct = 0; ct < 16; ct++) {
		msg = Common::String::format("%d %d\n", _counters[ct], _roomSaved[ct]);
		ws->write(msg.c_str(), msg.size());
		ws->writeByte(0);
	}

	msg = Common::String::format("%u %d %d %d %d %d\n",
								 _bitFlags, (_bitFlags & (1 << DARKBIT)) ? 1 : 0,
								 MY_LOC, _currentCounter, _savedRoom, _gameHeader._lightTime);
	ws->write(msg.c_str(), msg.size());
	ws->writeByte(0);

	for (int ct = 0; ct <= _gameHeader._numItems; ct++) {
		msg = Common::String::format("%hd\n", (short)_items[ct]._location);
		ws->write(msg.c_str(), msg.size());
		ws->writeByte(0);
	}

	output(_("Saved.\n"));
	return Common::kNoError;
}

Common::Error Scott::readSaveData(Common::SeekableReadStream *rs) {
	Common::String line;
	int ct = 0;
	short lo;
	short darkFlag;

	for (ct = 0; ct < 16; ct++) {
		line = QuetzalReader::readString(rs);
		sscanf(line.c_str(), "%d %d", &_counters[ct], &_roomSaved[ct]);
	}

	line = QuetzalReader::readString(rs);
	sscanf(line.c_str(), "%u %hd %d %d %d %d\n",
		   &_bitFlags, &darkFlag, &MY_LOC, &_currentCounter, &_savedRoom,
		   &_gameHeader._lightTime);

	// Backward compatibility
	if (darkFlag)
		_bitFlags |= (1 << 15);
	for (ct = 0; ct <= _gameHeader._numItems; ct++) {
		line = QuetzalReader::readString(rs);
		sscanf(line.c_str(), "%hd\n", &lo);
		_items[ct]._location = (unsigned char)lo;
	}

	return Common::kNoError;
}

int Scott::getInput(int *vb, int *no) {
	char buf[256];
	char verb[10], noun[10];
	int vc, nc;
	int num;

	do {
		do {
			output("\nTell me what to do ? ");
			lineInput(buf, sizeof buf);
			if (g_vm->shouldQuit())
				return 0;

			num = sscanf(buf, "%9s %9s", verb, noun);
		} while (num == 0 || *buf == '\n');

		if (scumm_stricmp(verb, "restore") == 0) {
			loadGame();
			return -1;
		}
		if (num == 1)
			*noun = 0;
		if (*noun == 0 && strlen(verb) == 1) {
			switch (Common::isUpper((unsigned char)*verb) ? tolower((unsigned char)*verb) : *verb) {
			case 'n':
				strcpy(verb, "NORTH");
				break;
			case 'e':
				strcpy(verb, "EAST");
				break;
			case 's':
				strcpy(verb, "SOUTH");
				break;
			case 'w':
				strcpy(verb, "WEST");
				break;
			case 'u':
				strcpy(verb, "UP");
				break;
			case 'd':
				strcpy(verb, "DOWN");
				break;
			// Brian Howarth interpreter also supports this
			case 'i':
				strcpy(verb, "INVENTORY");
				break;
			default:
				break;
			}
		}
		nc = whichWord(verb, _nouns);
		// The Scott Adams system has a hack to avoid typing 'go'
		if (nc >= 1 && nc <= 6) {
			vc = 1;
		} else {
			vc = whichWord(verb, _verbs);
			nc = whichWord(noun, _nouns);
		}
		*vb = vc;
		*no = nc;
		if (vc == -1) {
			output(_("You use word(s) I don't know! "));
		}
	} while (vc == -1);

	strcpy(_nounText, noun); // Needed by GET/DROP hack
	return 0;
}

int Scott::performLine(int ct) {
	int continuation = 0;
	int param[5], pptr = 0;
	int act[4];
	int cc = 0;

	while (cc < 5) {
		int cv, dv;
		cv = _actions[ct]._condition[cc];
		dv = cv / 20;
		cv %= 20;
		switch (cv) {
		case 0:
			param[pptr++] = dv;
			break;
		case 1:
			if (_items[dv]._location != CARRIED)
				return 0;
			break;
		case 2:
			if (_items[dv]._location != MY_LOC)
				return 0;
			break;
		case 3:
			if (_items[dv]._location != CARRIED &&
					_items[dv]._location != MY_LOC)
				return 0;
			break;
		case 4:
			if (MY_LOC != dv)
				return 0;
			break;
		case 5:
			if (_items[dv]._location == MY_LOC)
				return 0;
			break;
		case 6:
			if (_items[dv]._location == CARRIED)
				return 0;
			break;
		case 7:
			if (MY_LOC == dv)
				return 0;
			break;
		case 8:
			if ((_bitFlags & (1 << dv)) == 0)
				return 0;
			break;
		case 9:
			if (_bitFlags & (1 << dv))
				return 0;
			break;
		case 10:
			if (countCarried() == 0)
				return 0;
			break;
		case 11:
			if (countCarried())
				return 0;
			break;
		case 12:
			if (_items[dv]._location == CARRIED || _items[dv]._location == MY_LOC)
				return 0;
			break;
		case 13:
			if (_items[dv]._location == 0)
				return 0;
			break;
		case 14:
			if (_items[dv]._location)
				return 0;
			break;
		case 15:
			if (_currentCounter > dv)
				return 0;
			break;
		case 16:
			if (_currentCounter <= dv)
				return 0;
			break;
		case 17:
			if (_items[dv]._location != _items[dv]._initialLoc)
				return 0;
			break;
		case 18:
			if (_items[dv]._location == _items[dv]._initialLoc)
				return 0;
			break;
		case 19:
			// Only seen in Brian Howarth games so far
			if (_currentCounter != dv)
				return 0;
			break;
		default:
			break;
		}
		cc++;
	}

	// _actions
	act[0] = _actions[ct]._action[0];
	act[2] = _actions[ct]._action[1];
	act[1] = act[0] % 150;
	act[3] = act[2] % 150;
	act[0] /= 150;
	act[2] /= 150;
	cc = 0;
	pptr = 0;
	while (cc < 4) {
		if (act[cc] >= 1 && act[cc] < 52) {
			output(_messages[act[cc]]);
			output("\n");
		} else if (act[cc] > 101) {
			output(_messages[act[cc] - 50]);
			output("\n");
		} else {
			switch (act[cc]) {
			case 0:// NOP
				break;
			case 52:
				if (countCarried() == _gameHeader._maxCarry) {
					if (_options & YOUARE)
						output(_("You are carrying too much. "));
					else
						output(_("I've too much to carry! "));
					break;
				}
				_items[param[pptr++]]._location = CARRIED;
				break;
			case 53:
				_items[param[pptr++]]._location = MY_LOC;
				break;
			case 54:
				MY_LOC = param[pptr++];
				break;
			case 55:
				_items[param[pptr++]]._location = 0;
				break;
			case 56:
				_bitFlags |= 1 << DARKBIT;
				break;
			case 57:
				_bitFlags &= ~(1 << DARKBIT);
				break;
			case 58:
				_bitFlags |= (1 << param[pptr++]);
				break;
			case 59:
				_items[param[pptr++]]._location = 0;
				break;
			case 60:
				_bitFlags &= ~(1 << param[pptr++]);
				break;
			case 61:
				if (_options & YOUARE)
					output(_("You are dead.\n"));
				else
					output(_("I am dead.\n"));
				_bitFlags &= ~(1 << DARKBIT);
				MY_LOC = _gameHeader._numRooms;// It seems to be what the code says!
				break;
			case 62: {
				// Bug fix for some systems - before it could get parameters wrong */
				int i = param[pptr++];
				_items[i]._location = param[pptr++];
				break;
			}
			case 63:
doneit:
				output(_("The game is now over.\n"));
				glk_exit();
				return 0;
			case 64:
				break;
			case 65: {
				int i = 0;
				int n = 0;
				while (i <= _gameHeader._numItems) {
					if (_items[i]._location == _gameHeader._treasureRoom &&
							_items[i]._text.hasPrefix("*"))
						n++;
					i++;
				}
				if (_options & YOUARE)
					output(_("You have stored "));
				else
					output(_("I've stored "));
				outputNumber(n);
				output(_(" treasures.  On a scale of 0 to 100, that rates "));
				outputNumber((n * 100) / _gameHeader._treasures);
				output(".\n");
				if (n == _gameHeader._treasures) {
					output(_("Well done.\n"));
					goto doneit;
				}
				break;
			}
			case 66: {
				int i = 0;
				int f = 0;
				if (_options & YOUARE)
					output(_("You are carrying:\n"));
				else
					output(_("I'm carrying:\n"));
				while (i <= _gameHeader._numItems) {
					if (_items[i]._location == CARRIED) {
						if (f == 1) {
							if (_options & TRS80_STYLE)
								output(". ");
							else
								output(" - ");
						}
						f = 1;
						output(_items[i]._text);
					}
					i++;
				}
				if (f == 0)
					output(_("Nothing"));
				output(".\n");
				break;
			}
			case 67:
				_bitFlags |= (1 << 0);
				break;
			case 68:
				_bitFlags &= ~(1 << 0);
				break;
			case 69:
				_gameHeader._lightTime = _lightRefill;
				_items[LIGHT_SOURCE]._location = CARRIED;
				_bitFlags &= ~(1 << LIGHTOUTBIT);
				break;
			case 70:
				clearScreen(); // pdd.
				break;
			case 71:
				saveGame();
				break;
			case 72: {
				int i1 = param[pptr++];
				int i2 = param[pptr++];
				int t = _items[i1]._location;
				_items[i1]._location = _items[i2]._location;
				_items[i2]._location = t;
				break;
			}
			case 73:
				continuation = 1;
				break;
			case 74:
				_items[param[pptr++]]._location = CARRIED;
				break;
			case 75: {
				int i1, i2;
				i1 = param[pptr++];
				i2 = param[pptr++];
				_items[i1]._location = _items[i2]._location;
				break;
			}
			case 76:
				// Looking at adventure ..
				break;
			case 77:
				if (_currentCounter >= 0)
					_currentCounter--;
				break;
			case 78:
				outputNumber(_currentCounter);
				break;
			case 79:
				_currentCounter = param[pptr++];
				break;
			case 80: {
				int t = MY_LOC;
				MY_LOC = _savedRoom;
				_savedRoom = t;
				break;
			}
			case 81: {
				// This is somewhat guessed. Claymorgue always seems to do
				// select counter n, thing, select counter n, but uses one value that always
				// seems to exist. Trying a few options I found this gave sane results on ageing
				int t = param[pptr++];
				int c1 = _currentCounter;
				_currentCounter = _counters[t];
				_counters[t] = c1;
				break;
			}
			case 82:
				_currentCounter += param[pptr++];
				break;
			case 83:
				_currentCounter -= param[pptr++];
				if (_currentCounter < -1)
					_currentCounter = -1;
				// Note: This seems to be needed. I don't yet know if there
				// is a maximum value to limit too
				break;
			case 84:
				output(_nounText);
				break;
			case 85:
				output(_nounText);
				output("\n");
				break;
			case 86:
				output("\n");
				break;
			case 87: {
				// Changed this to swap location<->roomflag[x] not roomflag 0 and x
				int p = param[pptr++];
				int sr = MY_LOC;
				MY_LOC = _roomSaved[p];
				_roomSaved[p] = sr;
				break;
			}
			case 88:
				delay(2);
				break;
			case 89:
				pptr++;
				// SAGA draw picture n
				// Spectrum Seas of Blood - start combat ?
				// Poking this into older spectrum games causes a crash
				break;
			default:
				error("Unknown action %d [Param begins %d %d]\n",
					  act[cc], param[pptr], param[pptr + 1]);
				break;
			}
		}

		cc++;
	}

	return 1 + continuation;
}

int Scott::performActions(int vb, int no) {
	static bool disableSysFunc = false; // Recursion lock
	int d = _bitFlags & (1 << DARKBIT);

	int ct = 0;
	int fl;
	int doagain = 0;
	if (vb == 1 && no == -1) {
		output(_("Give me a direction too."));
		return 0;
	}
	if (vb == 1 && no >= 1 && no <= 6) {
		int nl;
		if (_items[LIGHT_SOURCE]._location == MY_LOC ||
				_items[LIGHT_SOURCE]._location == CARRIED)
			d = 0;
		if (d)
			output(_("Dangerous to move in the dark! "));
		nl = _rooms[MY_LOC]._exits[no - 1];
		if (nl != 0) {
			MY_LOC = nl;
			return 0;
		}
		if (d) {
			if (_options & YOUARE)
				output(_("You fell down and broke your neck. "));
			else
				output(_("I fell down and broke my neck. "));
			glk_exit();
			return 0;
		}
		if (_options & YOUARE)
			output(_("You can't go in that direction. "));
		else
			output(_("I can't go in that direction. "));
		return 0;
	}

	fl = -1;
	while (ct <= _gameHeader._numActions) {
		int vv, nv;
		vv = _actions[ct]._vocab;
		// Think this is now right. If a line we run has an action73
		// run all following lines with vocab of 0,0
		if (vb != 0 && (doagain && vv != 0))
			break;
		// Oops.. added this minor cockup fix 1.11
		if (vb != 0 && !doagain && fl == 0)
			break;
		nv = vv % 150;
		vv /= 150;
		if ((vv == vb) || (doagain && _actions[ct]._vocab == 0)) {
			if ((vv == 0 && randomPercent(nv)) || doagain ||
					(vv != 0 && (nv == no || nv == 0))) {
				int f2;
				if (fl == -1)
					fl = -2;
				if ((f2 = performLine(ct)) > 0) {
					// ahah finally figured it out !
					fl = 0;
					if (f2 == 2)
						doagain = 1;
					if (vb != 0 && doagain == 0)
						return 0;
				}

				if (shouldQuit())
					return 0;
			}
		}
		ct++;

		// Previously this did not check ct against _gameHeader._numActions and would read
		// past the end of _actions.  I don't know what should happen on the last action,
		// but doing nothing is better than reading one past the end.
		// --Chris
		if (ct <= _gameHeader._numActions && _actions[ct]._vocab != 0)
			doagain = 0;
	}
	if (fl != 0 && disableSysFunc == 0) {
		int item;
		if (_items[LIGHT_SOURCE]._location == MY_LOC ||
				_items[LIGHT_SOURCE]._location == CARRIED)
			d = 0;
		if (vb == 10 || vb == 18) {
			// Yes they really _are_ hardcoded values
			if (vb == 10) {
				if (scumm_stricmp(_nounText, "ALL") == 0) {
					int i = 0;
					int f = 0;

					if (d) {
						output(_("It is dark.\n"));
						return 0;
					}
					while (i <= _gameHeader._numItems) {
						if (_items[i]._location == MY_LOC && _items[i]._autoGet != nullptr && _items[i]._autoGet[0] != '*') {
							no = whichWord(_items[i]._autoGet.c_str(), _nouns);
							disableSysFunc = true;    // Don't recurse into auto get !
							performActions(vb, no);   // Recursively check each items table code
							disableSysFunc = false;
							if (shouldQuit())
								return 0;

							if (countCarried() == _gameHeader._maxCarry) {
								if (_options & YOUARE)
									output(_("You are carrying too much. "));
								else
									output(_("I've too much to carry. "));
								return 0;
							}
							_items[i]._location = CARRIED;
							output(_items[i]._text);
							output(_(": O.K.\n"));
							f = 1;
						}
						i++;
					}
					if (f == 0)
						output(_("Nothing taken."));
					return 0;
				}
				if (no == -1) {
					output(_("What ? "));
					return 0;
				}
				if (countCarried() == _gameHeader._maxCarry) {
					if (_options & YOUARE)
						output(_("You are carrying too much. "));
					else
						output(_("I've too much to carry. "));
					return 0;
				}
				item = matchUpItem(_nounText, MY_LOC);
				if (item == -1) {
					if (_options & YOUARE)
						output(_("It is beyond your power to do that. "));
					else
						output(_("It's beyond my power to do that. "));
					return 0;
				}
				_items[item]._location = CARRIED;
				output(_("O.K. "));
				return 0;
			}
			if (vb == 18) {
				if (scumm_stricmp(_nounText, "ALL") == 0) {
					int i = 0;
					int f = 0;
					while (i <= _gameHeader._numItems) {
						if (_items[i]._location == CARRIED && !_items[i]._autoGet.empty()
								&& !_items[i]._autoGet.hasPrefix("*")) {
							no = whichWord(_items[i]._autoGet.c_str(), _nouns);
							disableSysFunc = true;
							performActions(vb, no);
							disableSysFunc = false;
							if (shouldQuit())
								return 0;

							_items[i]._location = MY_LOC;
							output(_items[i]._text);
							output(_(": O.K.\n"));
							f = 1;
						}
						i++;
					}
					if (f == 0)
						output(_("Nothing dropped.\n"));
					return 0;
				}
				if (no == -1) {
					output(_("What ? "));
					return 0;
				}
				item = matchUpItem(_nounText, CARRIED);
				if (item == -1) {
					if (_options & YOUARE)
						output(_("It's beyond your power to do that.\n"));
					else
						output(_("It's beyond my power to do that.\n"));
					return 0;
				}
				_items[item]._location = MY_LOC;
				output("O.K. ");
				return 0;
			}
		}
	}

	return fl;
}

void Scott::readInts(Common::SeekableReadStream *f, size_t count, ...) {
	va_list va;
	va_start(va, count);
	unsigned char c = f->readByte();

	for (size_t idx = 0; idx < count; ++idx) {
		while (f->pos() < f->size() && Common::isSpace(c))
			c = f->readByte();

		// Get the next value
		int *val = va_arg(va, int *);
		*val = 0;

		int factor = c == '-' ? -1 : 1;
		if (factor == -1)
			c = f->readByte();

		while (Common::isDigit(c)) {
			*val = (*val * 10) + (c - '0');
			c = f->readByte();
		}

		*val *= factor;		// Handle negatives
	}

	va_end(va);
}

} // End of namespace Scott
} // End of namespace Glk
