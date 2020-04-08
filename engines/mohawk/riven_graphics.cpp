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

#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_video.h"

#include "common/system.h"
#include "common/memstream.h"

#include "engines/util.h"

#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/colormasks.h"

namespace Mohawk {

class TransitionEffect {
public:
	TransitionEffect(OSystem *system, Graphics::Surface *mainScreen, Graphics::Surface *effectScreen,
		                 RivenTransition type, uint duration, const Common::Rect &rect) :
			_system(system),
			_mainScreen(mainScreen),
			_effectScreen(effectScreen),
			_type(type),
			_duration(duration),
			_timeBased(false),
			_rect(rect) {
	}

	virtual ~TransitionEffect() {}

	bool isTimeBased() const { return _timeBased; }

	virtual bool drawFrame(uint32 elapsed) = 0;

protected:
	Common::Rect makeDirectionalInitalArea() const {
		Common::Rect initialArea = _rect;
		switch (_type) {
			case kRivenTransitionWipeLeft:
			case kRivenTransitionPanLeft:
				initialArea.left = _rect.right;
				break;
			case kRivenTransitionWipeRight:
			case kRivenTransitionPanRight:
				initialArea.right = _rect.left;
				break;
			case kRivenTransitionWipeUp:
			case kRivenTransitionPanUp:
				initialArea.top = _rect.bottom;
				break;
			case kRivenTransitionWipeDown:
			case kRivenTransitionPanDown:
				initialArea.bottom = _rect.top;
				break;
			default:
				error("Unhandled transition type: %d", _type);
		}

		return initialArea;
	}

	OSystem *_system;

	RivenTransition _type;
	uint _duration;
	Common::Rect _rect;
	bool _timeBased;

	Graphics::Surface *_mainScreen;
	Graphics::Surface *_effectScreen;
};

class TransitionEffectWipe : public TransitionEffect {
public:
	TransitionEffectWipe(OSystem *system, Graphics::Surface *mainScreen, Graphics::Surface *effectScreen,
		                     RivenTransition type, uint duration, const Common::Rect &rect) :
			TransitionEffect(system, mainScreen, effectScreen, type, duration, rect) {

		_timeBased = true;
		_lastCopyArea = makeDirectionalInitalArea();
	}

	bool drawFrame(uint32 elapsed) override {
		Common::Rect copyArea;
		switch (_type) {
			case kRivenTransitionWipeLeft:
				copyArea.top = _lastCopyArea.top;
				copyArea.bottom = _lastCopyArea.bottom;
				copyArea.right = _lastCopyArea.left;
				copyArea.left = _rect.width() - elapsed * _rect.width() / _duration;
				break;
			case kRivenTransitionWipeRight:
				copyArea.top = _lastCopyArea.top;
				copyArea.bottom = _lastCopyArea.bottom;
				copyArea.left = _lastCopyArea.right;
				copyArea.right = elapsed * _rect.width() / _duration;
				break;
			case kRivenTransitionWipeUp:
				copyArea.left = _lastCopyArea.left;
				copyArea.right = _lastCopyArea.right;
				copyArea.bottom = _lastCopyArea.top;
				copyArea.top = _rect.height() - elapsed * _rect.height() / _duration;
				break;
			case kRivenTransitionWipeDown:
				copyArea.left = _lastCopyArea.left;
				copyArea.right = _lastCopyArea.right;
				copyArea.top = _lastCopyArea.bottom;
				copyArea.bottom = elapsed * _rect.height() / _duration;
				break;
			default:
				error("Unhandled transition type: %d", _type);
		}

		_lastCopyArea = copyArea;

		if (copyArea.isEmpty()) {
			// Nothing to draw
			return false;
		}

		_effectScreen->copyRectToSurface(*_mainScreen, copyArea.left, copyArea.top, copyArea);
		_system->copyRectToScreen(_effectScreen->getBasePtr(copyArea.left, copyArea.top), _effectScreen->pitch,
		                          copyArea.left, copyArea.top, copyArea.width(), copyArea.height());

		return false;
	}

private:
	Common::Rect _lastCopyArea;
};

class TransitionEffectPan : public TransitionEffect {
public:
	TransitionEffectPan(OSystem *system, Graphics::Surface *mainScreen, Graphics::Surface *effectScreen,
	                    RivenTransition type, uint duration, const Common::Rect &rect, int16 offset) :
			TransitionEffect(system, mainScreen, effectScreen, type, duration, rect) {

		_timeBased = true;
		_offset = offset;
		_initialArea = makeDirectionalInitalArea();
		 complete = false;
	}

	bool drawFrame(uint32 elapsed) override {
		Common::Rect newArea;
		switch (_type) {
			case kRivenTransitionPanLeft:
				newArea.top = _initialArea.top;
				newArea.bottom = _initialArea.bottom;
				newArea.right = _initialArea.right;
				newArea.left = _rect.width() - elapsed * _rect.width() / _duration;
				break;
			case kRivenTransitionPanRight:
				newArea.top = _initialArea.top;
				newArea.bottom = _initialArea.bottom;
				newArea.left = _initialArea.left;
				newArea.right = elapsed * _rect.width() / _duration;
				break;
			case kRivenTransitionPanUp:
				newArea.left = _initialArea.left;
				newArea.right = _initialArea.right;
				newArea.bottom = _initialArea.bottom;
				newArea.top = _rect.height() - elapsed * _rect.height() / _duration;
				break;
			case kRivenTransitionPanDown:
				newArea.left = _initialArea.left;
				newArea.right = _initialArea.right;
				newArea.top = _initialArea.top;
				newArea.bottom = elapsed * _rect.height() / _duration;
				break;
			default:
				error("Unhandled transition type: %d", _type);
		}

		if (newArea.isEmpty()) {
			// Nothing to draw
			return false;
		}

		Common::Rect oldArea;
		if (newArea != _rect) {
			oldArea = Common::Rect(
					newArea.right != _rect.right ? _rect.left + newArea.width() : _rect.left,
					newArea.bottom != _rect.bottom ? _rect.top + newArea.height() : _rect.top,
					newArea.left != _rect.left ? _rect.right - newArea.width() : _rect.right,
					newArea.top != _rect.top ? _rect.bottom - newArea.height() : _rect.bottom
			);
		}

		int oldX = newArea.left != _rect.left ? _rect.left + newArea.width() : _rect.left;
		int oldY = newArea.top != _rect.top ? _rect.top + newArea.height() : _rect.top;

		int newX = newArea.right != _rect.right ? _rect.left + oldArea.width() : _rect.left;
		int newY = newArea.bottom != _rect.bottom ? _rect.top + oldArea.height() : _rect.top;

		if (_offset != -1) {
			if (_type == kRivenTransitionPanDown && oldArea.height() - _offset > 0) {
				newY -= _offset;
			} else if (_type == kRivenTransitionPanUp && newArea.height() + _offset < _rect.height()) {
				newY += _offset;
			} else if (_type == kRivenTransitionPanRight && oldArea.width() - _offset > 0) {
				newX -= _offset;
			} else if (_type == kRivenTransitionPanLeft && newArea.width() + _offset < _rect.width()) {
				newX += _offset;
			} else {
				newX = 0;
				newY = 0;
				newArea = _rect;
				oldArea = Common::Rect();
			}
		}

		if (!oldArea.isEmpty()) {
			_system->copyRectToScreen(_effectScreen->getBasePtr(oldX, oldY), _effectScreen->pitch,
			                          oldArea.left, oldArea.top, oldArea.width(), oldArea.height());
		}

		if (!newArea.isEmpty()) {
			_system->copyRectToScreen(_mainScreen->getBasePtr(newX, newY), _mainScreen->pitch,
			                          newArea.left, newArea.top, newArea.width(), newArea.height());
		}

		if (newArea == _rect) {
			_effectScreen->copyRectToSurface(*_mainScreen, _rect.left, _rect.top, _rect);
			return true; // The transition is complete
		} else {
			return false;
		}
	}

private:
	Common::Rect _initialArea;
	int16 _offset;
	bool complete;
};

class TransitionEffectBlend : public TransitionEffect {
public:
	TransitionEffectBlend(OSystem *system, Graphics::Surface *mainScreen, Graphics::Surface *effectScreen,
	                    RivenTransition type, uint duration, const Common::Rect &rect) :
			TransitionEffect(system, mainScreen, effectScreen, type, duration, rect) {

		_timeBased = false;
	}

	bool drawFrame(uint32 elapsed) override {
		assert(_effectScreen->format == _mainScreen->format);
		assert(_effectScreen->format == _system->getScreenFormat());

		if (elapsed == _duration) {
			_effectScreen->copyRectToSurface(*_mainScreen, 0, 0, Common::Rect(_mainScreen->w, _mainScreen->h));
			_system->copyRectToScreen(_effectScreen->getBasePtr(0, 0), _effectScreen->pitch, 0, 0, _effectScreen->w, _effectScreen->h);
			return true; // The transition is complete
		} else {
			Graphics::Surface *screen = _system->lockScreen();

			uint alpha = elapsed * 255 / _duration;
			for (uint y = 0; y < _mainScreen->h; y++) {
				uint16 *src1 = (uint16 *) _mainScreen->getBasePtr(0, y);
				uint16 *src2 = (uint16 *) _effectScreen->getBasePtr(0, y);
				uint16 *dst = (uint16 *) screen->getBasePtr(0, y);
				for (uint x = 0; x < _mainScreen->w; x++) {
					uint8 r1, g1, b1, r2, g2, b2;
					Graphics::colorToRGB< Graphics::ColorMasks<565> >(*src1++, r1, g1, b1);
					Graphics::colorToRGB< Graphics::ColorMasks<565> >(*src2++, r2, g2, b2);

					uint r = r1 * alpha + r2 * (255 - alpha);
					uint g = g1 * alpha + g2 * (255 - alpha);
					uint b = b1 * alpha + b2 * (255 - alpha);

					r /= 255;
					g /= 255;
					b /= 255;

					*dst++ = (uint16) Graphics::RGBToColor< Graphics::ColorMasks<565> >(r, g, b);
				}
			}

			_system->unlockScreen();
			return false;
		}
	}
};

RivenGraphics::RivenGraphics(MohawkEngine_Riven* vm) :
		GraphicsManager(),
		_vm(vm),
		_screenUpdateNesting(0),
		_screenUpdateRunning(false),
		_enableCardUpdateScript(true),
		_scheduledTransition(kRivenTransitionNone),
		_dirtyScreen(false),
		_creditsImage(kRivenCreditsZeroImage),
		_creditsPos(0),
		_transitionMode(kRivenTransitionModeFastest),
		_transitionOffset(-1),
		_waterEffect(nullptr),
		_fliesEffect(nullptr),
		_menuFont(nullptr),
		_transitionFrames(0),
		_transitionDuration(0) {
	_bitmapDecoder = new MohawkBitmap();

	// Restrict ourselves to a single pixel format to simplify the effects implementation
	_pixelFormat = Graphics::createPixelFormat<565>();
	initGraphics(608, 436, &_pixelFormat);

	// The actual game graphics only take up the first 392 rows. The inventory
	// occupies the rest of the screen and we don't use the buffer to hold that.
	_mainScreen = new Graphics::Surface();
	_mainScreen->create(608, 392, _pixelFormat);

	_effectScreen = new Graphics::Surface();
	_effectScreen->create(608, 392, _pixelFormat);

	if (_vm->isGameVariant(GF_25TH)) {
		loadMenuFont();
	}
}

RivenGraphics::~RivenGraphics() {
	_effectScreen->free();
	delete _effectScreen;
	_mainScreen->free();
	delete _mainScreen;
	delete _bitmapDecoder;
	clearFliesEffect();
	clearWaterEffect();
	delete _menuFont;
}

MohawkSurface *RivenGraphics::decodeImage(uint16 id) {
	Common::SeekableReadStream *resourceStream = _vm->getResource(ID_TBMP, id);
	Common::SeekableReadStream *memResourceStream = resourceStream->readStream(resourceStream->size());
	delete resourceStream;

	MohawkSurface *surface = _bitmapDecoder->decodeImage(memResourceStream);
	surface->convertToTrueColor();
	return surface;
}

void RivenGraphics::copyImageToScreen(uint16 image, uint32 left, uint32 top, uint32 right, uint32 bottom) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	beginScreenUpdate();

	// Clip the width to fit on the screen. Fixes some images.
	if (left + surface->w > 608)
		surface->w = 608 - left;

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(left, i + top), surface->getBasePtr(0, i), surface->w * surface->format.bytesPerPixel);

	_dirtyScreen = true;
	applyScreenUpdate();
}

void RivenGraphics::updateScreen() {
	if (_dirtyScreen) {
		// Copy to screen if there's no transition. Otherwise transition.
		if (_scheduledTransition == kRivenTransitionNone
		    || _transitionMode == kRivenTransitionModeDisabled) {
			const Common::Rect updateRect = Common::Rect(0, 0, 608, 392);

			// mainScreen -> effectScreen -> systemScreen
			_effectScreen->copyRectToSurface(*_mainScreen, updateRect.left, updateRect.top, updateRect);
			_vm->_system->copyRectToScreen(_effectScreen->getBasePtr(updateRect.left, updateRect.top), _effectScreen->pitch, updateRect.left, updateRect.top, updateRect.width(), updateRect.height());

			_scheduledTransition = kRivenTransitionNone;
		} else {
			runScheduledTransition();
		}

		_dirtyScreen = false;
	}
}

void RivenGraphics::scheduleWaterEffect(uint16 sfxeID) {
	clearWaterEffect();

	_waterEffect = new WaterEffect(_vm, sfxeID);
}

void RivenGraphics::clearWaterEffect() {
	delete _waterEffect;
	_waterEffect = nullptr;
}

WaterEffect::WaterEffect(MohawkEngine_Riven *vm, uint16 sfxeID) :
		_vm(vm) {
	Common::SeekableReadStream *sfxeStream = _vm->getResource(ID_SFXE, sfxeID);

	if (sfxeStream->readUint16BE() != 'SL')
		error ("Unknown sfxe tag");

	// Read in header info
	uint16 frameCount = sfxeStream->readUint16BE();
	uint32 offsetTablePosition = sfxeStream->readUint32BE();
	_rect.left = sfxeStream->readUint16BE();
	_rect.top = sfxeStream->readUint16BE();
	_rect.right = sfxeStream->readUint16BE();
	_rect.bottom = sfxeStream->readUint16BE();
	_speed = sfxeStream->readUint16BE();
	// Skip the rest of the fields...

	// Read in offsets
	sfxeStream->seek(offsetTablePosition);
	Common::Array<uint32> frameOffsets;
	frameOffsets.resize(frameCount);
	for (uint16 i = 0; i < frameCount; i++)
		frameOffsets[i] = sfxeStream->readUint32BE();

	// Read in the scripts
	sfxeStream->seek(frameOffsets[0]);
	for (uint16 i = 0; i < frameCount; i++) {
		uint scriptLength = (i == frameCount - 1) ? sfxeStream->size() - frameOffsets[i] : frameOffsets[i + 1] - frameOffsets[i];
		_frameScripts.push_back(sfxeStream->readStream(scriptLength));
	}

	// Set it to the first frame
	_curFrame = 0;
	_lastFrameTime = 0;

	delete sfxeStream;
}

void WaterEffect::update() {
	if (_vm->_system->getMillis() <= _lastFrameTime + 1000 / _speed) {
		return; // Nothing to do yet
	}

	// Make sure the script is at the starting point
	Common::SeekableReadStream *script = _frameScripts[_curFrame];
	script->seek(0);

	Graphics::Surface *screen = _vm->_system->lockScreen();
	Graphics::Surface *mainScreen = _vm->_gfx->getBackScreen();
	assert(screen->format == mainScreen->format);

	// Run script
	uint16 curRow = 0;
	for (uint16 op = script->readUint16BE(); op != 4; op = script->readUint16BE()) {
		if (op == 1) {        // Increment Row
			curRow++;
		} else if (op == 3) { // Copy Pixels
			uint16 dstLeft = script->readUint16BE();
			uint16 srcLeft = script->readUint16BE();
			uint16 srcTop = script->readUint16BE();
			uint16 rowWidth = script->readUint16BE();

			byte *src = (byte *)mainScreen->getBasePtr(srcLeft, srcTop);
			byte *dst = (byte *)screen->getBasePtr(dstLeft, curRow + _rect.top);

			memcpy(dst, src, rowWidth * screen->format.bytesPerPixel);
		} else if (op != 4) { // End of Script
			error ("Unknown SFXE opcode %d", op);
		}
	}

	_vm->_system->unlockScreen();

	// Increment frame
	_curFrame++;
	if (_curFrame == _frameScripts.size())
		_curFrame = 0;

	// Set the new time
	_lastFrameTime = _vm->_system->getMillis();
}

WaterEffect::~WaterEffect() {
	for (uint i = 0; i < _frameScripts.size(); i++) {
		delete _frameScripts[i];
	}
}

void RivenGraphics::setTransitionMode(RivenTransitionMode mode) {
	_transitionMode = mode;
	switch (_transitionMode) {
		case kRivenTransitionModeFastest:
			_transitionFrames   = 8;
			_transitionDuration = 300;
			break;
		case kRivenTransitionModeNormal:
			_transitionFrames   = 16;
			_transitionDuration = 500;
			break;
		case kRivenTransitionModeBest:
			_transitionFrames   = 32;
			_transitionDuration = 700;
			break;
		case kRivenTransitionModeDisabled:
			_transitionFrames   = 0;
			_transitionDuration = 0;
			break;
		default:
			error("Unknown transition mode %d", _transitionMode);
	}
}

RivenTransitionMode RivenGraphics::sanitizeTransitionMode(int mode) {
	if (mode != kRivenTransitionModeDisabled
	    && mode != kRivenTransitionModeFastest
	    && mode != kRivenTransitionModeNormal
	    && mode != kRivenTransitionModeBest) {
		return kRivenTransitionModeFastest;
	}

	return static_cast<RivenTransitionMode>(mode);
}

void RivenGraphics::scheduleTransition(RivenTransition id, const Common::Rect &rect) {
	_scheduledTransition = id;
	_transitionRect = rect;

	RivenHotspot *hotspot = _vm->getCard()->getCurHotspot();
	if (hotspot) {
		_transitionOffset = hotspot->getTransitionOffset();
	} else {
		_transitionOffset = -1;
	}
}

void RivenGraphics::runScheduledTransition() {
	if (_scheduledTransition == kRivenTransitionNone)
		return;

	// Note: Transitions 0-11 are actual transitions, but none are used in-game.
	// There's no point in implementing them if they're not used. These extra
	// transitions were found by hacking scripts.

	TransitionEffect *effect = nullptr;
	switch (_scheduledTransition) {
		case kRivenTransitionWipeLeft:
		case kRivenTransitionWipeRight:
		case kRivenTransitionWipeUp:
		case kRivenTransitionWipeDown: {
			effect = new TransitionEffectWipe(_vm->_system, _mainScreen, _effectScreen,
			                                  _scheduledTransition, _transitionDuration, _transitionRect);
			break;
		}
		case kRivenTransitionPanLeft:
		case kRivenTransitionPanRight:
		case kRivenTransitionPanUp:
		case kRivenTransitionPanDown: {
			effect = new TransitionEffectPan(_vm->_system, _mainScreen, _effectScreen,
			                                 _scheduledTransition, _transitionDuration, _transitionRect, _transitionOffset);
			break;
		}
		case kRivenTransitionBlend:
		case kRivenTransitionBlend2: // (tspit CARD 155)
			effect = new TransitionEffectBlend(_vm->_system, _mainScreen, _effectScreen,
			                                   _scheduledTransition, _transitionFrames, _transitionRect);
			break;
		default:
			error("Unhandled transition type: %d", _scheduledTransition);
	}

	if (effect->isTimeBased()) {
		uint32 startTime = _vm->_system->getMillis();
		uint32 timeElapsed = 0;
		bool transitionComplete = false;
		while (timeElapsed < _transitionDuration && !transitionComplete && !_vm->hasGameEnded()) {
			transitionComplete = effect->drawFrame(timeElapsed);

			_vm->doFrame();
			timeElapsed = _vm->_system->getMillis() - startTime;
		}

		if (!transitionComplete) {
			effect->drawFrame(_transitionDuration);
		}
	} else {
		for (uint frame = 1; frame <= _transitionFrames && !_vm->hasGameEnded(); frame++) {
			effect->drawFrame(frame);

			_vm->doFrame();
		}
	}
	delete effect;

	_scheduledTransition = kRivenTransitionNone; // Clear scheduled transition
	_transitionOffset = -1;
}

void RivenGraphics::clearMainScreen() {
	_mainScreen->fillRect(Common::Rect(0, 0, 608, 392), _pixelFormat.RGBToColor(0, 0, 0));
}

void RivenGraphics::fadeToBlack() {
	// The transition speed is forced to best here
	setTransitionMode(kRivenTransitionModeBest);
	scheduleTransition(kRivenTransitionBlend);
	clearMainScreen();
	runScheduledTransition();
}

void RivenGraphics::drawExtrasImageToScreen(uint16 id, const Common::Rect &rect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	_vm->_system->copyRectToScreen(surface->getPixels(), surface->pitch, rect.left, rect.top, surface->w, surface->h);

	delete mhkSurface;
}

void RivenGraphics::drawRect(const Common::Rect &rect, bool active) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (active)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenGraphics::drawImageRect(uint16 id, const Common::Rect &srcRect, const Common::Rect &dstRect) {
	// Draw tBMP id from srcRect to dstRect
	Graphics::Surface *surface = findImage(id)->getSurface();

	assert(srcRect.width() == dstRect.width() && srcRect.height() == dstRect.height());

	for (uint16 i = 0; i < srcRect.height(); i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(srcRect.left, i + srcRect.top), srcRect.width() * surface->format.bytesPerPixel);

	_dirtyScreen = true;
}

void RivenGraphics::drawExtrasImage(uint16 id, const Common::Rect &dstRect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	assert(dstRect.width() == surface->w);

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(0, i), surface->pitch);

	delete mhkSurface;
	_dirtyScreen = true;
}

void RivenGraphics::beginCredits() {
	// Clear the old cache
	clearCache();

	_creditsImage = kRivenCreditsZeroImage;
	_creditsPos = 0;

	// Now cache all the credits images
	for (uint16 i = kRivenCreditsZeroImage; i <= kRivenCreditsLastImage; i++) {
		MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, i));
		surface->convertToTrueColor();
		addImageToCache(i, surface);
	}

	// And clear our screen too
	clearMainScreen();
	_effectScreen->fillRect(Common::Rect(0, 0, 608, 392), _pixelFormat.RGBToColor(0, 0, 0));
}

void RivenGraphics::updateCredits() {
	if ((_creditsImage == kRivenCreditsFirstImage || _creditsImage == kRivenCreditsSecondImage) && _creditsPos == 0)
		fadeToBlack();

	if (_creditsImage < kRivenCreditsSecondImage) {
		// For the first two credit images, they are faded from black to the image and then out again
		scheduleTransition(kRivenTransitionBlend);

		Graphics::Surface *frame = findImage(_creditsImage++)->getSurface();
		for (int y = 0; y < frame->h; y++)
			memcpy(_mainScreen->getBasePtr(124, y), frame->getBasePtr(0, y), frame->pitch);

		runScheduledTransition();
	} else {
		// Otherwise, we're scrolling
		// This is done by 1) moving the screen up one row and 
		// 2) adding a new row at the bottom that is the current row of the current image or 
		// not and it defaults to being empty (a black row).

		// Move the screen up one row
		memmove(_mainScreen->getPixels(), _mainScreen->getBasePtr(0, 1), _mainScreen->pitch * (_mainScreen->h - 1));

		// Copy the next row to the bottom of the screen and keep incrementing the credit images and which row we are on until we reach the last.
		if (_creditsImage <= kRivenCreditsLastImage) {
			Graphics::Surface *frame = findImage(_creditsImage)->getSurface();
			memcpy(_mainScreen->getBasePtr(124, _mainScreen->h - 1), frame->getBasePtr(0, _creditsPos), frame->pitch);
			_creditsPos++;
			if (_creditsPos == _mainScreen->h) {
				_creditsImage++;
				_creditsPos = 0;
			}
		}

		// Now flush the new screen
		_vm->_system->copyRectToScreen(_mainScreen->getPixels(), _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
	}
}

void RivenGraphics::beginScreenUpdate() {
	_screenUpdateNesting++;
}

void RivenGraphics::applyScreenUpdate(bool force) {
	if (force) {
		_screenUpdateNesting = 0;
	} else {
		_screenUpdateNesting--;
	}

	// The screen is only updated when the outermost screen update ends
	if (_screenUpdateNesting <= 0 && !_screenUpdateRunning) {
		_screenUpdateRunning = true;

		if (_enableCardUpdateScript) {
			_vm->getCard()->runScript(kCardUpdateScript);
		}
		_vm->_sound->triggerDrawSound();
		updateScreen();

		_screenUpdateNesting = 0;
		_screenUpdateRunning = false;
	}
}

void RivenGraphics::setFliesEffect(uint16 count, bool fireflies) {
	delete _fliesEffect;
	_fliesEffect = new FliesEffect(_vm, count, fireflies);
}

void RivenGraphics::clearFliesEffect() {
	delete _fliesEffect;
	_fliesEffect = nullptr;
}

Graphics::Surface *RivenGraphics::getBackScreen() {
	return _mainScreen;
}

Graphics::Surface *RivenGraphics::getEffectScreen() {
	return _effectScreen;
}

void RivenGraphics::updateEffects() {
	if (_waterEffect && _vm->_vars["waterenabled"] != 0) {
		_waterEffect->update();
	}

	if (_fliesEffect) {
		_fliesEffect->update();
	}
}

void RivenGraphics::copySystemRectToScreen(const Common::Rect &rect) {
	Graphics::Surface *screen = _vm->_system->lockScreen();
	_mainScreen->copyRectToSurface(*screen, rect.left, rect.top, rect);
	_effectScreen->copyRectToSurface(*screen, rect.left, rect.top, rect);
	_vm->_system->unlockScreen();
}

void RivenGraphics::enableCardUpdateScript(bool enable) {
	_enableCardUpdateScript = enable;
}

void RivenGraphics::drawText(const Common::U32String &text, const Common::Rect &dest, uint8 greyLevel) {
	_mainScreen->fillRect(dest, _pixelFormat.RGBToColor(0, 0, 0));

	uint32 color = _pixelFormat.RGBToColor(greyLevel, greyLevel, greyLevel);

	const Graphics::Font *font = getMenuFont();
	font->drawString(_mainScreen, text, dest.left, dest.top, dest.width(), color);

	_dirtyScreen = true;
}

void RivenGraphics::loadMenuFont() {
	delete _menuFont;
	_menuFont = nullptr;

	const char *fontName;

	if (_vm->getLanguage() != Common::JA_JPN) {
		fontName = "FreeSans.ttf";
	} else {
		fontName = "mplus-2c-regular.ttf";
	}

#if defined(USE_FREETYPE2)
	int fontHeight;

	if (_vm->getLanguage() != Common::JA_JPN) {
		fontHeight = 12;
	} else {
		fontHeight = 11;
	}

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fontName);
	if (stream) {
		_menuFont = Graphics::loadTTFFont(*stream, fontHeight);
		delete stream;
	}
#endif

	if (!_menuFont) {
		warning("Cannot load font %s", fontName);
	}
}

const Graphics::Font *RivenGraphics::getMenuFont() const {
	const Graphics::Font *font;

	if (_menuFont) {
		font = _menuFont;
	} else {
		font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	}

	return font;
}

const FliesEffect::FliesEffectData FliesEffect::_firefliesParameters = {
		true,
		true,
		true,
		true,
		3.0F,
		0.7F,
		40,
		2.0F,
		1.0F,
		8447718,
		30,
		10
};

const FliesEffect::FliesEffectData FliesEffect::_fliesParameters = {
		false,
		false,
		false,
		true,
		8.0F,
		3.0F,
		80,
		3.0F,
		1.0F,
		661528,
		30,
		10
};

FliesEffect::FliesEffect(MohawkEngine_Riven *vm, uint16 count, bool fireflies) :
		_vm(vm) {

	_effectSurface = _vm->_gfx->getEffectScreen();
	_backSurface = _vm->_gfx->getBackScreen();
	_gameRect = Common::Rect(608, 392);

	if (fireflies) {
		_parameters = &_firefliesParameters;
	} else {
		_parameters = &_fliesParameters;
	}

	_updatePeriodMs = 66;
	_nextUpdateTime = _vm->_system->getMillis();

	initFlies(count);
}

FliesEffect::~FliesEffect() {

}

void FliesEffect::initFlies(uint16 count) {
	_fly.resize(count);
	for (uint16 i = 0; i < _fly.size(); i++) {
		initFlyRandomPosition(i);
	}
}

void FliesEffect::initFlyRandomPosition(uint index) {
	int posX = _vm->_rnd->getRandomNumber(_gameRect.right - 4);
	int posY = _vm->_rnd->getRandomNumber(_gameRect.bottom - 4);

	if (posY < 100) {
		posY = 100;
	}

	initFlyAtPosition(index, posX, posY, 15);
}

int FliesEffect::randomBetween(int min, int max) {
	return _vm->_rnd->getRandomNumber(max - min) + min;
}

void FliesEffect::initFlyAtPosition(uint index, int posX, int posY, int posZ) {
	FliesEffectEntry &fly = _fly[index];

	fly.posX = posX;
	fly.posXFloat = posX;
	fly.posY = posY;
	fly.posYFloat = posY;
	fly.posZ = posZ;
	fly.light = true;

	fly.framesTillLightSwitch = randomBetween(_parameters->minFramesLit, _parameters->minFramesLit + _parameters->maxLightDuration);

	fly.hasBlur = false;
	fly.directionAngleRad = randomBetween(0, 300) / 100.0F;
	fly.directionAngleRadZ = randomBetween(0, 300) / 100.0F;
	fly.speed = randomBetween(0, 100) / 100.0F;
}

void FliesEffect::update() {
	if (_nextUpdateTime <= _vm->_system->getMillis()) {
		_nextUpdateTime = _updatePeriodMs + _vm->_system->getMillis();

		updateFlies();
		draw();
		updateScreen();
	}
}

void FliesEffect::updateFlies() {
	for (uint i = 0; i < _fly.size(); i++) {
		updateFlyPosition(i);

		if (_fly[i].posX < 1 || _fly[i].posX > _gameRect.right - 4 || _fly[i].posY > _gameRect.bottom - 4) {
			initFlyRandomPosition(i);
		}

		if (_parameters->lightable) {
			_fly[i].framesTillLightSwitch--;

			if (_fly[i].framesTillLightSwitch <= 0) {
				_fly[i].light = !_fly[i].light;
				_fly[i].framesTillLightSwitch = randomBetween(_parameters->minFramesLit, _parameters->minFramesLit + _parameters->maxLightDuration);
				_fly[i].hasBlur = false;
			}
		}
	}
}

void FliesEffect::updateFlyPosition(uint index) {
	FliesEffectEntry &fly = _fly[index];

	if (fly.directionAngleRad > 2.0F * M_PI) {
		fly.directionAngleRad = fly.directionAngleRad - 2.0F * M_PI;
	}
	if (fly.directionAngleRad < 0.0F) {
		fly.directionAngleRad = fly.directionAngleRad + 2.0F * M_PI;
	}
	if (fly.directionAngleRadZ > 2.0F * M_PI) {
		fly.directionAngleRadZ = fly.directionAngleRadZ - 2.0F * M_PI;
	}
	if (fly.directionAngleRadZ < 0.0F) {
		fly.directionAngleRadZ = fly.directionAngleRadZ + 2.0F * M_PI;
	}
	fly.posXFloat += cos(fly.directionAngleRad) * fly.speed;
	fly.posYFloat += sin(fly.directionAngleRad) * fly.speed;
	fly.posX = fly.posXFloat;
	fly.posY = fly.posYFloat;
	selectAlphaMap(
			fly.posXFloat - fly.posX >= 0.5,
			fly.posYFloat - fly.posY >= 0.5,
			&fly.alphaMap,
			&fly.width,
			&fly.height);
	fly.posZFloat += cos(fly.directionAngleRadZ) * (fly.speed / 2.0F);
	fly.posZ = fly.posZFloat;
	if (_parameters->canBlur && fly.speed > _parameters->blurSpeedTreshold) {
		fly.hasBlur = true;
		float blurPosXFloat = cos(fly.directionAngleRad + M_PI) * _parameters->blurDistance + fly.posXFloat;
		float blurPosYFloat = sin(fly.directionAngleRad + M_PI) * _parameters->blurDistance + fly.posYFloat;

		fly.blurPosX = blurPosXFloat;
		fly.blurPosY = blurPosYFloat;
		selectAlphaMap(
				blurPosXFloat - fly.blurPosX >= 0.5,
				blurPosYFloat - fly.blurPosY >= 0.5,
				&fly.blurAlphaMap,
				&fly.blurWidth,
				&fly.blurHeight);
	}
	if (fly.posY >= 100) {
		int maxAngularSpeed = _parameters->maxAcceleration;
		if (fly.posZ > 15) {
			maxAngularSpeed /= 2;
		}
		int angularSpeed = randomBetween(-maxAngularSpeed, maxAngularSpeed);
		fly.directionAngleRad += angularSpeed / 100.0F;
	} else {
		// Make the flies go down if they are too high in the screen
		int angularSpeed = randomBetween(0, 50);
		if (fly.directionAngleRad >= M_PI / 2.0F && fly.directionAngleRad <= 3.0F * M_PI / 2.0F) {
			// Going down
			fly.directionAngleRad -= angularSpeed / 100.0F;
		} else {
			// Going up
			fly.directionAngleRad += angularSpeed / 100.0F;
		}
		if (fly.posY < 1) {
			initFlyRandomPosition(index);
		}
	}
	if (fly.posZ >= 0) {
		int distanceToScreenEdge;
		if (fly.posX / 10 >= (_gameRect.right - fly.posX) / 10) {
			distanceToScreenEdge = (_gameRect.right - fly.posX) / 10;
		} else {
			distanceToScreenEdge = fly.posX / 10;
		}
		if (distanceToScreenEdge > (_gameRect.bottom - fly.posY) / 10) {
			distanceToScreenEdge = (_gameRect.bottom - fly.posY) / 10;
		}
		if (distanceToScreenEdge > 30) {
			distanceToScreenEdge = 30;
		}
		if (fly.posZ <= distanceToScreenEdge) {
			fly.directionAngleRadZ += randomBetween(-_parameters->maxAcceleration, _parameters->maxAcceleration) / 100.0F;
		} else {
			fly.posZ = distanceToScreenEdge;
			fly.directionAngleRadZ += (float)M_PI;
		}
	} else {
		fly.posZ = 0;
		fly.directionAngleRadZ += (float)M_PI;
	}
	float minSpeed = _parameters->minSpeed - fly.posZ / 40.0F;
	float maxSpeed = _parameters->maxSpeed - fly.posZ / 20.0F;
	fly.speed += randomBetween(-_parameters->maxAcceleration, _parameters->maxAcceleration) / 100.0F;
	if (fly.speed > maxSpeed) {
		fly.speed -= randomBetween(0, 50) / 100.0F;
	}
	if (fly.speed < minSpeed) {
		fly.speed += randomBetween(0, 50) / 100.0F;
	}
}

void FliesEffect::selectAlphaMap(bool horGridOffset, bool vertGridoffset, const uint16 **alphaMap, uint *width, uint *height) {
	static const uint16 alpha1[12] = {
			 8, 16,  8,
			16, 32, 16,
			 8, 16,  8,
			 0,  0,  0
	};

	static const uint16 alpha2[12] = {
			4, 12, 12, 4,
			8, 24, 24, 8,
			4, 12, 12, 4
	};

	static const uint16 alpha3[12] = {
			 4,  8,  4,
			12, 24, 12,
			12, 24, 12,
			 4,  8,  4
	};

	static const uint16 alpha4[16] = {
			2,  6,  6, 2,
			6, 18, 18, 6,
			6, 18, 18, 6,
			2,  6,  6, 2
	};

	static const uint16 alpha5[12] = {
			4,  8, 4,
			8, 32, 8,
			4,  8, 4,
			0,  0, 0
	};

	static const uint16 alpha6[12] = {
			2,  6,  6, 2,
			4, 24, 24, 4,
			2,  6,  6, 2
	};

	static const uint16 alpha7[12] = {
			2,  4, 2,
			6, 24, 6,
			6, 24, 6,
			2,  4, 2
	};

	static const uint16 alpha8[16] = {
			1,  3,  3, 1,
			3, 18, 18, 3,
			3, 18, 18, 3,
			1,  3,  3, 1
	};

	struct AlphaMap {
		bool horizontalGridOffset;
		bool verticalGridOffset;
		bool isLarge;
		uint16 width;
		uint16 height;
		const uint16 *pixels;
	};

	static const AlphaMap alphaSelector[] = {
			{ true,  true,  true,  4, 4, alpha4 },
			{ true,  true,  false, 4, 4, alpha8 },
			{ true,  false, true,  4, 3, alpha2 },
			{ true,  false, false, 4, 3, alpha6 },
			{ false, true,  true,  3, 4, alpha3 },
			{ false, true,  false, 3, 4, alpha7 },
			{ false, false, true,  3, 3, alpha1 },
			{ false, false, false, 3, 3, alpha5 }
	};

	for (uint i = 0; i < ARRAYSIZE(alphaSelector); i++) {
		if (alphaSelector[i].horizontalGridOffset == horGridOffset
		    && alphaSelector[i].verticalGridOffset == vertGridoffset
		    && alphaSelector[i].isLarge == _parameters->isLarge) {
			*alphaMap = alphaSelector[i].pixels;
			*width = alphaSelector[i].width;
			*height = alphaSelector[i].height;
			return;
		}
	}

	error("Unknown flies alpha map case");
}

void FliesEffect::draw() {
	const Graphics::PixelFormat format = _effectSurface->format;

	for (uint i = 0; i < _fly.size(); i++) {
		FliesEffectEntry &fly = _fly[i];
		uint32 color = _parameters->color32;
		if (!fly.light) {
			color = _fliesParameters.color32;
		}

		bool hoveringBrightBackground = false;
		for (uint y = 0; y < fly.height; y++) {
			uint16 *pixel = (uint16 *) _effectSurface->getBasePtr(fly.posX, fly.posY + y);

			for (uint x = 0; x < fly.width; x++) {
				byte r, g, b;
				format.colorToRGB(*pixel, r, g, b);

				if (_parameters->unlightIfTooBright) {
					if (r >= 192 || g >= 192 || b >= 192) {
						hoveringBrightBackground = true;
					}
				}
				colorBlending(color, r, g, b, fly.alphaMap[fly.width * y + x] - fly.posZ);

				*pixel = format.RGBToColor(r, g, b);
				++pixel;
			}
		}

		Common::Rect drawRect = Common::Rect(fly.width, fly.height);
		drawRect.translate(fly.posX, fly.posY);
		addToScreenDirtyRects(drawRect);
		addToEffectsDirtyRects(drawRect);

		if (fly.hasBlur) {
			for (uint y = 0; y < fly.blurHeight; y++) {
				uint16 *pixel = (uint16 *) _effectSurface->getBasePtr(fly.blurPosX, fly.blurPosY + y);
				for (uint x = 0; x < fly.blurWidth; x++) {
					byte r, g, b;
					format.colorToRGB(*pixel, r, g, b);

					colorBlending(color, r, g, b, fly.blurAlphaMap[fly.blurWidth * y + x] - fly.posZ);

					*pixel = format.RGBToColor(r, g, b);
					++pixel;
				}
			}

			Common::Rect drawRect2 = Common::Rect(fly.blurWidth, fly.blurHeight);
			drawRect2.translate(fly.blurPosX, fly.blurPosY);
			addToScreenDirtyRects(drawRect2);
			addToEffectsDirtyRects(drawRect2);

			fly.hasBlur = false;
		}

		if (hoveringBrightBackground) {
			fly.hasBlur = false;
			if (_parameters->lightable) {
				fly.light = false;
				fly.framesTillLightSwitch = randomBetween(_parameters->minFramesLit, _parameters->minFramesLit + _parameters->maxLightDuration);
			}

			if (_vm->_rnd->getRandomBit()) {
				fly.directionAngleRad += (float)M_PI / 2.0F;
			} else {
				fly.directionAngleRad -= (float)M_PI / 2.0F;
			}
		}
	}
}

void FliesEffect::colorBlending(uint32 flyColor, byte &r, byte &g, byte &b, int alpha) {
	alpha = CLIP(alpha, 0, 32);
	byte flyR = (flyColor & 0x000000FF) >> 0;
	byte flyG = (flyColor & 0x0000FF00) >> 8;
	byte flyB = (flyColor & 0x00FF0000) >> 16;

	r = (32 * r + alpha * (flyR - r)) / 32;
	g = (32 * g + alpha * (flyG - g)) / 32;
	b = (32 * b + alpha * (flyB - b)) / 32;
}

void FliesEffect::updateScreen() {
	for (uint i = 0; i < _screenSurfaceDirtyRects.size(); i++) {
		const Common::Rect &rect = _screenSurfaceDirtyRects[i];
		_vm->_system->copyRectToScreen(_effectSurface->getBasePtr(rect.left, rect.top),
		                               _effectSurface->pitch, rect.left, rect.top,
		                               rect.width(), rect.height()
		);
	}
	_screenSurfaceDirtyRects.clear();

	restoreEffectsSurface();
}

void FliesEffect::addToScreenDirtyRects(const Common::Rect &rect) {
	for (uint i = 0; i < _screenSurfaceDirtyRects.size(); i++) {
		if (rect.intersects(_screenSurfaceDirtyRects[i])) {
			_screenSurfaceDirtyRects[i].extend(rect);
			return;
		}
	}

	_screenSurfaceDirtyRects.push_back(rect);
}

void FliesEffect::addToEffectsDirtyRects(const Common::Rect &rect) {
	for (uint i = 0; i < _effectsSurfaceDirtyRects.size(); i++) {
		if (rect.intersects(_effectsSurfaceDirtyRects[i])) {
			_effectsSurfaceDirtyRects[i].extend(rect);
			return;
		}
	}

	_effectsSurfaceDirtyRects.push_back(rect);
}

void FliesEffect::restoreEffectsSurface() {
	for (uint i = 0; i < _effectsSurfaceDirtyRects.size(); i++) {
		const Common::Rect &rect = _effectsSurfaceDirtyRects[i];
		_effectSurface->copyRectToSurface(*_backSurface, rect.left, rect.top, rect);
		addToScreenDirtyRects(rect);
	}

	_effectsSurfaceDirtyRects.clear();
}

} // End of namespace Mohawk
