/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/config-manager.h"
#include "common/translation.h"
#include "common/ustr.h"
#include "glk/quetzal.h"
#include "glk/scott/scott.h"
#include "glk/scott/command_parser.h"
#include "glk/scott/definitions.h"
#include "glk/scott/load_game.h"
#include "glk/scott/game_info.h"
#include "glk/scott/globals.h"
#include "glk/scott/hulk.h"
#include "glk/scott/layout_text.h"
#include "glk/scott/line_drawing.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/restore_state.h"
#include "glk/scott/robin_of_sherwood.h"
#include "glk/scott/gremlins.h"
#include "glk/scott/seas_of_blood.h"
#include "glk/scott/game_specific.h"
#include "glk/scott/ti99_4a_terp.h"

namespace Glk {
namespace Scott {

Scott *g_scott;

Scott::Scott(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc) {
	g_scott = this;
	Common::fill(&_nounText[0], &_nounText[16], '\0');
}

void Scott::runGame() {
	int vb, no;
	initialize();

	glk_stylehint_set(wintype_TextBuffer, style_User1, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_TextBuffer, style_User1, stylehint_Indentation, 20);
	glk_stylehint_set(wintype_TextBuffer, style_User1, stylehint_ParaIndentation, 20);
	glk_stylehint_set(wintype_TextBuffer, style_Preformatted, stylehint_Justification, stylehint_just_Centered);

	_G(_bottomWindow) = glk_window_open(0, 0, 0, wintype_TextBuffer, GLK_BUFFER_ROCK);
	if (_G(_bottomWindow) == nullptr)
		glk_exit();
	glk_set_window(_G(_bottomWindow));

	for (int i = 0; i < MAX_SYSMESS; i++) {
		_G(_sys)[i] = g_sysDict[i];
	}

	const char **dictpointer;

	if (_G(_options) & YOUARE)
		dictpointer = g_sysDict;
	else
		dictpointer = g_sysDictIAm;

	for (int i = 0; i < MAX_SYSMESS && dictpointer[i] != nullptr; i++) {
		_G(_sys)[i] = dictpointer[i];
	}

	loadGameFile(&_gameFile);

	if (CURRENT_GAME == SCOTTFREE)
		loadDatabase(&_gameFile, (_G(_options) & DEBUGGING) ? 1 : 0);

	if (!CURRENT_GAME)
		fatal("Unsupported game!");

	if (CURRENT_GAME != SCOTTFREE && CURRENT_GAME != TI994A) {
		_G(_options) |= SPECTRUM_STYLE;
		_splitScreen = 1;
	} else {
		if (CURRENT_GAME != TI994A)
			_G(_options) |= TRS80_STYLE;
		_splitScreen = 1;
	}

	if (!_G(_titleScreen).empty()) {
		if (_splitScreen)
			printTitleScreenGrid();
		else
			printTitleScreenBuffer();
	}

	if (_G(_options) & TRS80_STYLE) {
		_topWidth = 64;
		_topHeight = 11;
	} else {
		_topWidth = 80;
		_topHeight = 10;
	}

	if (CURRENT_GAME == TI994A) {
		display(_G(_bottomWindow), "In this adventure, you may abbreviate any word \
				by typing its first %d letters, and directions by typing \
				one letter.\n\nDo you want to restore previously saved game?\n",
				_G(_gameHeader)->_wordLength);
		if (yesOrNo())
			loadGame();
		clearScreen();
	}

	openTopWindow();

	if (CURRENT_GAME == SCOTTFREE)
		output("ScummVM support adapted from Scott Free, A Scott Adams game driver in C.\n\n");

	// Check for savegame
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	if (!shouldQuit())
		_G(_initialState) = saveCurrentState();

	while (!shouldQuit()) {
		glk_tick();

		if (_G(_shouldRestart))
			restartGame();

		if (!_G(_stopTime))
			performActions(0, 0);

		if (shouldQuit())
			break;

		if (!(_G(_currentCommand) && _G(_currentCommand)->_allFlag && !(_G(_currentCommand)->_allFlag & LASTALL))) {
			_printLookToTranscript = _G(_shouldLookInTranscript);
			look();
			_printLookToTranscript = _G(_shouldLookInTranscript) = 0;
			if (!_G(_stopTime) && !_G(_shouldRestart))
				saveUndo();
		}

		if (_G(_shouldRestart))
			continue;

		if (getInput(&vb, &no) == 1)
			continue;
		if (g_vm->shouldQuit())
			break;

		switch (performActions(vb, no)) {
		case ER_RAN_ALL_LINES_NO_MATCH:
			if (!recheckForExtraCommand()) {
				output(_G(_sys)[I_DONT_UNDERSTAND]);
				freeCommands();
			}
			break;
		case ER_RAN_ALL_LINES:
			output(_G(_sys)[YOU_CANT_DO_THAT_YET]);
			freeCommands();
			break;
		default:
			_G(_justStarted) = 0;
		}

		/* Brian Howarth games seem to use -1 for forever */
		if (_G(_items)[LIGHT_SOURCE]._location != DESTROYED && _G(_gameHeader)->_lightTime != -1 && !_G(_stopTime)) {
			_G(_gameHeader)->_lightTime--;
			if (_G(_gameHeader)->_lightTime < 1) {
				_G(_bitFlags) |= (1 << LIGHTOUTBIT);
				if (_G(_items)[LIGHT_SOURCE]._location == CARRIED || _G(_items)[LIGHT_SOURCE]._location == MY_LOC) {
					output(_G(_sys)[LIGHT_HAS_RUN_OUT]);
				}
				if ((_G(_options) & PREHISTORIC_LAMP) || (_G(_game)->_subType & MYSTERIOUS) || CURRENT_GAME == TI994A)
					_G(_items)[LIGHT_SOURCE]._location = DESTROYED;
			} else if (_G(_gameHeader)->_lightTime < 25) {
				if (_G(_items)[LIGHT_SOURCE]._location == CARRIED || _G(_items)[LIGHT_SOURCE]._location == MY_LOC) {
					if ((_G(_options) & SCOTTLIGHT) || (_G(_game)->_subType & MYSTERIOUS)) {
						display(_G(_bottomWindow), "%s %d %s\n", _G(_sys)[LIGHT_RUNS_OUT_IN].c_str(), _G(_gameHeader)->_lightTime, _G(_sys)[TURNS].c_str());
					} else {
						if (_G(_gameHeader)->_lightTime % 5 == 0)
							output(_G(_sys)[LIGHT_GROWING_DIM].c_str());
					}
				}
			}
		}
		if (_G(_stopTime))
			_G(_stopTime)--;
	}
}

void Scott::initialize() {
	if (ConfMan.hasKey("YOUARE")) {
		if (ConfMan.getBool("YOUARE"))
			_G(_options) |= YOUARE;
		else
			_G(_options) &= ~YOUARE;
	}
	if (gDebugLevel > 0)
		_G(_options) |= DEBUGGING;
	if (ConfMan.hasKey("SCOTTLIGHT") && ConfMan.getBool("SCOTTLIGHT"))
		_G(_options) |= SCOTTLIGHT;
	if (ConfMan.hasKey("TRS80_STYLE") && ConfMan.getBool("TRS80_STYLE"))
		_G(_options) |= TRS80_STYLE;
	if (ConfMan.hasKey("PREHISTORIC_LAMP") && ConfMan.getBool("PREHISTORIC_LAMP"))
		_G(_options) |= PREHISTORIC_LAMP;
}

void Scott::display(winid_t w, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	Common::String msg = Common::String::vformat(fmt, ap);
	va_end(ap);

	glk_put_string_stream(glk_window_get_stream(w), msg.c_str());
	if (_G(_transcript))
		glk_put_string_stream(_G(_transcript), msg.c_str());
}

void Scott::display_u32_internal(winid_t w, const Common::U32String *fmt, ...) {
	Common::U32String msg;

	va_list ap;

	va_start(ap, fmt);
	Common::U32String::vformat(fmt->begin(), fmt->end(), msg, ap);
	va_end(ap);

	glk_put_string_stream_uni(glk_window_get_stream(w), msg.u32_str());
	if (_G(_transcript))
		glk_put_string_stream_uni(_G(_transcript), msg.u32_str());
}

void Scott::updateSettings() {
	if (drawingVector())
		glk_request_timer_events(20);

	PaletteType previousPal = _G(_palChosen);
	if (_G(_options) & FORCE_PALETTE_ZX)
		_G(_palChosen) = ZXOPT;
	else if (_G(_options) & FORCE_PALETTE_C64) {
		if (_G(_game)->_pictureFormatVersion == 99)
			_G(_palChosen) = C64A;
		else
			_G(_palChosen) = C64B;
	} else
		_G(_palChosen) = _G(_game)->_palette;
	if (_G(_palChosen) != previousPal) {
		definePalette();
		if (_G(_vectorState) != NO_VECTOR_IMAGE)
			drawSomeVectorPixels(1);
	}
}

uint Scott::getRandomNumber(uint max) {
	return _random.getRandomNumber(max);
}

void Scott::updates(event_t ev) {
	if (ev.type == evtype_Arrange) {
		updateSettings();

		_G(_vectorState) = NO_VECTOR_IMAGE;

		closeGraphicsWindow();
		openGraphicsWindow();

		if (_splitScreen) {
			look();
		}
	} else if (ev.type == evtype_Timer) {
		switch (_G(_game->_type)) {
		case SHERWOOD_VARIANT:
			updateRobinOfSherwoodAnimations();
			break;
		case GREMLINS_VARIANT:
			updateGremlinsAnimations();
			break;
		case SECRET_MISSION_VARIANT:
			// TODO
			// UpdateSecretAnimations();
			break;
		default:
			if (_G(_game)->_pictureFormatVersion == 99 && drawingVector())
				drawSomeVectorPixels((_G(_vectorState) == NO_VECTOR_IMAGE));
			break;
		}
	}
}

void Scott::delay(double seconds) {
	if (_G(_options) & NO_DELAYS)
		return;

	event_t ev;

	if (!glk_gestalt(gestalt_Timer, 0))
		return;

	glk_request_char_event(_G(_bottomWindow));
	glk_cancel_char_event(_G(_bottomWindow));

	if (drawingVector()) {
		do {
			glk_select(&ev);
			updates(ev);
		} while (drawingVector() && !g_vm->shouldQuit());
		if (_G(_gliSlowDraw))
			seconds = 0.5;
	}

	glk_request_timer_events(1000 * seconds);

	do {
		glk_select(&ev);
		updates(ev);
	} while (ev.type != evtype_Timer && !g_vm->shouldQuit());

	glk_request_timer_events(0);
}

void Scott::fatal(const char *x) {
	display(_G(_bottomWindow), "%s\n", x);
	cleanupAndExit();
}

void Scott::clearScreen(void) {
	glk_window_clear(_G(_bottomWindow));
}

bool Scott::randomPercent(uint n) {
	return _random.getRandomNumber(99) < n;
}

int Scott::countCarried(void) {
	int ct = 0;
	int n = 0;
	while (ct <= _G(_gameHeader)->_numItems) {
		if (_G(_items)[ct]._location == CARRIED)
			n++;
		ct++;
	}
	return n;
}

const char *Scott::mapSynonym(int noun) {
	int n = 1;
	const char *tp;
	static char lastword[16]; // Last non synonym
	while (n <= _G(_gameHeader)->_numWords) {
		tp = _G(_nouns)[n].c_str();
		if (*tp == '*')
			tp++;
		else
			Common::strcpy_s(lastword, tp);
		if (n == noun)
			return lastword;
		n++;
	}
	return nullptr;
}

int Scott::matchUpItem(int noun, int loc) {
	const char *word = mapSynonym(noun);
	int ct = 0;

	if (word == nullptr)
		word = _G(_nouns)[noun].c_str();

	while (ct <= _G(_gameHeader)->_numItems) {
		if (!_G(_items)[ct]._autoGet.empty() && _G(_items)[ct]._location == loc &&
			scumm_strnicmp(_G(_items)[ct]._autoGet.c_str(), word, _G(_gameHeader)->_wordLength) == 0)
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

	_G(_gameHeader)->_numItems = ni;
	_G(_items).resize(ni + 1);
	_G(_gameHeader)->_numActions = na;
	_G(_actions).resize(na + 1);
	_G(_gameHeader)->_numWords = nw;
	_G(_gameHeader)->_wordLength = wl;
	_G(_verbs).resize(nw + 1);
	_G(_nouns).resize(nw + 1);
	_G(_gameHeader)->_numRooms = nr;
	_G(_rooms).resize(nr + 1);
	_G(_gameHeader)->_maxCarry = mc;
	_G(_gameHeader)->_playerRoom = pr;
	_G(_gameHeader)->_treasures = tr;
	_G(_gameHeader)->_lightTime = lt;
	_G(_lightRefill) = lt;
	_G(_gameHeader)->_numMessages = mn;
	_G(_messages).resize(mn + 1);
	_G(_gameHeader)->_treasureRoom = trm;

	// Load the actions
	if (loud)
		debug("Reading %d actions.", na);

	for (int idx = 0; idx < na + 1; ++idx) {
		Action &a = _G(_actions)[idx];
		readInts(f, 8,
				 &a._vocab, &a._condition[0], &a._condition[1], &a._condition[2],
				 &a._condition[3], &a._condition[4], &a._action[0], &a._action[1]);
	}

	if (loud)
		debug("Reading %d word pairs.", nw);
	for (int idx = 0; idx < nw + 1; ++idx) {
		_G(_verbs)[idx] = readString(f);
		_G(_nouns)[idx] = readString(f);
	}

	if (loud)
		debug("Reading %d rooms.", nr);
	for (int idx = 0; idx < nr + 1; ++idx) {
		Room &r = _G(_rooms)[idx];
		readInts(f, 6, &r._exits[0], &r._exits[1], &r._exits[2],
				 &r._exits[3], &r._exits[4], &r._exits[5]);
		r._text = readString(f);
	}

	if (loud)
		debug("Reading %d messages.", mn);
	for (int idx = 0; idx < mn + 1; ++idx)
		_G(_messages)[idx] = readString(f);

	if (loud)
		debug("Reading %d items.", ni);
	for (int idx = 0; idx < ni + 1; ++idx) {
		Item &i = _G(_items)[idx];
		i._text = readString(f);

		const char *p = strchr(i._text.c_str(), '/');
		if (p) {
			i._autoGet = Common::String(p);

			// Some games use // to mean no auto get/drop word!
			if (!i._autoGet.hasPrefix("//") && !i._autoGet.hasPrefix("/*")) {
				i._text = Common::String(i._text.c_str(), p);
				i._autoGet.deleteChar(0);

				const char *t = strchr(i._autoGet.c_str(), '/');
				if (t) {
					i._autoGet = Common::String(i._autoGet.c_str(), t);
				}
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
		display(_G(_bottomWindow), "%s", a.c_str());
}

void Scott::output(const Common::U32String &a) {
	if (_saveSlot == -1)
		display(_G(_bottomWindow), Common::U32String("%S"), a.c_str());
}

void Scott::outputNumber(int a) {
	display(_G(_bottomWindow), "%d", a);
}

void Scott::look(void) {
	drawRoomImage();

	if (_splitScreen && _G(_topWindow) == nullptr)
		return;

	char *buf = new char[1000];
	buf = static_cast<char *>(memset(buf, 0, 1000));
	_roomDescriptionStream = glk_stream_open_memory(buf, 1000, filemode_Write, 0);

	Room *r;
	int ct, f;

	if (!_splitScreen) {
		writeToRoomDescriptionStream("\n");
	} else if (_G(_transcript) && _printLookToTranscript) {
		glk_put_char_stream_uni(_G(_transcript), 10);
	}

	if ((_G(_bitFlags) & (1 << DARKBIT)) && _G(_items)[LIGHT_SOURCE]._location != CARRIED && _G(_items)[LIGHT_SOURCE]._location != MY_LOC) {
		writeToRoomDescriptionStream("%s", _G(_sys)[TOO_DARK_TO_SEE].c_str());
		flushRoomDescription(buf);
		return;
	}

	r = &_G(_rooms)[MY_LOC];

	if (r->_text.empty())
		return;

	if (r->_text.hasPrefix("*"))
		writeToRoomDescriptionStream("%s", r->_text.substr(1).c_str());
	else {
		writeToRoomDescriptionStream("%s%s", _G(_sys)[YOU_ARE].c_str(), r->_text.c_str());
	}

	if (!(_G(_options) & SPECTRUM_STYLE)) {
		listExits();
		writeToRoomDescriptionStream(".\n");
	}

	ct = 0;
	f = 0;
	while (ct <= _G(_gameHeader)->_numItems) {
		if (_G(_items)[ct]._location == MY_LOC) {
			if (_G(_items)[ct]._text[0] == 0) {
				error("Scott::look(): Invisible item in room: %d", ct);
				ct++;
				continue;
			}
			if (f == 0) {
				writeToRoomDescriptionStream("%s", _G(_sys)[YOU_SEE].c_str());
				f++;
				if (_G(_options) & SPECTRUM_STYLE)
					writeToRoomDescriptionStream("\n");
			} else if (!(_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE))) {
				writeToRoomDescriptionStream("%s", _G(_sys)[ITEM_DELIMITER].c_str());
			}
			writeToRoomDescriptionStream("%s", _G(_items)[ct]._text.c_str());
			if (_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE)) {
				writeToRoomDescriptionStream("%s", _G(_sys)[ITEM_DELIMITER].c_str());
			}
		}
		ct++;
	}

	if ((_G(_options) & TI994A_STYLE) && f) {
		writeToRoomDescriptionStream("%s", ".");
	}

	if (_G(_options) & SPECTRUM_STYLE) {
		listExitsSpectrumStyle();
	} else if (f) {
		writeToRoomDescriptionStream("\n");
	}

	if ((_G(_autoInventory) || (_G(_options) & FORCE_INVENTORY)) && !(_G(_options) & FORCE_INVENTORY_OFF))
		listInventoryInUpperWindow();

	flushRoomDescription(buf);
}

int Scott::whichWord(const char *word, const Common::StringArray &list) {
	int n = 1;
	int ne = 1;
	const char *tp;
	while (ne <= _G(_gameHeader)->_numWords) {
		tp = list[ne].c_str();
		if (*tp == '*')
			tp++;
		else
			n = ne;
		if (scumm_strnicmp(word, tp, _G(_gameHeader)->_wordLength) == 0)
			return n;
		ne++;
	}
	return -1;
}

Common::Error Scott::writeGameData(Common::WriteStream *ws) {
	Common::String msg;

	for (int ct = 0; ct < 16; ct++) {
		msg = Common::String::format("%d %d\n", _G(_counters)[ct], _G(_roomSaved)[ct]);
		ws->write(msg.c_str(), msg.size());
		ws->writeByte(0);
	}

	msg = Common::String::format("%u %d %d %d %d %d %d\n",
								 _G(_bitFlags), (_G(_bitFlags) & (1 << DARKBIT)) ? 1 : 0,
								 MY_LOC, _G(_currentCounter), _G(_savedRoom), _G(_gameHeader)->_lightTime, _G(_autoInventory));
	ws->write(msg.c_str(), msg.size());
	ws->writeByte(0);

	for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
		msg = Common::String::format("%hd\n", (short)_G(_items)[ct]._location);
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

	int previousAutoInventory = _G(_autoInventory);

	SavedState *state = saveCurrentState();

	int result = 0;

	for (ct = 0; ct < 16; ct++) {
		line = QuetzalReader::readString(rs);
		result = sscanf(line.c_str(), "%d %d", &_G(_counters)[ct], &_G(_roomSaved)[ct]);
		if (result != 2 || _G(_roomSaved)[ct] > _G(_gameHeader)->_numRooms) {
			recoverFromBadRestore(state);
			return Common::kNoError;
		}
	}

	line = QuetzalReader::readString(rs);
	result = sscanf(line.c_str(), "%u %hd %d %d %d %d %d\n",
		   &_G(_bitFlags), &darkFlag, &MY_LOC, &_G(_currentCounter), &_G(_savedRoom),
		   &_G(_gameHeader)->_lightTime, &_G(_autoInventory));

	if (result == 6)
		_G(_autoInventory) = previousAutoInventory;
	if ((result != 7 && result != 6) || MY_LOC > _G(_gameHeader)->_numRooms || MY_LOC < 1 || _G(_savedRoom) > _G(_gameHeader)->_numRooms) {
		recoverFromBadRestore(state);
		return Common::kNoError;
	}
	// Backward compatibility
	if (darkFlag)
		_G(_bitFlags) |= (1 << 15);
	for (ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
		line = QuetzalReader::readString(rs);
		result = sscanf(line.c_str(), "%hd\n", &lo);
		_G(_items)[ct]._location = (unsigned char)lo;
		if (result != 1 || (_G(_items)[ct]._location > _G(_gameHeader)->_numRooms && _G(_items)[ct]._location != CARRIED)) {
			recoverFromBadRestore(state);
			return Common::kNoError;
		}
	}

	saveUndo();
	_G(_justStarted) = 0;
	_G(_stopTime) = 1;
	return Common::kNoError;
}

ActionResultType Scott::performLine(int ct) {
#ifdef DEBUG_ACTIONS
	debug("Performing line %d: ", ct);
#endif
	int continuation = 0, dead = 0;
	int param[5], pptr = 0;
	int p;
	int act[4];
	int cc = 0;
	while (cc < 5) {
		int cv, dv;
		cv = _G(_actions)[ct]._condition[cc];
		dv = cv / 20;
		cv %= 20;
#ifdef DEBUG_ACTIONS
		debug("Testing condition %d: ", cv);
#endif
		switch (cv) {
		case 0:
			param[pptr++] = dv;
			break;
		case 1:
#ifdef DEBUG_ACTIONS
			debug("Does the player carry %s?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location != CARRIED)
				return ACT_FAILURE;
			break;
		case 2:
#ifdef DEBUG_ACTIONS
			debug("Is %s in location?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location != MY_LOC)
				return ACT_FAILURE;
			break;
		case 3:
#ifdef DEBUG_ACTIONS
			debug("Is %s held or in location?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location != CARRIED && _G(_items)[dv]._location != MY_LOC)
				return ACT_FAILURE;
			break;
		case 4:
#ifdef DEBUG_ACTIONS
			debug("Is location %s?\n", _G(_rooms)[dv].Text);
#endif
			if (MY_LOC != dv)
				return ACT_FAILURE;
			break;
		case 5:
#ifdef DEBUG_ACTIONS
			debug("Is %s NOT in location?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location == MY_LOC)
				return ACT_FAILURE;
			break;
		case 6:
#ifdef DEBUG_ACTIONS
			debug("Does the player NOT carry %s?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location == CARRIED)
				return ACT_FAILURE;
			break;
		case 7:
#ifdef DEBUG_ACTIONS
			debug("Is location NOT %s?\n", _G(_rooms)[dv].Text);
#endif
			if (MY_LOC == dv)
				return ACT_FAILURE;
			break;
		case 8:
#ifdef DEBUG_ACTIONS
			debug("Is bitflag %d set?\n", dv);
#endif
			if ((_G(_bitFlags) & (1 << dv)) == 0)
				return ACT_FAILURE;
			break;
		case 9:
#ifdef DEBUG_ACTIONS
			debug("Is bitflag %d NOT set?\n", dv);
#endif
			if (_G(_bitFlags) & (1 << dv))
				return ACT_FAILURE;
			break;
		case 10:
#ifdef DEBUG_ACTIONS
			debug("Does the player carry anything?\n");
#endif
			if (countCarried() == 0)
				return ACT_FAILURE;
			break;
		case 11:
#ifdef DEBUG_ACTIONS
			debug("Does the player carry nothing?\n");
#endif
			if (countCarried())
				return ACT_FAILURE;
			break;
		case 12:
#ifdef DEBUG_ACTIONS
			debug("Is %s neither carried nor in room?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location == CARRIED || _G(_items)[dv]._location == MY_LOC)
				return ACT_FAILURE;
			break;
		case 13:
#ifdef DEBUG_ACTIONS
			debug("Is %s (%d) in play?\n", _G(_items)[dv].Text, dv);
#endif
			if (_G(_items)[dv]._location == 0)
				return ACT_FAILURE;
			break;
		case 14:
#ifdef DEBUG_ACTIONS
			debug("Is %s NOT in play?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location)
				return ACT_FAILURE;
			break;
		case 15:
#ifdef DEBUG_ACTIONS
			debug("Is _G(_currentCounter) <= %d?\n", dv);
#endif
			if (_G(_currentCounter) > dv)
				return ACT_FAILURE;
			break;
		case 16:
#ifdef DEBUG_ACTIONS
			debug("Is _G(_currentCounter) > %d?\n", dv);
#endif
			if (_G(_currentCounter) <= dv)
				return ACT_FAILURE;
			break;
		case 17:
#ifdef DEBUG_ACTIONS
			debug("Is %s still in initial room?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location != _G(_items)[dv]._initialLoc)
				return ACT_FAILURE;
			break;
		case 18:
#ifdef DEBUG_ACTIONS
			debug("Has %s been moved?\n", _G(_items)[dv].Text);
#endif
			if (_G(_items)[dv]._location == _G(_items)[dv]._initialLoc)
				return ACT_FAILURE;
			break;
		case 19: /* Only seen in Brian Howarth games so far */
#ifdef DEBUG_ACTIONS
			debug("Is current counter == %d?\n", dv);
			if (_G(_currentCounter) != dv)
				debug("Nope, current counter is %d\n", _G(_currentCounter));
#endif
			if (_G(_currentCounter) != dv)
				return ACT_FAILURE;
			break;
		}
#ifdef DEBUG_ACTIONS
		debug("YES\n");
#endif
		cc++;
	}
#if defined(__clang__)
#pragma mark Subcommands
#endif

	/* Actions */
	act[0] = _G(_actions)[ct]._action[0];
	act[2] = _G(_actions)[ct]._action[1];
	act[1] = act[0] % 150;
	act[3] = act[2] % 150;
	act[0] /= 150;
	act[2] /= 150;
	cc = 0;
	pptr = 0;
	while (cc < 4) {
#ifdef DEBUG_ACTIONS
		debug("Performing action %d: ", act[cc]);
#endif
		if (act[cc] >= 1 && act[cc] < 52) {
			printMessage(act[cc]);
		} else if (act[cc] > 101) {
			printMessage(act[cc] - 50);
		} else
			switch (act[cc]) {
			case 0: /* NOP */
				break;
			case 52:
				if (countCarried() >= _G(_gameHeader)->_maxCarry) {
					output(_G(_sys)[YOURE_CARRYING_TOO_MUCH]);
					return ACT_SUCCESS;
				}
				_G(_items)[param[pptr++]]._location = CARRIED;
				break;
			case 53:
#ifdef DEBUG_ACTIONS
				debug("item %d (\"%s\") is now in location.\n", param[pptr],
					  _G(_items)[param[pptr]].Text);
#endif
				_G(_items)[param[pptr++]]._location = MY_LOC;
				_G(_shouldLookInTranscript) = 1;
				break;
			case 54:
#ifdef DEBUG_ACTIONS
				debug("player location is now room %d (%s).\n", param[pptr],
					  _G(_rooms)[param[pptr]].Text);
#endif
				MY_LOC = param[pptr++];
				_G(_shouldLookInTranscript) = 1;
				look();
				break;
			case 55:
#ifdef DEBUG_ACTIONS
				fprintf(stderr,
						"Item %d (%s) is removed from the game (put in room 0).\n",
						param[pptr], _G(_items)[param[pptr]].Text);
#endif
				_G(_items)[param[pptr++]]._location = 0;
				break;
			case 56:
				_G(_bitFlags) |= 1 << DARKBIT;
				break;
			case 57:
				_G(_bitFlags) &= ~(1 << DARKBIT);
				break;
			case 58:
#ifdef DEBUG_ACTIONS
				debug("Bitflag %d is set\n", param[pptr]);
#endif
				_G(_bitFlags) |= (1 << param[pptr++]);
				break;
			case 59:
#ifdef DEBUG_ACTIONS
				debug("Item %d (%s) is removed from play.\n", param[pptr],
					  _G(_items)[param[pptr]].Text);
#endif
				_G(_items)[param[pptr++]]._location = 0;
				break;
			case 60:
#ifdef DEBUG_ACTIONS
				debug("BitFlag %d is cleared\n", param[pptr]);
#endif
				_G(_bitFlags) &= ~(1 << param[pptr++]);
				break;
			case 61:
				playerIsDead();
				break;
			case 62:
				p = param[pptr++];
				putItemAInRoomB(p, param[pptr++]);
				break;
			case 63:
#ifdef DEBUG_ACTIONS
				debug("Game over.\n");
#endif
				doneIt();
				dead = 1;
				break;
			case 64:
				break;
			case 65:
				dead = printScore();
				_G(_stopTime) = 2;
				break;
			case 66:
				if (_G(_game)->_type == SEAS_OF_BLOOD_VARIANT)
					adventureSheet();
				else
					listInventory();
				_G(_stopTime) = 2;
				break;
			case 67:
				_G(_bitFlags) |= (1 << 0);
				break;
			case 68:
				_G(_bitFlags) &= ~(1 << 0);
				break;
			case 69:
				_G(_gameHeader)->_lightTime = _G(_lightRefill);
				_G(_items)[LIGHT_SOURCE]._location = CARRIED;
				_G(_bitFlags) &= ~(1 << LIGHTOUTBIT);
				break;
			case 70:
				clearScreen(); /* pdd. */
				break;
			case 71:
				saveGame();
				_G(_stopTime) = 2;
				break;
			case 72:
				p = param[pptr++];
				swapItemLocations(p, param[pptr++]);
				break;
			case 73:
#ifdef DEBUG_ACTIONS
				debug("Continue with next line\n");
#endif
				continuation = 1;
				break;
			case 74:
				_G(_items)[param[pptr++]]._location = CARRIED;
				break;
			case 75:
				p = param[pptr++];
				moveItemAToLocOfItemB(p, param[pptr++]);
				break;
			case 76: /* Looking at adventure .. */
#ifdef DEBUG_ACTIONS
				debug("LOOK\n");
#endif
				if (_splitScreen)
					look();
				_G(_shouldLookInTranscript) = 1;
				break;
			case 77:
				if (_G(_currentCounter) >= 1)
					_G(_currentCounter)--;
#ifdef DEBUG_ACTIONS
				fprintf(stderr,
						"decrementing current counter. Current counter is now %d.\n",
						_G(_currentCounter));
#endif
				break;
			case 78:
				outputNumber(_G(_currentCounter));
				output(" ");
				break;
			case 79:
#ifdef DEBUG_ACTIONS
				debug("_G(_currentCounter) is set to %d.\n", param[pptr]);
#endif
				_G(_currentCounter) = param[pptr++];
				break;
			case 80:
				goToStoredLoc();
				break;
			case 81:
				swapCounters(param[pptr++]);
				break;
			case 82:
				_G(_currentCounter) += param[pptr++];
				break;
			case 83:
				_G(_currentCounter) -= param[pptr++];
				if (_G(_currentCounter) < -1)
					_G(_currentCounter) = -1;
				/* Note: This seems to be needed. I don't yet
						 know if there is a maximum value to limit too */
				break;
			case 84:
				if (_G(_currentCommand))
					glk_put_string_stream_uni(
						glk_window_get_stream(_G(_bottomWindow)),
						_G(_unicodeWords)[_G(_currentCommand)->_nounWordIndex]);
				break;
			case 85:
				if (_G(_currentCommand))
					glk_put_string_stream_uni(
						glk_window_get_stream(_G(_bottomWindow)),
						_G(_unicodeWords)[_G(_currentCommand)->_nounWordIndex]);
				output("\n");
				break;
			case 86:
				if (!(_G(_options) & SPECTRUM_STYLE))
					output("\n");
				break;
			case 87:
				swapLocAndRoomFlag(param[pptr++]);
				break;
			case 88:
#ifdef DEBUG_ACTIONS
				debug("Delay\n");
#endif
				delay(1);
				break;
			case 89:
#ifdef DEBUG_ACTIONS
				debug("Action 89, parameter %d\n", param[pptr]);
#endif
				p = param[pptr++];
				switch (CURRENT_GAME) {
				case SPIDERMAN:
				case SPIDERMAN_C64:
					drawBlack();
					break;
				case SECRET_MISSION:
				case SECRET_MISSION_C64:
					// TODO
					// SecretAction(p);
					break;
				case ADVENTURELAND:
				case ADVENTURELAND_C64:
					adventurelandAction(p);
					break;
				case SEAS_OF_BLOOD:
				case SEAS_OF_BLOOD_C64:
					bloodAction(p);
					break;
				case ROBIN_OF_SHERWOOD:
				case ROBIN_OF_SHERWOOD_C64:
					sherwoodAction(p);
					break;
				case GREMLINS:
				case GREMLINS_SPANISH:
				case GREMLINS_GERMAN:
				case GREMLINS_GERMAN_C64:
					gremlinsAction(p);
					break;
				default:
					break;
				}
				break;

			case 90:
#ifdef DEBUG_ACTIONS
				debug("Draw Hulk image, parameter %d\n", param[pptr]);
#endif
				if (CURRENT_GAME != HULK && CURRENT_GAME != HULK_C64) {
					pptr++;
				} else if (!(_G(_bitFlags) & (1 << DARKBIT)))
					drawHulkImage(param[pptr++]);
				break;
			default:
				debug("Unknown action %d [Param begins %d %d]\n", act[cc],
					  param[pptr], param[pptr + 1]);
				break;
			}
		cc++;
	}

	if (dead) {
		return ACT_GAMEOVER;
	} else if (continuation) {
		return ACT_CONTINUE;
	} else {
		return ACT_SUCCESS;
	}
}

ExplicitResultType Scott::performActions(int vb, int no) {
	int dark = _G(_bitFlags) & (1 << DARKBIT);
	int ct = 0;
	ExplicitResultType flag = ER_NO_RESULT;
	int doAgain = 0;
	int foundMatch = 0;

	if (vb == GO && no == -1) {
		output(_G(_sys)[DIRECTION]);
		return ER_SUCCESS;
	}
	if (vb == 1 && no >= 1 && no <= 6) {
		int nl;
		if (_G(_items)[LIGHT_SOURCE]._location == MY_LOC || _G(_items)[LIGHT_SOURCE]._location == CARRIED)
			dark = 0;
		if (dark)
			output(_G(_sys)[DANGEROUS_TO_MOVE_IN_DARK]);
		nl = _G(_rooms)[MY_LOC]._exits[no - 1];
		if (nl != 0) {
			/* Seas of Blood needs this to be able to flee back to the last room */
			if (_G(_game)->_type == SEAS_OF_BLOOD_VARIANT)
				_G(_savedRoom) = MY_LOC;
			if (_G(_options) & (SPECTRUM_STYLE | TI994A_STYLE))
				output(_G(_sys)[OK]);
			MY_LOC = nl;
			_G(_shouldLookInTranscript) = 1;
			if (_G(_currentCommand) && _G(_currentCommand)->_next) {
				lookWithPause();
			}
			return ER_SUCCESS;
		}
		if (dark) {
			_G(_bitFlags) &= ~(1 << DARKBIT);
			MY_LOC = _G(_gameHeader)->_numRooms; /* It seems to be what the code says! */
			output(_G(_sys)[YOU_FELL_AND_BROKE_YOUR_NECK]);
			_G(_bitFlags) &= ~(1 << DARKBIT);
			MY_LOC = _G(_gameHeader)->_numRooms; /* It seems to be what the code says! */
			return ER_SUCCESS;
		}
		output(_G(_sys)[YOU_CANT_GO_THAT_WAY]);
		return ER_SUCCESS;
	}

	if ((CURRENT_GAME == HULK || CURRENT_GAME == HULK_C64) && vb == 39 && !dark) {
		hulkShowImageOnExamine(no);
	}

	if (_G(_currentCommand) && _G(_currentCommand)->_allFlag && vb == _G(_currentCommand)->_verb && !(dark && vb == TAKE)) {
		output(_G(_items)[_G(_currentCommand)->_item]._text);
		output("....");
	}
	flag = ER_RAN_ALL_LINES_NO_MATCH;
	if (CURRENT_GAME != TI994A) {
		while (ct <= _G(_gameHeader)->_numActions) {
			int verbvalue, nounvalue;
			verbvalue = _G(_actions)[ct]._vocab;
			/* Think this is now right. If a line we run has an action73
		   run all following lines with vocab of 0,0 */
			if (vb != 0 && (doAgain && verbvalue != 0))
				break;
			/* Oops.. added this minor cockup fix 1.11 */
			if (vb != 0 && !doAgain && flag == 0)
				break;
			nounvalue = verbvalue % 150;
			verbvalue /= 150;
			if ((verbvalue == vb) || (doAgain && _G(_actions)[ct]._vocab == 0)) {
				if ((verbvalue == 0 && randomPercent(nounvalue)) || doAgain || (verbvalue != 0 && (nounvalue == no || nounvalue == 0))) {
					if (verbvalue == vb && vb != 0 && nounvalue == no)
						foundMatch = 1;
					ActionResultType flag2;
					if (flag == ER_RAN_ALL_LINES_NO_MATCH)
						flag = ER_RAN_ALL_LINES;
					if ((flag2 = performLine(ct)) != ACT_FAILURE) {
						/* ahah finally figured it out ! */
						flag = ER_SUCCESS;
						if (flag2 == ACT_CONTINUE)
							doAgain = 1;
						else if (flag2 == ACT_GAMEOVER)
							return ER_SUCCESS;
						if (vb != 0 && doAgain == 0)
							return ER_SUCCESS;
					}
				}
			}

			ct++;

			/* Previously this did not check ct against
			 * GameHeader.NumActions and would read past the end of
			 * Actions.  I don't know what should happen on the last
			 * action, but doing nothing is better than reading one
			 * past the end.
			 * --Chris
			 */
			if (ct <= _G(_gameHeader)->_numActions && _G(_actions)[ct]._vocab != 0)
				doAgain = 0;
		}
	} else {
		if (vb == 0) {
			runImplicitTI99Actions();
			return ER_NO_RESULT;
		} else {
			flag = runExplicitTI99Actions(vb, no);
		}
	}

	if (foundMatch)
		return flag;

	if (flag != ER_SUCCESS) {
		int item = 0;
		if (_G(_items)[LIGHT_SOURCE]._location == MY_LOC || _G(_items)[LIGHT_SOURCE]._location == CARRIED)
			dark = 0;

		if (vb == TAKE || vb == DROP) {
			if (_G(_currentCommand)->_allFlag) {
				if (vb == TAKE && dark) {
					output(_G(_sys)[TOO_DARK_TO_SEE]);
					while (!(_G(_currentCommand)->_allFlag & LASTALL)) {
						_G(_currentCommand) = _G(_currentCommand)->_next;
					}
					return ER_SUCCESS;
				}
				item = _G(_currentCommand)->_item;
				int location = CARRIED;
				if (vb == TAKE)
					location = MY_LOC;
				while (_G(_items)[item]._location != location && !(_G(_currentCommand)->_allFlag & LASTALL)) {
					_G(_currentCommand) = _G(_currentCommand)->_next;
				}
				if (_G(_items)[item]._location != location)
					return ER_SUCCESS;
			}

			/* Yes they really _are_ hardcoded values */
			if (vb == TAKE) {
				if (no == -1) {
					output(_G(_sys)[WHAT]);
					return ER_SUCCESS;
				}
				if (countCarried() >= _G(_gameHeader)->_maxCarry) {
					output(_G(_sys)[YOURE_CARRYING_TOO_MUCH]);
					return ER_SUCCESS;
				}
				if (!item)
					item = matchUpItem(no, MY_LOC);
				if (item == -1) {
					item = matchUpItem(no, CARRIED);
					if (item == -1) {
						item = matchUpItem(no, 0);
						if (item == -1) {
							output(_G(_sys)[THATS_BEYOND_MY_POWER]);
						} else {
							output(_G(_sys)[YOU_DONT_SEE_IT]);
						}
					} else {
						output(_G(_sys)[YOU_HAVE_IT]);
					}
					return ER_SUCCESS;
				}
				_G(_items)[item]._location = CARRIED;
				printTakenOrDropped(TAKEN);
				return ER_SUCCESS;
			}

			if (vb == DROP) {
				if (no == -1) {
					output(_G(_sys)[WHAT]);
					return ER_SUCCESS;
				}
				if (!item)
					item = matchUpItem(no, CARRIED);
				if (item == -1) {
					item = matchUpItem(no, 0);
					if (item == -1) {
						output(_G(_sys)[THATS_BEYOND_MY_POWER]);
					} else {
						output(_G(_sys)[YOU_HAVENT_GOT_IT]);
					}
					return ER_SUCCESS;
				}
				_G(_items)[item]._location = MY_LOC;
				printTakenOrDropped(DROPPED);
				return ER_SUCCESS;
			}
		}
	}
	return flag;
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

		*val *= factor; // Handle negatives
	}

	va_end(va);
}

void Scott::writeToRoomDescriptionStream(const char *fmt, ...) {
	if (_roomDescriptionStream == nullptr)
		return;
	va_list ap;

	va_start(ap, fmt);
	Common::String msg = Common::String::vformat(fmt, ap);
	va_end(ap);

	glk_put_string_stream(_roomDescriptionStream, msg.c_str());
}

void Scott::flushRoomDescription(char *buf) {
	glk_stream_close(_roomDescriptionStream, 0);

	strid_t storedTranscript = _G(_transcript);
	if (!_printLookToTranscript)
		_G(_transcript) = nullptr;

	int printDelimiter = (_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE | TI994A_STYLE));

	if (_splitScreen) {
		glk_window_clear(_G(_topWindow));
		glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
		int rows, length;
		char *textWithBreaks = lineBreakText(buf, _topWidth, &rows, &length);

		uint bottomheight;
		glk_window_get_size(_G(_bottomWindow), nullptr, &bottomheight);
		winid_t o2 = glk_window_get_parent(_G(_topWindow));
		if (!(bottomheight < 3 && _topHeight < rows)) {
			glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
			glk_window_set_arrangement(o2, winmethod_Above | winmethod_Fixed, rows, _G(_topWindow));
		} else {
			printDelimiter = 0;
		}

		int line = 0;
		int index = 0;
		int i;
		char *string = new char[_topWidth + 1];
		for (line = 0; line < rows && index < length; line++) {
			for (i = 0; i < _topWidth; i++) {
				string[i] = textWithBreaks[index++];
				if (string[i] == 10 || string[i] == 13 || index >= length)
					break;
			}
			if (i < _topWidth + 1) {
				string[i++] = '\n';
			}
			string[i] = 0;
			if (strlen(string) == 0)
				break;
			glk_window_move_cursor(_G(_topWindow), 0, line);
			display(_G(_topWindow), "%s", string);
		}

		if (line < rows - 1) {
			glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
			glk_window_set_arrangement(o2, winmethod_Above | winmethod_Fixed, MIN(rows - 1, _topHeight - 1), _G(_topWindow));
		}

		delete[] textWithBreaks;
	} else {
		display(_G(_bottomWindow), "%s", buf);
	}

	if (printDelimiter) {
		printWindowDelimiter();
	}

	if (_pauseNextRoomDescription) {
		delay(0.8);
		_pauseNextRoomDescription = 0;
	}

	_G(_transcript) = storedTranscript;
	if (buf != nullptr) {
		delete[] buf;
		buf = nullptr;
	}
}

void Scott::printWindowDelimiter() {
	glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
	glk_window_move_cursor(_G(_topWindow), 0, _topHeight - 1);
	glk_stream_set_current(glk_window_get_stream(_G(_topWindow)));
	if (_G(_options) & SPECTRUM_STYLE)
		for (int i = 0; i < _topWidth; i++)
			glk_put_char('*');
	else {
		glk_put_char('<');
		for (int i = 0; i < _topWidth - 2; i++)
			glk_put_char('-');
		glk_put_char('>');
	}
}

void Scott::listExits() {
	int ct = 0;
	int f = 0;

	writeToRoomDescriptionStream("\n\n%s", _G(_sys)[EXITS].c_str());

	while (ct < 6) {
		if ((&_G(_rooms)[MY_LOC])->_exits[ct] != 0) {
			if (f) {
				writeToRoomDescriptionStream("%s", _G(_sys)[EXITS_DELIMITER].c_str());
			}
			/* _G(_sys)[] begins with the exit names */
			writeToRoomDescriptionStream("%s", _G(_sys)[ct].c_str());
			f = 1;
		}
		ct++;
	}
	if (f == 0)
		writeToRoomDescriptionStream("%s", _G(_sys)[NONE].c_str());
	return;
}

void Scott::listExitsSpectrumStyle() {
	int ct = 0;
	int f = 0;

	while (ct < 6) {
		if ((&_G(_rooms)[MY_LOC])->_exits[ct] != 0) {
			if (f == 0) {
				writeToRoomDescriptionStream("\n\n%s", _G(_sys)[EXITS].c_str());
			} else {
				writeToRoomDescriptionStream("%s", _G(_sys)[EXITS_DELIMITER].c_str());
			}
			/* sys[] begins with the exit names */
			writeToRoomDescriptionStream("%s", _G(_sys)[ct].c_str());
			f = 1;
		}
		ct++;
	}
	writeToRoomDescriptionStream("\n");
	return;
}

void Scott::listInventoryInUpperWindow() {
	int i = 0;
	int lastitem = -1;
	writeToRoomDescriptionStream("\n%s", _G(_sys)[INVENTORY].c_str());
	while (i <= _G(_gameHeader)->_numItems) {
		if (_G(_items)[i]._location == CARRIED) {
			if (_G(_items)[i]._text[0] == 0) {
				error("Scott::listInventoryInUpperWindow(): Invisible item in inventory: %d", i);
				i++;
				continue;
			}
			if (lastitem > -1 && (_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE)) == 0) {
				writeToRoomDescriptionStream("%s", _G(_sys)[ITEM_DELIMITER].c_str());
			}
			lastitem = i;
			writeToRoomDescriptionStream("%s", _G(_items)[i]._text.c_str());
			if (_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE)) {
				writeToRoomDescriptionStream("%s", _G(_sys)[ITEM_DELIMITER].c_str());
			}
		}
		i++;
	}
	if (lastitem == -1) {
		writeToRoomDescriptionStream("%s\n", _G(_sys)[NOTHING].c_str());
	} else {
		if (_G(_options) & TI994A_STYLE && !itemEndsWithPeriod(lastitem))
			writeToRoomDescriptionStream(".");
		writeToRoomDescriptionStream("\n");
	}
}

int Scott::itemEndsWithPeriod(int item) {
	if (item < 0 || item > _G(_gameHeader)->_numItems)
		return 0;
	Common::String desc = _G(_items)[item]._text;
	if (!desc.empty() && desc[0] != 0) {
		const char lastchar = desc[desc.size() - 1];
		if (lastchar == '.' || lastchar == '!') {
			return 1;
		}
	}
	return 0;
}

void Scott::closeGraphicsWindow() {
	if (_G(_graphics) == nullptr)
		_G(_graphics) = findGlkWindowWithRock(GLK_GRAPHICS_ROCK);
	if (_G(_graphics)) {
		glk_window_close(_G(_graphics), nullptr);
		_G(_graphics) = nullptr;
		glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
	}
}

winid_t Scott::findGlkWindowWithRock(glui32 rock) {
	uint rockptr;
	winid_t win = glk_window_iterate(nullptr, &rockptr);
	while (win) {
		if (rockptr == rock)
			return win;
		win = glk_window_iterate(win, &rockptr);
	}
	return 0;
}

void Scott::openGraphicsWindow() {
	if (!glk_gestalt(gestalt_Graphics, 0))
		return;
	uint graphwidth, graphheight, optimalWidth, optimalHeight;

	if (_G(_topWindow) == nullptr)
		_G(_topWindow) = findGlkWindowWithRock(GLK_STATUS_ROCK);
	if (_G(_graphics) == nullptr)
		_G(_graphics) = findGlkWindowWithRock(GLK_GRAPHICS_ROCK);
	if (_G(_graphics) == nullptr && _G(_topWindow) != nullptr) {
		glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
		glk_window_close(_G(_topWindow), nullptr);
		_G(_graphics) = glk_window_open(_G(_bottomWindow), winmethod_Above | winmethod_Proportional, 60, wintype_Graphics, GLK_GRAPHICS_ROCK);
		glk_window_get_size(_G(_graphics), &graphwidth, &graphheight);
		_G(_pixelSize) = optimalPictureSize(&optimalWidth, &optimalHeight);
		_G(_xOffset) = ((int)graphwidth - (int)optimalWidth) / 2;

		if (graphheight > optimalHeight) {
			winid_t parent = glk_window_get_parent(_G(_graphics));
			glk_window_set_arrangement(parent, winmethod_Above | winmethod_Fixed, optimalHeight, nullptr);
		}

		// Set the graphics window background to match the main window background, best as we can, and clear the window.
		uint backgroundColor;
		if (glk_style_measure(_G(_bottomWindow), style_Normal, stylehint_BackColor, &backgroundColor)) {
			glk_window_set_background_color(_G(_graphics), backgroundColor);
			glk_window_clear(_G(_graphics));
		}

		_G(_topWindow) = glk_window_open(_G(_bottomWindow), winmethod_Above | winmethod_Fixed, _topHeight, wintype_TextGrid, GLK_STATUS_ROCK);
		glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, (uint *)&_topHeight);
	} else {
		if (!_G(_graphics))
			_G(_graphics) = glk_window_open(_G(_bottomWindow), winmethod_Above | winmethod_Proportional, 60, wintype_Graphics, GLK_GRAPHICS_ROCK);
		glk_window_get_size(_G(_graphics), &graphwidth, &graphheight);
		_G(_pixelSize) = optimalPictureSize(&optimalWidth, &optimalHeight);
		_G(_xOffset) = (graphwidth - optimalWidth) / 2;
		winid_t parent = glk_window_get_parent(_G(_graphics));
		glk_window_set_arrangement(parent, winmethod_Above | winmethod_Fixed, optimalHeight, nullptr);
	}
}

glui32 Scott::optimalPictureSize(uint *width, uint *height) {
	*width = 255;
	*height = 96;
	int multiplier = 1;
	uint graphwidth, graphheight;
	glk_window_get_size(_G(_graphics), &graphwidth, &graphheight);
	multiplier = graphheight / 96;
	if (static_cast<glui32>(255 * multiplier) > graphwidth)
		multiplier = graphwidth / 255;

	if (multiplier == 0)
		multiplier = 1;

	*width = 255 * multiplier;
	*height = 96 * multiplier;

	return multiplier;
}

void Scott::openTopWindow() {
	_G(_topWindow) = findGlkWindowWithRock(GLK_STATUS_ROCK);
	if (_G(_topWindow) == nullptr) {
		if (_splitScreen) {
			_G(_topWindow) = glk_window_open(_G(_bottomWindow), winmethod_Above | winmethod_Fixed, _topHeight, wintype_TextGrid, GLK_STATUS_ROCK);
			if (_G(_topWindow) == nullptr) {
				_splitScreen = 0;
				_G(_topWindow) = _G(_bottomWindow);
			} else {
				glk_window_get_size(_G(_topWindow), (uint *)&_topWidth, nullptr);
			}
		} else {
			_G(_topWindow) = _G(_bottomWindow);
		}
	}
}

void Scott::cleanupAndExit() {
	if (_G(_transcript))
		glk_stream_close(_G(_transcript), nullptr);
	if (drawingVector()) {
		_G(_gliSlowDraw) = 0;
		drawSomeVectorPixels(0);
	}
	glk_exit();
}

void Scott::drawBlack() {
	glk_window_fill_rect(_G(_graphics), 0, _G(_xOffset), 0, 32 * 8 * _G(_pixelSize), 12 * 8 * _G(_pixelSize));
}

void Scott::drawImage(int image) {
	if (!glk_gestalt(gestalt_Graphics, 0))
		return;
	openGraphicsWindow();
	if (_G(_graphics) == nullptr) {
		error("drawImage: Graphic window nullptr?");
		return;
	}
	if (_G(_game)->_pictureFormatVersion == 99)
		drawVectorPicture(image);
	else
		drawSagaPictureNumber(image);
}

void Scott::drawRoomImage() {
	if (CURRENT_GAME == ADVENTURELAND || CURRENT_GAME == ADVENTURELAND_C64) {
		adventurelandDarkness();
	}

	int dark = ((_G(_bitFlags) & (1 << DARKBIT)) && _G(_items)[LIGHT_SOURCE]._location != CARRIED && _G(_items)[LIGHT_SOURCE]._location != MY_LOC);

	if (dark && _G(_graphics) != nullptr && !(_G(_rooms)[MY_LOC]._image == 255)) {
		_G(_vectorImageShown) = -1;
		_G(_vectorState) = NO_VECTOR_IMAGE;
		glk_request_timer_events(0);
		drawBlack();
		return;
	}

	switch (CURRENT_GAME) {
	case SEAS_OF_BLOOD:
	case SEAS_OF_BLOOD_C64:
		seasOfBloodRoomImage();
		return;
	case ROBIN_OF_SHERWOOD:
	case ROBIN_OF_SHERWOOD_C64:
		robinOfSherwoodLook();
		return;
	case HULK:
	case HULK_C64:
		hulkLook();
		return;
	default:
		break;
	}

	if (_G(_rooms)[MY_LOC]._image == 255) {
		closeGraphicsWindow();
		return;
	}

	if (dark)
		return;

	if (_G(_game)->_pictureFormatVersion == 99) {
		drawImage(MY_LOC - 1);
		return;
	}

	if (_G(_game)->_type == GREMLINS_VARIANT) {
		gremlinsLook();
	} else {
		drawImage(_G(_rooms)[MY_LOC]._image & 127);
	}
	for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++)
		if (_G(_items)[ct]._image && _G(_items)[ct]._location == MY_LOC) {
			if ((_G(_items)[ct]._flag & 127) == MY_LOC) {
				drawImage(_G(_items)[ct]._image);
				/* Draw the correct image of the bear on the beach */
			} else if (_G(_game)->_type == SAVAGE_ISLAND_VARIANT && ct == 20 && MY_LOC == 8) {
				drawImage(9);
			}
		}
}

void Scott::restartGame() {
	if (_G(_currentCommand))
		freeCommands();
	restoreState(_G(_initialState));
	_G(_justStarted) = 0;
	_G(_stopTime) = 0;
	glk_window_clear(_G(_bottomWindow));
	openTopWindow();
	_G(_shouldRestart) = 0;
}

void Scott::transcriptOn() {
	frefid_t ref;

	if (_G(_transcript)) {
		output(_G(_sys)[TRANSCRIPT_ALREADY]);
		return;
	}

	ref = glk_fileref_create_by_prompt(fileusage_TextMode | fileusage_Transcript, filemode_Write, 0);
	if (ref == nullptr)
		return;

	_G(_transcript) = glk_stream_open_file_uni(ref, filemode_Write, 0);
	glk_fileref_destroy(ref);

	if (_G(_transcript) == nullptr) {
		output(_G(_sys)[FAILED_TRANSCRIPT]);
		return;
	}

	glui32 *startOfTranscript = toUnicode(_G(_sys)[TRANSCRIPT_START].c_str());
	glk_put_string_stream_uni(_G(_transcript), startOfTranscript);
	delete[] startOfTranscript;
	glk_put_string_stream(glk_window_get_stream(_G(_bottomWindow)), _G(_sys)[TRANSCRIPT_ON].c_str());
}

void Scott::transcriptOff() {
	if (_G(_transcript) == nullptr) {
		output(_G(_sys)[NO_TRANSCRIPT]);
		return;
	}

	glui32 *endOfTranscript = toUnicode(_G(_sys)[TRANSCRIPT_END].c_str());
	glk_put_string_stream_uni(_G(_transcript), endOfTranscript);
	delete[] endOfTranscript;

	glk_stream_close(_G(_transcript), nullptr);
	_G(_transcript) = nullptr;
	output(_G(_sys)[TRANSCRIPT_OFF]);
}

int Scott::performExtraCommand(int extraStopTime) {
	Command command = *_G(_currentCommand);
	int verb = command._verb;
	if (verb > _G(_gameHeader)->_numWords)
		verb -= _G(_gameHeader)->_numWords;
	int noun = command._noun;
	if (noun > _G(_gameHeader)->_numWords)
		noun -= _G(_gameHeader)->_numWords;
	else if (noun) {
		const char *nounWord = _G(_charWords)[_G(_currentCommand)->_nounWordIndex];
		int newnoun = whichWord(nounWord, _G(_extraNouns));
		newnoun = _G(_extraNounsKey)[newnoun];
		if (newnoun)
			noun = newnoun;
	}

	_G(_stopTime) = 1 + extraStopTime;

	switch (verb) {
	case RESTORE:
		if (noun == 0 || noun == GAME) {
			loadGame();
			return 1;
		}
		break;
	case RESTART:
		if (noun == 0 || noun == GAME) {
			output(_G(_sys)[ARE_YOU_SURE]);
			if (yesOrNo()) {
				_G(_shouldRestart) = 1;
			}
			return 1;
		}
		break;
	case SAVE:
		if (noun == 0 || noun == GAME) {
			saveGame();
			return 1;
		}
		break;
	case UNDO:
		if (noun == 0 || noun == COMMAND) {
			restoreUndo();
			return 1;
		}
		break;
	case RAM:
		if (noun == RAMLOAD) {
			ramRestore();
			return 1;
		} else if (noun == RAMSAVE) {
			ramSave();
			return 1;
		}
		break;
	case RAMSAVE:
		if (noun == 0) {
			ramSave();
			return 1;
		}
		break;
	case RAMLOAD:
		if (noun == 0) {
			ramRestore();
			return 1;
		}
		break;
	case SCRIPT:
		if (noun == ON || noun == 0) {
			transcriptOn();
			return 1;
		} else if (noun == OFF) {
			transcriptOff();
			return 1;
		}
		break;
	case EXCEPT:
		freeCommands();
	}

	_G(_stopTime) = 0;
	return 0;
}

int Scott::yesOrNo() {
	glk_request_char_event(_G(_bottomWindow));

	event_t ev;
	int result = 0;
	const char y = tolower((unsigned char)_G(_sys)[YES][0]);
	const char n = tolower((unsigned char)_G(_sys)[NO][0]);

	do {
		glk_select(&ev);
		if (ev.type == evtype_CharInput) {
			const char reply = tolower(ev.val1);
			if (reply == y) {
				result = 1;
			} else if (reply == n) {
				result = 2;
			} else {
				output(_G(_sys)[ANSWER_YES_OR_NO]);
				glk_request_char_event(_G(_bottomWindow));
			}
		} else
			updates(ev);
	} while (result == 0 && !g_vm->shouldQuit());

	return (result == 1);
}

void Scott::hitEnter() {
	glk_request_char_event(_G(_bottomWindow));

	event_t ev;
	int result = 0;
	do {
		glk_select(&ev);
		if (ev.type == evtype_CharInput) {
			if (ev.val1 == keycode_Return) {
				result = 1;
			} else {
				glk_request_char_event(_G(_bottomWindow));
			}
		} else
			updates(ev);
	} while (result == 0 && !g_vm->shouldQuit());

	return;
}

void Scott::listInventory() {
	int i = 0;
	int lastitem = -1;
	output(_G(_sys)[INVENTORY]);
	while (i <= _G(_gameHeader)->_numItems) {
		if (_G(_items)[i]._location == CARRIED) {
			if (_G(_items)[i]._text[0] == 0) {
				warning("Invisible item in inventory: %d\n", i);
				i++;
				continue;
			}
			if (lastitem > -1 && (_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE)) == 0) {
				output(_G(_sys)[ITEM_DELIMITER]);
			}
			lastitem = i;
			output(_G(_items)[i]._text);
			if (_G(_options) & (TRS80_STYLE | SPECTRUM_STYLE)) {
				output(_G(_sys)[ITEM_DELIMITER]);
			}
		}
		i++;
	}
	if (lastitem == -1)
		output(_G(_sys)[NOTHING]);
	else if (_G(_options) & TI994A_STYLE) {
		if (!itemEndsWithPeriod(lastitem))
			output(".");
		output(" ");
	}
	if (_G(_transcript)) {
		glk_put_char_stream_uni(_G(_transcript), 10);
	}
}

void Scott::lookWithPause() {
	char fc = _G(_rooms)[MY_LOC]._text[0];
	if (_G(_rooms)[MY_LOC]._text.empty() || MY_LOC == 0 || fc == 0 || fc == '.' || fc == ' ')
		return;
	_G(_shouldLookInTranscript) = 1;
	_pauseNextRoomDescription = 1;
	look();
}

void Scott::doneIt() {
	if (_splitScreen && _G(_topWindow))
		look();
	output("\n\n");
	output(_G(_sys)[PLAY_AGAIN]);
	output("\n");
	if (yesOrNo()) {
		_G(_shouldRestart) = 1;
	} else {
		cleanupAndExit();
	}
}

int Scott::printScore() {
	int i = 0;
	int n = 0;
	while (i <= _G(_gameHeader)->_numItems) {
		if (_G(_items)[i]._location == _G(_gameHeader)->_treasureRoom && _G(_items)[i]._text[0] == '*')
			n++;
		i++;
	}
	display(_G(_bottomWindow), "%s %d %s%s %d.\n", _G(_sys)[IVE_STORED].c_str(), n, _G(_sys)[TREASURES].c_str(),
			_G(_sys)[ON_A_SCALE_THAT_RATES].c_str(), (n * 100) / _G(_gameHeader)->_treasures);
	if (n == _G(_gameHeader)->_treasures) {
		output(_G(_sys)[YOUVE_SOLVED_IT].c_str());
		doneIt();
		return 1;
	}
	return 0;
}

void Scott::printNoun() {
	if (_G(_currentCommand))
		glk_put_string_stream_uni(glk_window_get_stream(_G(_bottomWindow)), _G(_unicodeWords)[_G(_currentCommand)->_nounWordIndex]);
}

void Scott::moveItemAToLocOfItemB(int itemA, int itemB) {
	_G(_items)
	[itemA]._location = _G(_items)[itemB]._location;
	if (_G(_items)[itemB]._location == MY_LOC)
		_G(_shouldLookInTranscript) = 1;
}

void Scott::goToStoredLoc() {
#ifdef DEBUG_ACTIONS
	debug("switch location to stored location (%d) (%s).\n",
		  SavedRoom, _G(_rooms)[SavedRoom].Text);
#endif
	int t = MY_LOC;
	MY_LOC = _G(_savedRoom);
	_G(_savedRoom) = t;
	_G(_shouldLookInTranscript) = 1;
}

void Scott::swapLocAndRoomFlag(int index) {
#ifdef DEBUG_ACTIONS
	debug("swap location<->roomflag[%d]\n", index);
#endif
	int temp = MY_LOC;
	MY_LOC = _G(_roomSaved)[index];
	_G(_roomSaved)[index] = temp;
	_G(_shouldLookInTranscript) = 1;
	look();
}

void Scott::swapItemLocations(int itemA, int itemB) {
	int temp = _G(_items)[itemA]._location;
	_G(_items)[itemA]._location = _G(_items)[itemB]._location;
	_G(_items)[itemB]._location = temp;
	if (_G(_items)[itemA]._location == MY_LOC || _G(_items)[itemB]._location == MY_LOC)
		_G(_shouldLookInTranscript) = 1;
}

void Scott::putItemAInRoomB(int itemA, int roomB) {
#ifdef DEBUG_ACTIONS
	debug("Item %d (%s) is put in room %d (%s). MY_LOC: %d (%s)\n",
		  itemA, _G(_items)[arg1].Text, roomB, _G(_rooms)[roomB].Text, MY_LOC,
		  _G(_rooms)[MY_LOC].Text);
#endif
	if (_G(_items)[itemA]._location == MY_LOC)
		lookWithPause();
	_G(_items)[itemA]._location = roomB;
}

void Scott::swapCounters(int index) {
#ifdef DEBUG_ACTIONS
	debug("Select a counter.Current counter is swapped with backup "
		  "counter %d\n",
		  index);
#endif
	if (index > 15) {
		error("Scott::swapCounters(int index): ERROR! parameter out of range. Max 15, got %d", index);
		index = 15;
	}
	int temp = _G(_currentCounter);

	_G(_currentCounter) = _G(_counters)[index];
	_G(_counters)[index] = temp;
#ifdef DEBUG_ACTIONS
	debug("Value of new selected counter is %d\n", _G(_currentCounter));
#endif
}

void Scott::printMessage(int index) {
#ifdef DEBUG_ACTIONS
	debug("Print message %d: \"%s\"\n", index, Messages[index]);
#endif
	Common::String message = _G(_messages)[index];
	if (!message.empty() && message[0] != 0) {
		output(message);
		const char lastchar = message[message.size() - 1];
		if (lastchar != 13 && lastchar != 10)
			output(_G(_sys)[MESSAGE_DELIMITER]);
	}
}

void Scott::playerIsDead() {
#ifdef DEBUG_ACTIONS
	debug("Player is dead\n");
#endif
	output(_G(_sys)[IM_DEAD]);
	_G(_bitFlags) &= ~(1 << DARKBIT);
	MY_LOC = _G(_gameHeader)->_numRooms; /* It seems to be what the code says! */
}

void Scott::printTakenOrDropped(int index) {
	output(_G(_sys[index]));
	int length = _G(_sys)[index].size();
	char last = _G(_sys)[index][length - 1];
	if (last == 10 || last == 13)
		return;
	output(" ");
	if ((!(_G(_currentCommand)->_allFlag & LASTALL)) || _splitScreen == 0) {
		output("\n");
	}
}

void Scott::printTitleScreenBuffer() {
	glk_stream_set_current(glk_window_get_stream(_G(_bottomWindow)));
	glk_set_style(style_User1);
	clearScreen();
	output(_G(_titleScreen));
	glk_set_style(style_Normal);
	hitEnter();
	clearScreen();
}

void Scott::printTitleScreenGrid() {
	int titleLength = _G(_titleScreen).size();
	int rows = 0;
	for (int i = 0; i < titleLength; i++)
		if (_G(_titleScreen)[i] == '\n')
			rows++;
	winid_t titlewin = glk_window_open(_G(_bottomWindow), winmethod_Above | winmethod_Fixed, rows + 2,
									   wintype_TextGrid, 0);
	uint width, height;
	glk_window_get_size(titlewin, &width, &height);
	if (width < 40 || height < static_cast<glui32>(rows + 2)) {
		glk_window_close(titlewin, nullptr);
		printTitleScreenBuffer();
		return;
	}
	int offset = (width - 40) / 2;
	int pos = 0;
	for (int i = 1; i <= rows; i++) {
		glk_window_move_cursor(titlewin, offset, i);
		while (_G(_titleScreen)[pos] != '\n' && pos < titleLength)
			display(titlewin, "%c", _G(_titleScreen)[pos++]);
		pos++;
	}
	hitEnter();
	glk_window_close(titlewin, nullptr);
}

void writeToRoomDescriptionStream(const char *fmt, ...) {
	if (_G(_roomDescriptionStream == nullptr))
		return;
	va_list ap;

	va_start(ap, fmt);
	Common::String msg = Common::String::vformat(fmt, ap);
	va_end(ap);

	g_scott->glk_put_string_stream(_G(_roomDescriptionStream), msg.c_str());
}

} // End of namespace Scott
} // End of namespace Glk
