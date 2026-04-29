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

#include "common/file.h"
#include "common/textconsole.h"
#include "mm/mm1/views_enh/create_characters.h"
#include "mm/shared/utils/strings.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define RIGHT_X 200
#define NAMES_FILENAME "mm1_names.txt"
#define NAME_X 160
#define NAME_Y 110
#define NAME_MAX_LEN 15
#define NAME_CONFIRM_X 21
#define NAME_CANCEL_X 51
#define NAME_CONFIRM_Y 15

struct SuggestedName {
	CharacterClass _class;
	Sex _sex;
	Common::String _name;
};

static Common::Array<SuggestedName> g_suggestedNames;
static bool g_suggestedNamesLoaded = false;

static bool openSuggestedNamesFile(Common::File &f, Common::Path &filename) {
	filename = Common::Path(NAMES_FILENAME);
	if (f.open(filename))
		return true;

	filename = Common::Path("mm1").appendComponent(NAMES_FILENAME);
	if (f.open(filename))
		return true;

	filename = Common::Path("files/mm1").appendComponent(NAMES_FILENAME);
	return f.open(filename);
}

static void addSuggestedName(CharacterClass classId, Sex sexId,
		const Common::String &name) {
	SuggestedName entry;
	entry._class = classId;
	entry._sex = sexId;
	entry._name = name;
	g_suggestedNames.push_back(entry);
}

static void loadSuggestedNames() {
	if (g_suggestedNamesLoaded)
		return;

	Common::File f;
	Common::Path filename;
	if (!openSuggestedNamesFile(f, filename)) {
		warning("MM1: Could not open suggested names file");
		g_suggestedNamesLoaded = true;
		return;
	}
	g_suggestedNamesLoaded = true;

	int invalidLines = 0;
	while (!f.eos()) {
		Common::String line = f.readLine();
		line.trim();
		if (line.empty() || line.hasPrefix("#"))
			continue;

		size_t p1 = line.findFirstOf('\t');
		size_t p2 = p1 == Common::String::npos ? Common::String::npos :
			line.findFirstOf('\t', p1 + 1);
		if (p1 == Common::String::npos || p2 == Common::String::npos) {
			++invalidLines;
			continue;
		}

		int classId = atoi(Common::String(line.c_str(), line.c_str() + p1).c_str());
		int sexId = atoi(Common::String(line.c_str() + p1 + 1, line.c_str() + p2).c_str());
		Common::String name(line.c_str() + p2 + 1);
		addSuggestedName((CharacterClass)classId, (Sex)sexId, name);
	}
}

static Common::String getAlignmentString(Alignment alignment) {
	if (alignment == NEUTRAL)
		return "Neutral";

	return STRING[Common::String::format("stats.alignments.%d", alignment)];
}

void CreateCharacters::NewCharacter::clear() {
	Common::fill(_attribs1, _attribs1 + 7, 0);
	Common::fill(_attribs2, _attribs2 + 7, 0);
	_class = KNIGHT;
	_race = HUMAN;
	_alignment = GOOD;
	_sex = MALE;
	_name = "";

	Common::fill(_classesAllowed, _classesAllowed + 7, 0);
}

void CreateCharacters::NewCharacter::reroll() {
	clear();

	// Generate attributes
	for (int attrib = INTELLECT; attrib <= LUCK; ++attrib)
		_attribs1[attrib] = g_engine->getRandomNumber(4, 17);
	Common::copy(_attribs1, _attribs1 + 7, _attribs2);

	// Select which classes are available
	_classesAllowed[KNIGHT] = _attribs1[MIGHT] >= 12;
	_classesAllowed[PALADIN] = _attribs1[MIGHT] >= 12 && _attribs1[PERSONALITY] >= 12 &&
		_attribs1[ENDURANCE] >= 12;
	_classesAllowed[ARCHER] = _attribs1[INTELLECT] >= 12 && _attribs1[ACCURACY] >= 12;
	_classesAllowed[CLERIC] = _attribs1[PERSONALITY] >= 12;
	_classesAllowed[SORCERER] = _attribs1[INTELLECT] >= 12;
	_classesAllowed[ROBBER] = true;
}

void CreateCharacters::NewCharacter::loadPortrait() {
	Common::Path cname(Common::String::format("char%02d.fac",
		_portrait * 2 + (_sex == MALE ? 0 : 1) + 1));
	_portraits.load(cname);
}

void CreateCharacters::NewCharacter::save() {
	uint i = 0;
	while (i < ROSTER_COUNT && g_globals->_roster._towns[i])
		++i;

	g_globals->_roster._towns[i] = Maps::SORPIGAL;
	g_globals->_currCharacter = &g_globals->_roster[i];
	Character &re = *g_globals->_currCharacter;
	re.clear();

	Common::strcpy_s(re._name, _name.c_str());
	re._sex = _sex;
	re._alignment = re._alignmentInitial = _alignment;
	re._race = _race;
	re._class = _class;
	re._intelligence = _attribs1[INTELLECT];
	re._might = _attribs1[MIGHT];
	re._personality = _attribs1[PERSONALITY];
	re._endurance = _attribs1[ENDURANCE];
	re._speed = _attribs1[SPEED];
	re._accuracy = _attribs1[ACCURACY];
	re._luck = _attribs1[LUCK];

	switch (_class) {
	case KNIGHT:
		setHP(12);
		break;
	case PALADIN:
	case ARCHER:
		setHP(10);
		break;
	case CLERIC:
		setHP(8);
		setSP(_attribs1[PERSONALITY]);
		break;
	case SORCERER:
		setHP(6);
		setSP(_attribs1[INTELLECT]);
		break;
	case ROBBER:
		setHP(8);
		re._trapCtr = 50;
		break;
	default:
		break;
	}

	switch (_race) {
	case HUMAN:
		re._resistances._s._fear = 70;
		re._resistances._s._psychic = 25;
		break;
	case ELF:
		re._resistances._s._fear = 70;
		break;
	case DWARF:
		re._resistances._s._poison = 25;
		break;
	case GNOME:
		re._resistances._s._magic = 20;
		break;
	case HALF_ORC:
		re._resistances._s._psychic = 50;
		break;
	}

	re._food = 10;
	re._backpack[0]._id = 1;
	const int ALIGNMENT_VALS[3] = { 0, 0x10, 0x20 };
	re._alignmentCtr = ALIGNMENT_VALS[re._alignmentInitial];
	re._portrait = _portrait;
	re.loadFaceSprites();

	g_globals->_roster.save();
}

void CreateCharacters::NewCharacter::setHP(int hp) {
	Character &re = *g_globals->_currCharacter;

	if (_attribs1[ENDURANCE] >= 19)
		hp += 4;
	else if (_attribs1[ENDURANCE] >= 17)
		hp += 3;
	else if (_attribs1[ENDURANCE] >= 15)
		hp += 2;
	else if (_attribs1[ENDURANCE] >= 13)
		hp += 1;
	else if (_attribs1[ENDURANCE] < 5)
		hp -= 2;
	else if (_attribs1[ENDURANCE] < 8)
		hp -= 1;

	re._hpCurrent = re._hp = re._hpMax = hp;

	int ac = 0;
	if (_attribs1[SPEED] >= 19)
		ac = 4;
	else if (_attribs1[SPEED] >= 17)
		ac = 3;
	else if (_attribs1[SPEED] >= 15)
		ac = 2;
	if (_attribs1[SPEED] >= 13)
		ac = 1;

	re._ac = ac;
}

void CreateCharacters::NewCharacter::setSP(int amount) {
	Character &re = *g_globals->_currCharacter;

	int level = 0;
	if (amount >= 19)
		level = 4;
	else if (amount >= 17)
		level = 3;
	else if (amount >= 15)
		level = 2;
	else if (amount >= 13)
		level = 1;

	re._sp._base = re._sp._current = level + 3;
	re._spellLevel = 1;
}

/*------------------------------------------------------------------------*/

void CreateCharacters::NameEntry::addConfirmIcons() {
	_confirmBounds = Common::Rect(NAME_CONFIRM_X, NAME_CONFIRM_Y,
		NAME_CONFIRM_X + GLYPH_W, NAME_CONFIRM_Y + GLYPH_H);
	_cancelBounds = Common::Rect(NAME_CANCEL_X, NAME_CONFIRM_Y,
		NAME_CANCEL_X + GLYPH_W, NAME_CONFIRM_Y + GLYPH_H);
	int right = MAX((int)_bounds.right, _bounds.left + _cancelBounds.right);
	_bounds = Common::Rect(_bounds.left, _bounds.top,
		right, _bounds.top + _confirmBounds.bottom);
}

void CreateCharacters::NameEntry::draw() {
	TextEntry::draw();

	Graphics::ManagedSurface s = getSurface();
	g_globals->_confirmIcons.draw(&s, _confirmHover ? 1 : 0,
		Common::Point(_confirmBounds.left, _confirmBounds.top));
	g_globals->_confirmIcons.draw(&s, 2 + (_cancelHover ? 1 : 0),
		Common::Point(_cancelBounds.left, _cancelBounds.top));
}

bool CreateCharacters::NameEntry::msgMouseDown(const MouseDownMessage &msg) {
	Common::Point pt(msg._pos.x - _bounds.left, msg._pos.y - _bounds.top);
	return msg._button == MouseMessage::MB_LEFT &&
		(_confirmBounds.contains(pt) || _cancelBounds.contains(pt));
}

bool CreateCharacters::NameEntry::msgMouseUp(const MouseUpMessage &msg) {
	Common::Point pt(msg._pos.x - _bounds.left, msg._pos.y - _bounds.top);
	if (msg._button == MouseMessage::MB_LEFT && _confirmBounds.contains(pt)) {
		_owner->acceptName();
		return true;
	}
	if (msg._button == MouseMessage::MB_LEFT && _cancelBounds.contains(pt)) {
		_owner->abortName();
		return true;
	}

	return false;
}

bool CreateCharacters::NameEntry::msgMouseMove(const MouseMoveMessage &msg) {
	Common::Point pt(msg._pos.x - _bounds.left, msg._pos.y - _bounds.top);
	bool confirmHover = _confirmBounds.contains(pt);
	bool cancelHover = _cancelBounds.contains(pt);
	if (confirmHover != _confirmHover || cancelHover != _cancelHover) {
		_confirmHover = confirmHover;
		_cancelHover = cancelHover;
		redraw();
	}

	return true;
}

/*------------------------------------------------------------------------*/

CreateCharacters::CreateCharacters() : ScrollView("CreateCharacters"),
		_textEntry(this) {
	_icons.load("create.icn");

	addButton(&_icons, Common::Point(120, 172), 4, KEYBIND_ESCAPE, true);
	addButton(&_icons, Common::Point(40, 120), 0, Common::KEYCODE_r);
	addButton(&_icons, Common::Point(190, 110), 6, Common::KEYCODE_UP);
	addButton(&_icons, Common::Point(190, 130), 8, Common::KEYCODE_DOWN);
	addButton(&_icons, Common::Point(220, 120), 2, KEYBIND_SELECT);

	setButtonEnabled(2, false);
	setButtonEnabled(3, false);
	setButtonEnabled(4, false);
}

bool CreateCharacters::msgFocus(const FocusMessage &msg) {
	if (dynamic_cast<TextEntry *>(msg._priorView) == nullptr)
		_newChar.reroll();
	return true;
}

void CreateCharacters::draw() {
	if (_state == SELECT_CLASS || _state == SELECT_RACE ||
			_state == SELECT_ALIGNMENT || _state == SELECT_SEX)
		removeButtons(5, -1);

	ScrollView::draw();
	printAttributes();

	if ((int)_state >= SELECT_NAME) {
		Graphics::ManagedSurface s = getSurface();
		_newChar._portraits.draw(&s, 0, Common::Point(10, 10));
	}

	writeString(135, 174, STRING["enhdialogs.misc.go_back"]);
	writeString(70, 125, STRING["enhdialogs.create_characters.roll"]);

	switch (_state) {
	case SELECT_CLASS:
		printClasses();
		if (g_globals->_roster.full())
			writeLine(9, STRING["dialogs.create_characters.full"], ALIGN_MIDDLE, 190);
		break;

	case SELECT_RACE:
		printRaces();
		break;

	case SELECT_ALIGNMENT:
		printAlignments();
		break;

	case SELECT_SEX:
		printSexes();
		break;

	case SELECT_PORTRAIT:
		printPortraits();
		break;

	case SELECT_NAME:
		printSelectName();
		break;

	case SAVE_PROMPT:
		printSummary();
		break;

	default:
		break;
	}
}

void CreateCharacters::printAttributes() {
	writeLine(0, STRING["dialogs.create_characters.title"], ALIGN_MIDDLE);

	writeLine(5, STRING["enhdialogs.create_characters.intellect"], ALIGN_RIGHT, 90);
	writeLine(6, STRING["enhdialogs.create_characters.might"], ALIGN_RIGHT, 90);
	writeLine(7, STRING["enhdialogs.create_characters.personality"], ALIGN_RIGHT, 90);
	writeLine(8, STRING["enhdialogs.create_characters.endurance"], ALIGN_RIGHT, 90);
	writeLine(9, STRING["enhdialogs.create_characters.speed"], ALIGN_RIGHT, 90);
	writeLine(10, STRING["enhdialogs.create_characters.accuracy"], ALIGN_RIGHT, 90);
	writeLine(11, STRING["enhdialogs.create_characters.luck"], ALIGN_RIGHT, 90);

	for (int i = 0; i < 7; ++i, _textPos.y += 2) {
		writeLine(5 + i,
			Common::String::format("%u", _newChar._attribs1[i]),
			ALIGN_RIGHT, 110);
	}
}

int CreateCharacters::addSelection(int yStart, int num, const Common::String &text) {
	Common::Rect r(170, 0, 170 + getStringWidth(text),
		g_globals->_fontNormal.getFontHeight());
	r.translate(0, (yStart + num) * 9);

	return addButton(r, Common::KeyState((Common::KeyCode)(Common::KEYCODE_0 + num), '0' + num));
}

void CreateCharacters::printClasses() {
	bool rosterFull = g_globals->_roster.full();
	for (int classNum = KNIGHT; classNum <= SORCERER; ++classNum) {
		Common::String text = Common::String::format("%d) %s",
			classNum,
			STRING[Common::String::format("stats.classes.%d", classNum)].c_str()
		);
		bool enabled = _newChar._classesAllowed[classNum] && !rosterFull;
		int buttonNum = enabled ? addSelection(4, classNum, text) : -1;
		setTextColor(enabled ? (_selectedButton == buttonNum ? 15 : 0) : 1);
		writeLine(4 + classNum, text, ALIGN_LEFT, 170);

	}

	Common::String text = Common::String::format("6) %s", STRING["stats.classes.6"].c_str());
	int buttonNum = rosterFull ? -1 : addSelection(4, ROBBER, text);
	setTextColor(rosterFull ? 1 : (_selectedButton == buttonNum ? 15 : 0));
	writeLine(10, text, ALIGN_LEFT, 170);

	setTextColor(0);
	writeLine(13, STRING["dialogs.create_characters.select_class"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(1-6)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printRaces() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);

	for (int i = 1; i <= 5; ++i) {
		Common::String text = Common::String::format("%d) %s", i,
			STRING[Common::String::format("stats.races.%d", i)].c_str());
		int buttonNum = addSelection(6, i, text);
		byte oldColor = setTextColor(_selectedButton == buttonNum ? 15 : 0);
		writeLine(6 + i, text, ALIGN_LEFT, 170);
		setTextColor(oldColor);
	}

	writeLine(13, STRING["dialogs.create_characters.select_race"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(1-5)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printAlignments() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeLine(6, STRING["enhdialogs.create_characters.race"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);

	for (int i = 1; i <= 3; ++i) {
		Common::String text = Common::String::format("%d) %s", i,
			getAlignmentString((Alignment)i).c_str());
		int buttonNum = addSelection(7, i, text);
		byte oldColor = setTextColor(_selectedButton == buttonNum ? 15 : 0);
		writeLine(7 + i, text, ALIGN_LEFT, 170);
		setTextColor(oldColor);
	}

	writeLine(13, STRING["dialogs.create_characters.select_alignment"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(1-3)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printSexes() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeLine(6, STRING["enhdialogs.create_characters.race"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);
	writeLine(7, STRING["enhdialogs.create_characters.alignment"], ALIGN_RIGHT, RIGHT_X);
	writeString(getAlignmentString(_newChar._alignment));

	Common::String male = Common::String::format("1) %s", STRING["stats.sex.1"].c_str());
	Common::String female = Common::String::format("2) %s", STRING["stats.sex.2"].c_str());
	int maleButton = addSelection(8, 1, male);
	byte oldColor = setTextColor(_selectedButton == maleButton ? 15 : 0);
	writeLine(9, male, ALIGN_LEFT, 170);
	setTextColor(oldColor);
	int femaleButton = addSelection(8, 2, female);
	oldColor = setTextColor(_selectedButton == femaleButton ? 15 : 0);
	writeLine(10, female, ALIGN_LEFT, 170);
	setTextColor(oldColor);

	writeLine(14, STRING["dialogs.create_characters.select_sex"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(15, "(1-2)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printSelections() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeLine(6, STRING["enhdialogs.create_characters.race"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);
	writeLine(7, STRING["enhdialogs.create_characters.alignment"], ALIGN_RIGHT, RIGHT_X);
	writeString(getAlignmentString(_newChar._alignment));
	writeLine(8, STRING["enhdialogs.create_characters.sex"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.sex.%d", _newChar._sex)]);
}

void CreateCharacters::printPortraits() {
	printSelections();
	writeLine(10, STRING["enhdialogs.create_characters.select_portrait"], ALIGN_MIDDLE, RIGHT_X);

	Graphics::ManagedSurface s = getSurface();
	_newChar._portraits.draw(&s, 0, Common::Point(160, 120));

	writeString(250, 126, STRING["enhdialogs.create_characters.select"]);
}

void CreateCharacters::printSelectName() {
	printSelections();
	writeLine(10, STRING["enhdialogs.create_characters.enter_name"], ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printSummary() {
	printSelections();
	writeLine(9, STRING["enhdialogs.create_characters.name"], ALIGN_RIGHT, RIGHT_X);
	writeString(_newChar._name);

	writeLine(12, STRING["dialogs.create_characters.save_character"], ALIGN_MIDDLE, RIGHT_X);
}

bool CreateCharacters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_r && _state != SELECT_NAME) {
		setState(SELECT_CLASS);
		_newChar.reroll();
		redraw();
		return true;
	}

	switch (_state) {
	case SELECT_CLASS:
		if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_6) {
			if (_newChar._classesAllowed[msg.keycode - Common::KEYCODE_0] &&
					!g_globals->_roster.full()) {
				// Selected a valid class
				_newChar._class = (CharacterClass)(msg.keycode - Common::KEYCODE_0);
				setState(SELECT_RACE);
				redraw();
			}
		}
		break;

	case SELECT_RACE:
		if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_5) {
			// Selected a race
			_newChar._race = (Race)(msg.keycode - Common::KEYCODE_0);

			switch (_newChar._race) {
			case ELF:
				_newChar._attribs1[INTELLECT]++;
				_newChar._attribs1[ACCURACY]++;
				_newChar._attribs1[MIGHT]--;
				_newChar._attribs1[ENDURANCE]--;
				break;
			case DWARF:
				_newChar._attribs1[ENDURANCE]++;
				_newChar._attribs1[LUCK]++;
				_newChar._attribs1[INTELLECT]--;
				_newChar._attribs1[SPEED]--;
				break;
			case GNOME:
				_newChar._attribs1[LUCK] += 2;
				_newChar._attribs1[SPEED]--;
				_newChar._attribs1[ACCURACY]--;
				break;
			case HALF_ORC:
				_newChar._attribs1[MIGHT]++;
				_newChar._attribs1[ENDURANCE]++;
				_newChar._attribs1[INTELLECT]--;
				_newChar._attribs1[PERSONALITY]--;
				_newChar._attribs1[LUCK]--;
				break;
			default:
				break;
			}

			setState(SELECT_ALIGNMENT);
			redraw();
		}
		break;

	case SELECT_ALIGNMENT:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_3) {
			// Selected a valid alignment
			_newChar._alignment = (Alignment)(msg.keycode - Common::KEYCODE_0);
			setState(SELECT_SEX);
			redraw();
		}
		break;

	case SELECT_SEX:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_2) {
			// Selected a valid sex
			_newChar._sex = (Sex)(msg.keycode - Common::KEYCODE_0);
			_newChar.loadPortrait();
			setState(SELECT_PORTRAIT);
			redraw();
		}
		break;

	case SELECT_PORTRAIT:
		switch (msg.keycode) {
		case Common::KEYCODE_UP:
			_newChar._portrait = (_newChar._portrait == 0) ?
				NUM_PORTRAITS - 1 : _newChar._portrait - 1;
			_newChar.loadPortrait();
			redraw();
			break;
		case Common::KEYCODE_DOWN:
			_newChar._portrait = (_newChar._portrait + 1) % NUM_PORTRAITS;
			_newChar.loadPortrait();
			redraw();
			break;
		case Common::KEYCODE_s:
			msgAction(ActionMessage(KEYBIND_SELECT));
			break;
		default:
			break;
		}

		return true;

	case SAVE_PROMPT:
		if (msg.keycode == Common::KEYCODE_y)
			_newChar.save();

		setState(SELECT_CLASS);
		redraw();
		break;

	case SELECT_NAME:
		break;
	}

	return true;
}

bool CreateCharacters::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		if (_state == SELECT_CLASS) {
			close();
		} else {
			setState(SELECT_CLASS);
			_newChar.reroll();
			redraw();
		}
		return true;

	case KEYBIND_SELECT:
		switch (_state) {
		case SELECT_CLASS:
			// Re-roll attributes
			_newChar.reroll();
			redraw();
			break;
		case SELECT_PORTRAIT:
			setState(SELECT_NAME);
			break;
		case SELECT_NAME:
			acceptName();
			break;
		case SAVE_PROMPT:
			_newChar.save();

			setState(SELECT_CLASS);
			_newChar.reroll();
			redraw();
			break;
		default:
			break;
		}
		return true;

	default:
		break;
	}

	return false;
}

bool CreateCharacters::msgMouseMove(const MouseMoveMessage &msg) {
	int selectedButton = getButtonAt(msg._pos);
	if (selectedButton != _selectedButton) {
		_selectedButton = selectedButton;
		redraw();
	}

	return true;
}

void CreateCharacters::abortFunc() {
	CreateCharacters *view = static_cast<CreateCharacters *>(g_events->focusedView());
	view->setState(SELECT_CLASS);
}

void CreateCharacters::enterFunc(const Common::String &name) {
	CreateCharacters *view = static_cast<CreateCharacters *>(g_events->focusedView());

	view->_newChar._name = camelCase(name);
	view->setState(SAVE_PROMPT);
}

Common::String CreateCharacters::getSuggestedName() {
	loadSuggestedNames();
	if (g_suggestedNames.empty()) {
		return "";
	}

	Common::Array<uint> matches;
	for (uint i = 0; i < g_suggestedNames.size(); ++i) {
		if (g_suggestedNames[i]._class == _newChar._class &&
				g_suggestedNames[i]._sex == _newChar._sex)
			matches.push_back(i);
	}

	if (matches.empty()) {
		for (uint i = 0; i < g_suggestedNames.size(); ++i) {
			if (g_suggestedNames[i]._class == _newChar._class)
				matches.push_back(i);
		}
	}

	if (matches.empty()) {
		return "";
	}

	uint idx = matches.size() == 1 ? matches[0] : matches[g_engine->getRandomNumber(matches.size()) - 1];
	return g_suggestedNames[idx]._name;
}

void CreateCharacters::acceptName() {
	if (_textEntry._text.empty())
		return;

	Common::String name = _textEntry._text;
	if (g_events->focusedView() == &_textEntry)
		_textEntry.close();

	_newChar._name = camelCase(name);
	setState(SAVE_PROMPT);
}

void CreateCharacters::abortName() {
	if (g_events->focusedView() == &_textEntry)
		_textEntry.close();

	setState(SELECT_CLASS);
}

void CreateCharacters::setState(State state) {
	_state = state;
	resetSelectedButton();

	setButtonEnabled(2, _state == SELECT_PORTRAIT);
	setButtonEnabled(3, _state == SELECT_PORTRAIT);
	setButtonEnabled(4, _state == SELECT_PORTRAIT);
	removeButtons(5, -1);

	if (_state == SELECT_CLASS) {
		_newChar.reroll();
	} else if (_state == SAVE_PROMPT) {
		addButton(&g_globals->_confirmIcons, Common::Point(185, 122), 0,
			Common::KeyState(Common::KEYCODE_y, 'y'));
		addButton(&g_globals->_confirmIcons, Common::Point(215, 122), 2,
			Common::KeyState(Common::KEYCODE_n, 'n'));
	}

	if (_state == SELECT_NAME) {
		_newChar._name = getSuggestedName();
		draw();
		_textEntry.display(NAME_X, NAME_Y, NAME_MAX_LEN, false, abortFunc, enterFunc,
			_newChar._name);
		_textEntry.addConfirmIcons();
		_textEntry.draw();
	} else {
		redraw();
	}
}

#undef NAME_CONFIRM_Y
#undef NAME_CANCEL_X
#undef NAME_CONFIRM_X
#undef NAME_X
#undef NAME_Y
#undef NAME_MAX_LEN
#undef NAMES_FILENAME

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
