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

#include "scumm/insane/mortimer/castle.h"
#include "scumm/insane/mortimer/campaign.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/surface.h"

namespace Scumm {

// SHELL 4439e0: explanation, starting animal, then five correct/distractor pairs.
const byte kCastlePuzzles[3][8][12] = {
	{
		{0, 34, 13, 11, 6, 1, 12, 0, 25, 4, 31, 21},
		{1, 30, 24, 6, 32, 35, 0, 25, 13, 12, 2, 22},
		{3, 24, 22, 32, 12, 5, 26, 4, 10, 36, 25, 0},
		{5, 11, 22, 25, 26, 13, 5, 36, 23, 0, 35, 32},
		{6, 10, 13, 21, 11, 32, 26, 35, 5, 12, 34, 33},
		{8, 34, 16, 2, 3, 24, 15, 32, 0, 31, 20, 13},
		{9, 21, 26, 32, 3, 13, 15, 25, 4, 31, 24, 11},
		{10, 32, 33, 21, 13, 10, 31, 26, 11, 15, 6, 0}
	},
	{
		{0, 34, 13, 30, 6, 4, 12, 35, 25, 2, 31, 20},
		{1, 30, 24, 36, 32, 22, 0, 35, 13, 6, 2, 12},
		{4, 22, 13, 5, 25, 20, 21, 14, 12, 2, 11, 15},
		{5, 11, 22, 13, 26, 30, 5, 0, 23, 13, 35, 21},
		{6, 10, 13, 22, 11, 12, 26, 36, 5, 23, 34, 25},
		{7, 26, 11, 35, 21, 32, 24, 12, 22, 36, 23, 33},
		{8, 34, 16, 13, 3, 10, 15, 32, 0, 11, 20, 5},
		{10, 32, 33, 0, 13, 12, 31, 15, 11, 3, 6, 10}
	},
	{
		{3, 24, 22, 3, 12, 14, 26, 6, 10, 0, 25, 32},
		{4, 22, 13, 3, 25, 20, 21, 36, 12, 0, 11, 10},
		{7, 26, 11, 20, 21, 0, 24, 25, 22, 2, 23, 12},
		{11, 21, 20, 3, 6, 15, 16, 34, 10, 13, 12, 14},
		{12, 24, 35, 22, 4, 10, 33, 3, 2, 26, 5, 0},
		{13, 5, 13, 0, 32, 15, 24, 35, 16, 14, 2, 3},
		{9, 21, 26, 20, 3, 11, 15, 2, 4, 16, 24, 1},
		{11, 24, 20, 34, 6, 15, 16, 13, 10, 4, 12, 0}
	}
};

MortimerCastle::MortimerCastle() : MortimerFlight(false), _random("mortimer-castle"),
	_nextVoice(0), _flightDifficulty(0), _riddleDifficulty(0), _puzzle(0), _step(0), _damage(0), _checkpointDamage(0),
	_wrongChoices(0), _crashes(0), _frame(0), _choiceFrame(0), _hintFrames(0), _holdFrames(0), _answer(0), _selection(-1),
	_stage(kEntry), _moviePending(false), _choosing(false), _leaving(false), _correct(false), _resourcesLoaded(false) {
	memset(_rooms, 0, sizeof(_rooms));
	memset(_sides, 0, sizeof(_sides));
}

bool MortimerCastle::loadText() {
	Common::File file;
	if (!file.open("RESOURCE/DATA/RIDDLE_F.DAT"))
		return false;
	int answer = -1, difficulty = -1;
	while (!file.eos() && !file.err()) {
		Common::String line = file.readLine();
		line.trim();
		if (line.empty())
			continue;
		if (line[0] == '#') {
			const Common::String heading = line.substr(1);
			if (heading == "Medium" || heading == "Hard" || heading == "Easy") {
				difficulty = heading == "Medium" ? 0 : heading == "Hard" ? 1 : -1;
			} else {
				answer = heading == "Mortimer" ? 0 : heading == "Lodius" ? 1 : -1;
				difficulty = -1;
			}
		} else if (answer >= 0 && difficulty >= 0 && line[0] != 0x1a) {
			Common::String &text = _riddles[answer][difficulty];
			if (!text.empty())
				text += '\n';
			text += line;
		}
	}
	if (file.err())
		return false;
	file.close();
	if (!file.open("RESOURCE/DATA/CASTLE.DAT"))
		return false;
	bool hint = false;
	while (!file.eos() && !file.err()) {
		Common::String line = file.readLine();
		line.trim();
		if (line.empty())
			continue;
		if (line[0] == '#') {
			const Common::String heading = line.substr(1);
			hint = heading == (context().girl ? "In Common g" : "In Common b");
		} else if (hint && line[0] != 0x1a) {
			if (!_hint.empty())
				_hint += '\n';
			_hint += line;
		}
	}
	for (uint i = 0; i < 2; ++i)
		for (uint j = 0; j < 2; ++j)
			if (_riddles[i][j].empty())
				return false;
	return !file.err() && !_hint.empty();
}

bool MortimerCastle::loadResources() {
	if (_resourcesLoaded)
		return true;
	struct Bank {
		const char *name;
		Common::Array<MortimerSprite> *sprites;
		uint count;
	};
	const Bank banks[] = {
		{"RESOURCE/CASTLE/GOGGLES.NUT", &_goggles, 11},
		{"RESOURCE/CASTLE/C_ICON_H.NUT", &_icons, 28},
		{"RESOURCE/CASTLE/SPIN.NUT", &_spin, 4},
		{"RESOURCE/CASTLE/FLAME1.NUT", &_flames[0], 5},
		{"RESOURCE/CASTLE/FLAME2.NUT", &_flames[1], 5},
		{"RESOURCE/CASTLE/FLAME3.NUT", &_flames[2], 5},
		{"RESOURCE/CASTLE/FLAME4.NUT", &_flames[3], 5},
		{"RESOURCE/CASTLE/CG_MOUTH.NUT", &_mouth, 7},
		{"RESOURCE/GENERAL/ARROWS.NUT", &_cursor, 4}
	};
	Common::File file;
	for (uint i = 0; i < ARRAYSIZE(banks); ++i) {
		if (!file.open(Common::Path(banks[i].name)) ||
			!loadMortimerSprites(file, *banks[i].sprites) || banks[i].sprites->size() < banks[i].count) {
			_error = Common::String::format("Missing or invalid %s", banks[i].name);
			return false;
		}
		file.close();
	}
	for (uint region = 0; region < 4; ++region) {
		for (uint i = 0; i < 7; ++i) {
			const Common::String name = Common::String::format("RESOURCE/CASTLE/PLAQ_%s.NUT", mortimerRegion(region).animals[i]);
			if (!file.open(Common::Path(name)) || !loadMortimerSprites(file, _plaques[region * 7 + i]) || _plaques[region * 7 + i].size() < 2) {
				_error = Common::String::format("Missing or invalid %s", name.c_str());
				return false;
			}
			file.close();
		}
	}
	if (!_font.load("RESOURCE/GENERAL/MORTFNTH.NUT") || !_captionFont.load("RESOURCE/GENERAL/ESTRO20.NUT")) {
		_error = "Missing or invalid Mortimer castle fonts";
		return false;
	}
	_resourcesLoaded = true;
	return true;
}

bool MortimerCastle::start(int scene, uint flightDifficulty, uint riddleDifficulty, bool girl) {
	_error.clear();
	context().girl = girl;
	_flightDifficulty = MIN(flightDifficulty, 2U);
	_riddleDifficulty = MIN(riddleDifficulty, 2U);
	setCollisionRadius(20 * (_flightDifficulty + 1));
	for (uint i = 0; i < 2; ++i)
		for (uint j = 0; j < 2; ++j)
			_riddles[i][j].clear();
	_hint.clear();
	if (!loadResources())
		return false;
	if (!loadText()) {
		_error = "Missing or invalid Mortimer castle text";
		return false;
	}
	const Common::String name = Common::String::format("RESOURCE/CASTLE/CAS_A_H%c.SAD", girl ? 'G' : 'B');
	Common::File file;
	if (!file.open(Common::Path(name)) || !loadMortimerSound(file, _hintSound)) {
		_error = Common::String::format("Missing or invalid %s", name.c_str());
		return false;
	}
	_frame = _hintFrames = 0;
	_voices.clear();
	_nextVoice = 0;
	_answer = _random.getRandomBit();
	context().riddle = _answer * 10 + (_riddleDifficulty != 0);
	if (scene == 21)
		return newPuzzle();
	enter(scene == 16 ? kGate : scene == 22 ? kSong : kIdle);
	return true;
}

bool MortimerCastle::newPuzzle() {
	_puzzle = _random.getRandomNumber(7);
	_step = _damage = _checkpointDamage = _wrongChoices = _crashes = 0;
	uint rooms[] = {0, 1, 2, 3, 4};
	for (uint i = 0; i < 5; ++i) {
		const uint choice = _random.getRandomNumber(4 - i);
		_rooms[i] = rooms[choice];
		for (uint j = choice; j < 4 - i; ++j)
			rooms[j] = rooms[j + 1];
		_sides[i] = _random.getRandomBit();
	}
	// SHELL includes the entry in its side slots, leaving the last pair on the right.
	_sides[5] = 0;
	const Common::String name = Common::String::format("RESOURCE/CASTLE/C_CUZ%02u%c.SAD",
		kCastlePuzzles[_riddleDifficulty][_puzzle][0], context().girl ? 'G' : 'B');
	Common::File file;
	if (!file.open(Common::Path(name)) || !loadMortimerSound(file, _explanation)) {
		_error = Common::String::format("Missing or invalid %s", name.c_str());
		return false;
	}
	enter(kEntry);
	debug(1, "Mortimer castle: puzzle %u, flight difficulty %u, riddle difficulty %u", _puzzle, _flightDifficulty, _riddleDifficulty);
	return true;
}

void MortimerCastle::enter(Stage stage) {
	_stage = stage;
	_choosing = _leaving = false;
	_selection = -1;
	_choiceFrame = 0;
	_holdFrames = 0;
	_voices.clear();
	_nextVoice = 0;
	const char *name = nullptr;
	switch (stage) {
	case kEntry: name = "CS_ENTRY.SAN"; break;
	case kRoom:
		_movieName = Common::String::format("RESOURCE/CASTLE/CS_%c.SAN", 'A' + _rooms[_step ? _step - 1 : 0]);
		break;
	case kLeft: name = "CS_LEFT.SAN"; break;
	case kDrop: name = "CS_DROP.SAN"; break;
	case kExit: name = "CS_EXIT.SAN"; _voices.push_back(3); break;
	case kGate:
		_answer = _random.getRandomBit();
		context().riddle = _answer * 10 + (_riddleDifficulty != 0);
		name = "CS_GATE.SAN";
		break;
	case kSong: name = "C_SONG_1.SAN"; break;
	case kIdle: name = "CASTLE_I.NUT"; break;
	case kRiddle: name = "C_RIDD_1.SAN"; break;
	case kWrong: name = "C_WRONGH.SAN"; break;
	case kCorrect: name = "CGYESH.SAN"; break;
	case kFinished: break;
	}
	if (name)
		_movieName = Common::String("RESOURCE/CASTLE/") + name;
	_moviePending = stage != kFinished;
}

bool MortimerCastle::loadMovie() {
	const MortimerMovieContext savedContext = context();
	clearProgress();
	context() = savedContext;
	for (uint i = 0; i < 2; ++i) {
		_speechBlocks[i] = SpeechBlock();
		_speech[i] = MortimerSound();
	}
	Common::File file;
	if (!file.open(Common::Path(_movieName))) {
		_error = Common::String::format("Missing %s", _movieName.c_str());
		return false;
	}
	if (!MortimerFlight::load(file, _stage <= kExit && _stage != kEntry)) {
		_error = Common::String::format("%s: %s", _movieName.c_str(), MortimerMovie::errorMessage().c_str());
		return false;
	}
	setWeapon(kSlimeGun);
	_moviePending = false;
	debug(1, "Mortimer castle: %s, step %u, damage %u, wrong choices %u, crashes %u",
		_movieName.c_str(), _step, _damage, _wrongChoices, _crashes);
	return true;
}

int MortimerCastle::animal(uint side) const {
	const byte *puzzle = kCastlePuzzles[_riddleDifficulty][_puzzle];
	if (!_step)
		return side ? -1 : puzzle[1];
	return _step <= 5 ? puzzle[_step * 2 + (_sides[_step] != side)] : -1;
}

void MortimerCastle::handleAction(const byte *data, uint32 size) {
	MortimerFlight::handleAction(data, size);
	const uint code = READ_LE_UINT16(data), flags = READ_LE_UINT16(data + 2);
	if (code == 6 && flags == 39 && (_stage == kEntry || _stage == kRoom)) {
		showChoices();
	} else if (code == 8 && flags == 46 && size >= 18) {
		const uint id = READ_LE_UINT16(data + 6);
		if (id / 1000 != 7 || id / 100 - 70 != uint(context().girl))
			return;
		const uint block = READ_LE_UINT16(data + 10), count = READ_LE_UINT16(data + 12);
		const uint remaining = READ_LE_UINT32(data + 14);
		const uint length = size - 18;
		if (block >= count || length > remaining || remaining > 25000) {
			_error = Common::String::format("Invalid castle speech %u block %u/%u in %s at frame %u",
				id, block, count, _movieName.c_str(), frame());
			return;
		}
		for (uint side = 0; side < 2; ++side) {
			if (int(id % 100) != animal(side))
				continue;
			SpeechBlock &speech = _speechBlocks[side];
			if (!block) {
				speech = SpeechBlock();
				speech.count = count;
				speech.size = remaining;
			}
			if (speech.next != block || speech.count != count || speech.size - speech.data.size() != remaining) {
				_error = Common::String::format("Incomplete castle speech %u block %u/%u in %s at frame %u",
					id, block, count, _movieName.c_str(), frame());
				return;
			}
			const uint offset = speech.data.size();
			speech.data.resize(offset + length);
			memcpy(speech.data.begin() + offset, data + 18, length);
			++speech.next;
		}
	}
}

void MortimerCastle::showChoices() {
	_choosing = true;
	_choiceFrame = 0;
	_selection = -1;
	_voices.clear();
	_nextVoice = 0;
	// The left name is spoken first; the entry has only a right plaque.
	for (int side = 1; side >= 0; --side) {
		if (animal(side) < 0)
			continue;
		const SpeechBlock &speech = _speechBlocks[side];
		const uint size = speech.data.size() >= 8 ? READ_BE_UINT32(speech.data.begin() + 4) : 0;
		if (!speech.count || speech.next != speech.count || speech.data.size() != speech.size ||
			speech.data.size() < 8 || size != speech.data.size() - 8) {
			_error = Common::String::format("Incomplete castle plaque speech in %s for animal %d", _movieName.c_str(), animal(side));
			return;
		}
		Common::MemoryReadStream stream(speech.data.begin(), size + 8);
		if (!loadMortimerSound(stream, _speech[side])) {
			_error = Common::String::format("Missing or invalid castle plaque speech in %s for animal %d", _movieName.c_str(), animal(side));
			return;
		}
		_voices.push_back(side);
	}
	if (_step == 1)
		_voices.push_back(2);
}

const MortimerSound *MortimerCastle::takeSpeech(bool playing) {
	if (playing || !hasSpeech())
		return nullptr;
	const uint voice = _voices[_nextVoice++];
	if (voice == 2) {
		_hintFrames = 60;
		return &_hintSound;
	}
	return voice == 3 ? &_explanation : &_speech[voice];
}

void MortimerCastle::update() {
	++_frame;
	if (_holdFrames)
		--_holdFrames;
	if (_hintFrames)
		--_hintFrames;
	if (!_choosing)
		return;
	++_choiceFrame;
	if (_selection < 0 || _choiceFrame < 2)
		return;
	_choosing = false;
	_leaving = true;
	_hintFrames = 0;
	if (_selection == 1)
		enter(kLeft);
}

void MortimerCastle::click(const Common::Point &point) {
	if (_choosing && _selection < 0) {
		_selection = _step ? (point.x <= 310 ? 1 : 0) : 0;
		_correct = !_step || uint(_selection) == _sides[_step];
		_choiceFrame = 0;
		_voices.clear();
		_nextVoice = 0;
	} else if (_stage == kIdle) {
		if (Common::Rect(184, 341, 459, 447).contains(point)) {
			enter(kRiddle);
		} else if (Common::Rect(0, 70, 85, 372).contains(point)) {
			enter(_answer == 0 ? kCorrect : kWrong);
		} else if (Common::Rect(558, 70, 640, 371).contains(point)) {
			enter(_answer == 1 ? kCorrect : kWrong);
		} else if (Common::Rect(295, 201, 353, 245).contains(point) ||
			Common::Rect(385, 205, 422, 247).contains(point) || Common::Rect(225, 208, 260, 247).contains(point)) {
			enter(kWrong);
		}
	}
}

void MortimerCastle::resolveChoice() {
	if (!_correct) {
		enter(kDrop);
		return;
	}
	_damage -= _damage / (2 + _flightDifficulty);
	_checkpointDamage = _damage;
	++_step;
	enter(_step < 6 ? kRoom : kExit);
}

void MortimerCastle::retry(bool crash) {
	const uint allowances[] = {5, 3, 3};
	const bool resetPuzzle = crash ? ++_crashes >= 4 : ++_wrongChoices >= allowances[_flightDifficulty];
	if (resetPuzzle) {
		newPuzzle();
	} else {
		_damage = _checkpointDamage;
		enter(kRoom);
		if (crash)
			_holdFrames = 7;
	}
}

void MortimerCastle::hit() {
	if (_damage < 14)
		++_damage;
	else
		retry(true);
}

void MortimerCastle::movieFinished() {
	switch (_stage) {
	case kEntry:
	case kRoom:
		if (_leaving)
			resolveChoice();
		else
			_error = Common::String::format("Castle movie %s ended without a plaque choice", _movieName.c_str());
		break;
	case kLeft: resolveChoice(); break;
	case kDrop: retry(false); break;
	case kExit: enter(kGate); break;
	case kGate: enter(kSong); break;
	case kSong:
	case kRiddle: enter(kIdle); break;
	case kWrong: newPuzzle(); break;
	case kCorrect: enter(kFinished); break;
	case kIdle:
	case kFinished: break;
	}
}

bool MortimerCastle::skip() {
	if (_choosing || _stage == kIdle)
		return true;
	if ((_stage == kEntry || _stage == kRoom) && !_leaving) {
		while (!_choosing) {
			if (!advance()) {
				if (errorMessage().empty())
					movieFinished();
				break;
			}
			if (!_error.empty())
				break;
		}
	} else {
		movieFinished();
	}
	return errorMessage().empty();
}

int MortimerCastle::sceneId() const {
	return _stage <= kExit ? 21 : _stage == kGate ? 16 : _stage == kSong ? 22 : 23;
}

const char *MortimerCastle::subtitleName() const {
	return _stage == kGate || _stage == kCorrect ? "RESOURCE/DATA/ENDTEXT.TRS" : "RESOURCE/DATA/CAS_SONG.TRS";
}

void MortimerCastle::drawText(Graphics::Surface &screen, const Common::String &text, int x, int y, int width, byte color) const {
	Common::Array<Common::String> lines;
	_font.wordWrapText(text, width, lines);
	y -= lines.size() * _font.getFontHeight() / 2;
	for (uint i = 0; i < lines.size(); ++i)
		_font.drawString(&screen, lines[i], x, y + i * _font.getFontHeight(), width, color, Graphics::kTextAlignCenter);
}

void MortimerCastle::drawOverlay(Graphics::Surface &screen, const Common::Point &mouse, const Common::Point &origin, byte mouth) const {
	if (courseView()) {
		_goggles[0].draw(screen, 0, 0);
		const uint gauge = _damage >= 13 ? 9 + _frame % 2 : _damage >= 10 ? _damage - 4 : 1 + _damage / 2;
		_goggles[gauge].draw(screen, 0, 0);
		const Common::Point origins[] = {{355, 85}, {413, 86}, {470, 87}, {527, 88}, {584, 90}};
		const byte *puzzle = kCastlePuzzles[_riddleDifficulty][_puzzle];
		for (uint i = 0; i < MIN(_step, 5U); ++i) {
			const uint code = puzzle[i ? i * 2 : 1];
			_icons[code / 10 * 7 + code % 10].draw(screen, origins[i].x, origins[i].y, true);
		}
		if (_choosing) {
			const Common::Rect clip(8, 104, 632, 448);
			for (uint side = 0; side < 2; ++side) {
				const int code = animal(side);
				if (code < 0)
					continue;
				const Common::Array<MortimerSprite> &plaque = _plaques[code / 10 * 7 + code % 10];
				const MortimerSprite &sprite = _selection >= 0 ? _spin[2 + MIN(_choiceFrame, 1U)] :
					_choiceFrame < 2 ? _spin[_choiceFrame] : plaque[(_choiceFrame / 2) % 2];
				sprite.draw(screen, origin.x + (side ? 248 : 404), origin.y + 132, false, &clip, 2);
				drawText(screen, mortimerRegion(code / 10).names[code % 10], side ? 26 : 416, 35, 208, 5);
			}
			const Common::Point flames[] = {{43, 131}, {101, 68}, {258, 68}, {325, 131}};
			for (uint i = 0; i < 4; ++i)
				if (_step || (i != 0 && i != 3))
					_flames[i][_frame % 5].draw(screen, origin.x + flames[i].x * 2, origin.y + flames[i].y * 2, false, &clip, 2);
		}
		if (_hintFrames) {
			Common::Array<Common::String> lines;
			_captionFont.wordWrapText(_hint, 600, lines);
			for (uint i = 0; i < lines.size(); ++i)
				_captionFont.drawString(&screen, lines[i], 20, 480 - (lines.size() - i) * _captionFont.getFontHeight(), 600,
					context().girl ? 253 : 5, Graphics::kTextAlignCenter);
		}
	} else if (_stage == kIdle || _stage == kRiddle || _stage == kSong) {
		_mouth[MIN<uint>(mouth, 6)].draw(screen, 199, _stage == kSong ? 263 : 264);
		if (_stage == kIdle) {
			drawText(screen, _riddles[_answer][_riddleDifficulty != 0], 184, 394, 274, 5);
			_cursor[0].draw(screen, mouse.x, mouse.y);
		}
	}
}

} // End of namespace Scumm
