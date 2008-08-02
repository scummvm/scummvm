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

#include "common/util.h"

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

	void getStringExtent(Font *font, char *text, uint16 maxwidth, int16* width, int16* height);
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

	getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+5, h, winding, 1);
	Balloon *balloon = &_intBalloons[id];

	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

	// TODO: extract some text to make a name for obj
	balloon->obj = _gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = x;
	balloon->obj->y = y;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	return id;
}

int BalloonManager_ns::setDialogueBalloon(char *text, uint16 winding, byte textColor) {

	int16 w, h;

	getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+5, h, winding, 1);
	Balloon *balloon = &_intBalloons[id];

	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

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
	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);
}


int BalloonManager_ns::setLocationBalloon(char *text, bool endGame) {

	int16 w, h;

	getStringExtent(_vm->_dialogueFont, text, MAX_BALLOON_WIDTH, &w, &h);

	int id = createBalloon(w+(endGame ? 5 : 10), h+5, -1, BALLOON_TRANSPARENT_COLOR_NS);
	Balloon *balloon = &_intBalloons[id];
	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, 0, MAX_BALLOON_WIDTH);

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

// TODO: get rid of parseNextToken from here. Use the
// StringTokenizer instead.
void BalloonManager_ns::drawWrappedText(Font *font, Graphics::Surface* surf, char *text, byte color, int16 wrapwidth) {

	uint16 lines = 0;
	uint16 linewidth = 0;

	uint16 rx = 10;
	uint16 ry = 4;

	uint16 blankWidth = font->getStringWidth(" ");
	uint16 tokenWidth = 0;

	char token[MAX_TOKEN_LEN];

	if (wrapwidth == -1)
		wrapwidth = _vm->_screenWidth;

	while (strlen(text) > 0) {

		text = parseNextToken(text, token, MAX_TOKEN_LEN, "   ", true);

		if (!scumm_stricmp(token, "%p")) {
			lines++;
			rx = 10;
			ry = 4 + lines*10;	// y

			strcpy(token, "> .......");
			strncpy(token+2, _password, strlen(_password));
			tokenWidth = font->getStringWidth(token);
		} else {
			tokenWidth = font->getStringWidth(token);

			linewidth += tokenWidth;

			if (linewidth > wrapwidth) {
				// wrap line
				lines++;
				rx = 10;			// x
				ry = 4 + lines*10;	// y
				linewidth = tokenWidth;
			}

			if (!scumm_stricmp(token, "%s")) {
				sprintf(token, "%d", _score);
			}

		}

		_gfx->drawText(font, surf, rx, ry, token, color);

		rx += tokenWidth + blankWidth;
		linewidth += blankWidth;

		text = Common::ltrim(text);
	}

}

// TODO: get rid of parseNextToken from here. Use the
// StringTokenizer instead.
void BalloonManager_ns::getStringExtent(Font *font, char *text, uint16 maxwidth, int16* width, int16* height) {

	uint16 lines = 0;
	uint16 w = 0;
	*width = 0;

	uint16 blankWidth = font->getStringWidth(" ");
	uint16 tokenWidth = 0;

	char token[MAX_TOKEN_LEN];

	while (strlen(text) != 0) {

		text = parseNextToken(text, token, MAX_TOKEN_LEN, "   ", true);
		tokenWidth = font->getStringWidth(token);

		w += tokenWidth;

		if (!scumm_stricmp(token, "%p")) {
			lines++;
		} else {
			if (w > maxwidth) {
				w -= tokenWidth;
				lines++;
				if (w > *width)
					*width = w;

				w = tokenWidth;
			}
		}

		w += blankWidth;
		text = Common::ltrim(text);
	}

	if (*width < w) *width = w;
	*width += 10;

	*height = lines * 10 + 20;

	return;
}





class BalloonManager_br : public BalloonManager {

	struct Balloon {
		Common::Rect box;
		Graphics::Surface *surface;
		GfxObj	*obj;
	} _intBalloons[3];

	uint	_numBalloons;

	Disk *_disk;
	Gfx *_gfx;

	Frames *_leftBalloon;
	Frames *_rightBalloon;

	void cacheAnims();
	void getStringExtent(Font *font, const char *text, uint16 maxwidth, int16* width, int16* height);
	void drawWrappedText(Font *font, Graphics::Surface* surf, char *text, byte color, int16 wrapwidth);
	int createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness);
	Balloon *getBalloon(uint id);
	Graphics::Surface *expandBalloon(Frames *data, int frameNum);

	void textSetupRendering(const Common::String &text, Graphics::Surface *dest, Font *font, byte color);
	void textEmitCenteredLine();
	void textAccum(const Common::String &token, uint16 width);
	void textNewLine();

	Common::String _textLine;
	Graphics::Surface *_textSurf;
	Font *_textFont;
	uint16 _textX, _textY;
	byte _textColor;
	uint16 _textLines, _textWidth;

	void extentSetup(Font *font, int16 *width, int16 *height);
	void extentAction();

	int16 *_extentWidth, *_extentHeight;


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

	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

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

	drawWrappedText(_vm->_dialogueFont, balloon->surface, text, textColor, MAX_BALLOON_WIDTH);

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


void BalloonManager_br::extentSetup(Font *font, int16 *width, int16 *height) {
	_extentWidth = width;
	_extentHeight = height;

	_textLine.clear();
	_textLines = 0;
	_textWidth = 0;
	_textFont = font;
}

void BalloonManager_br::extentAction() {
	if (_textWidth > *_extentWidth) {
		*_extentWidth = _textWidth;
	}
	*_extentHeight = _textLines * _textFont->height();
}

void BalloonManager_br::textSetupRendering(const Common::String &text, Graphics::Surface *dest, Font *font, byte color) {
	uint16 maxWidth = 216;

	int16 w, h;
	getStringExtent(font, text.c_str(), maxWidth, &w, &h);

	w += 10;
	h += 12;

	_textLine.clear();
	_textSurf = dest;
	_textFont = font;
	_textX = 0;
	_textY = (_textSurf->h - h) / 2;
	_textColor = color;
	_textLines = 0;
	_textWidth = 0;
}

void BalloonManager_br::textEmitCenteredLine() {
	if (_textLine.empty()) {
		return;
	}
	uint16 rx = _textX + (_textSurf->w - _textWidth) / 2;
	uint16 ry = _textY + _textLines * _textFont->height();	// y
	_gfx->drawText(_textFont, _textSurf, rx, ry, _textLine.c_str(), _textColor);
}

void BalloonManager_br::textAccum(const Common::String &token, uint16 width) {
	if (token.empty()) {
		return;
	}

	_textWidth += width;
	_textLine += token;
}

void BalloonManager_br::textNewLine() {
	_textLines++;
	_textWidth = 0;
	_textLine.clear();
}


// TODO: really, base this and getStringExtent on some kind of LineTokenizer, instead of
// repeating the algorithm and changing a couple of lines.
void BalloonManager_br::drawWrappedText(Font *font, Graphics::Surface* surf, char *text, byte color, int16 wrapWidth) {
	textSetupRendering(text, surf, font, color);

	wrapWidth = 216;

	Common::StringTokenizer	tokenizer(text, " ");
	Common::String token;
	Common::String blank(" ");

	uint16 blankWidth = font->getStringWidth(" ");
	uint16 tokenWidth = 0;

	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();

		if (token == '/') {
			tokenWidth = 0;
			textEmitCenteredLine();
			textNewLine();
		} else {
			// todo: expand '%'
			tokenWidth = font->getStringWidth(token.c_str());

			if (_textWidth == 0) {
				textAccum(token, tokenWidth);
			} else {
				if (_textWidth + blankWidth + tokenWidth <= wrapWidth) {
					textAccum(blank, blankWidth);
					textAccum(token, tokenWidth);
				} else {
					textEmitCenteredLine();
					textNewLine();
					textAccum(token, tokenWidth);
				}
			}
		}
	}

	textEmitCenteredLine();
}



void BalloonManager_br::getStringExtent(Font *font, const char *text, uint16 maxwidth, int16* width, int16* height) {
	extentSetup(font, width, height);

	Common::StringTokenizer	tokenizer(text, " ");
	Common::String token;
	Common::String blank(" ");

	uint16 blankWidth = font->getStringWidth(" ");
	uint16 tokenWidth = 0;

	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();

		if (token == '/') {
			tokenWidth = 0;
			extentAction();
			textNewLine();
		} else {
			// todo: expand '%'
			tokenWidth = font->getStringWidth(token.c_str());

			if (_textWidth == 0) {
				textAccum(token, tokenWidth);
			} else {
				if (_textWidth + blankWidth + tokenWidth <= maxwidth) {
					textAccum(blank, blankWidth);
					textAccum(token, tokenWidth);
				} else {
					extentAction();
					textNewLine();
					textAccum(token, tokenWidth);
				}
			}
		}
	}

	extentAction();
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
