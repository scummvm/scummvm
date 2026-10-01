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
#include "scumm/insane/mortimer/campaign.h"

#include "common/file.h"
#include "common/tokenizer.h"
#include "common/util.h"
#include "graphics/surface.h"

namespace Scumm {

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

MortimerInterview::MortimerInterview(bool isDemo) : _random("mortimer"), _isDemo(isDemo),
	_region(0), _difficulty(0), _textDifficulty(0), _tried(0), _stage(kIdle), _effect(kNoEffect),
	_answer(-1), _previousAnswer(-1), _selected(-1), _question(0), _guess(-1), _lastAction(46), _textScroll(0), _guesses(3),
	_species(0), _stones(0), _frame(0), _switchFrame(6), _payment(0),
	_moviePending(false), _riddleView(false), _solving(false), _help(false), _helpExplained(false), _restartRegion(false) {
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
				answer = heading == "Easy" ? 0 : heading == "Medium" ? 1 : 2;
			} else {
				animal = answer = -1;
				for (uint i = 0; i < ARRAYSIZE(kAnimals); ++i)
					if (heading.equalsIgnoreCase(_isDemo ? kAnimals[i] : mortimerRegion(_region).names[i]))
						animal = i;
			}
		} else if (animal >= 0 && answer >= 0 && answer < 3) {
			Common::String &text = riddles ? _riddles[animal][answer] : _dialogue[animal][answer];
			if (!text.empty())
				text += '\n';
			text += line;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(kAnimals); ++i) {
		for (uint j = 0; j < (riddles && _isDemo ? 1U : 3U); ++j)
			if ((riddles ? _riddles[i][j] : _dialogue[i][j]).empty())
				return false;
	}
	return !file.err();
}

bool MortimerInterview::loadResources(uint region) {
	_region = region;
	for (uint i = 0; i < 7; ++i)
		for (uint j = 0; j < 3; ++j) {
			_dialogue[i][j].clear();
			_riddles[i][j].clear();
		}
	const MortimerRegion &resources = mortimerRegion(_region);
	const Common::String portraits = Common::String::format("%s/%c_ANIM_H.NUT", resources.directory, resources.prefix);
	const Common::String gate = Common::String::format("%s/%c_GATE_H.NUT", resources.directory, resources.prefix);
	struct Bank {
		const char *name;
		Common::Array<MortimerSprite> *sprites;
		uint count;
		byte *palette;
	};
	const Bank banks[] = {
		{"RESOURCE/GENERAL/INTERV_H.NUT", &_panel, 21, nullptr},
		{portraits.c_str(), &_portraits, 8, _palette},
		{gate.c_str(), &_gate, 1, _gatePalette},
		{_isDemo ? "RESOURCE/GENERAL/CURS_I_H.NUT" : "RESOURCE/GENERAL/ARROWS.NUT", &_cursor, 4, nullptr}
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
	if (!_font.load("RESOURCE/GENERAL/MORTFNTH.NUT") || !_captionFont.load(_isDemo ? "RESOURCE/GENERAL/SUBTFONT.NUT" : "RESOURCE/GENERAL/ESTRO20.NUT")) {
		_error = "Missing or invalid Mortimer font";
		return false;
	}
	if (!loadText("RESOURCE/DATA/DIALOG_F.DAT", false) || !loadText("RESOURCE/DATA/RIDDLE_F.DAT", true)) {
		_error = "Missing or invalid Mortimer dialogue or riddle text";
		return false;
	}
	if (!_isDemo)
		return true;
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
	_previousAnswer = _answer;
	_answer = -1;
	resetAttempts();
}

void MortimerInterview::resetAttempts() {
	_tried = 0;
	_guesses = _isDemo ? 3 : mortimerGuessAllowance(_difficulty);
}

void MortimerInterview::start(uint species, uint stones, int scene) {
	_species = species;
	_stones = stones;
	if (_isDemo || _answer < 0)
		_guesses = _isDemo ? 3 : mortimerGuessAllowance(_difficulty);
	_restartRegion = false;
	_frame = _payment = 0;
	_switchFrame = 6;
	_selected = -1;
	_question = _textScroll = 0;
	_lastAction = 46;
	_riddleView = _solving = _help = false;
	_effect = kNoEffect;
	if (_answer < 0) {
		if (_isDemo) {
			// SHELL 40cd65 restricts the demo to baboon, elephant and oxpecker.
			const int answers[] = {1, 3, 6};
			_answer = answers[_random.getRandomNumber(2)];
			_textDifficulty = 0;
		} else {
			do {
				_answer = _random.getRandomNumber(6);
			} while (_answer == _previousAnswer);
			const uint roll = _random.getRandomNumber(99);
			_textDifficulty = _difficulty == 1 ? (roll < 10 ? 0 : roll < 60 ? 1 : 2) : _difficulty;
		}
	}
	for (int i = 0; i < 7 && _selected < 0; ++i)
		if (_species & (1 << i))
			_selected = i;
	if (!_isDemo) {
		enter(scene == int(4 + _region) ? kPopup : scene == int(8 + _region) ? kIdle :
			scene == int(17 + _region) ? kSong : kGate);
		return;
	}
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
	if (!_isDemo) {
		const MortimerRegion &r = mortimerRegion(_region);
		switch (stage) {
		case kGate: _movieName = Common::String::format("%s/%cGATE.SAN", r.directory, r.prefix); break;
		case kSong: _movieName = Common::String::format("%s/%c_SONG_%d.SAN", r.directory, r.prefix, _answer < 4 ? 1 : 2); break;
		case kPopup: _movieName = Common::String::format("%s/%c_POPUPH.SAN", r.directory, r.prefix); break;
		case kAnswer: _movieName = Common::String::format("%s/%c_%s_%d.SAN", r.directory, r.prefix, r.animals[_selected], _question + 1); break;
		case kRiddle: _movieName = Common::String::format("%s/%c_RIDD_%d.SAN", r.directory, r.prefix, _answer < 4 ? 1 : 2); break;
		case kSolve: _movieName = Common::String::format("%s/%c_ANSWER.SAN", r.directory, r.prefix); break;
		case kCost: _movieName = Common::String::format("%s/%c_GLO_%u.SAN", r.directory, r.prefix, _difficulty); break;
		case kGuess: _movieName = Common::String::format("%s/ANS_%sH.SAN", r.directory, r.animals[_guess]); break;
		case kCorrect: _movieName = Common::String::format("%s/ANS%s_%u.SAN", r.directory, r.animals[_answer], _textDifficulty); break;
		case kWrong: _movieName = Common::String::format("%s/%c_WRONGH.SAN", r.directory, r.prefix); break;
		case kOpenGate: _movieName = Common::String::format("%s/%cGYESH.SAN", r.directory, r.prefix); break;
		case kNoAnimals: _movieName = "RESOURCE/GENERAL/NOSCOPE.SAN"; break;
		case kTried: _movieName = "RESOURCE/GENERAL/TRIEDIT.SAN"; break;
		default: break;
		}
	}
	_moviePending = !_movieName.empty();
}

int MortimerInterview::sceneId() const {
	if (_stage == kGate)
		return 12 + _region;
	if (_stage == kSong)
		return 17 + _region;
	if (_stage == kPopup)
		return 4 + _region;
	return 8 + _region;
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
			if (_isDemo && !_help && c.id >= 55 && c.id <= 61 && !(_species & (1 << (c.id - 55))))
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
	if (fullScreen() || _stage == kPay || _stage == kGuess || _stage == kCorrect || _stage == kWrong || _stage == kSolve || _stage == kNoStones || _stage == kCost || _stage == kNoAnimals)
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
		if (_isDemo && _help && !_helpExplained) {
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
	if (_selected < 0 && _isDemo)
		return;
	if (action == 62 || action == 63)
		action = _selected < 0 ? 52 : _lastAction;
	if (action >= 55 && action <= 61) {
		if (!_isDemo && !(_species & (1 << (action - 55)))) {
			enter(kHelp);
			_movieName = "RESOURCE/GENERAL/FLYAGAIN.SAN";
			_moviePending = true;
		} else if (_solving) {
			_guess = action - 55;
			if (!_isDemo && _difficulty == 0 && (_tried & (1 << _guess))) {
				enter(kTried);
				return;
			}
			_solving = false;
			_payment = _isDemo ? 5 : mortimerGuessCost(_region, _difficulty);
			if (_stones < _payment) {
				_payment = 0;
				enter(kNoStones);
				return;
			}
			_tried |= 1 << _guess;
			if (_guesses > 0)
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
		enter(action == 52 ? kRiddle : (!_isDemo && !_species ? kNoAnimals : kSolve));
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
		if (!_isDemo) {
			enter(kCost);
			break;
		}
		// Fall through.
	case kCost:
		if (_stones < (_isDemo ? 5 : mortimerGuessCost(_region, _difficulty))) {
			enter(kNoStones);
		} else {
			_solving = true;
			enter(kIdle);
		}
		break;
	case kGuess: enter(_guess == _answer ? kCorrect : kWrong); break;
	case kCorrect: enter(kOpenGate); break;
	case kWrong:
		if (!_guesses) {
			_restartRegion = !_isDemo;
			if (_restartRegion)
				resetRiddle();
			enter(kFlyAway);
		} else
			enter(kIdle);
		break;
	case kOpenGate: enter(_isDemo ? kEnding : kFinished); break;
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
		_font.drawString(&screen, lines[first + i], 29, top + i * height, 330, _isDemo ? 0 : 1, Graphics::kTextAlignCenter);
}

void MortimerInterview::draw(Graphics::Surface &screen, const MortimerMovie &movie) const {
	if (fullScreen()) {
		screen.copyRectToSurface(movie.pixels(), MortimerMovie::kScreenWidth, 0, 0, 640, 480);
		for (uint i = 0; _isDemo && i < movie.captions().size(); ++i) {
			const MortimerMovie::Caption &caption = movie.captions()[i];
			if (caption.id < ARRAYSIZE(_captions))
				_captionFont.drawString(&screen, _captions[caption.id], 0, caption.y, MIN<int>(640, caption.x * 2), 0, Graphics::kTextAlignCenter);
		}
		return;
	}
	screen.fillRect(Common::Rect(640, 480), 10);
	const MortimerSprite &portrait = _riddleView ? _gate[0] : _portraits[_selected < 0 ? 7 : _selected];
	portrait.draw(screen, 354, 69);
	if (playingMovie() && _stage != kHelp && _stage != kNoStones) {
		Common::Rect clip(354, 69, 640, 389);
		clip.clip(Common::Rect(screen.w, screen.h));
		if (!clip.isEmpty())
			screen.copyRectToSurfaceWithKey(movie.pixels(), MortimerMovie::kScreenWidth,
				clip.left, clip.top, clip.width(), clip.height(), 0);
	}
	if (_switchFrame < 6)
		_panel[10 + _switchFrame].draw(screen, 354, 69);
	_panel[0].draw(screen, 0, 0);
	if (_riddleView || _selected < 0) {
		_panel[7].draw(screen, 123, 175);
		_panel[1].draw(screen, (_frame / 4) % 2 ? 102 : 30, (_frame / 4) % 2 ? 282 : 286);
	} else if (_selected >= 0) {
		_panel[4 + _question].draw(screen, kQuestionOrigins[_question].x, kQuestionOrigins[_question].y);
	}
	if (_help)
		_panel[9].draw(screen, 400, 6);
	if (_solving)
		_panel[2].draw(screen, 11, 174);
	if (_selected >= 0 || _riddleView)
		drawText(screen, _riddleView ? _riddles[_answer][_textDifficulty] : _dialogue[_selected][_question]);
}

void MortimerInterview::drawCursor(Graphics::Surface &screen, const Common::Point &point) const {
	if (!fullScreen()) {
		if (_stage == kPay)
			_panel[19 + _frame % 2].draw(screen, 508, 451);
		const int cursor = (_help ? 2 : 0) + (hitControl(point) < 0 ? 1 : 0);
		_cursor[cursor].draw(screen, point.x, point.y);
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
