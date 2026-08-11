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

#ifndef STARK_UI_DIALOG_BOX_H
#define STARK_UI_DIALOG_BOX_H

#include "engines/stark/stark.h"
#include "engines/stark/ui/window.h"
#include "engines/stark/gfx/color.h"

#include "common/keyboard.h"
#include "common/scummsys.h"

namespace Stark {

namespace Gfx {
class SurfaceRenderer;
class Bitmap;
}

typedef Common::Functor0<void> ConfirmCallback;

class VisualText;

/**
 * A confirmation dialog with two buttons
 *
 * The cancel button closes the dialog without action.
 * The confirm button executes a callback.
 */
class DialogBox : public Window {
public:
	DialogBox(StarkEngine *vm, Gfx::Driver *gfx, Cursor *cursor);
	~DialogBox() override;

	/** Make the dialog visible with the specified message */
	void open(const Common::String &message, ConfirmCallback *confirmCallback,
	          const Common::String &confirmLabel, const Common::String &cancelLabel);

	/** Hide the dialog performing no action */
	void close();

	/** Called when the screen resolution changes */
	void onScreenChanged();

	/** Called when a keyboard key is pressed and the dialog is active */
	void onKeyPress(const Common::CustomEventType customType);

protected:
	void onRender() override;
	void onClick(const Common::Point &pos) override;

private:
	Gfx::Bitmap *loadBackground(Gfx::Driver *gfx);
	static void drawBevel(Graphics::Surface *surface, const Common::Rect &rect);
	static Common::Rect centerRect(const Common::Rect &container, const Common::Rect &size);

	void freeForeground();
	void recomputeLayout();

	StarkEngine *_vm;

	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Bitmap *_background;
	Gfx::Bitmap *_foreground;

	VisualText *_messageVisual;
	VisualText *_confirmLabelVisual;
	VisualText *_cancelLabelVisual;

	Common::Rect _confirmButtonRect;
	Common::Rect _cancelButtonRect;
	Common::Rect _messageRect;

	const Gfx::Color _textColor = Gfx::Color(0xFF, 0xFF, 0xFF);
	const Gfx::Color _backgroundColor = Gfx::Color(26, 28, 57);

	ConfirmCallback *_confirmCallback;
};

} // End of namespace Stark

#endif // STARK_UI_DIALOG_BOX_H
