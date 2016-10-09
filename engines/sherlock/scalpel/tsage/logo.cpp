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

#include "common/scummsys.h"
#include "sherlock/scalpel/tsage/logo.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {
namespace Scalpel {
namespace TsAGE {

TLib *Visage::_tLib;

Visage::Visage() {
	_resNum = -1;
	_rlbNum = -1;
	_stream = nullptr;
}

void Visage::setVisage(int resNum, int rlbNum) {
	if ((_resNum != resNum) || (_rlbNum != rlbNum)) {
		_resNum = resNum;
		_rlbNum = rlbNum;
		delete _stream;

		// Games after Ringworld have an extra indirection via the visage index file
		Common::SeekableReadStream *stream = _tLib->getResource(RES_VISAGE, resNum, 9999);
		if (rlbNum == 0)
			rlbNum = 1;

		// Check how many slots there are
		uint16 count = stream->readUint16LE();
		if (rlbNum > count)
			rlbNum = count;

		// Get the flags/rlbNum to use
		stream->seek((rlbNum - 1) * 4 + 2);
		uint32 v = stream->readUint32LE();
		int flags = v >> 30;

		if (flags & 3) {
			rlbNum = (int)(v & 0xff);
		}
		assert((flags & 3) == 0);
		delete stream;

		_stream = _tLib->getResource(RES_VISAGE, resNum, rlbNum);
	}
}

void Visage::clear() {
	delete _stream;
	_stream = nullptr;
}

Visage::~Visage() {
	delete _stream;
}

void Visage::getFrame(ObjectSurface &s, int frameNum) {
	_stream->seek(0);
	int numFrames = _stream->readUint16LE();
	if (frameNum > numFrames)
		frameNum = numFrames;
	if (frameNum > 0)
		--frameNum;

	_stream->seek(frameNum * 4 + 2);
	int offset = _stream->readUint32LE();
	_stream->seek(offset);

	surfaceFromRes(s);
}

int Visage::getFrameCount() const {
	_stream->seek(0);
	return _stream->readUint16LE();
}

bool Visage::isLoaded() const {
	return _stream != nullptr;
}

void Visage::surfaceFromRes(ObjectSurface &s) {
	int frameWidth = _stream->readUint16LE();
	int frameHeight = _stream->readUint16LE();
	Common::Rect r(0, 0, frameWidth, frameHeight);
	s.create(r.width(), r.height());

	s._centroid.x = _stream->readSint16LE();
	s._centroid.y = _stream->readSint16LE();

	_stream->skip(1);
	byte flags = _stream->readByte();
	bool rleEncoded = (flags & 2) != 0;

	byte *destP = (byte *)s.getPixels();

	if (!rleEncoded) {
		_stream->read(destP, r.width() * r.height());
	} else {
		Common::fill(destP, destP + (r.width() * r.height()), 0xff);

		for (int yp = 0; yp < r.height(); ++yp) {
			int width = r.width();
			destP = (byte *)s.getBasePtr(0, yp);

			while (width > 0) {
				uint8 controlVal = _stream->readByte();
				if ((controlVal & 0x80) == 0) {
					// Copy specified number of bytes
					_stream->read(destP, controlVal);
					width -= controlVal;
					destP += controlVal;
				} else if ((controlVal & 0x40) == 0) {
					// Skip a specified number of output pixels
					destP += controlVal & 0x3f;
					width -= controlVal & 0x3f;
				} else {
					// Copy a specified pixel a given number of times
					controlVal &= 0x3f;
					int pixel = _stream->readByte();

					Common::fill(destP, destP + controlVal, pixel);
					destP += controlVal;
					width -= controlVal;
				}
			}
			assert(width == 0);
		}
	}
}

/*--------------------------------------------------------------------------*/

ScalpelEngine *Object::_vm;

Object::Object() {
	_vm = nullptr;
	_isAnimating = _finished = false;
	_frame = 0;
	_numFrames = 0;
	_frameChange = 0;
	_angle = _changeCtr = 0;
	_walkStartFrame = 0;
	_majorDiff = _minorDiff = 0;
	_updateStartFrame = 0;
}

void Object::setVisage(int visage, int strip) {
	_visage.setVisage(visage, strip);
}

void Object::setAnimMode(bool isAnimating) {
	_isAnimating = isAnimating;
	_finished = false;

	_updateStartFrame = _vm->_events->getFrameCounter();
	if (_numFrames)
		_updateStartFrame += 60 / _numFrames;
	_frameChange = 1;
}

void Object::setDestination(const Common::Point &pt) {
	_destination = pt;

	int moveRate = 10;
	_walkStartFrame = _vm->_events->getFrameCounter();
	_walkStartFrame += 60 / moveRate;

	calculateMoveAngle();

	// Get the difference
	int diffX = _destination.x - _position.x;
	int diffY = _destination.y - _position.y;
	int xSign = (diffX < 0) ? -1 : (diffX > 0 ? 1 : 0);
	int ySign = (diffY < 0) ? -1 : (diffY > 0 ? 1 : 0);
	diffX = ABS(diffX);
	diffY = ABS(diffY);

	if (diffX < diffY) {
		_minorDiff = diffX / 2;
		_majorDiff = diffY;
	} else {
		_minorDiff = diffY / 2;
		_majorDiff = diffX;
	}

	// Set the destination position
	_moveDelta = Common::Point(diffX, diffY);
	_moveSign = Common::Point(xSign, ySign);
	_changeCtr = 0;

	assert(diffX || diffY);
}

void Object::erase() {
	Screen &screen = *_vm->_screen;

	if (_visage.isLoaded() && !_oldBounds.isEmpty())
		screen.SHblitFrom(screen._backBuffer1, Common::Point(_oldBounds.left, _oldBounds.top), _oldBounds);
}

void Object::update() {
	Screen &screen = *_vm->_screen;

	if (_visage.isLoaded()) {
		if (isMoving()) {
			uint32 currTime = _vm->_events->getFrameCounter();
			if (_walkStartFrame <= currTime) {
				int moveRate = 10;
				int frameInc = 60 / moveRate;
				_walkStartFrame = currTime + frameInc;
				move();
			}
		}

		if (_isAnimating) {
			if (_frame < _visage.getFrameCount())
				_frame = changeFrame();
			else
				_finished = true;
		}

		// Get the new frame
		ObjectSurface s;
		_visage.getFrame(s, _frame);

		// Display the frame
		_oldBounds = Common::Rect(_position.x, _position.y, _position.x + s.width(), _position.y + s.height());
		_oldBounds.translate(-s._centroid.x, -s._centroid.y);
		screen.SHtransBlitFrom(s, Common::Point(_oldBounds.left, _oldBounds.top));
	}
}

int Object::changeFrame() {
	int frameNum = _frame;
	uint32 currentFrame = _vm->_events->getFrameCounter();

	if (_updateStartFrame <= currentFrame) {
		if (_numFrames > 0) {
			int v = 60 / _numFrames;
			_updateStartFrame = currentFrame + v;

			frameNum = getNewFrame();
		}
	}

	return frameNum;
}

int Object::getNewFrame() {
	int frameNum = _frame + _frameChange;

	if (_frameChange > 0) {
		if (frameNum > _visage.getFrameCount()) {
			frameNum = 1;
		}
	} else if (frameNum < 1) {
		frameNum = _visage.getFrameCount();
	}

	return frameNum;
}

void Object::calculateMoveAngle() {
	int xDiff = _destination.x - _position.x, yDiff = _position.y - _destination.y;

	if (!xDiff && !yDiff)
		_angle = 0;
	else if (!xDiff)
		_angle = (_destination.y >= _position.y) ? 180 : 0;
	else if (!yDiff)
		_angle = (_destination.x >= _position.x) ? 90 : 270;
	else {
		int result = (((xDiff * 100) / ((abs(xDiff) + abs(yDiff))) * 90) / 100);

		if (yDiff < 0)
			result = 180 - result;
		else if (xDiff < 0)
			result += 360;

		_angle = result;
	}
}

bool Object::isAnimEnded() const {
	return _finished;
}

bool Object::isMoving() const {
	return (_destination.x != 0) && (_destination != _position);
}

void Object::move() {
	Common::Point currPos = _position;
	Common::Point moveDiff(5, 3);
	int percent = 100;

	if (dontMove())
		return;

	if (_moveDelta.x >= _moveDelta.y) {
		int xAmount = _moveSign.x * moveDiff.x * percent / 100;
		if (!xAmount)
			xAmount = _moveSign.x;
		currPos.x += xAmount;

		int yAmount = ABS(_destination.y - currPos.y);
		int yChange = _majorDiff / ABS(xAmount);
		int ySign;

		if (!yChange)
			ySign = _moveSign.y;
		else {
			int v = yAmount / yChange;
			_changeCtr += yAmount % yChange;
			if (_changeCtr >= yChange) {
				++v;
				_changeCtr -= yChange;
			}

			ySign = _moveSign.y * v;
		}

		currPos.y += ySign;
		_majorDiff -= ABS(xAmount);
	} else {
		int yAmount = _moveSign.y * moveDiff.y * percent / 100;
		if (!yAmount)
			yAmount = _moveSign.y;
		currPos.y += yAmount;

		int xAmount = ABS(_destination.x - currPos.x);
		int xChange = _majorDiff / ABS(yAmount);
		int xSign;

		if (!xChange)
			xSign = _moveSign.x;
		else {
			int v = xAmount / xChange;
			_changeCtr += xAmount % xChange;
			if (_changeCtr >= xChange) {
				++v;
				_changeCtr -= xChange;
			}

			xSign = _moveSign.x * v;
		}

		currPos.x += xSign;
		_majorDiff -= ABS(yAmount);
	}

	_position = currPos;
	if (dontMove())
		_position = _destination;
}

bool Object::dontMove() const {
	return (_majorDiff <= 0);
}

void Object::endMove() {
	_position = _destination;
}

/*----------------------------------------------------------------*/

bool Logo::show(ScalpelEngine *vm) {
	Events &events = *vm->_events;
	Logo *logo = new Logo(vm);
	bool interrupted = false;

	while (!logo->finished()) {
		logo->nextFrame();

		// Erase areas from previous frame, and update and re-draw objects
		for (int idx = 0; idx < 4; ++idx)
			logo->_objects[idx].erase();
		for (int idx = 0; idx < 4; ++idx)
			logo->_objects[idx].update();

		events.delay(10);
		events.setButtonState();
		++logo->_frameCounter;

		interrupted = vm->shouldQuit() || events.kbHit() || events._pressed;
		if (interrupted) {
			// Keyboard or mouse button pressed, so break out of logo display
			events.clearEvents();
			break;
		}
	}

	delete logo;
	return !interrupted;
}

Logo::Logo(ScalpelEngine *vm) : _vm(vm), _lib("sf3.rlb") {
	Object::_vm = vm;
	Visage::_tLib = &_lib;

	_finished = false;

	// Initialize counter
	_counter = 0;
	_frameCounter = 0;

	// Initialize wait frame counters
	_waitFrames = 0;
	_waitStartFrame = 0;

	// Initialize animation counters
	_animateObject = 0;
	_animateStartFrame = 0;
	_animateFrameDelay = 0;
	_animateFrames = NULL;
	_animateFrame = 0;

	// Save a copy of the original palette
	_vm->_screen->getPalette(_originalPalette);

	// Set up the palettes
	Common::fill(&_palette1[0], &_palette1[PALETTE_SIZE], 0);
	Common::fill(&_palette1[0], &_palette2[PALETTE_SIZE], 0);
	Common::fill(&_palette1[0], &_palette3[PALETTE_SIZE], 0);

	_lib.getPalette(_palette1, 1111);
	_lib.getPalette(_palette1, 10);
	_lib.getPalette(_palette2, 1111);
	_lib.getPalette(_palette2, 1);
	_lib.getPalette(_palette3, 1111);
	_lib.getPalette(_palette3, 14);
}

Logo::~Logo() {
	// Restore the original palette
	_vm->_screen->setPalette(_originalPalette);
}

bool Logo::finished() const {
	return _finished;
}

const AnimationFrame handFrames[] = {
	{  1,  33,  91 }, {  2,  44, 124 }, {  3,  64, 153 }, {  4,  87, 174 },
	{  5, 114, 191 }, {  6, 125, 184 }, {  7, 154, 187 }, {  8, 181, 182 },
	{  9, 191, 167 }, { 10, 190, 150 }, { 11, 182, 139 }, { 11, 170, 130 },
	{ 11, 158, 121 }, {  0,   0,   0 }
};

const AnimationFrame companyFrames[] = {
	{  1, 155,  94 }, {  2, 155,  94 }, {  3, 155,  94 }, {  4, 155,  94 },
	{  5, 155,  94 }, {  6, 155,  94 }, {  7, 155,  94 }, {  8, 155,  94 },
	{  0,   0,   0 }
};

void Logo::nextFrame() {
	Screen &screen = *_vm->_screen;

	if (_waitFrames) {
		uint32 currFrame = _frameCounter;
		if (currFrame - _waitStartFrame < _waitFrames) {
			return;
		}
		_waitStartFrame = 0;
		_waitFrames = 0;
	}

	if (_animateFrames) {
		uint32 currFrame = _frameCounter;
		if (currFrame > _animateStartFrame + _animateFrameDelay) {
			AnimationFrame animationFrame = _animateFrames[_animateFrame];
			if (animationFrame.frame) {
				_objects[_animateObject]._frame = animationFrame.frame;
				_objects[_animateObject]._position = Common::Point(animationFrame.x, animationFrame.y);
				_animateStartFrame += _animateFrameDelay;
				_animateFrame++;
			} else {
				_animateObject = 0;
				_animateFrameDelay = 0;
				_animateFrames = NULL;
				_animateStartFrame = 0;
				_animateFrame = 0;
			}
		}
		if (_animateFrames)
			return;
	}

	switch (_counter++) {
	case 0:
		// Load the background and fade it in
		loadBackground();
		fade(_palette1);
		break;

	case 1:
		// First half of square, circle, and triangle arranging themselves
		_objects[0].setVisage(16, 1);
		_objects[0]._frame = 1;
		_objects[0]._position = Common::Point(169, 107);
		_objects[0]._numFrames = 7;
		_objects[0].setAnimMode(true);
		break;

	case 2:
		// Keep waiting until first animation ends
		if (!_objects[0].isAnimEnded()) {
			--_counter;
		} else {
			// Start second half of the shapes animation
			_objects[0].setVisage(16, 2);
			_objects[0]._frame = 1;
			_objects[0]._numFrames = 11;
			_objects[0].setAnimMode(true);
		}
		break;

	case 3:
		// Keep waiting until second animation of shapes ordering themselves ends
		if (!_objects[0].isAnimEnded()) {
			--_counter;
		} else {
			// Fade out the background but keep the shapes visible
			fade(_palette2);
			screen._backBuffer1.clear();
		}
		waitFrames(10);
		break;

	case 4:
		// Load the new palette
		byte palette[PALETTE_SIZE];
		Common::copy(&_palette2[0], &_palette2[PALETTE_SIZE], &palette[0]);
		_lib.getPalette(palette, 12);
		screen.clear();
		screen.setPalette(palette);

		// Morph into the EA logo
		_objects[0].setVisage(12, 1);
		_objects[0]._frame = 1;
		_objects[0]._numFrames = 7;
		_objects[0].setAnimMode(true);
		_objects[0]._position = Common::Point(170, 142);
		_objects[0].setDestination(Common::Point(158, 71));
		break;

	case 5:
		// Wait until the logo has expanded upwards to form EA logo
		if (_objects[0].isMoving())
			--_counter;
		break;

	case 6:
		fade(_palette3, 40);
		break;

	case 7:
		// Show the 'Electronic Arts' company name
		_objects[1].setVisage(14, 1);
		_objects[1]._frame = 1;
		_objects[1]._position = Common::Point(152, 98);
		waitFrames(120);
		break;

	case 8:
		// Start sequence of positioning and size hand cursor in an arc
		_objects[2].setVisage(18, 1);
		startAnimation(2, 5, &handFrames[0]);
		break;

	case 9:
		// Show a highlighting of the company name
		_objects[1].remove();
		_objects[2].erase();
		_objects[2].remove();
		_objects[3].setVisage(19, 1);
		startAnimation(3, 8, &companyFrames[0]);
		break;

	case 10:
		waitFrames(180);
		break;

	case 11:
		_finished = true;
		break;

	default:
		break;
	}
}

void Logo::waitFrames(uint frames) {
	_waitFrames = frames;
	_waitStartFrame = _frameCounter;
}

void Logo::startAnimation(uint object, uint frameDelay, const AnimationFrame *frames) {
	_animateObject = object;
	_animateFrameDelay = frameDelay;
	_animateFrames = frames;
	_animateStartFrame = _frameCounter;
	_animateFrame = 1;

	_objects[object]._frame = frames[0].frame;
	_objects[object]._position = Common::Point(frames[0].x, frames[0].y);
}

void Logo::loadBackground() {
	Screen &screen = *_vm->_screen;

	for (int idx = 0; idx < 4; ++idx) {
		// Get the portion of the screen
		Common::SeekableReadStream *stream = _lib.getResource(RES_BITMAP, 10, idx);

		// Load it onto the surface
		Common::Point pt((idx / 2) * (SHERLOCK_SCREEN_WIDTH / 2), (idx % 2) * (SHERLOCK_SCREEN_HEIGHT / 2));
		for (int y = 0; y < (SHERLOCK_SCREEN_HEIGHT / 2); ++y, ++pt.y) {
			byte *pDest = (byte *)screen._backBuffer1.getBasePtr(pt.x, pt.y);
			stream->read(pDest, SHERLOCK_SCREEN_WIDTH / 2);
		}

		//	_backgroundBounds = Rect(0, 0, READ_LE_UINT16(data), READ_LE_UINT16(data + 2));
		delete stream;
	}

	// Default to a blank palette
	byte palette[PALETTE_SIZE];
	Common::fill(&palette[0], &palette[PALETTE_SIZE], 0);
	screen.setPalette(palette);

	// Copy the surface to the screen
	screen.SHblitFrom(screen._backBuffer1);
}

void Logo::fade(const byte palette[PALETTE_SIZE], int step) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	byte startPalette[PALETTE_SIZE];
	byte tempPalette[PALETTE_SIZE];

	screen.getPalette(startPalette);

	for (int percent = 0; percent < 100; percent += step) {
		for (int palIndex = 0; palIndex < 256; ++palIndex) {
			const byte *pal1P = (const byte *)&startPalette[palIndex * 3];
			const byte *pal2P = (const byte *)&palette[palIndex * 3];
			byte *destP = &tempPalette[palIndex * 3];

			for (int rgbIndex = 0; rgbIndex < 3; ++rgbIndex, ++pal1P, ++pal2P, ++destP) {
				*destP = (int)*pal1P + ((int)*pal2P - (int)*pal1P) * percent / 100;
			}
		}

		screen.setPalette(tempPalette);
		events.wait(1);
	}

	// Set final palette
	screen.setPalette(palette);
}

} // end of namespace TsAGE
} // end of namespace Scalpel
} // end of namespace Sherlock
