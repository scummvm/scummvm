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

#ifndef STARK_UI_DIALOG_BOX_H
#define STARK_UI_DIALOG_BOX_H

#include "engines/stark/ui/window.h"

#include "common/keyboard.h"
#include "common/scummsys.h"

namespace Stark {

namespace Gfx {
class SurfaceRenderer;
class Texture;
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
	DialogBox(Gfx::Driver *gfx, Cursor *cursor);
	~DialogBox() override;

	/** Make the dialog visible with the specified message */
	void open(const Common::String &message, ConfirmCallback *confirmCallback,
	          const Common::String &confirmLabel, const Common::String &cancelLabel);

	/** Hide the dialog performing no action */
	void close();

	/** Called when the screen resolution changes */
	void onScreenChanged();

	/** Called when a keyboard key is pressed and the dialog is active */
	void onKeyPress(const Common::KeyState &keyState);

protected:
	void onRender() override;
	void onClick(const Common::Point &pos) override;

private:
	static Graphics::Surface *loadBackground();
	static void drawBevel(Graphics::Surface *surface, const Common::Rect &rect);
	static Common::Rect centerRect(const Common::Rect &container, const Common::Rect &size);

	void freeForeground();
	void recomputeLayout();

	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Texture *_backgroundTexture;
	Gfx::Texture *_foregroundTexture;

	VisualText *_messageVisual;
	VisualText *_confirmLabelVisual;
	VisualText *_cancelLabelVisual;

	Common::Rect _confirmButtonRect;
	Common::Rect _cancelButtonRect;
	Common::Rect _messageRect;

	ConfirmCallback *_confirmCallback;
};

} // End of namespace Stark

#endif // STARK_UI_DIALOG_BOX_H
