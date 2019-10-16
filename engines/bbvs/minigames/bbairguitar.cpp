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

#include "bbvs/minigames/bbairguitar.h"

#include "common/savefile.h"

#include "gui/message.h"
#include "gui/filebrowser-dialog.h"

namespace Bbvs {

static const char * const kNoteSoundFilenames[] = {
	"a.aif", "a#.aif", "b.aif", "c.aif", "c#.aif",
	"d.aif", "d#.aif", "e.aif", "f.aif", "f#.aif",
	"g.aif", "g#.aif", "a_oct.aif"
};

static const uint kNoteSoundFilenamesCount = ARRAYSIZE(kNoteSoundFilenames);

static const char * const kPatchDirectories[] = {
	"rock", "burp", "fart"
};

static const uint kPatchDirectoriesCount = ARRAYSIZE(kPatchDirectories);

static const BBPoint kPianoKeyArea1[] = {{29, 192}, {38, 192}, {38, 222}, {41, 222}, {41, 239}, {29, 239}};
static const BBPoint kPianoKeyArea2[] = {{38, 192}, {43, 192}, {43, 222}, {38, 222}};
static const BBPoint kPianoKeyArea3[] = {{43, 192}, {49, 192}, {49, 222}, {52, 222}, {52, 239}, {41, 239}, {41, 222}, {43, 222}};
static const BBPoint kPianoKeyArea4[] = {{49, 192}, {54, 192}, {54, 222}, {49, 222}};
static const BBPoint kPianoKeyArea5[] = {{54, 192}, {63, 192}, {63, 239}, {52, 239}, {52, 222}, {54, 222}};
static const BBPoint kPianoKeyArea6[] = {{63, 192}, {71, 192}, {71, 222}, {74, 222}, {74, 239}, {63, 239}};
static const BBPoint kPianoKeyArea7[] = {{71, 192}, {76, 192}, {76, 222}, {71, 222}};
static const BBPoint kPianoKeyArea8[] = {{76, 192}, {82, 192}, {82, 222}, {85, 222}, {85, 239}, {74, 239}, {74, 222}, {76, 222}};
static const BBPoint kPianoKeyArea9[] = {{82, 192}, {87, 192}, {87, 222}, {82, 222}};
static const BBPoint kPianoKeyArea10[] = {{87, 192}, {94, 192}, {94, 222}, {96, 222}, {96, 239}, {85, 239}, {85, 222}, {87, 222}};
static const BBPoint kPianoKeyArea11[] = {{94, 192}, {99, 192}, {99, 222}, {94, 222}};
static const BBPoint kPianoKeyArea12[] = {{99, 192}, {107, 192}, {107, 239}, {96, 239}, {96, 222}, {99, 222}};
static const BBPoint kPianoKeyArea13[] = {{107, 192}, {118, 192}, {118, 239}, {107, 239}};

static const BBPolygon kPianoKeyAreas[] = {
	{kPianoKeyArea1, ARRAYSIZE(kPianoKeyArea1)},
	{kPianoKeyArea2, ARRAYSIZE(kPianoKeyArea2)},
	{kPianoKeyArea3, ARRAYSIZE(kPianoKeyArea3)},
	{kPianoKeyArea4, ARRAYSIZE(kPianoKeyArea4)},
	{kPianoKeyArea5, ARRAYSIZE(kPianoKeyArea5)},
	{kPianoKeyArea6, ARRAYSIZE(kPianoKeyArea6)},
	{kPianoKeyArea7, ARRAYSIZE(kPianoKeyArea7)},
	{kPianoKeyArea8, ARRAYSIZE(kPianoKeyArea8)},
	{kPianoKeyArea9, ARRAYSIZE(kPianoKeyArea9)},
	{kPianoKeyArea10, ARRAYSIZE(kPianoKeyArea10)},
	{kPianoKeyArea11, ARRAYSIZE(kPianoKeyArea11)},
	{kPianoKeyArea12, ARRAYSIZE(kPianoKeyArea12)},
	{kPianoKeyArea13, ARRAYSIZE(kPianoKeyArea13)},
};

static const BBPoint kObjPoints[] = {
	{161, 189}, {269, 189}, {161, 208}, {279, 208}, {172, 208},
	{141, 224}, {257, 191}, {257, 199}, {148, 223}, {124, 224},
	{ 29, 192}, {182, 220}, {245, 220}, {269, 220}, {161, 220},
	{203, 220}, {224, 220}, {123, 189}, {123, 199}, {123, 209},
	{134, 224}, { 29, 185}, {124, 224}, {226, 127}, {226, 127},
	{209, 141}, {244, 141}, {226, 127}, { 99, 107}, { 99, 107},
	{ 76, 137}, {118, 136}, { 99, 107}, {195, 104}, {100,  78}
};

static const MinigameBbAirGuitar::PianoKeyInfo kPianoKeyInfos[] = {
	{ 30, 192, 0},
	{ 38, 192, 5},
	{ 41, 192, 1},
	{ 49, 192, 5},
	{ 52, 192, 2},
	{ 63, 192, 3},
	{ 71, 192, 5},
	{ 74, 192, 1},
	{ 82, 192, 5},
	{ 85, 192, 1},
	{ 94, 192, 5},
	{ 96, 192, 2},
	{107, 192, 4}
};

static const Rect kRect2 = {29, 189, 290, 239};
static const Rect kPianoRect = {29, 192, 118, 239};

static const Rect kPlayerButtonRects[] = {
	{123, 189, 145, 199},
	{123, 199, 145, 209},
	{123, 209, 145, 220},
	{148, 223, 156, 236},
	{161, 189, 182, 205},
	{161, 208, 171, 218},
	{161, 220, 182, 231},
	{182, 220, 203, 231},
	{203, 220, 224, 231},
	{224, 220, 245, 231},
	{245, 220, 266, 231},
	{269, 220, 290, 231},
	{269, 189, 290, 205},
	{279, 208, 290, 218}
};

static const BBPoint kPointsTbl1[] = {
	{196, 191},	{202, 191}, {207, 191}, {212, 191}, {217, 191},
	{223, 191}, {228, 191}, {233, 191}, {238, 191}, {244, 191},
	{249, 191}
};

static const BBPoint kPointsTbl2[] = {
	{196, 199}, {202, 199}, {207, 199}, {212, 199}, {217, 199},
	{223, 199}, {228, 199}, {233, 199}, {238, 199}, {244, 199},
	{249, 199}
};

static const struct { int frameIndex; byte flag; } kNoteFrameTbl[13] = {
	{2, 0}, {2, 1}, {3, 0}, {3, 1}, {4, 0},
	{5, 0}, {5, 1}, {6, 0}, {6, 1}, {0, 0},
	{0, 1}, {1, 0}, {2, 0}
};

const int kTrackBarMinX = 172;
const int kTrackBarMaxX = 272;

bool MinigameBbAirGuitar::ptInRect(const Rect *r, int x, int y) {
	return r && Common::Rect(r->left, r->top, r->right, r->bottom).contains(x, y);
}

bool MinigameBbAirGuitar::ptInPoly(const BBPolygon *poly, int x, int y) {
	if (!poly)
		return false;
	const BBPoint *points = poly->points;
	int pointsCount = poly->pointsCount;
	bool result = false;
	if (pointsCount > 0)
		for (int i = 0, j = pointsCount - 1; i < pointsCount; j = i++)
			if (((points[i].y > y) != (points[j].y > y)) &&
				(x < (points[j].x - points[i].x) * (y - points[i].y) /
				(points[j].y - points[i].y) + points[i].x))
				result = !result;
	return result;
}

void MinigameBbAirGuitar::buildDrawList(DrawList &drawList) {
	switch (_gameState) {
	case 0:
		buildDrawList0(drawList);
		break;
	case 1:
		buildDrawList1(drawList);
		break;
	default:
		break;
	}
}

void MinigameBbAirGuitar::buildDrawList0(DrawList &drawList) {

	drawList.add(_objects[0].anim->frameIndices[0], _objects[0].x, _objects[0].y, 2000);

	for (int i = 1; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->y + 16);
	}

	if (_titleScreenSpriteIndex> 0)
		drawList.add(_titleScreenSpriteIndex, 0, 0, 0);

}

void MinigameBbAirGuitar::buildDrawList1(DrawList &drawList) {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, 255 - i);
	}

	if (_movingTrackBar) {
		_trackBarX = _trackBarMouseX;
	} else if (_totalTrackLength > 0) {
		_trackBarX = 100 * _currTrackPos / _totalTrackLength + kTrackBarMinX;
	} else {
		_trackBarX = kTrackBarMinX;
	}

	if (_trackBarX > kTrackBarMaxX)
		_trackBarX = kTrackBarMaxX;

	_trackBarThumbRect.top = 208;
	_trackBarThumbRect.bottom = 218;
	_trackBarThumbRect.left = _trackBarX;
	_trackBarThumbRect.right = _trackBarX + 6;

	drawList.add(_objects[5].anim->frameIndices[0], _trackBarX, 208, 100);

	if (_playerMode != 0) {
		for (int i = 36; i < _vuMeterLeft2 + 36; ++i) {
			int frameIndex = 0;
			if (i >= 45)
				frameIndex = 3;
			else if (i >= 43)
				frameIndex = 2;
			else if (i >= 41)
				frameIndex = 1;
			drawList.add(_objects[i].anim->frameIndices[frameIndex], kPointsTbl1[i - 36].x, kPointsTbl1[i - 36].y, 254);
		}
		for (int i = 47; i < _vuMeterRight2 + 47; ++i) {
			int frameIndex = 0;
			if (i >= 56)
				frameIndex = 3;
			else if (i >= 54)
				frameIndex = 2;
			else if (i >= 52)
				frameIndex = 1;
			drawList.add(_objects[i].anim->frameIndices[frameIndex], kPointsTbl2[i - 47].x, kPointsTbl2[i - 47].y, 254);
		}
	}

	if (_backgroundSpriteIndex > 0)
		drawList.add(_backgroundSpriteIndex, 0, 0, 0);

}

void MinigameBbAirGuitar::drawSprites() {
	DrawList drawList;
	buildDrawList(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

void MinigameBbAirGuitar::initObjs() {
	for (int i = 0; i < kMaxObjectsCount; ++i)
		_objects[i].kind = 0;
}

MinigameBbAirGuitar::Obj *MinigameBbAirGuitar::getFreeObject() {
	for (int i = 0; i < kMaxObjectsCount; ++i)
		if (_objects[i].kind == 0)
			return &_objects[i];
	return 0;
}

void MinigameBbAirGuitar::initObjects() {
	switch (_gameState) {
	case 0:
		initObjects0();
		break;
	case 1:
		initObjects1();
		break;
	default:
		break;
	}
}

void MinigameBbAirGuitar::initObjects0() {
	_objects[0].anim = getAnimation(0);
	_objects[0].frameIndex = 0;
	_objects[0].ticks = getAnimation(0)->frameTicks[0];
	_objects[0].x = 160;
	_objects[0].y = 120;
	_objects[0].kind = 1;
	_objects[1].anim = getAnimation(37);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = getAnimation(37)->frameTicks[0];
	_objects[1].x = 40;
	_objects[1].y = 240;
	_objects[1].kind = 2;
	_objects[2].anim = getAnimation(36);
	_objects[2].frameIndex = 0;
	_objects[2].ticks = getAnimation(36)->frameTicks[0];
	_objects[2].x = 280;
	_objects[2].y = 240;
	_objects[2].kind = 2;

}

void MinigameBbAirGuitar::initObjects1() {

	for (int i = 0; i < 60; ++i)
		_objects[i].kind = 0;

	_objects[0].kind = 0;
	_objects[0].kind = 1;
	_objects[0].anim = getAnimation(0);
	_objects[0].ticks = getAnimation(0)->frameTicks[0];
	_objects[1].anim = getAnimation(1);
	_objects[1].ticks = getAnimation(1)->frameTicks[0];
	_objects[2].anim = getAnimation(2);
	_objects[2].ticks = getAnimation(2)->frameTicks[0];
	_objects[3].anim = getAnimation(3);
	_objects[3].ticks = getAnimation(3)->frameTicks[0];
	_objects[4].anim = getAnimation(4);
	_objects[4].ticks = getAnimation(4)->frameTicks[0];
	_objects[5].anim = getAnimation(5);
	_objects[5].ticks = getAnimation(5)->frameTicks[0];
	_objects[6].anim = getAnimation(6);
	_objects[6].ticks = getAnimation(6)->frameTicks[0];
	_objects[7].anim = getAnimation(8);
	_objects[7].ticks = getAnimation(8)->frameTicks[0];
	_objects[8].anim = getAnimation(9);
	_objects[8].ticks = getAnimation(9)->frameTicks[0];
	_objects[9].anim = getAnimation(10);
	_objects[9].ticks = getAnimation(10)->frameTicks[0];
	_objects[10].anim = getAnimation(11);
	_objects[10].ticks = getAnimation(11)->frameTicks[0];
	_objects[11].anim = getAnimation(12);
	_objects[11].ticks = getAnimation(12)->frameTicks[0];
	_objects[12].anim = getAnimation(13);
	_objects[12].ticks = getAnimation(13)->frameTicks[0];
	_objects[13].anim = getAnimation(14);
	_objects[13].ticks = getAnimation(14)->frameTicks[0];
	_objects[14].anim = getAnimation(15);
	_objects[14].ticks = getAnimation(15)->frameTicks[0];
	_objects[15].anim = getAnimation(16);
	_objects[15].ticks = getAnimation(16)->frameTicks[0];
	_objects[16].anim = getAnimation(17);
	_objects[16].ticks = getAnimation(17)->frameTicks[0];
	_objects[17].anim = getAnimation(18);
	_objects[17].ticks = getAnimation(18)->frameTicks[0];
	_objects[18].anim = getAnimation(19);
	_objects[18].ticks = getAnimation(19)->frameTicks[0];
	_objects[19].anim = getAnimation(20);
	_objects[19].ticks = getAnimation(20)->frameTicks[0];
	_objects[20].anim = getAnimation(21);
	_objects[20].ticks = getAnimation(21)->frameTicks[0];
	_objects[21].anim = getAnimation(11);
	_objects[21].ticks = getAnimation(11)->frameTicks[0];
	_objects[22].anim = getAnimation(22);
	_objects[22].ticks = getAnimation(22)->frameTicks[0];
	_objects[23].anim = getAnimation(23);
	_objects[23].ticks = getAnimation(23)->frameTicks[0];
	_objects[24].anim = getAnimation(24);
	_objects[24].ticks = getAnimation(24)->frameTicks[0];
	_objects[25].anim = getAnimation(25);
	_objects[25].ticks = getAnimation(25)->frameTicks[0];
	_objects[26].anim = getAnimation(26);
	_objects[26].ticks = getAnimation(26)->frameTicks[0];
	_objects[27].anim = getAnimation(27);
	_objects[27].ticks = getAnimation(27)->frameTicks[0];
	_objects[28].anim = getAnimation(28);
	_objects[28].ticks = getAnimation(28)->frameTicks[0];
	_objects[29].anim = getAnimation(29);
	_objects[29].ticks = getAnimation(29)->frameTicks[0];
	_objects[30].anim = getAnimation(30);
	_objects[30].ticks = getAnimation(30)->frameTicks[0];
	_objects[31].anim = getAnimation(31);
	_objects[31].ticks = getAnimation(31)->frameTicks[0];
	_objects[32].anim = getAnimation(32);
	_objects[32].ticks = getAnimation(32)->frameTicks[0];
	_objects[33].anim = getAnimation(33);
	_objects[33].ticks = getAnimation(33)->frameTicks[0];
	_objects[34].anim = getAnimation(34);
	_objects[34].ticks = getAnimation(34)->frameTicks[0];
	_objects[35].anim = getAnimation(35);
	_objects[35].ticks = getAnimation(35)->frameTicks[0];

	for (int i = 36; i <= 57; ++i) {
		_objects[i].anim = getAnimation(7);
		_objects[i].ticks = getAnimation(7)->frameTicks[0];
	}

	for (int i = 1; i <= 35; ++i) {
		_objects[i].x = kObjPoints[i - 1].x;
		_objects[i].y = kObjPoints[i - 1].y;
	}

	_objects[22].kind = 1;
	_objects[6].kind = 1;
	_objects[26].kind = 1;
	_objects[26].frameIndex = 3;
	_objects[27].kind = 1;
	_objects[27].frameIndex = 3;
	_objects[31].kind = 1;
	_objects[31].frameIndex = 3;
	_objects[32].kind = 1;
	_objects[32].frameIndex = 3;
	_objects[28].kind = 1;
	_objects[33].kind = 1;
	_objects[34].kind = 1;
	_objects[35].kind = 1;

	_track[0].noteNum = -1;
	stop();
	changePatch(0);

}

bool MinigameBbAirGuitar::updateStatus(int mouseX, int mouseY, uint mouseButtons) {
	switch (_gameState) {
	case 0:
		return updateStatus0(mouseX, mouseY, mouseButtons);
	case 1:
		return updateStatus1(mouseX, mouseY, mouseButtons);
	default:
		break;
	}
	return false;
}

bool MinigameBbAirGuitar::updateStatus0(int mouseX, int mouseY, uint mouseButtons) {

	if (mouseButtons & kAnyButtonDown) {
		stopSound(1);
		_rockTunePlaying = false;
		_gameState = 1;
		initObjects();
		_gameTicks = 0;
	} else {

		if (!_rockTunePlaying) {
			_rockTunePlaying = true;
			playSound(1, true);
		}

		_objects[0].x = mouseX;
		_objects[0].y = mouseY;

		for (int i = 1; i < kMaxObjectsCount; ++i) {
			Obj *obj = &_objects[i];
			if (obj->kind && --obj->ticks == 0) {
				++obj->frameIndex;
				if (obj->frameIndex >= obj->anim->frameCount)
					obj->frameIndex = 0;
				obj->ticks = obj->anim->frameTicks[obj->frameIndex];
			}
		}

	}

  	return true;
}

bool MinigameBbAirGuitar::updateStatus1(int mouseX, int mouseY, uint mouseButtons) {

	int currTicks = _vm->_system->getMillis();

	if (_playerMode == 1 && _track[_trackIndex].ticks <= currTicks - _noteStartTime) {
		noteOff(_track[_trackIndex].noteNum);
		if (_trackIndex < _trackCount && _track[++_trackIndex].noteNum != -1)
			noteOn(_track[_trackIndex].noteNum);
		else
			stop();
	}

	if (_vuMeterLeft1 - 2 <= _vuMeterLeft2) {
		if (_vuMeterLeft1 + 1 >= _vuMeterLeft2) {
			int incr = MIN(_vm->getRandom(4), 2) - 1;
			if (incr < 0 && _vuMeterLeft2 == 0)
				incr = -incr;
			if (incr > 0 && _vuMeterLeft2 == 11)
				incr = -incr;
			_vuMeterLeft2 += incr;
		} else {
			--_vuMeterLeft2;
		}
	} else {
		++_vuMeterLeft2;
	}

	if (_vuMeterRight1 - 2 <= _vuMeterRight2) {
		if (_vuMeterRight1 + 1 >= _vuMeterRight2) {
			int incr = MIN(_vm->getRandom(4), 2) - 1;
			if (incr < 0 && _vuMeterRight2 == 0)
				incr = -incr;
			if (incr > 0 && _vuMeterRight2 == 11)
				incr = -incr;
			_vuMeterRight2 += incr;
		} else {
			--_vuMeterRight2;
		}
	} else {
		++_vuMeterRight2;
	}

	if (_resetAnims && _vm->_system->getMillis() - _noteStartTime >= 1000)
		resetObjs();

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	_trackBarMouseX = CLIP(mouseX, kTrackBarMinX, kTrackBarMaxX);

	bool checkClick = false;

	if (mouseButtons & kAnyButtonClicked) {
		checkClick = true;
	} else if (!(mouseButtons & kAnyButtonDown)) {
		afterButtonReleased();
	} else if (!_movingTrackBar && ((_currButtonNum >= 14 && ptInPoly(_currPianoKeyArea, mouseX, mouseY)) || ptInRect(_currPlayerButtonRect, mouseX, mouseY))) {
		if (_currButtonNum == 5 && _trackIndex > 0) {
			--_trackIndex;
			calcTotalTicks2();
		} else if (_currButtonNum == 13 && _trackIndex < _trackCount) {
			++_trackIndex;
			calcTotalTicks2();
		}
	} else if (!_movingTrackBar)
		checkClick = true;

	if (checkClick) {

		afterButtonReleased();
		_objects[0].frameIndex = 1;

		if (ptInRect(&kRect2, mouseX, mouseY)) {

			if (_playerMode != 1 && ptInRect(&kPianoRect, mouseX, mouseY)) {
				for (int i = 0; i <= 12; ++i) {
					if (ptInPoly(&kPianoKeyAreas[i], mouseX, mouseY)) {
						_currButtonNum = i + 14;
						_currPianoKeyArea = &kPianoKeyAreas[i];
						_objects[11].kind = 1;
						_objects[11].x = kPianoKeyInfos[i].x;
						_objects[11].y = kPianoKeyInfos[i].y;
						_objects[11].frameIndex = kPianoKeyInfos[i].frameIndex;
						noteOn(i);
						break;
					}
				}
			} else if (_playerMode != 1 && ptInRect(&_trackBarThumbRect, mouseX, mouseY)) {
				_movingTrackBar = true;
			} else {

				int playerButtonNum = -1;
				for (int i = 0; i < 14; ++i) {
					if (ptInRect(&kPlayerButtonRects[i], mouseX, mouseY)) {
						playerButtonNum = i;
						break;
					}
				}

				if (playerButtonNum >= 0) {
					_currButtonNum = playerButtonNum;
					_currPlayerButtonRect = &kPlayerButtonRects[playerButtonNum];

					switch (playerButtonNum) {

					case 0:
						if (_playerMode == 0) {
							changePatch(0);
							_currFrameIndex = &_objects[18 + 0].frameIndex;
							*_currFrameIndex = 0;
						}
						break;

					case 1:
						if (_playerMode == 0) {
							changePatch(1);
							_currFrameIndex = &_objects[18 + 1].frameIndex;
							*_currFrameIndex = 0;
						}
						break;

					case 2:
						if (_playerMode == 0) {
							changePatch(2);
							_currFrameIndex = &_objects[18 + 2].frameIndex;
							*_currFrameIndex = 0;
						}
						break;

					case 3:
						_btn3KindToggle = !_btn3KindToggle;
						_objects[9].kind = _btn3KindToggle ? 0 : 1;
						_objects[22].frameIndex = _btn3KindToggle ? 0 : 1;
						break;

					case 4:
						if (_playerMode == 0) {
							_objects[1].kind = 1;
							_currFrameIndex = &_objects[1].frameIndex;
							_objects[1].frameIndex = 0;
						}
						break;

					case 5:
						if (_playerMode == 0) {
							if (_trackIndex > 0)
								--_trackIndex;
							_objects[3].kind = 1;
							calcTotalTicks2();
						}
						break;

					case 6:
						stop();
						_currFrameIndex = &_objects[15].frameIndex;
						_objects[15].frameIndex = 0;
						break;

					case 7:
						if (_playerMode == 0) {
							play();
							_currFrameIndex = &_objects[12].frameIndex;
							_objects[12].frameIndex = 0;
						}
						break;

					case 8:
						if (_playerMode == 0) {
							_trackIndex = 0;
							_objects[16].kind = 1;
							calcTotalTicks2();
						}
						break;

					case 9:
						if (_playerMode == 0) {
							_trackIndex = _trackCount;
							_objects[17].kind = 1;
							calcTotalTicks2();
						}
						break;

					case 10:
						if (_playerMode == 0) {
							record();
							_currFrameIndex = &_objects[13].frameIndex;
							_objects[13].frameIndex = 0;
						}
						break;

					case 11:
						if (_playerMode == 0) {
							setPlayerMode3();
							_currFrameIndex = &_objects[14].frameIndex;
							_objects[14].frameIndex = 0;
						}
						break;

					case 12:
						if (_playerMode == 0) {
							_objects[2].kind = 1;
							_currFrameIndex = &_objects[2].frameIndex;
							_objects[2].frameIndex = 0;
						}
						break;

					case 13:
						if (_playerMode == 0) {
							if (_trackIndex < _trackCount)
								++_trackIndex;
							_objects[4].kind = 1;
							calcTotalTicks2();
						}
						break;

					default:
						break;
					}
				}
			}
		}
	}

	if (_playerMode != 0) {
		_currTrackPos = currTicks + _actionStartTrackPos - _actionStartTime;
		if (_currTrackPos > _actionTrackPos && _playerMode != 1) {
			if (_currTrackPos >= 15000) {
				_currTrackPos = 15000;
				_actionTrackPos = 15000;
				stop();
			} else {
				_actionTrackPos = currTicks + _actionStartTrackPos - _actionStartTime;
			}
		}
	}

	if (_buttonClickTicks + 1000 < currTicks)
		_buttonClickTicks = currTicks;

	int newKind = _buttonClickTicks + 500 < currTicks ? 1 : 0;

	switch (_playerMode) {

	case 1:
		if (_currButtonNum == 7) {
			_objects[12].kind = 1;
			_objects[12].frameIndex = 0;
		} else {
			_objects[12].kind = newKind;
			_objects[12].frameIndex = 1;
		}
		break;

	case 2:
		if (_currButtonNum == 10) {
			_objects[13].kind = 1;
			_objects[13].frameIndex = 0;
		} else {
			_objects[13].kind = newKind;
			_objects[13].frameIndex = 1;
		}
		break;

	case 3:
		if (_currButtonNum == 11) {
			_objects[14].kind = 1;
			_objects[14].frameIndex = 0;
		} else {
			_objects[14].kind = newKind;
			_objects[14].frameIndex = 1;
		}
		break;

	default:
		break;
	}

	updateObjs();

	return true;
}

void MinigameBbAirGuitar::updateObjs() {
	for (int i = 24; i <= 33; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind && --obj->ticks == 0) {
			if (obj->frameIndex + 1 >= obj->anim->frameCount) {
				obj->ticks = -1;
			} else {
				++obj->frameIndex;
				obj->ticks = obj->anim->frameTicks[obj->frameIndex];
			}
		}
	}
}

bool MinigameBbAirGuitar::run(bool fromMainGame) {

	memset(_objects, 0, sizeof(_objects));

	_modified = false;
	_currPatchNum = -1;
	_btn3KindToggle = 0;
	_currButtonNum = 27;
	_actionStartTime = 0;
	_currFrameIndex = 0;
	_currPlayerButtonRect = 0;
	_currPianoKeyArea = 0;
	_trackCount = 0;
	_trackIndex = 0;
	_totalTrackLength = 0;
	_actionTrackPos = 0;
	_noteStartTime = 0;
	_actionStartTrackPos = 0;
	_trackBarX = kTrackBarMinX;
	_currTrackPos = 0;
	_currNoteNum = -2;
	_resetAnims = false;
	_vuMeterLeft2 = 0;
	_vuMeterRight2 = 0;
	_vuMeterLeft1 = 0;
	_vuMeterRight1 = 0;
	_rockTunePlaying = false;

	_backgroundSpriteIndex = 97;
	_titleScreenSpriteIndex = 98;

	_fromMainGame = fromMainGame;

	_gameState = 0;
	_gameTicks = 0;
	_gameResult = false;
	_gameDone = false;
	initObjects();

	_spriteModule = new SpriteModule();
	_spriteModule->load("bbairg/bbairg.000");

	Palette palette = _spriteModule->getPalette();
	_vm->_screen->setPalette(palette);

	loadSounds();

	while (!_vm->shouldQuit() &&!_gameDone) {
		_vm->updateEvents();
		update();
	}

	_vm->_sound->unloadSounds();

	delete _spriteModule;

	return _gameResult;
}

void MinigameBbAirGuitar::update() {

	int inputTicks;

	if (_gameTicks > 0) {
		int currTicks = _vm->_system->getMillis();
		inputTicks = 3 * (currTicks - _gameTicks) / 50;
		_gameTicks = currTicks - (currTicks - _gameTicks - 50 * inputTicks / 3);
	} else {
		inputTicks = 1;
		_gameTicks = _vm->_system->getMillis();
	}

	if (_vm->_keyCode == Common::KEYCODE_ESCAPE) {
		_gameDone = querySaveModifiedTracks();
		return;
	}

	if (inputTicks == 0)
		return;

	bool done;

	do {
		done = !updateStatus(_vm->_mouseX, _vm->_mouseY, _vm->_mouseButtons);
		_vm->_mouseButtons &= ~kLeftButtonClicked;
		_vm->_mouseButtons &= ~kRightButtonClicked;
		_vm->_keyCode = Common::KEYCODE_INVALID;
	} while (--inputTicks && _gameTicks > 0 && !done);

	drawSprites();

	_vm->_system->delayMillis(10);

}

void MinigameBbAirGuitar::play() {
	if (_track[_trackIndex].noteNum != -1) {
		_playerMode = 1;
		_objects[7].kind = 1;
		_objects[8].kind = 0;
		_objects[15].kind = 0;
		_actionStartTime = _vm->_system->getMillis();
		_actionStartTrackPos = _currTrackPos;
		noteOn(_track[_trackIndex].noteNum);
	}
}


void MinigameBbAirGuitar::record() {
	_playerMode = 2;
	_objects[7].kind = 1;
	_objects[8].kind = 0;
	_objects[15].kind = 0;
	_totalTrackLength = 15000;
	_actionStartTime = _vm->_system->getMillis();
	_actionStartTrackPos = _currTrackPos;
	_noteStartTime = _vm->_system->getMillis();
	_actionTrackPos = _currTrackPos;
	_trackCount = _trackIndex;
	_vuMeterRight1 = 0;
	_vuMeterRight2 = 0;
	_vuMeterLeft1 = 0;
	_vuMeterLeft2 = 0;
	_modified = true;
	_track[_trackIndex].noteNum = -2;
}

void MinigameBbAirGuitar::setPlayerMode3() {
	_playerMode = 3;
	_objects[7].kind = 1;
	_objects[8].kind = 0;
	_objects[15].kind = 0;
	_totalTrackLength = 15000;
	_actionStartTime = _vm->_system->getMillis();
	_actionStartTrackPos = _currTrackPos;
	_noteStartTime = _vm->_system->getMillis();
	_actionTrackPos = _currTrackPos;
	_trackCount = _trackIndex;
	_vuMeterRight1 = 0;
	_vuMeterRight2 = 0;
	_vuMeterLeft1 = 0;
	_vuMeterLeft2 = 0;
	_modified = true;
	_track[_trackIndex].noteNum = -2;
}

void MinigameBbAirGuitar::stop() {
	noteOff(_currNoteNum);
	if (_playerMode == 2 || _playerMode == 3) {
		_totalTrackLength = _actionTrackPos;
		_track[_trackCount].noteNum = -1;
	}
	_playerMode = 0;
	_objects[7].kind = 0;
	_objects[8].kind = 1;
	_objects[15].kind = 1;
	_objects[15].frameIndex = 1;
	_objects[12].kind = 0;
	_objects[13].kind = 0;
	_objects[14].kind = 0;
	resetObjs();
}

void MinigameBbAirGuitar::changePatch(int patchNum) {

	resetObjs();

	if (patchNum == -1 || patchNum != _currPatchNum)
		_currPatchNum = -1;

	_objects[20].kind = 0;
	_objects[19].kind = _objects[20].kind;
	_objects[18].kind = _objects[19].kind;
	_objects[patchNum + 18].kind = 1;
	_objects[patchNum + 18].frameIndex = 1;
	_objects[6].frameIndex = patchNum;
	_currPatchNum = patchNum;
}

void MinigameBbAirGuitar::afterButtonReleased() {
	if (_movingTrackBar) {
		_movingTrackBar = false;
		_currTrackPos = _totalTrackLength * (_trackBarX - kTrackBarMinX) / 100;
		calcTotalTicks1();
	} else {
		switch (_currButtonNum) {
		case 0:
		case 1:
		case 2:
			*_currFrameIndex = 1;
			break;
		case 3:
			break;
		case 4:
			*_currFrameIndex = 1;
			loadTracks();
			_objects[1].kind = 0;
			break;
		case 5:
			_objects[3].kind = 0;
			break;
		case 6:
			*_currFrameIndex = 1;
			break;
		case 7:
			*_currFrameIndex = 1;
			break;
		case 8:
			_objects[16].kind = 0;
			break;
		case 9:
			_objects[17].kind = 0;
			break;
		case 10:
			*_currFrameIndex = 1;
			break;
		case 11:
			*_currFrameIndex = 1;
			break;
		case 12:
			*_currFrameIndex = 1;
			saveTracks();
			_objects[2].kind = 0;
			break;
		case 13:
			_objects[4].kind = 0;
			break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
			noteOff(_currButtonNum - 14);
			break;
		default:
			break;
		}
	}

	_objects->frameIndex = 0;
	_currPlayerButtonRect = 0;
	_currPianoKeyArea = 0;
	_currButtonNum = 27;
}

void MinigameBbAirGuitar::calcTotalTicks2() {
	_currTrackPos = 0;
	for (int i = 0; i < _trackIndex; ++i)
		_currTrackPos += _track[i].ticks;
}

void MinigameBbAirGuitar::calcTotalTicks1() {
	int totalTicks = 0;
	// TODO Try to clean this up
	_trackIndex = 0;
	if (_track[0].ticks <= _currTrackPos) {
		do {
			totalTicks += _track[_trackIndex].ticks;
			if (_trackIndex >= _trackCount)
				break;
			++_trackIndex;
		} while (totalTicks + _track[_trackIndex].ticks <= _currTrackPos);
	}
	_currTrackPos = totalTicks;
}

void MinigameBbAirGuitar::noteOn(int noteNum) {

	if (_currNoteNum != -2) {
		if (noteNum == _currNoteNum)
			return;
		noteOff(_currNoteNum);
	}

	if (noteNum == -2) {
		_vuMeterRight1 = 0;
		_vuMeterRight2 = 0;
		_vuMeterLeft1 = 0;
		_vuMeterLeft2 = 0;
	} else {
		playNote(noteNum);
		_vuMeterRight1 = 10;
		_vuMeterRight2 = 10;
		_vuMeterLeft1 = 10;
		_vuMeterLeft2 = 10;
		if (_btn3KindToggle) {
			_objects[23].kind = 1;
			_objects[23].frameIndex = noteNum;
		} else {
			_objects[10].kind = 1;
			_objects[10].frameIndex = kNoteFrameTbl[noteNum].frameIndex;
			if (kNoteFrameTbl[noteNum].flag) {
				_objects[21].kind = 1;
				_objects[21].frameIndex = 7;
			}
		}
	}

	_currNoteNum = noteNum;

	if (_playerMode == 2 || _playerMode == 3) {
		_ticksDelta = _vm->_system->getMillis() - _noteStartTime;
		_track[_trackCount].ticks = _ticksDelta;
		if (_trackCount < kMaxTracks - 1)
			++_trackCount;
		_track[_trackCount].noteNum = noteNum;
	}

	_noteStartTime = _vm->_system->getMillis();

	if (noteNum != -2) {
		_resetAnims = false;
		if (_currPatchNum == 0) {
			_objects[25].kind = 1;
			_objects[28].kind = 0;
			_objects[25].frameIndex = 0;
			_objects[25].ticks = getAnimation(25)->frameTicks[0];
			_objects[26].frameIndex = 0;
			_objects[26].ticks = getAnimation(26)->frameTicks[0];
			_objects[27].frameIndex = 0;
			_objects[27].ticks = getAnimation(27)->frameTicks[0];
			_objects[30].kind = 1;
			_objects[33].kind = 0;
			_objects[30].frameIndex = 0;
			_objects[30].ticks = getAnimation(30)->frameTicks[0];
			_objects[31].frameIndex = 0;
			_objects[31].ticks = getAnimation(31)->frameTicks[0];
			_objects[32].frameIndex = 0;
			_objects[32].ticks = getAnimation(32)->frameTicks[0];
		} else if (_currPatchNum == 1) {
			_objects[29].kind = 1;
			_objects[33].kind = 0;
			_objects[29].frameIndex = 0;
			_objects[29].ticks = getAnimation(29)->frameTicks[0];
			_objects[31].frameIndex = 0;
			_objects[31].ticks = getAnimation(31)->frameTicks[0];
			_objects[32].frameIndex = 0;
			_objects[32].ticks = getAnimation(32)->frameTicks[0];
		} else if (_currPatchNum == 2) {
			_objects[24].kind = 1;
			_objects[28].kind = 0;
			_objects[24].frameIndex = 0;
			_objects[24].ticks = getAnimation(24)->frameTicks[0];
			_objects[26].frameIndex = 0;
			_objects[26].ticks = getAnimation(26)->frameTicks[0];
			_objects[27].frameIndex = 0;
			_objects[27].ticks = getAnimation(27)->frameTicks[0];
		}
	}

}

void MinigameBbAirGuitar::noteOff(int noteNum) {

	if (_currNoteNum != noteNum)
		return;

	if (noteNum != -2)
		stopNote(noteNum);

	_objects[21].kind = 0;
	_objects[23].kind = _objects[21].kind;
	_objects[10].kind = _objects[23].kind;

	_vuMeterRight1 = 0;
	_vuMeterRight2 = 0;
	_vuMeterLeft1 = 0;
	_vuMeterLeft2 = 0;

	_currNoteNum = -2;

	_objects[11].kind = 0;

	_ticksDelta = _vm->_system->getMillis() - _noteStartTime;

	if (_playerMode == 2 || _playerMode == 3) {
		if (_actionTrackPos + _ticksDelta > 15000)
			_ticksDelta = 15000 - _actionTrackPos;
		_track[_trackCount].ticks = _ticksDelta;
		if (_trackCount + 1 < 2048)
			++_trackCount;
		_track[_trackCount].noteNum = -2;
		_noteStartTime = _vm->_system->getMillis();
	}

	if (noteNum != -2) {
		if (_playerMode == 0) {
			_resetAnims = true;
			_noteStartTime = _vm->_system->getMillis();
		}
		if (_currPatchNum == 0) {
			_objects[25].frameIndex = 3;
			_objects[25].ticks = -1;
			_objects[26].frameIndex = 3;
			_objects[26].ticks = -1;
			_objects[27].frameIndex = 3;
			_objects[27].ticks = -1;
			_objects[30].frameIndex = 3;
			_objects[30].ticks = -1;
			_objects[31].frameIndex = 3;
			_objects[31].ticks = -1;
			_objects[32].frameIndex = 3;
			_objects[32].ticks = -1;
		} else if (_currPatchNum == 1) {
			_objects[29].frameIndex = 3;
			_objects[29].ticks = -1;
			_objects[31].frameIndex = 3;
			_objects[31].ticks = -1;
			_objects[32].frameIndex = 3;
			_objects[32].ticks = -1;
		} else if (_currPatchNum == 2) {
			_objects[24].frameIndex = 2;
			_objects[24].ticks = -1;
			_objects[26].frameIndex = 3;
			_objects[26].ticks = -1;
			_objects[27].frameIndex = 3;
			_objects[27].ticks = -1;
		}
	}

}

void MinigameBbAirGuitar::resetObjs() {
	_resetAnims = false;
	_objects[25].kind = 0;
	_objects[24].kind = 0;
	_objects[28].kind = 1;
	_objects[26].frameIndex = 0;
	_objects[26].ticks = -1;
	_objects[27].frameIndex = 0;
	_objects[27].ticks = -1;
	_objects[30].kind = 0;
	_objects[29].kind = 0;
	_objects[33].kind = 1;
	_objects[31].frameIndex = 0;
	_objects[31].ticks = -1;
	_objects[32].frameIndex = 0;
	_objects[32].ticks = -1;
}

void MinigameBbAirGuitar::loadSounds() {
	_vm->_sound->loadSound("bbairg/audio/rocktune.aif");
	for (uint i = 0; i < kPatchDirectoriesCount; ++i) {
		const char *patchDirectory = kPatchDirectories[i];
		for (uint j = 0; j < kNoteSoundFilenamesCount; ++j) {
			Common::String filename = Common::String::format("bbairg/audio/%s/%s", patchDirectory, kNoteSoundFilenames[j]);
			_vm->_sound->loadSound(filename.c_str());
		}
	}
}

void MinigameBbAirGuitar::playNote(int noteNum) {
	if (noteNum >= 0 && _currPatchNum >= 0)
		playSound(2 + _currPatchNum * kNoteSoundFilenamesCount + noteNum);
}

void MinigameBbAirGuitar::stopNote(int noteNum) {
	if (noteNum >= 0 && _currPatchNum >= 0)
		stopSound(2 + _currPatchNum * kNoteSoundFilenamesCount + noteNum);
}

bool MinigameBbAirGuitar::getLoadFilename(Common::String &filename) {
	GUI::FileBrowserDialog browser(0, "air", GUI::kFBModeLoad);

	if (browser.runModal() > 0) {
		filename = browser.getResult();
		return true;
	}

	return false;
}

bool MinigameBbAirGuitar::getSaveFilename(Common::String &filename) {
	GUI::FileBrowserDialog browser(0, "air", GUI::kFBModeSave);

	if (browser.runModal() > 0) {
		filename = browser.getResult();
		return true;
	}

	return false;
}

bool MinigameBbAirGuitar::querySaveModifiedDialog() {
	/* NOTE The original button captions don't fit so shortened variants are used
		Original ok button caption: "Yeah, heh, heh, save it!"
		Original discard button caption: "Who cares?  It sucked!"
	*/
	GUI::MessageDialog query("Hey Beavis - you didn't save that last Jam!",
		"Save it!",
		"It sucked!");
	return query.runModal() == GUI::kMessageOK;
}

bool MinigameBbAirGuitar::querySaveModifiedTracks() {
	if (_modified && querySaveModifiedDialog()) {
		if (!saveTracks())
			return false;
	}
	return true;
}

bool MinigameBbAirGuitar::loadTracks() {
	if (_playerMode != 0)
		return false;

	if (!querySaveModifiedTracks())
		return false;

	Common::String filename;
	if (!getLoadFilename(filename))
		return false;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::InSaveFile *stream = saveFileMan->openForLoading(filename);
	if (!loadFromStream(stream)) {
		Common::String msg = Common::String::format("%s is not a valid Air Guitar file", filename.c_str());
		GUI::MessageDialog dialog(msg);
		dialog.runModal();
	}
	delete stream;

	return true;
}

bool MinigameBbAirGuitar::saveTracks() {
	if (_playerMode != 0)
		return false;

	Common::String filename;
	if (!getSaveFilename(filename))
		return false;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::OutSaveFile *stream = saveFileMan->openForSaving(filename);
	saveToStream(stream);
	delete stream;
	_modified = false;

	return true;
}

bool MinigameBbAirGuitar::loadFromStream(Common::ReadStream *stream) {
	uint32 magic = stream->readUint32BE();
	if (magic != MKTAG('A', 'I', 'R', 'G'))
		return false;
	for (uint i = 0; i < kMaxTracks; ++i) {
		_track[i].noteNum = stream->readByte();
		_track[i].ticks = stream->readUint16LE();
	}
	_trackCount = 0;
	_actionTrackPos = 0;
	while (_track[_trackCount].noteNum != -1) {
		_actionTrackPos += _track[_trackCount].ticks;
		++_trackCount;
	}
	_totalTrackLength = _actionTrackPos;
	_trackIndex = 0;
	_currTrackPos = 0;
	return true;
}

void MinigameBbAirGuitar::saveToStream(Common::WriteStream *stream) {
	stream->writeUint32BE(MKTAG('A', 'I', 'R', 'G'));
	for (uint i = 0; i < kMaxTracks; ++i) {
		stream->writeByte(_track[i].noteNum);
		stream->writeUint16LE(_track[i].ticks);
	}
}

} // End of namespace Bbvs
