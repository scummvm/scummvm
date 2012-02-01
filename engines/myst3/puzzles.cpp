/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/puzzles.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"
#include "engines/myst3/sound.h"

namespace Myst3 {

Puzzles::Puzzles(Myst3Engine *vm) :
	_vm(vm) {
}

Puzzles::~Puzzles() {
}

void Puzzles::run(uint16 id, uint16 arg0, uint16 arg1, uint16 arg2) {
	switch (id) {
	case 1:
		leversBall(arg0);
		break;
	case 2:
		tesla(arg0, arg1, arg2);
		break;
	case 8:
		journalSaavedro(arg0);
		break;
	case 9:
		journalAtrus(arg0, arg1);
		break;
	case 10:
		symbolCodesInit(arg0, arg1, arg2);
		break;
	case 11:
		symbolCodesClick(arg0);
		break;
	case 14:
		projectorLoadBitmap(arg0);
		break;
	case 15:
		projectorAddSpotItem(arg0, arg1, arg2);
		break;
	case 16:
		projectorUpdateCoordinates();
		break;
	case 20:
		saveLoadMenu(arg0, arg1);
		break;
	case 21:
		mainMenu(arg0);
		break;
	default:
		warning("Puzzle %d is not implemented", id);
	}
}

void Puzzles::_drawForVarHelper(uint16 var, int32 startValue, int32 endValue) {
		uint startFrame = _vm->_state->getFrameCount();
		uint currentFrame = startFrame;
		uint numValues = abs(endValue - startValue);
		uint endFrame = startFrame + 2 * numValues;

		if (startFrame < endFrame) {
			int currentValue = -9999;
			while (1) {
				int nextValue = (currentFrame - startFrame) / 2;
				if (currentValue != nextValue) {
					currentValue = nextValue;

					int16 varValue;
					if (endValue > startValue)
						varValue = startValue + currentValue;
					else
						varValue = startValue - currentValue;

					_vm->_state->setVar(var, varValue);
				}

				_vm->processInput(true);
				_vm->drawFrame();
				currentFrame = _vm->_state->getFrameCount();

				if (currentFrame > endFrame)
					break;
			}
		}

		_vm->_state->setVar(var, endValue);
}

void Puzzles::leversBall(int16 var) {
	struct NewPosition {
		bool newLeft;
		bool newRight;
		uint16 newBallPosition;
		uint16 movieStart;
		uint16 movieEnd;
		uint16 movieBallStart;
		uint16 movieBallEnd;
	};

	struct Move {
		bool oldLeft;
		bool oldRight;
		uint16 oldBallPosition;
		NewPosition p[2];
	};

	static const Move moves[] =	{
		{   0,   1,   2, { { 1,   1,   2, 127, 147,   0,   0 }, { 0,   0,   0, 703, 735,   0,   0 } } },
		{   0,   0,   4, { { 1,   0,   4,  43,  63,   0,   0 }, { 0,   1,   4,  64,  84,   0,   0 } } },
		{   0,   0,   1, { { 1,   0,   1,  85, 105,   0,   0 }, { 0,   1,   1,  22,  42,   0,   0 } } },
		{   1,   0,   4, { { 1,   1,   3, 514, 534, 169, 217 }, { 0,   0,   4, 577, 597,   0,   0 } } },
		{   1,   0,   3, { { 1,   1,   3, 493, 513,   0,   0 }, { 0,   0,   4, 451, 471, 410, 450 } } },
		{   1,   0,   1, { { 1,   1,   2, 472, 492, 312, 360 }, { 0,   0,   1, 598, 618,   0,   0 } } },
		{   0,   1,   4, { { 1,   1,   3, 148, 168, 169, 217 }, { 0,   0,   4, 619, 639,   0,   0 } } },
		{   0,   1,   2, { { 1,   1,   2, 127, 147,   0,   0 }, { 0,   0,   1,   1,  21, 271, 311 } } },
		{   0,   1,   1, { { 1,   1,   2, 106, 126, 312, 360 }, { 0,   0,   1, 640, 660,   0,   0 } } },
		{   1,   1,   3, { { 1,   0,   3, 661, 681,   0,   0 }, { 0,   1,   2, 535, 555, 218, 270 } } },
		{   1,   1,   2, { { 1,   0,   3, 556, 575, 361, 409 }, { 0,   1,   2, 682, 702,   0,   0 } } },
		{   0,   0,   0, { { 1,   0,   0, 757, 777,   0,   0 }, { 0,   1,   0, 736, 756,   0,   0 } } },
		{   1,   0,   0, { { 1,   1,   0, 799, 819,   0,   0 }, { 0,   0,   0, 841, 861,   0,   0 } } },
		{   0,   1,   0, { { 1,   1,   0, 778, 798,   0,   0 }, { 0,   0,   0, 820, 840,   0,   0 } } },
		{   1,   1,   0, { { 1,   0,   0, 883, 903,   0,   0 }, { 0,   1,   0, 862, 882,   0,   0 } } },
		{  -1,   0,   0, { { 0,   0,   0,   0,   0,   0,   0 }, { 0,   0,   0,   0,   0,   0,   0 } } }
	};

	uint16 oldPosition = _vm->_state->getBallPosition();
	uint16 oldLeverLeft = _vm->_state->getBallLeverLeft();
	uint16 oldLeverRight = _vm->_state->getBallLeverRight();

	// Toggle lever position
	_vm->_state->setVar(var, !_vm->_state->getVar(var));

	uint16 newLeverLeft = _vm->_state->getBallLeverLeft();
	uint16 newLeverRight = _vm->_state->getBallLeverRight();

	const Move *move = 0;
	for (uint i = _vm->_state->getBallDoorOpen() ? 0 : 1; i < ARRAYSIZE(moves); i++)
		if (moves[i].oldBallPosition == oldPosition
				&& moves[i].oldLeft == oldLeverLeft
				&& moves[i].oldRight == oldLeverRight) {
			move = &moves[i];
			break;
		}

	if (!move)
		error("Unable to find move with old levers l:%d r:%d p:%d", oldLeverLeft, oldLeverRight, oldPosition);

	const NewPosition *position = 0;
	for (uint i = 0; i < ARRAYSIZE(move->p); i++)
		if (move->p[i].newLeft == newLeverLeft
				&& move->p[i].newRight == newLeverRight) {
			position = &move->p[i];
			break;
		}

	if (!position)
		error("Unable to find position with levers l:%d r:%d", newLeverLeft, newLeverRight);

	_vm->_sound->play(789, 50);
	_drawForVarHelper(35, position->movieStart, position->movieEnd);

	if (position->newBallPosition != oldPosition) {
		uint16 sound;
		if (position->newBallPosition == 0) {
			sound = 792;
		} else if (position->newBallPosition == 1 || position->newBallPosition == 4) {
			sound = 790;
		} else {
			sound = 791;
		}

		_vm->_sound->play(sound, 50);
		_drawForVarHelper(35, position->movieBallStart, position->movieBallEnd);
	}

	_vm->_state->setBallPosition(position->newBallPosition);
	_vm->_state->setBallFrame(_vm->_state->getVar(35));
}

void Puzzles::tesla(int16 movie, int16 var, int16 move) {
	uint16 node = _vm->_state->getLocationNode();

	int16 movieStart = 0;
	switch (node) {
	case 114:
		movieStart = 0;
		break;
	case 116:
		movieStart = 320;
		break;
	case 118:
		movieStart = 240;
		break;
	case 120:
		movieStart = 160;
		break;
	case 122:
		movieStart = 80;
		break;
	}

	_vm->_state->setTeslaMovieStart(movieStart);

	uint16 position = movieStart + _vm->_state->getVar(var);

	if (position > 400)
		position -= 400;

	_vm->_state->setVar(32, node % 100);
	_vm->_state->setVar(33, node % 100 + 10000);

	if (movie) {
		_vm->_sound->play(1243, 100);
		_vm->_state->setMovieSynchronized(true);
		_vm->playSimpleMovie(movie);
	}

	if (move) {
		uint16 sound = _vm->_rnd->getRandomNumberRng(1244, 1245);
		_vm->_sound->play(sound, 100);
	}

	if (move > 0) {
		_drawForVarHelper(var - 303, position + 1, position + 19);
		position += 20;
	} else if (move < 0) {
		if (position == 1)
			position = 401;

		_drawForVarHelper(var - 303, position - 1, position - 19);
		position -= 20;
	}

	if (position < 1)
		position = 381;
	else if (position > 400)
		position = 1;

	_vm->_state->setVar(var - 303, position);

	int16 absPosition = position - movieStart;

	if (absPosition < 1)
		absPosition += 400;

	_vm->_state->setVar(var, absPosition);

	bool puzzleSolved = _vm->_state->getTeslaTopAligned() == 1
			&& _vm->_state->getTeslaMiddleAligned() == 1
			&& _vm->_state->getTeslaBottomAligned() == 1;

	_vm->_state->setTeslaAllAligned(puzzleSolved);
}

void Puzzles::journalSaavedro(int16 move) {
	uint16 chapter = _vm->_state->getJournalSaavedroChapter();
	int16 page = _vm->_state->getJournalSaavedroPageInChapter();

	if (!_journalSaavedroHasChapter(chapter))
		chapter = _journalSaavedroNextChapter(chapter, true);

	if (move > 0) {
		// Go to the next available page
		int16 pageCount = _journalSaavedroPageCount(chapter);
		page++;

		if (page == pageCount) {
			chapter = _journalSaavedroNextChapter(chapter, true);
			page = 0;
		}

		_vm->_state->setJournalSaavedroChapter(chapter);
		_vm->_state->setJournalSaavedroPageInChapter(page);
	} else if (move < 0) {
		// Go to the previous available page
		page--;

		if (page < 0) {
			chapter = _journalSaavedroNextChapter(chapter, false);
			page = _journalSaavedroPageCount(chapter) - 1;
		}

		_vm->_state->setJournalSaavedroChapter(chapter);
		_vm->_state->setJournalSaavedroPageInChapter(page);
	} else {
		// Display current page
		int16 chapterStartNode = _journalSaavedroGetNode(chapter);
		int16 closed = 0;
		int16 opened = 0;
		int16 lastPage = 0;

		if (chapter > 0) {
			opened = 1;
			if (chapter == 21)
				lastPage = 2;
			else
				lastPage = 1;

		} else {
			closed = 1;
		}

		uint16 nodeRight;
		uint16 nodeLeft;
		if (page || !chapter) {
			nodeRight = chapterStartNode + page;
			nodeLeft = chapterStartNode + page;
		} else {
			nodeRight = chapterStartNode + page;
			uint16 chapterLeft = _journalSaavedroNextChapter(chapter, false);
			if (chapterLeft > 0)
				nodeLeft = _journalSaavedroGetNode(chapterLeft + 1);
			else
				nodeLeft = 2;
		}

		_vm->_state->setJournalSaavedroClosed(closed);
		_vm->_state->setJournalSaavedroOpen(opened);
		_vm->_state->setJournalSaavedroLastPage(lastPage);

		// TODO: Draw nodeLeft on the left part of the screen
		_vm->loadNodeFrame(nodeRight);
	}
}

uint16 Puzzles::_journalSaavedroGetNode(uint16 chapter) {
	const DirectorySubEntry *desc = _vm->getFileDescription(0, 1200, 0, DirectorySubEntry::kNumMetadata);

	if (!desc)
		error("Node 1200 does not exist");

	return desc->getMiscData(chapter) + 199;
}

uint16 Puzzles::_journalSaavedroPageCount(uint16 chapter) {
	uint16 chapterStartNode = _journalSaavedroGetNode(chapter);
	if (chapter != 21)
		return _journalSaavedroGetNode(chapter + 1) - chapterStartNode;
	else
		return 1;
}

bool Puzzles::_journalSaavedroHasChapter(uint16 chapter) {
	return _vm->_state->getVar(285 + chapter) != 0;
}

uint16 Puzzles::_journalSaavedroNextChapter(uint16 chapter, bool forward) {
	do {
		if (forward)
			chapter++;
		else
			chapter--;
	} while (!_journalSaavedroHasChapter(chapter));

	return chapter;
}

void Puzzles::journalAtrus(uint16 node, uint16 var) {
	uint numPages = 0;

	while (_vm->getFileDescription(0, node++, 0, DirectorySubEntry::kFrame))
		numPages++;

	_vm->_state->setVar(var, numPages - 1);
}

void Puzzles::symbolCodesInit(uint16 var, uint16 posX, uint16 posY) {
	struct Point {
		uint16 x;
		uint16 y;
	};

	struct CodeData {
		uint16 node;
		uint16 movie;
		bool flag;
		Point coords[20];
	};

	static const CodeData codes[] = {
			{
					144, 10144, 0,
					{
							{ 296, 120 }, { 312, 128 }, { 296, 144 }, { 296, 128 }, { 312, 120 },
							{ 328, 120 }, { 312, 144 }, { 312, 128 }, { 296, 136 }, { 312, 144 },
							{ 296, 160 }, { 296, 144 }, { 312, 136 }, { 328, 144 }, { 312, 160 },
							{ 312, 144 }, { 296, 112 }, { 328, 120 }, { 296, 160 }, { 288, 120 }
					}
			}, {
					244, 10244, 1,
					{
							{ 288, 16 }, { 336, 32 }, { 294, 72 }, { 280, 24 }, { 336, 16 },
							{ 376, 24 }, { 336, 72 }, { 328, 32 }, { 288, 64 }, { 336, 80 },
							{ 288, 120 }, { 280, 72 }, { 336, 64 }, { 384, 72 }, { 336, 120 },
							{ 328, 80 }, { 288, 0 }, { 384, 24 }, { 288, 120 }, { 264, 24 }
					}
			}, {
					148, 10148, 0,
					{
							{ 280, 24 }, { 304, 32 }, { 288, 48 }, { 280, 24 }, { 304, 24 },
							{ 320, 32 }, { 304, 48 }, { 296, 32 }, { 288, 40 }, { 304, 48 },
							{ 280, 64 }, { 280, 48 }, { 304, 48 }, { 320, 48 }, { 304, 64 },
							{ 296, 48 }, { 280, 16 }, { 320, 24 }, { 280, 64 }, { 272, 24 }
					}
			}, {
					248, 10248, 1,
					{
							{ 280, 48 }, { 320, 56 }, { 287, 88 }, { 272, 56 }, { 320, 48 },
							{ 360, 56 }, { 328, 96 }, { 312, 56 }, { 288, 88 }, { 320, 96 },
							{ 280, 128 }, { 271, 96 }, { 328, 88 }, { 360, 96 }, { 320, 128 },
							{ 312, 96 }, { 280, 32 }, { 360, 48 }, { 280, 128 }, { 264, 48 }
					}
			}, {
					348, 10348, 1,
					{
							{ 336, 24 }, { 376, 32 }, { 336, 80 }, { 328, 32 }, { 376, 24 },
							{ 424, 32 }, { 384, 80 }, { 368, 40 }, { 336, 72 }, { 376, 80 },
							{ 336, 120 }, { 328, 80 }, { 384, 72 }, { 424, 80 }, { 376, 120 },
							{ 368, 80 }, { 328, 8 }, { 424, 32 }, { 328, 128 }, { 312, 32 }
					}
			}, {
					448, 10448, 1,
					{
							{ 224, 32 }, { 264, 40 }, { 224, 80 }, { 208, 40 }, { 264, 32 },
							{ 304, 40 }, { 270, 88 }, { 256, 40 }, { 224, 72 }, { 264, 88 },
							{ 224, 128 }, { 208, 88 }, { 272, 72 }, { 312, 88 }, { 264, 128 },
							{ 256, 88 }, { 216, 16 }, { 312, 40 }, { 216, 128 }, { 200, 40 }
					}
			}
	};

	uint16 node = _vm->_state->getLocationNode();

	const CodeData *code = 0;
	for (uint i = 0; i < ARRAYSIZE(codes); i++)
		if (codes[i].node == node) {
			code = &codes[i];
			break;
		}

	if (!node)
		error("Unable to find puzzle data for node %d", node);

	int32 value = _vm->_state->getVar(var);

	for (uint i = 0; i < 20; i++) {
		if (code->flag || value & (1 << i)) {
			_vm->_state->setMoviePreloadToMemory(true);
			_vm->_state->setMovieScriptDriven(true);
			_vm->_state->setMovieOverridePosition(true);
			_vm->_state->setMovieOverridePosU(posX + code->coords[i].x);
			_vm->_state->setMovieOverridePosV(posY + code->coords[i].y);
			_vm->_state->setMovieConditionBit(i + 1);
			_vm->loadMovie(code->movie + i * 1000, var, false, true);
		}
	}
}

void Puzzles::symbolCodesClick(int16 var) {
	// Toggle clicked symbol element
	if (var > 0) {
		int32 value = _vm->_state->getVar(var);
		value ^= 1 << _vm->_state->getHotspotActiveRect();
		_vm->_state->setVar(var, value);
	}

	// Check puzzle with one symbol solution
	static const SymbolCodeSolution smallSolution = { 330080, 53575, 241719, 116411 };
	if (_vm->_state->getSymbolCode1AllSolved()) {
		bool code2Solved = _symbolCodesCheckSolution(490, smallSolution);
		_vm->_state->setSymbolCode2Solved(code2Solved);
	}

	// Check puzzle with 3 symbols solution
	static const SymbolCodeSolution solutions[] = {
			{ 208172, 131196, 252945, 788771 },
			{ 431060, 418863, 558738, 653337 },
			{ 472588, 199440, 155951, 597954 }
	};


	_vm->_state->setSymbolCode1CurrentSolved(false);

	for (uint i = 1; i <= ARRAYSIZE(solutions); i++) {
		int32 solutionsFound = _symbolCodesFound();

		// Symbol already found, don't allow it another time
		if (solutionsFound & (1 << i))
			continue;

		if (_symbolCodesCheckSolution(498, solutions[i - 1])) {
			_vm->_state->setSymbolCode1TopSolved(i);
			_vm->_state->setSymbolCode1CurrentSolved(true);
		}

		if (_symbolCodesCheckSolution(502, solutions[i - 1])) {
			_vm->_state->setSymbolCode1LeftSolved(i);
			_vm->_state->setSymbolCode1CurrentSolved(true);
		}

		if (_symbolCodesCheckSolution(507, solutions[i - 1])) {
			_vm->_state->setSymbolCode1LeftSolved(i);
			_vm->_state->setSymbolCode1CurrentSolved(true);
		}
	}

	bool allSolved = _symbolCodesFound() == 14;
	_vm->_state->setSymbolCode1AllSolved(allSolved);
}

bool Puzzles::_symbolCodesCheckSolution(uint16 var, const SymbolCodeSolution &solution) {
	bool solved = true;

	for (uint i = 0; i < ARRAYSIZE(solution); i++) {
		int32 value = _vm->_state->getVar(var + i);
		if (value != solution[i]) {
			solved = false;
			break;
		}
	}

	return solved;
}

int32 Puzzles::_symbolCodesFound() {
	int32 top = _vm->_state->getSymbolCode1TopSolved();
	int32 left = _vm->_state->getSymbolCode1LeftSolved();
	int32 right = _vm->_state->getSymbolCode1RightSolved();
	return (1 << top) | (1 << left) | (1 << right);
}

void Puzzles::mainMenu(uint16 action) {
	_vm->setMenuAction(action);
}

void Puzzles::saveLoadMenu(uint16 action, uint16 item) {
	switch (action) {
	case 0:
		_vm->_menu->loadMenuOpen();
		break;
	case 1:
		_vm->_menu->loadMenuSelect(item);
		break;
	case 2:
		_vm->_menu->loadMenuLoad();
		break;
	case 3:
		_vm->_menu->saveMenuOpen();
		break;
	case 4:
		_vm->_menu->saveMenuSelect(item);
		break;
	case 5:
		_vm->_menu->saveMenuSave();
		break;
	case 6:
		_vm->_menu->loadMenuChangePage();
		break;
	case 7:
		_vm->_menu->saveMenuChangePage();
		break;
	case 8:
		_vm->_menu->saveLoadErase();
		break;
	default:
		warning("Save load menu action %d for item %d is not implemented", action, item);
	}
}

static void copySurfaceRect(Graphics::Surface *dest, const Common::Point &destPoint, const Graphics::Surface *src) {
	for (uint16 i = 0; i < src->h; i++)
		memcpy(dest->getBasePtr(destPoint.x, i + destPoint.y), src->getBasePtr(0, i), src->pitch);
}

void Puzzles::projectorLoadBitmap(uint16 bitmap) {
	assert(_vm->_projectorBackground == 0 && "Previous background not yet used.");

	// This surface is freed by the destructor of the movie that uses it
	_vm->_projectorBackground = new Graphics::Surface();
	_vm->_projectorBackground->create(1024, 1024, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	const DirectorySubEntry *movieDesc = _vm->getFileDescription(0, bitmap, 0, DirectorySubEntry::kStillMovie);

	if (!movieDesc)
		error("Movie %d does not exist", bitmap);

	// Rebuild the complete background image from the frames of the bink movie
	Common::MemoryReadStream *movieStream = movieDesc->getData();
	Video::SeekableBinkDecoder bink;
	bink.loadStream(movieStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	for (uint i = 0; i < 1024; i += 256)
		for (uint j = 0; j < 1024; j += 256) {
			const Graphics::Surface *frame = bink.decodeNextFrame();
			copySurfaceRect(_vm->_projectorBackground, Common::Point(j, i), frame);
		}
}

void Puzzles::projectorAddSpotItem(uint16 bitmap, uint16 x, uint16 y) {
	assert(_vm->_projectorBackground != 0 && "Projector background already used.");

	// Nothing to do if the spotitem is not enabled
	if (!_vm->_state->getVar(26))
		return;

	const DirectorySubEntry *movieDesc = _vm->getFileDescription(0, bitmap, 0, DirectorySubEntry::kStillMovie);

	if (!movieDesc)
		error("Movie %d does not exist", bitmap);

	// Rebuild the complete background image from the frames of the bink movie
	Common::MemoryReadStream *movieStream = movieDesc->getData();
	Video::SeekableBinkDecoder bink;
	bink.loadStream(movieStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	const Graphics::Surface *frame = bink.decodeNextFrame();
	copySurfaceRect(_vm->_projectorBackground, Common::Point(x, y), frame);
}

void Puzzles::projectorUpdateCoordinates() {
	int16 x = CLIP<int16>(_vm->_state->getProjectorX(), 840, 9400);
	int16 y = CLIP<int16>(_vm->_state->getProjectorY(), 840, 9400);
	int16 zoom = CLIP<int16>(_vm->_state->getProjectorZoom(), 1280, 5120);
	int16 blur = CLIP<int16>(_vm->_state->getProjectorBlur(), 400, 2470);

	int16 halfZoom = zoom / 2;
	if (x - halfZoom < 0)
		x = halfZoom;
	if (x + halfZoom > 10240)
		x = 10240 - halfZoom;
	if (y - halfZoom < 0)
		y = halfZoom;
	if (y + halfZoom > 10240)
		y = 10240 - halfZoom;

	int16 angleXOffset = _vm->_state->getProjectorAngleXOffset();
	int16 angleYOffset = _vm->_state->getProjectorAngleYOffset();
	int16 angleZoomOffset = _vm->_state->getProjectorAngleZoomOffset();
	int16 angleBlurOffset = _vm->_state->getProjectorAngleBlurOffset();

	int16 angleX = (angleXOffset + 200 * (5 * x - 4200) / 8560) % 1000;
	int16 angleY = (angleYOffset + 200 * (5 * y - 4200) / 8560) % 1000;
	int16 angleZoom = (angleZoomOffset + 200 * (5 * zoom - 6400) / 3840) % 1000;
	int16 angleBlur = (angleBlurOffset + 200 * (5 * blur - 2000) / 2070) % 1000;

	_vm->_state->setProjectorAngleX(angleX);
	_vm->_state->setProjectorAngleY(angleY);
	_vm->_state->setProjectorAngleZoom(angleZoom);
	_vm->_state->setProjectorAngleBlur(angleBlur);

	_vm->_state->setProjectorX(x);
	_vm->_state->setProjectorY(y);
	_vm->_state->setProjectorZoom(zoom);
	_vm->_state->setProjectorBlur(blur);
}



} /* namespace Myst3 */
