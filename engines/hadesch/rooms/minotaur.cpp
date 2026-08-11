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
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/ambient.h"

namespace Hadesch {
static const char *kHighlightImage = "r6010ol0";
static const char *kMaterialsImage = "r6010ok0";
static const char *kMaterialsMoveImage = "r6020ba0";
static const char *kMinotaurImage = "r6040ba0";
static const char *kDigits = "0123456789";
static const int numSquares = 25;
static const char *minotaurStates[] = {
	"",
	"",
	"",
	"inactive",
	"encourage",
	"trapped",
	"escaped",
	"critical",
	"level",
	"idle"
};

enum DaedalusDialogState {
	kMinotaur0 = 0,
	kMinotaur1 = 1,
	kMinotaur2 = 2,
	kMinotaurInactive = 3,
	kMinotaurEncourage = 4,
	kMinotaurTrapped = 5,
	kMinotaurEscaped = 6,
	kMinotaurCritical = 7,
	kMinotaurLevel = 8,
	kMinotaurIdle = 9
};

enum Strength {
	kStrengthStraw = 1,
	kStrengthWood = 2,
	kStrengthBricks = 3,
	kStrengthStone = 4
};

enum Position {
	kDown = 0,
	kLeft = 1,
	kUp = 2,
	kRight = 3
};

static const char *dirnames[] = {
	"down",
	"left",
	"up",
	"right"
};

enum {
	kRerenderLabyrinth = 1017001
};

static const char *daedalusSoundSMK[] = {
	"R6100nA0",
	"R6100wA0",
	"R6100nB0",
	"R6100wC0",
	"R6100nD0",
};

static const TranscribedSound daedalusSoundsAIF[] = {
	{"R6100nH0", _hs("Help us to move the walls so that they are strong enough to stop the minotaur")},
	{"R6100nL0", _hs("Click on a square to rotate the walls")},
	{"R6100nG0", _hs("Some walls are already locked in place and won't rotate")},
	{"R6100nK0", _hs("If you need help, refer to workman's equations")},
	{"R6170nA0", _hs("Careful, my friend. Some of the walls are not strong enough")},
	{"R6150nA0", _hs("You're a brave bullfighter, my friend")},
	{"R6150nB0", _hs("Keep it up. It looks like he's tiring")},
	{"R6150nC0", _hs("That's taking the bull by the horns")},
	{"R6150nD0", _hs("Don't give up. You can't beat him")},
	{"R6180nA0", _hs("You have beaten the Minotaur. You have the makings of a hero")},
	{"R6180nC0", _hs("You have beaten the beast at last")},
	{"R6180nD0", _hs("You have done it. The people of Crete are once again safe")},
	{"R6170nC0", _hs("Let's try again")},
	{"R6170nD0", _hs("Warn the people of Crete: the Minotaur has escaped. Workers, keep the Minotaur back in the labyrinth")},
	{"R6170nE0", _hs("I believe you and the Minotaur have not seen the last of one another")},
	{"R6170nF0", _hs("Ah that was a nobble effort, my friend")},
	{"R6160nA0", _hs("The Minotaur has broken though a critical wall. Workers, calm on the beast")},
	{"R6090eA0", _hs("Eh. Hm")},
	{"R6190nA0", _hs("Ok. Onto level two")},
	{"R6190nB0", _hs("Onto level three")},
};

struct Wall {
	int _id;
	bool _isCritical;
	int _inTransit;
	Strength _strength;
	Position _position;

	Wall() {
		_id = -1;
		_isCritical = false;
		_strength = kStrengthStraw;
		_position = kDown;
		_inTransit = 0;
	}
};

struct Cell {
	Common::Array<Wall> _movableWalls;
	Common::Array<Wall> _immovableWalls;
	bool _isRotatable;

	Cell() {
		_isRotatable = false;
	}
};

struct Labyrinth {
	Cell _cells[25];
};

class MinotaurHandler : public Handler {
public:
	MinotaurHandler() {
		_highlight = -1;

		_dialogState = kMinotaur0;
		_minotaurX = 1;
		_minotaurY = 2;
		_minotaurTileId = 7;
		_soundCounter = 0;
		_soundMax = 5;

		_lastChargeSound = -1;
		_lastTrappedSound = -1;
		_lastEscapedSound = -1;
		_levelId = 0;

		// consts
		xVector = Common::Point(-55, -33);
		yVector = Common::Point(+55, -33);
	}

	void handleClick(const Common::String &name) override {
		if (name.firstChar() >= '0' && name.firstChar() <= '9') {
			rotate(name.asUint64());
			renderLabyrinth();
			return;
		}
		/* TODO: MNSH: Daedalus */
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		switch (eventId) {
		case kRerenderLabyrinth:
			renderLabyrinth();
			break;
		case 17953:
			g_vm->addTimer(17954, 300);
			break;
		case 17954:
			treatDialogState();
			break;
		}
	}

	void handleMouseOver(const Common::String &name) override {
		if (name.firstChar() >= '0' && name.firstChar() <= '9')
			_highlight = name.asUint64();
		else
			_highlight = -1;
		renderLabyrinth();
	}
	void handleMouseOut(const Common::String &name) override {
		_highlight = -1;
		renderLabyrinth();
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->loadHotZones("minotaur.hot", true);
		// TODO: load other puzzles
		loadPuzzle("3x3j");
		room->addStaticLayer("r6010pA0", 10000);
		room->addStaticLayer("r6010tA0", 6400);
		room->addStaticLayer("r6010oA0", 5500);
		room->addStaticLayer("r6010oB0", 4000);
		renderLabyrinth();
		g_vm->getHeroBelt()->setColour(HeroBelt::kCool);
		setDialogState(kMinotaur1);
	}

	bool handleCheat(const Common::String &cheat) override {
		for (unsigned i = 0; i < ARRAYSIZE(minotaurStates); i++)
			if (minotaurStates[i][0] && minotaurStates[i] == cheat) {
				setDialogState(DaedalusDialogState(i));
				return true;
			}
		return false;
	}

private:
	void setDialogState(DaedalusDialogState state) {
		if (_dialogState) {
			// TODO
		} else {
			_dialogState = state;
		}
	}

	void playDaedalusSound(int index) {
		// TODO: balance
		_currentSound = index;
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (index < ARRAYSIZE(daedalusSoundSMK))
			room->playVideo(daedalusSoundSMK[index], 17953);
		else
			room->playSpeech(daedalusSoundsAIF[index-ARRAYSIZE(daedalusSoundSMK)], 17953);
	}

	void playDaedalusSoundWrap() {
		Persistent *persistent = g_vm->getPersistent();
		int index = 0;
		switch (_dialogState) {
		case kMinotaur0:
			treatDialogState();
			return;
		case kMinotaur1:
			index = _soundCounter;
			break;
		case kMinotaur2:
			index = _soundCounter + 5;
			break;
		case kMinotaurInactive:
			index = 9;
			break;
		case kMinotaurEncourage:
			index = randomExcept(10, 13, _lastChargeSound);
			_lastChargeSound = index;
			break;
		case kMinotaurTrapped:
			if (persistent->_quest == kCreteQuest)
				index = 14;
			else
				index = randomExcept(14, 16, _lastTrappedSound);
			_lastTrappedSound = index;
			break;
		case kMinotaurEscaped:
			index = randomExcept(17, 20, _lastEscapedSound);
			_lastEscapedSound = index;
			break;
		case kMinotaurCritical:
			index = 21;
			break;
		case kMinotaurLevel:
			index = 23 + _levelId / 15;
			break;
		case kMinotaurIdle:
			index = 22;
			break;
		}

		playDaedalusSound(index);
	}

	int randomExcept(int from, int to, int except) {
		Common::RandomSource &r = g_vm->getRnd();
		if (except < from || except > to)
			return r.getRandomNumberRng(from, to);
		int x = r.getRandomNumberRng(from, to - 1);
		if (x >= except)
			x++;
		return x;
	}

	void treatDialogState() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		switch(_dialogState) {
		case kMinotaur1:
			if (_soundCounter < _soundMax) {
				playDaedalusSoundWrap();
				_soundCounter++;
				return;
			}
			setDialogState(kMinotaur2);
			return;
		case kMinotaur2:
			if (_soundCounter < _soundMax) {
				playDaedalusSoundWrap();
				_soundCounter++;
				return;
			}
			room->enableMouse();
			setDialogState(kMinotaur0);
			return;
		default:
			// TODO: implement this;
			return;
		}
	}

	void renderLabyrinth() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_highlight >= 0)
			room->selectFrame(kHighlightImage, 9990, 0,
					  getTileBase(_highlight)
					  - Common::Point(34, 52));
		else
			room->stopAnim(kHighlightImage);

		for (int cell = 0; cell < numSquares; cell++) {
			for (int j = 0; j < ARRAYSIZE(dirnames); j++) {
				room->stopAnim(
					LayerId(kMaterialsImage, cell, Common::String(dirnames[j]) + "outer"));
				room->stopAnim(
					LayerId(kMaterialsImage, cell, Common::String(dirnames[j]) + "inner"));
				room->stopAnim(
					LayerId(kMaterialsMoveImage, cell, "to-" + Common::String(dirnames[j])));
			}

			for (int j = 0; j < (int) _current._cells[cell]._movableWalls.size(); j++) {
				renderWall(cell, _current._cells[cell]._movableWalls[j], false);
			}

			// Both original engine and us we're not able to handle 3
			// walls in the same place. The only way to avoid this from every hapenning
			// is to never put an immovable wall between 2 cells with movable walls.
			// Hence if we have any movable walls then we can make immovable walls outer
			// and they will not conflict with existing labyrinths.
			// If we have no movable walls we can easily put all immovable walls as inner

			bool immovableAreOuter = !_current._cells[cell]._movableWalls.empty();

			for (int j = 0; j < (int) _current._cells[cell]._immovableWalls.size(); j++) {
				renderWall(cell, _current._cells[cell]._immovableWalls[j], immovableAreOuter);
			}
		}

		// TODO: which frame?
		room->selectFrame(kMinotaurImage, getMinotaurZ(), 30,
				  getTileBase(_minotaurX, _minotaurY) - Common::Point(/*114, 117*/30+82, 33*2+52));
	}

	int getMinotaurZ() {
		if (_minotaurX >= 5) {
			return 6500;
		}

		if (_minotaurX < 0) {
			return 4500;
		}

		if (_minotaurY >= 5) {
			return 5960;
		}

		if (_minotaurY < 0) {
			return 4500;
		}

		return 5000 + 150 * (_minotaurX + _minotaurY) + 60;
	}

	void renderWall(int cell, Wall &wall, bool outer) {
		Common::Point delta;
		if (wall._inTransit) {
			wall._inTransit--;
 			g_vm->getVideoRoom()->selectFrame(
				LayerId(kMaterialsMoveImage, cell, Common::String("to-") + dirnames[wall._position]),
				getWallZ(cell, wall._position, outer),
				(wall._strength - kStrengthStraw) * 4 + (wall._position + 1) % 4,
				getTileBase(cell) + Common::Point(-40, -88));
			g_vm->addTimer(kRerenderLabyrinth, kDefaultSpeed);
			return;
		}

		switch (wall._position) {
		case kUp:
			delta = xVector + xVector + yVector + Common::Point(-8, -3) +
				(outer ? Common::Point(0, 0) : Common::Point(+7, +5));
			break;
		case kDown:
			delta = xVector + yVector + Common::Point(-8, -3) +
				(outer ? Common::Point(+7, +5) : Common::Point(0, 0));
			break;
		case kLeft:
			delta = xVector + Common::Point(0, -33)
				+ (outer ? Common::Point(-7, +5) : Common::Point(0, 0));
			break;
		case kRight:
			delta = xVector + yVector + Common::Point(0, -33)
				+ (outer ? Common::Point(0, 0) : Common::Point(-7, +5));
			break;
		}

		Common::Point pos = getTileBase(cell) + delta;

		LayerId layer(kMaterialsImage, cell,
			      Common::String(dirnames[wall._position]) +
			      (outer ? "outer" : "inner"));
		int frame = (wall._strength - kStrengthStraw) * 2 + (wall._position % 2);
		g_vm->getVideoRoom()->selectFrame(layer, getWallZ(cell, wall._position, outer), frame, pos);
	}

	int getWallZ(int cell, Position pos, bool outer) {
		int zValue = 150 * (cell / 5 + cell % 5) + 5000;
		switch (pos) {
		case kUp:
			zValue += outer? 110 : 100;
			break;
		case kDown:
			zValue += outer ? -10 : 0;
			break;
		case kLeft:
			zValue += outer ? 40 : 50;
			break;
		case kRight:
			zValue += outer ? 80 : 70;
			break;
		}
		return zValue;
	}

	Common::Point getTileBase(int x, int y) {
		return Common::Point(320, 456) + xVector * x + yVector * y;
	}

	Common::Point getTileBase(int id) {
		return getTileBase(id / 5, id % 5);
	}

	void rotate(int cell) {
		for (int j = 0;
		     j < (int) _current._cells[cell]._movableWalls.size();
		     j++) {
			_current._cells[cell]._movableWalls[j]._position
				= (Position) ((_current._cells[cell]._movableWalls[j]._position + 1) % 4);
			_current._cells[cell]._movableWalls[j]._inTransit = 1;
		}
	}

	void loadPuzzle(const Common::String &name) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Common::SharedPtr<Common::SeekableReadStream> gameStream(room->openFile(name + ".mcf"));
		Common::SharedPtr<Common::SeekableReadStream> solStream(room->openFile(name + ".sol"));
		Common::SharedPtr<Common::SeekableReadStream> cwStream(room->openFile(name + ".cw"));
		readLabStream(_current, gameStream);
		readLabStream(_solution, solStream);
		for (int cell = 0; cell < numSquares; cell++) {
			room->setHotzoneEnabled(Common::String::format("%d", cell),
						!_current._cells[cell]._movableWalls.empty());
		}
	}

	static void readLabStream(Labyrinth &lab, Common::SharedPtr<Common::SeekableReadStream> stream) {
		stream->readLine(); // Level number
		int gridSize = stream->readLine().asUint64();

		if (gridSize == 0)
			gridSize = 1;

		stream->readLine(); // ?
		stream->readLine(); // ?
		int numLines = stream->readLine().asUint64();
		// Extra walls
		if (gridSize == 3 || gridSize == 4) {
			Wall w;
			w._isCritical = false;
			w._id = -1;
			w._strength = kStrengthStone;
			w._position = kRight;
			if (gridSize == 3)
				lab._cells[4]._immovableWalls.push_back(w);
			lab._cells[24]._immovableWalls.push_back(w);
		}
		for (int i = 0; i < numLines; i++) {
			Common::String line = stream->readLine();
			size_t cur = 0;
			int rawcellid = line.asUint64();
			int transformedcellid =
				5 * (rawcellid / gridSize) +
				(rawcellid % gridSize) + (5 - gridSize)
				+ 5 * ((5 - gridSize) / 2);
			cur = line.findFirstNotOf(kDigits, cur);
			cur = line.findFirstOf(kDigits, cur);
			int numWalls = line.substr(cur).asUint64();
			cur = line.findFirstNotOf(kDigits, cur);
			cur = line.findFirstOf(kDigits, cur);
			/*int rotatable =*/ line.substr(cur).asUint64();
			cur = line.findFirstNotOf(kDigits, cur);
			cur = line.findFirstOf(kDigits, cur);
			for (int j = 0; j < numWalls; j++) {
				Wall w;
				w._isCritical = false;
				w._id = line.substr(cur).asUint64();
				cur = line.findFirstNotOf(kDigits, cur);
				cur = line.findFirstOf(kDigits, cur);
				int pos = line.substr(cur).asUint64();
				cur = line.findFirstNotOf(kDigits, cur);
				cur = line.findFirstOf(kDigits, cur);
				w._strength = (Strength) line.substr(cur).asUint64();
				cur = line.findFirstNotOf(kDigits, cur);
				cur = line.findFirstOf(kDigits, cur);
				switch (pos % 4) {
				case 0:
					w._position = kLeft;
					break;
				case 1:
					w._position = kUp;
					break;
				case 2:
					w._position = kRight;
					break;
				case 3:
					w._position = kDown;
					break;
				}
				if (pos >= 4) {
					lab._cells[transformedcellid]._immovableWalls.push_back(w);
				} else {
					lab._cells[transformedcellid]._movableWalls.push_back(w);
				}
			}
		}
	}

	Common::Point xVector;
	Common::Point yVector;
	int _highlight;

	DaedalusDialogState _dialogState;
	int _minotaurX;
	int _minotaurY;
	int _minotaurTileId;
	int _lastChargeSound;
	int _lastTrappedSound;
	int _lastEscapedSound;
	int _levelId;
	int _currentSound;
	int _soundCounter;
	int _soundMax;

	Labyrinth _current;
	Labyrinth _solution;
};

Common::SharedPtr<Hadesch::Handler> makeMinotaurHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new MinotaurHandler());
}

}
