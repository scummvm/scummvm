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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/mortimer/interview.h"

#include "common/file.h"
#include "common/tokenizer.h"
#include "common/util.h"
#include "graphics/surface.h"

namespace Scumm {

namespace {

// SHELL 444030: the order also matches the rescued-animal flags and icons.
const char *const kAnimals[] = {"Rhinoceros", "Baboon", "Cheetah", "Elephant", "Giraffe", "Lion", "Oxpecker"};
const char *const kMovieNames[] = {"RHIN", "BABO", "CHEE", "ELEP", "GIRA", "LION", "OXPE"};
const char *const kRiddleNames[] = {"RHI", "BAB", "CHE", "ELE", "GIR", "LIO", "OXP"};

struct Control {
	int id, left, top, right, bottom;
	const char *help;
};

// SHELL 441bb0. The large text and picture regions repeat the last question.
const Control kControls[] = {
	{62, 9, 0, 376, 172, "HTEXTSCR"},
	{46, 126, 187, 366, 238, "HINTVIEW"},
	{47, 126, 239, 366, 284, "HINTVIEW"},
	{48, 126, 285, 360, 330, "HINTVIEW"},
	{63, 375, 65, 624, 326, "HIMAGWIN"},
	{64, 14, 330, 140, 476, "HSPEAKER"},
	{65, 486, 355, 587, 465, "HGLOWSTN"},
	{67, 182, 441, 442, 459, "HSHELSHK"},
	{66, 263, 465, 350, 479, "HSLMSCOP"},
	{50, 396, 6, 463, 64, "HBUTTON1"},
	{51, 493, 6, 616, 60, "HDONEBUT"},
	{52, 10, 242, 124, 329, "HPLAYRID"},
	{53, 8, 174, 125, 241, "HSOLVRID"},
	{55, 147, 343, 197, 407, "HPICTURE"},
	{56, 198, 336, 242, 407, "HPICTURE"},
	{57, 243, 331, 286, 407, "HPICTURE"},
	{58, 287, 330, 332, 407, "HPICTURE"},
	{59, 333, 331, 377, 407, "HPICTURE"},
	{60, 378, 335, 423, 407, "HPICTURE"},
	{61, 428, 341, 469, 407, "HPICTURE"}
};

const Common::Point kQuestionOrigins[] = {{122, 191}, {120, 234}, {124, 278}};

void blit(Graphics::Surface &screen, const MortimerSprite &sprite, int x, int y) {
	x += sprite.x;
	y += sprite.y;
	Common::Rect clip(x, y, x + sprite.width, y + sprite.height);
	clip.clip(Common::Rect(screen.w, screen.h));
	if (clip.isEmpty())
		return;
	for (int row = clip.top; row < clip.bottom; ++row) {
		const byte *src = sprite.pixels.begin() + (row - y) * sprite.width;
		byte *dst = (byte *)screen.getBasePtr(0, row);
		for (int col = clip.left; col < clip.right; ++col)
			if (src[col - x])
				dst[col] = src[col - x];
	}
}

} // End of anonymous namespace

bool MortimerFont::load(const char *path) {
	Common::File file;
	if (!file.open(Common::Path(path)) || !loadMortimerSprites(file, _glyphs) || _glyphs.size() <= 126)
		return false;
	_width = _height = 0;
	for (uint i = 0; i < _glyphs.size(); ++i) {
		_width = MAX(_width, _glyphs[i].width);
		_height = MAX(_height, _glyphs[i].height);
	}
	return true;
}

int MortimerFont::getCharWidth(uint32 chr) const {
	return chr < _glyphs.size() ? _glyphs[chr].width : 0;
}

void MortimerFont::drawChar(Graphics::Surface *surface, uint32 chr, int x, int y, uint32 color) const {
	if (chr < _glyphs.size())
		blit(*surface, _glyphs[chr], x, y);
}

MortimerInterview::MortimerInterview() : _random("mortimer"), _stage(kIdle), _effect(kNoEffect),
	_answer(-1), _selected(-1), _question(0), _guess(-1), _lastAction(46), _textScroll(0),
	_species(0), _stones(0), _guesses(3), _frame(0), _switchFrame(6), _payment(0),
	_moviePending(false), _riddleView(false), _solving(false), _help(false), _helpExplained(false) {
	memset(_palette, 0, sizeof(_palette));
	memset(_gatePalette, 0, sizeof(_gatePalette));
}

bool MortimerInterview::loadText(const char *path, bool riddles) {
	Common::File file;
	if (!file.open(Common::Path(path)))
		return false;
	int animal = -1, answer = -1;
	while (!file.eos() && !file.err()) {
		Common::String line = file.readLine();
		line.trim();
		if (line.empty())
			continue;
		if (line[0] == 0x1a)
			break;
		if (line[0] == '#') {
			const Common::String heading = line.substr(1);
			if (heading.hasPrefix("Answer") && heading.size() == 7) {
				answer = heading[6] - '1';
			} else if (heading == "Easy" || heading == "Medium" || heading == "Hard") {
				answer = heading == "Easy" ? 0 : -1;
			} else {
				animal = answer = -1;
				for (uint i = 0; i < ARRAYSIZE(kAnimals); ++i)
					if (heading.equalsIgnoreCase(kAnimals[i]))
						animal = i;
			}
		} else if (animal >= 0 && answer >= 0 && answer < 3) {
			Common::String &text = riddles ? _riddles[animal] : _dialogue[animal][answer];
			if (!text.empty())
				text += '\n';
			text += line;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(kAnimals); ++i) {
		if (riddles && _riddles[i].empty())
			return false;
		for (uint j = 0; !riddles && j < 3; ++j)
			if (_dialogue[i][j].empty())
				return false;
	}
	return !file.err();
}

bool MortimerInterview::loadResources() {
	struct Bank {
		const char *name;
		Common::Array<MortimerSprite> *sprites;
		uint count;
		byte *palette;
	};
	const Bank banks[] = {
		{"RESOURCE/GENERAL/INTERV_H.NUT", &_panel, 21, nullptr},
		{"RESOURCE/SAVANNA/S_ANIM_H.NUT", &_portraits, 8, _palette},
		{"RESOURCE/SAVANNA/S_GATE_H.NUT", &_gate, 1, _gatePalette},
		{"RESOURCE/GENERAL/CURS_I_H.NUT", &_cursor, 4, nullptr}
	};
	Common::File file;
	for (uint i = 0; i < ARRAYSIZE(banks); ++i) {
		if (!file.open(Common::Path(banks[i].name)) ||
			!loadMortimerSprites(file, *banks[i].sprites, banks[i].palette) || banks[i].sprites->size() < banks[i].count) {
			_error = Common::String::format("Missing or invalid %s", banks[i].name);
			return false;
		}
		file.close();
	}
	if (!_font.load("RESOURCE/GENERAL/MORTFNTH.NUT") || !_captionFont.load("RESOURCE/GENERAL/SUBTFONT.NUT")) {
		_error = "Missing or invalid Mortimer font";
		return false;
	}
	if (!loadText("RESOURCE/DATA/DIALOG_F.DAT", false) || !loadText("RESOURCE/DATA/RIDDLE_F.DAT", true)) {
		_error = "Missing or invalid Mortimer dialogue or riddle text";
		return false;
	}
	if (!file.open("RESOURCE/DATA/SAV_SONG.TRS")) {
		_error = "Missing Mortimer song captions";
		return false;
	}
	uint id = 0;
	while (!file.eos() && !file.err()) {
		Common::String line = file.readLine();
		line.trim();
		if (line.hasPrefix("#define")) {
			Common::StringTokenizer tokens(line);
			tokens.nextToken();
			tokens.nextToken();
			id = atoi(tokens.nextToken().c_str());
		} else if (line.hasPrefix("//") && id < ARRAYSIZE(_captions)) {
			_captions[id] = line.substr(2);
		}
	}
	return !file.err();
}

void MortimerInterview::resetRiddle() {
	_answer = -1;
}

void MortimerInterview::start(uint species, uint stones, int scene) {
	_species = species;
	_stones = stones;
	_guesses = 3;
	_frame = _payment = 0;
	_switchFrame = 6;
	_selected = -1;
	_question = _textScroll = 0;
	_lastAction = 46;
	_riddleView = _solving = _help = false;
	_effect = kNoEffect;
	if (_answer < 0) {
		// SHELL 40cd65 restricts the demo to baboon, elephant and oxpecker.
		const int answers[] = {1, 3, 6};
		_answer = answers[_random.getRandomNumber(2)];
	}
	for (int i = 0; i < 7 && _selected < 0; ++i)
		if (_species & (1 << i))
			_selected = i;
	switch (scene) {
	case 4: enter(kIdle); break;
	case 8: enter(kPopup); break;
	case 18: enter(kSong); break;
	case 25: enter(kEnding); break;
	default: enter(kGate); break;
	}
}

Common::String MortimerInterview::animalMovie(const char *pattern, int animal) const {
	return Common::String::format(pattern, kRiddleNames[animal]);
}

void MortimerInterview::enter(Stage stage) {
	_stage = stage;
	_movieName.clear();
	switch (stage) {
	case kGate: _movieName = "RESOURCE/SAVANNA/SGATE.SAN"; break;
	case kSong: _movieName = animalMovie("RESOURCE/SAVANNA/S%sSNG0.SAN", _answer); break;
	case kPopup: _movieName = "RESOURCE/SAVANNA/S_POPUPH.SAN"; break;
	case kAnswer:
		_movieName = Common::String::format("RESOURCE/SAVANNA/S%s%d_H.SAN", kMovieNames[_selected], _question + 1);
		break;
	case kRiddle: _movieName = animalMovie("RESOURCE/SAVANNA/S%sRIH0.SAN", _answer); break;
	case kSolve: _movieName = "RESOURCE/SAVANNA/S_SOLVEH.SAN"; break;
	case kNoStones: _movieName = "RESOURCE/GENERAL/SGMOR13.SAN"; break;
	case kGuess: _movieName = animalMovie("RESOURCE/SAVANNA/ANS_%sH.SAN", _guess); break;
	case kCorrect: _movieName = animalMovie("RESOURCE/SAVANNA/ANS%sH0.SAN", _answer); break;
	case kWrong: _movieName = "RESOURCE/SAVANNA/S_WRONGH.SAN"; break;
	case kOpenGate: _movieName = "RESOURCE/SAVANNA/SGYESH.SAN"; break;
	case kEnding: _movieName = "RESOURCE/ENDING/DEMOENDH.SAN"; break;
	default: break;
	}
	_moviePending = !_movieName.empty();
}

bool MortimerInterview::fullScreen() const {
	return _stage == kGate || _stage == kSong || _stage == kPopup || _stage == kOpenGate || _stage == kEnding || _stage == kFinished;
}

bool MortimerInterview::movieDoubled() const {
	return _stage == kGate || _stage == kSong || _stage == kOpenGate;
}

const byte *MortimerInterview::palette(const MortimerMovie &movie) const {
	if (playingMovie() && _stage != kHelp && _stage != kNoStones)
		return movie.palette();
	return _riddleView ? _gatePalette : _palette;
}

int MortimerInterview::hitControl(const Common::Point &point) const {
	for (uint i = 0; i < ARRAYSIZE(kControls); ++i) {
		const Control &c = kControls[i];
		if (point.x >= c.left && point.x <= c.right && point.y >= c.top && point.y <= c.bottom) {
			if (!_help && c.id >= 55 && c.id <= 61 && !(_species & (1 << (c.id - 55))))
				return -1;
			if (!_help && c.id >= 46 && c.id <= 48 && (_riddleView || _selected < 0))
				return -1;
			return c.id;
		}
	}
	return -1;
}

void MortimerInterview::selectAnimal(int animal) {
	if (_selected != animal || _riddleView) {
		_switchFrame = 0;
		_effect = kSwitchEffect;
	}
	_selected = animal;
	_riddleView = false;
	_question = _textScroll = 0;
	_lastAction = 46;
	enter(kIdle);
}

void MortimerInterview::click(const Common::Point &point) {
	if (fullScreen() || _stage == kPay || _stage == kGuess || _stage == kCorrect || _stage == kWrong || _stage == kSolve || _stage == kNoStones)
		return;
	int action = hitControl(point);
	if (action < 0)
		return;
	_effect = kButtonEffect;
	if (action < 55 || action > 61)
		_solving = false;
	if (action == 50) {
		_help = !_help;
		enter(kIdle);
		if (_help && !_helpExplained) {
			_helpExplained = true;
			enter(kHelp);
			_movieName = "RESOURCE/GENERAL/HELP/HBUTTON1.SAN";
			_moviePending = true;
		}
		return;
	}
	if (_help) {
		for (uint i = 0; i < ARRAYSIZE(kControls); ++i)
			if (kControls[i].id == action) {
				enter(kHelp);
				_movieName = Common::String::format("RESOURCE/GENERAL/HELP/%s.SAN", kControls[i].help);
				_moviePending = true;
				_help = false;
				return;
			}
	}
	if (action == 51) {
		enter(kFlyAway);
		return;
	}
	if (_selected < 0)
		return;
	if (action == 62 || action == 63)
		action = _lastAction;
	if (action >= 55 && action <= 61) {
		if (_solving) {
			_guess = action - 55;
			_solving = false;
			_payment = 5;
			--_guesses;
			enter(kPay);
		} else {
			selectAnimal(action - 55);
		}
	} else if (action >= 46 && action <= 48 && !_riddleView) {
		_solving = false;
		_question = action - 46;
		_textScroll = 0;
		_lastAction = action;
		enter(kAnswer);
	} else if (action == 52 || action == 53) {
		_solving = false;
		_riddleView = true;
		_textScroll = 0;
		_lastAction = 52;
		enter(action == 52 ? kRiddle : kSolve);
	}
}

void MortimerInterview::scroll(int direction) {
	_textScroll = MAX(0, _textScroll + direction);
}

MortimerInterview::Effect MortimerInterview::takeEffect() {
	Effect result = _effect;
	_effect = kNoEffect;
	return result;
}

void MortimerInterview::update() {
	++_frame;
	if (_switchFrame < 6)
		++_switchFrame;
	if (_stage == kPay && (_frame % 3) == 0) {
		--_stones;
		--_payment;
		_effect = kPayoutEffect;
		if (!_payment)
			enter(kGuess);
	}
}

void MortimerInterview::movieFinished() {
	switch (_stage) {
	case kGate: enter(kSong); break;
	case kSong: enter(kPopup); break;
	case kSolve:
		if (_stones < 5) {
			enter(kNoStones);
		} else {
			_solving = true;
			enter(kIdle);
		}
		break;
	case kGuess: enter(_guess == _answer ? kCorrect : kWrong); break;
	case kCorrect: enter(kOpenGate); break;
	case kWrong:
		if (!_guesses)
			enter(kFlyAway);
		else
			enter(kIdle);
		break;
	case kOpenGate: enter(kEnding); break;
	case kEnding: enter(kFinished); break;
	default: enter(kIdle); break;
	}
}

void MortimerInterview::skip() {
	if (playingMovie())
		movieFinished();
	else if (_stage != kPay)
		enter(kFlyAway);
}

void MortimerInterview::drawText(Graphics::Surface &screen, const Common::String &text) const {
	Common::Array<Common::String> lines;
	_font.wordWrapText(text, 330, lines, 0, Graphics::kWordWrapOnExplicitNewLines);
	const int height = _font.getFontHeight();
	const int visible = MIN<int>(lines.size(), 140 / height);
	const int first = MIN<int>(_textScroll, lines.size() - visible);
	const int top = 96 - visible * height / 2;
	for (int i = 0; i < visible; ++i)
		_font.drawString(&screen, lines[first + i], 29, top + i * height, 330, 0, Graphics::kTextAlignCenter);
}

void MortimerInterview::draw(Graphics::Surface &screen, const MortimerMovie &movie) const {
	if (fullScreen()) {
		screen.copyRectToSurface(movie.pixels(), MortimerMovie::kScreenWidth, 0, 0, 640, 480);
		for (uint i = 0; i < movie.captions().size(); ++i) {
			const MortimerMovie::Caption &caption = movie.captions()[i];
			if (caption.id < ARRAYSIZE(_captions))
				_captionFont.drawString(&screen, _captions[caption.id], 0, caption.y, MIN<int>(640, caption.x * 2), 0, Graphics::kTextAlignCenter);
		}
		return;
	}
	screen.fillRect(Common::Rect(640, 480), 10);
	blit(screen, _riddleView ? _gate[0] : _portraits[_selected < 0 ? 7 : _selected], 354, 69);
	if (playingMovie() && _stage != kHelp && _stage != kNoStones) {
		for (int y = 0; y < 320 && y + 69 < screen.h; ++y) {
			const byte *src = movie.pixels() + y * MortimerMovie::kScreenWidth;
			byte *dst = (byte *)screen.getBasePtr(354, y + 69);
			for (int x = 0; x < 640 - 354; ++x)
				if (src[x])
					dst[x] = src[x];
		}
	}
	if (_switchFrame < 6)
		blit(screen, _panel[10 + _switchFrame], 354, 69);
	blit(screen, _panel[0], 0, 0);
	if (_riddleView || _selected < 0) {
		blit(screen, _panel[7], 123, 175);
		blit(screen, _panel[1], (_frame / 4) % 2 ? 102 : 30, (_frame / 4) % 2 ? 282 : 286);
	} else if (_selected >= 0) {
		blit(screen, _panel[4 + _question], kQuestionOrigins[_question].x, kQuestionOrigins[_question].y);
	}
	if (_help)
		blit(screen, _panel[9], 400, 6);
	if (_solving)
		blit(screen, _panel[2], 11, 174);
	if (_selected >= 0)
		drawText(screen, _riddleView ? _riddles[_answer] : _dialogue[_selected][_question]);
}

void MortimerInterview::drawCursor(Graphics::Surface &screen, const Common::Point &point) const {
	if (!fullScreen()) {
		if (_stage == kPay)
			blit(screen, _panel[19 + _frame % 2], 508, 451);
		const int cursor = (_help ? 2 : 0) + (hitControl(point) < 0 ? 1 : 0);
		blit(screen, _cursor[cursor], point.x, point.y);
	}
}

int MortimerInterview::highlightedAnimal() const {
	if (_solving) {
		int animal = (_frame / 2) % 7;
		for (int i = 0; i < 7; ++i, animal = (animal + 1) % 7)
			if (_species & (1 << animal))
				return animal;
	}
	return _riddleView ? -1 : _selected;
}

} // End of namespace Scumm
