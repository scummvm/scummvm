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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "parallaction/graphics.h"
#include "parallaction/parallaction.h"

namespace Parallaction {


#define	BALLOON_TRANSPARENT_COLOR_NS 2
#define BALLOON_TRANSPARENT_COLOR_BR 0

#define BALLOON_TAIL_WIDTH	12
#define BALLOON_TAIL_HEIGHT	10


byte _resBalloonTail[2][BALLOON_TAIL_WIDTH*BALLOON_TAIL_HEIGHT] = {
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	},
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x02, 0x02
	}
};

class BalloonManager_ns : public BalloonManager {

	static int16 _dialogueBalloonX[5];

	struct Balloon {
		Common::Rect outerBox;
		Common::Rect innerBox;
		Graphics::Surface *surface;
		GfxObj	*obj;
	} _intBalloons[5];

	uint	_numBalloons;

	void drawWrappedText(Font *font, Graphics::Surface* surf, char *text, byte color, int16 wrapwidth);
	int createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness);
	Balloon *getBalloon(uint id);

	Gfx *_gfx;

public:
	BalloonManager_ns(Gfx *gfx);
	~BalloonManager_ns();

	void freeBalloons();
	int setLocationBalloon(char *text, bool endGame);
	int setDialogueBalloon(char *text, uint16 winding, byte textColor);
	int setSingleBalloon(char *text, uint16 x, uint16 y, uint16 winding, byte textColor);
	void setBalloonText(uint id, char *text, byte textColor);
	int hitTestDialogueBalloon(int x, int y);
};

int16 BalloonManager_ns::_dialogueBalloonX[5] = { 80, 120, 150, 150, 150 };

BalloonManager_ns::BalloonManager_ns(Gfx *gfx) : _numBalloons(0), _gfx(gfx) {

}

BalloonManager_ns::~BalloonManager_ns() {

}


BalloonManager_ns::Balloon* BalloonManager_ns::getBalloon(uint id) {
	assert(id < _numBalloons);
	return &_intBalloons[id];
}

int BalloonManager_ns::createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness) {
	assert(_numBalloons < 5);

	int id = _numBalloons;

	Balloon *balloon = &_intBalloons[id];

	int16 real_h = (winding == -1) ? h : h + 9;
	balloon->surface = new Graphics::Surface;
	balloon->surface->create(w, real_h, 1);
	balloon->surface->fillRect(Common::Rect(w, real_h), BALLOON_TRANSPARENT_COLOR_NS);

	Common::Rect r(w, h);
	balloon->surface->fillRect(r, 0);
	balloon->outerBox = r;

	r.grow(-borderThickness);
	balloon->surface->fillRect(r, 1);
	balloon->innerBox = r;

	if (winding != -1) {
		// draws tail
		// TODO: this bitmap tail should only be used for Dos games. Amiga should use a polygon fill.
		winding = (winding == 0 ? 1 : 0);
		Common::Rect s(BALLOON_TAIL_WIDTH, BALLOON_TAIL_HEIGHT);
		s.moveTo(r.width()/2 - 5, r.bottom - 1);
		_gfx->blt(s, _resBalloonTail[winding], balloon->surface, LAYER_FOREGROUND, BALLOON_TRANSPARENT_COLOR_NS);
	}

	_numBalloons++;

	return id;
}


int BalloonManager_ns::setSingleBalloon(char *text, uint16 x, uint16 y, uint16 winding, byte textColor) {

	int16 w, h;

	_gfx->getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+5, h, winding, 1);
	Balloon *balloon = &_intBalloons[id];

	_gfx->drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = x;
	balloon->obj->y = y;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	return id;
}

int BalloonManager_ns::setDialogueBalloon(char *text, uint16 winding, byte textColor) {

	int16 w, h;

	_gfx->getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+5, h, winding, 1);
	Balloon *balloon = &_intBalloons[id];

	_gfx->drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = _dialogueBalloonX[id];
	balloon->obj->y = 10;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	if (id > 0) {
		balloon->obj->y += _intBalloons[id - 1].obj->y + _intBalloons[id - 1].outerBox.height();
	}


	return id;
}

void BalloonManager_ns::setBalloonText(uint id, char *text, byte textColor) {
	Balloon *balloon = getBalloon(id);
	balloon->surface->fillRect(balloon->innerBox, 1);
	_gfx->drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);
}


int BalloonManager_ns::setLocationBalloon(char *text, bool endGame) {

	int16 w, h;

	_gfx->getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+(endGame ? 5 : 10), h+5, -1, BALLOON_TRANSPARENT_COLOR_NS);
	Balloon *balloon = &_intBalloons[id];
	_gfx->drawWrappedText(_vm->_dialogueFont, balloon->surface, text, 0, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = 5;
	balloon->obj->y = 5;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	return id;
}

int BalloonManager_ns::hitTestDialogueBalloon(int x, int y) {

	Common::Point p;

	for (uint i = 0; i < _numBalloons; i++) {
		p.x = x - _intBalloons[i].obj->x;
		p.y = y - _intBalloons[i].obj->y;

		if (_intBalloons[i].innerBox.contains(p))
			return i;
	}

	return -1;
}

void BalloonManager_ns::freeBalloons() {
	_gfx->destroyBalloons();

	for (uint i = 0; i < _numBalloons; i++) {
		_intBalloons[i].obj = 0;
		_intBalloons[i].surface = 0;	// no need to delete surface, since it is done by destroyBalloons
	}

	_numBalloons = 0;
}








class BalloonManager_br : public BalloonManager {

	struct Balloon {
		Common::Rect box;
		Graphics::Surface *surface;
		GfxObj	*obj;
	} _intBalloons[3];

	uint	_numBalloons;

	Frames *_leftBalloon;
	Frames *_rightBalloon;
	Disk *_disk;
	Gfx *_gfx;

	void cacheAnims();
	void drawWrappedText(Font *font, Graphics::Surface* surf, char *text, byte color, int16 wrapwidth);
	int createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness);
	Balloon *getBalloon(uint id);
	Graphics::Surface *expandBalloon(Frames *data, int frameNum);


public:
	BalloonManager_br(Disk *disk, Gfx *gfx);
	~BalloonManager_br();

	void freeBalloons();
	int setLocationBalloon(char *text, bool endGame);
	int setDialogueBalloon(char *text, uint16 winding, byte textColor);
	int setSingleBalloon(char *text, uint16 x, uint16 y, uint16 winding, byte textColor);
	void setBalloonText(uint id, char *text, byte textColor);
	int hitTestDialogueBalloon(int x, int y);
};



BalloonManager_br::Balloon* BalloonManager_br::getBalloon(uint id) {
	assert(id < _numBalloons);
	return &_intBalloons[id];
}

Graphics::Surface *BalloonManager_br::expandBalloon(Frames *data, int frameNum) {

	Common::Rect rect;
	data->getRect(frameNum, rect);

	rect.translate(-rect.left, -rect.top);

	Graphics::Surface *surf = new Graphics::Surface;
	surf->create(rect.width(), rect.height(), 1);

	_gfx->unpackBlt(rect, data->getData(frameNum), data->getRawSize(frameNum), surf, 0, BALLOON_TRANSPARENT_COLOR_BR);

	return surf;
}

int BalloonManager_br::setSingleBalloon(char *text, uint16 x, uint16 y, uint16 winding, byte textColor) {
	cacheAnims();

	int id = _numBalloons;
	Frames *src = 0;
	int srcFrame = 0;

	Balloon *balloon = &_intBalloons[id];

	if (winding == 0) {
		src = _rightBalloon;
		srcFrame = 0;
	} else
	if (winding == 1) {
		src = _leftBalloon;
		srcFrame = 0;
	}

	assert(src);

	balloon->surface = expandBalloon(src, srcFrame);
	src->getRect(srcFrame, balloon->box);

//	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = x + balloon->box.left;
	balloon->obj->y = y + balloon->box.top;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_BR;

	printf("balloon (%i, %i)\n", balloon->obj->x, balloon->obj->y);

	_numBalloons++;

	return id;
}

int BalloonManager_br::setDialogueBalloon(char *text, uint16 winding, byte textColor) {
	cacheAnims();

	int id = _numBalloons;
	Frames *src = 0;
	int srcFrame = 0;

	Balloon *balloon = &_intBalloons[id];

	if (winding == 0) {
		src = _rightBalloon;
		srcFrame = id;
	} else
	if (winding == 1) {
		src = _leftBalloon;
		srcFrame = 0;
	}

	assert(src);

	balloon->surface = expandBalloon(src, srcFrame);
	src->getRect(srcFrame, balloon->box);

//	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = balloon->box.left;
	balloon->obj->y = balloon->box.top;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_BR;

	if (id > 0) {
		balloon->obj->y += _intBalloons[id - 1].obj->y + _intBalloons[id - 1].box.height();
	}

	_numBalloons++;

	return id;
}

void BalloonManager_br::setBalloonText(uint id, char *text, byte textColor) { }

int BalloonManager_br::setLocationBalloon(char *text, bool endGame) {
/*
	int16 w, h;

	getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+(endGame ? 5 : 10), h+5, -1, BALLOON_TRANSPARENT_COLOR);
	Balloon *balloon = &_intBalloons[id];
	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, 0, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = 5;
	balloon->obj->y = 5;
*/
	return 0;
}

int BalloonManager_br::hitTestDialogueBalloon(int x, int y) {

	Common::Point p;

	for (uint i = 0; i < _numBalloons; i++) {
		p.x = x - _intBalloons[i].obj->x;
		p.y = y - _intBalloons[i].obj->y;

		if (_intBalloons[i].box.contains(p))
			return i;
	}

	return -1;
}

void BalloonManager_br::freeBalloons() {
	_gfx->destroyBalloons();

	for (uint i = 0; i < _numBalloons; i++) {
		_intBalloons[i].obj = 0;
		_intBalloons[i].surface = 0;	// no need to delete surface, since it is done by destroyBalloons
	}

	_numBalloons = 0;
}

void BalloonManager_br::cacheAnims() {
	if (!_leftBalloon) {
		_leftBalloon = _disk->loadFrames("fumetto.ani");
		_rightBalloon = _disk->loadFrames("fumdx.ani");
	}
}

BalloonManager_br::BalloonManager_br(Disk *disk, Gfx *gfx) : _numBalloons(0), _disk(disk), _gfx(gfx), _leftBalloon(0), _rightBalloon(0) {
}

BalloonManager_br::~BalloonManager_br() {
	delete _leftBalloon;
	delete _rightBalloon;
}

void Parallaction::setupBalloonManager() {
	if (_vm->getGameType() == GType_Nippon) {
		_balloonMan = new BalloonManager_ns(_vm->_gfx);
	} else
	if (_vm->getGameType() == GType_BRA) {
		_balloonMan = new BalloonManager_br(_vm->_disk, _vm->_gfx);
	} else {
		error("Unknown game type");
	}
}

} // namespace Parallaction
