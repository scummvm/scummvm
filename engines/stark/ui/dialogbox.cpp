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

#include "engines/stark/ui/dialogbox.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/fontprovider.h"
#include "engines/stark/ui/cursor.h"
#include "engines/stark/visual/text.h"

#include "common/memstream.h"
#include "common/stream.h"
#include "common/winexe_pe.h"
#include "graphics/surface.h"
#include "image/bmp.h"


namespace Stark {

static const uint dialogMaxWidth         = 450;
static const uint dialogHorizontalMargin = 10;
static const uint dialogVerticalMargin   = 20;
static const uint buttonHorizontalMargin = 25;
static const uint buttonVerticalMargin   = 5;
static const Color textColor = Color(0xFF, 0xFF, 0xFF);

DialogBox::DialogBox(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
		_foregroundTexture(nullptr),
		_confirmCallback(nullptr) {

	_surfaceRenderer = gfx->createSurfaceRenderer();

	Graphics::Surface *background = loadBackground();
	if (!background) {
		// If we were not able to load the background, fallback to dark blue
		background = new Graphics::Surface();
		background->create(256, 256, Gfx::Driver::getRGBAPixelFormat());

		uint32 blue = background->format.RGBToColor(26, 28, 57);
		background->fillRect(Common::Rect(256, 256), blue);
	}
	_backgroundTexture = gfx->createTexture(background);
	_backgroundTexture->setSamplingFilter(Gfx::Texture::kLinear);

	background->free();
	delete background;

	_messageVisual = new VisualText(gfx);
	_messageVisual->setColor(textColor);
	_messageVisual->setTargetWidth(dialogMaxWidth - 2 * dialogHorizontalMargin);
	_messageVisual->setAlign(Graphics::kTextAlignCenter);

	_confirmLabelVisual = new VisualText(gfx);
	_confirmLabelVisual->setColor(textColor);
	_confirmLabelVisual->setTargetWidth(96);

	_cancelLabelVisual = new VisualText(gfx);
	_cancelLabelVisual->setColor(textColor);
	_cancelLabelVisual->setTargetWidth(96);
}

DialogBox::~DialogBox() {
	close();

	delete _messageVisual;
	delete _confirmLabelVisual;
	delete _cancelLabelVisual;

	delete _backgroundTexture;

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

	_foregroundTexture = _gfx->createTexture(&foreground);
	_foregroundTexture->setSamplingFilter(Gfx::Texture::kLinear);

	foreground.free();

	Common::Point screenCenter(Gfx::Driver::kOriginalWidth / 2, Gfx::Driver::kOriginalHeight / 2);
	_position = Common::Rect::center(screenCenter.x, screenCenter.y,
	                                 _foregroundTexture->width(), _foregroundTexture->height());
}

void DialogBox::freeForeground() {
	delete _foregroundTexture;
	_foregroundTexture = nullptr;

	if (_messageVisual) {
		_messageVisual->resetTexture();
	}

	if (_confirmLabelVisual) {
		_confirmLabelVisual->resetTexture();
	}

	if (_cancelLabelVisual) {
		_cancelLabelVisual->resetTexture();
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

void DialogBox::onKeyPress(const Common::KeyState &keyState) {
	if (keyState.keycode == Common::KEYCODE_ESCAPE) {
		close();
	}
}

Graphics::Surface *DialogBox::loadBackground() {
	Common::PEResources executable;
	if (!executable.loadFromEXE("game.exe") && !executable.loadFromEXE("game.dll")) {
		warning("Unable to load 'game.exe' to read the modal dialog background image");
		return nullptr;
	}

	Common::SeekableReadStream *stream = executable.getResource(Common::kWinBitmap, 147);
	if (!stream) {
		warning("Unable to find the modal dialog background bitmap in 'game.exe'");
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

	Common::MemoryReadStream bitmapWithHeaderReadStream(bitmapWithHeader, bitmapWithHeaderLen);

	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(bitmapWithHeaderReadStream)) {
		warning("Unable decode the modal dialog background bitmap from 'game.exe'");
		return nullptr;
	}

	delete[] bitmapWithHeader;

	return decoder.getSurface()->convertTo(Gfx::Driver::getRGBAPixelFormat(), decoder.getPalette());
}

void DialogBox::onRender() {
	uint32 backgroundRepeatX = ceil(_foregroundTexture->width() / (float)_backgroundTexture->width());
	for (uint i = 0; i < backgroundRepeatX; i++) {
		_surfaceRenderer->render(_backgroundTexture, Common::Point(i * _backgroundTexture->width(), 0));
	}

	_surfaceRenderer->render(_foregroundTexture, Common::Point(0, 0));

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
