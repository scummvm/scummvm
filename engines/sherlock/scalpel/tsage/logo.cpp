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
	_animMode = ANIM_MODE_NONE;
	_frame = 0;
	_numFrames = 0;
	_frameChange = 0;
}

void Object::setVisage(int visage, int strip) {
	_visage.setVisage(visage, strip);
}

void Object::setAnimMode(AnimationMode mode) {
	_animMode = mode;
	_finished = false;

	_updateStartFrame = _vm->_events->getFrameCounter();
	if (_numFrames)
		_updateStartFrame += 60 / _numFrames;
	_frameChange = 1;
}

void Object::update() {
	Screen &screen = *_vm->_screen;

	if (_visage.isLoaded()) {
		switch (_animMode) {
		case ANIM_MODE_5:
			if (_frame < _visage.getFrameCount())
				_frame = changeFrame();
			else
				_finished = true;
			break;

		default:
			break;
		}

		// Erase previous frame, if any
		if (!_oldBounds.isEmpty())
			screen.blitFrom(screen._backBuffer1, Common::Point(_oldBounds.left, _oldBounds.top), _oldBounds);

		// Get the new frame
		ObjectSurface s;
		_visage.getFrame(s, _frame);

		// Display the frame
		_oldBounds = Common::Rect(_position.x, _position.y, _position.x + s.w(), _position.y + s.h());
		_oldBounds.translate(-s._centroid.x, -s._centroid.y);
		screen.transBlitFrom(s, Common::Point(_oldBounds.left, _oldBounds.top));
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

bool Object::isAnimEnded() const {
	return _finished;
}

/*----------------------------------------------------------------*/

bool Logo::show(ScalpelEngine *vm) {
	Events &events = *vm->_events;
	Logo *logo = new Logo(vm);
	bool interrupted = false;

	while (!logo->finished()) {
		logo->nextFrame();

		events.wait(2);
		events.setButtonState();

		for (int idx = 0; idx < 4; ++idx)
			logo->_objects[idx].update();

		interrupted = vm->shouldQuit() || events.kbHit() || events._pressed;
		if (interrupted) {
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

	// Initialize counter
	_counter = 0;
	
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
	return _counter >= 4;
}

void Logo::nextFrame() {
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
		_objects[0].setAnimMode(ANIM_MODE_5);
		break;

	case 2:
		// Keep waiting until first animation ends
		if (!_objects[0].isAnimEnded())
			--_counter;
		break;

	case 3:
		// Keep waiting until second animation of shapes ordering themselves ends
		return;

	default:
		break;
	}
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
	screen.blitFrom(screen._backBuffer1);
}

void Logo::fade(const byte palette[PALETTE_SIZE]) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	byte tempPalette[PALETTE_SIZE];

	for (int percent = 0; percent < 100; percent += 6) {
		for (int palIndex = 0; palIndex < 256; ++palIndex) {
			const byte *palP = (const byte *)&palette[palIndex * 3];
			byte *destP = &tempPalette[palIndex * 3];

			for (int rgbIndex = 0; rgbIndex < 3; ++rgbIndex, ++palP, ++destP) {
				*destP = (int)*palP * percent / 100;
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
