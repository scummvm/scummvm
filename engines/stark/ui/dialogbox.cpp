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
 */

#include "engines/stark/ui/dialogbox.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/bitmap.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/fontprovider.h"
#include "engines/stark/ui/cursor.h"
#include "engines/stark/visual/text.h"

#include "common/memstream.h"
#include "common/stream.h"
#include "common/formats/winexe_pe.h"

#include "graphics/surface.h"

#include "image/bmp.h"

namespace Stark {

static const uint dialogMaxWidth         = 450;
static const uint dialogHorizontalMargin = 10;
static const uint dialogVerticalMargin   = 20;
static const uint buttonHorizontalMargin = 25;
static const uint buttonVerticalMargin   = 5;

DialogBox::DialogBox(StarkEngine *vm, Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
		_background(nullptr),
		_foreground(nullptr),
		_confirmCallback(nullptr) {
	_vm = vm;
	_surfaceRenderer = gfx->createSurfaceRenderer();

	_background = loadBackground(gfx);
	if (_background) {
		_background->setSamplingFilter(Gfx::Bitmap::kLinear);
	}

	_messageVisual = new VisualText(gfx);
	_messageVisual->setColor(_textColor);
	_messageVisual->setTargetWidth(dialogMaxWidth - 2 * dialogHorizontalMargin);
	_messageVisual->setAlign(Graphics::kTextAlignCenter);

	_confirmLabelVisual = new VisualText(gfx);
	_confirmLabelVisual->setColor(_textColor);
	_confirmLabelVisual->setTargetWidth(96);

	_cancelLabelVisual = new VisualText(gfx);
	_cancelLabelVisual->setColor(_textColor);
	_cancelLabelVisual->setTargetWidth(96);
}

DialogBox::~DialogBox() {
	close();

	delete _messageVisual;
	delete _confirmLabelVisual;
	delete _cancelLabelVisual;

	delete _background;

	delete _surfaceRenderer;
}

void DialogBox::open(const Common::String &message, ConfirmCallback *confirmCallback,
		const Common::String &confirmLabel, const Common::String &cancelLabel) {
	assert(confirmCallback);

	_visible = true;

	_cursor->setCursorType(Cursor::kDefault);
	_cursor->setMouseHint("");

	_messageVisual->setText(message);
	_confirmLabelVisual->setText(confirmLabel);
	_cancelLabelVisual->setText(cancelLabel);

	_confirmCallback = confirmCallback;

	recomputeLayout();
}

void DialogBox::close() {
	freeForeground();

	delete _confirmCallback;
	_confirmCallback = nullptr;

	_visible = false;
}

void DialogBox::recomputeLayout() {
	freeForeground();

	_messageRect = _messageVisual->getRect();

	uint buttonY = dialogVerticalMargin * 2 + _messageRect.height();
	uint32 dialogWidth  = _messageRect.width() + 2 * dialogHorizontalMargin;

	// The button size is computed based on the largest label
	Common::Rect labelSize = _confirmLabelVisual->getRect();
	labelSize.extend(_cancelLabelVisual->getRect());

	_confirmButtonRect = Common::Rect(
			labelSize.width() + buttonHorizontalMargin * 2,
			labelSize.height() + buttonVerticalMargin * 2);

	_cancelButtonRect = Common::Rect(
			labelSize.width() + buttonHorizontalMargin * 2,
			labelSize.height() + buttonVerticalMargin * 2);

	uint buttonSpacing;
	if (dialogWidth > (uint32)(_confirmButtonRect.width() + _cancelButtonRect.width())) {
		buttonSpacing = (dialogWidth - _confirmButtonRect.width() - _cancelButtonRect.width()) / 3;
	} else {
		buttonSpacing = buttonHorizontalMargin;
		dialogWidth = buttonSpacing * 3 + _confirmButtonRect.width() + _cancelButtonRect.width();
	}

	_messageRect.translate((dialogWidth - _messageRect.width()) / 2, dialogVerticalMargin);

	_confirmButtonRect.translate(buttonSpacing, buttonY);
	_cancelButtonRect.translate(buttonSpacing * 2 + _confirmButtonRect.width(), buttonY);

	uint32 dialogHeight = _confirmButtonRect.bottom + dialogVerticalMargin;

	Graphics::Surface foreground;
	foreground.create(dialogWidth, dialogHeight, Gfx::Driver::getRGBAPixelFormat());

	drawBevel(&foreground, Common::Rect(0, 0, foreground.w, foreground.h));

	uint32 buttonColor = foreground.format.RGBToColor(0, 0, 0);
	foreground.fillRect(_confirmButtonRect, buttonColor);
	foreground.fillRect(_cancelButtonRect, buttonColor);

	drawBevel(&foreground, _confirmButtonRect);
	drawBevel(&foreground, _cancelButtonRect);

	_foreground = _gfx->createBitmap(&foreground);
	_foreground->setSamplingFilter(Gfx::Bitmap::kLinear);

	foreground.free();

	Common::Point screenCenter(Gfx::Driver::kOriginalWidth / 2, Gfx::Driver::kOriginalHeight / 2);
	_position = Common::Rect::center(screenCenter.x, screenCenter.y,
	                                 _foreground->width(), _foreground->height());
}

void DialogBox::freeForeground() {
	delete _foreground;
	_foreground = nullptr;

	if (_messageVisual) {
		_messageVisual->reset();
	}

	if (_confirmLabelVisual) {
		_confirmLabelVisual->reset();
	}

	if (_cancelLabelVisual) {
		_cancelLabelVisual->reset();
	}
}

void DialogBox::onScreenChanged() {
	recomputeLayout();
}

void DialogBox::onClick(const Common::Point &pos) {
	if (_cancelButtonRect.contains(pos)) {
		close();
	} else if (_confirmButtonRect.contains(pos)) {
		assert(_confirmCallback);

		(*_confirmCallback)();
		close();
	}
}

void DialogBox::onKeyPress(const Common::CustomEventType customType) {
	if (customType == kActionSkip) {
		close();
	}
}

Gfx::Bitmap *DialogBox::loadBackground(Gfx::Driver *gfx) {
	Common::PEResources *executable = new Common::PEResources();
	if (!executable->loadFromEXE("game.exe") && !executable->loadFromEXE("game.dll")) {
		warning("Unable to load 'game.exe' to read the modal dialog background image");
		delete executable;
		return nullptr;
	}

	// As of Aug 2021:
	// Steam version of The Longest Journey is 1.0.0.161 "Enno's two-CD Version"
	// GOG version of The Longest Journey is 1.0.0.142 "RC1" (Special Build: "Paper Sun")
	// Steam's game.exe does not contain a valid resource for the background bitmap id 147
	// so we skip trying to retrieve it.
	if (_vm->getGameFlags() & GF_MISSING_EXE_RESOURCES) {
		warning("Steam version does not contain the modal dialog background bitmap in 'game.exe'. Using fallback color for dialog background...");
		delete executable;
		return nullptr;
	}

	Common::SeekableReadStream *stream = executable->getResource(Common::kWinBitmap, 147);
	if (!stream) {
		warning("Unable to find the modal dialog background bitmap in 'game.exe'");
		delete executable;
		return nullptr;
	}

	const uint32 bitmapWithHeaderLen = stream->size() + 14;
	byte *bitmapWithHeader = new byte[bitmapWithHeaderLen];

	Common::MemoryWriteStream bitmapWithHeaderWriteStream(bitmapWithHeader, bitmapWithHeaderLen);
	bitmapWithHeaderWriteStream.write("BM", 2);
	bitmapWithHeaderWriteStream.writeUint32LE(bitmapWithHeaderLen); // Filesize
	bitmapWithHeaderWriteStream.writeUint32LE(0);                   // res1 & res2
	bitmapWithHeaderWriteStream.writeUint32LE(0x436);               // image offset

	stream->read(bitmapWithHeader + 14, stream->size());
	delete stream;
	delete executable;

	Common::MemoryReadStream bitmapWithHeaderReadStream(bitmapWithHeader, bitmapWithHeaderLen);

	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(bitmapWithHeaderReadStream)) {
		warning("Unable decode the modal dialog background bitmap from 'game.exe'");
		return nullptr;
	}

	delete[] bitmapWithHeader;

	return gfx->createBitmap(decoder.getSurface(), decoder.getPalette());
}

void DialogBox::onRender() {
	if (_background) {
		uint32 backgroundRepeatX = ceil(_foreground->width() / (float)_background->width());
		for (uint i = 0; i < backgroundRepeatX; i++) {
			_surfaceRenderer->render(_background, Common::Point(i * _background->width(), 0));
		}
	} else {
		_surfaceRenderer->fill(_backgroundColor, Common::Point(0, 0), _foreground->width(), _foreground->height());
	}

	_surfaceRenderer->render(_foreground, Common::Point(0, 0));

	_messageVisual->render(Common::Point(_messageRect.left, _messageRect.top));

	Common::Rect confirmLabelRect = centerRect(_confirmButtonRect, _confirmLabelVisual->getRect());
	Common::Rect cancelLabelRect = centerRect(_cancelButtonRect, _cancelLabelVisual->getRect());

	_confirmLabelVisual->render(Common::Point(confirmLabelRect.left, confirmLabelRect.top));
	_cancelLabelVisual->render(Common::Point(cancelLabelRect.left, cancelLabelRect.top));
}

void DialogBox::drawBevel(Graphics::Surface *surface, const Common::Rect &rect) {
	uint32 topColor1    = surface->format.RGBToColor(191, 191, 191);
	uint32 topColor2    = surface->format.RGBToColor(159, 159, 159);
	uint32 bottomColor1 = surface->format.RGBToColor(64, 64, 64);
	uint32 bottomColor2 = surface->format.RGBToColor(96, 96, 96);

	// Top
	surface->drawLine(rect.left,     rect.top,     rect.right - 1, rect.top,     topColor1);
	surface->drawLine(rect.left + 1, rect.top + 1, rect.right - 2, rect.top + 1, topColor2);

	// Left
	surface->drawLine(rect.left,     rect.top,     rect.left,     rect.bottom - 1, topColor1);
	surface->drawLine(rect.left + 1, rect.top + 1, rect.left + 1, rect.bottom - 2, topColor2);

	// Right
	surface->drawLine(rect.right - 1, rect.top,     rect.right - 1, rect.bottom - 1, bottomColor2);
	surface->drawLine(rect.right - 2, rect.top + 1, rect.right - 2, rect.bottom - 2, bottomColor1);

	// Bottom
	surface->drawLine(rect.left,     rect.bottom - 1, rect.right - 1, rect.bottom - 1, bottomColor2);
	surface->drawLine(rect.left + 1, rect.bottom - 2, rect.right - 2, rect.bottom - 2, bottomColor1);
}

Common::Rect DialogBox::centerRect(const Common::Rect &container, const Common::Rect &size) {
	Common::Point center(
			(container.left + container.right) / 2,
			(container.top + container.bottom) / 2);

	return Common::Rect::center(
			center.x, center.y,
			size.width(), size.height());
}

} // End of namespace Stark
