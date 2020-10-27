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

#ifndef STARK_UI_CURSOR_H
#define STARK_UI_CURSOR_H

#include "common/rect.h"
#include "common/scummsys.h"

namespace Stark {

class VisualImageXMG;
class VisualText;

namespace Gfx {
class Driver;
}

namespace Resources {
class Sound;
}

/**
 * Manager for the current game Cursor
 */
class Cursor {
public:
	explicit Cursor(Gfx::Driver *gfx);
	~Cursor();

	/** Render the Cursor */
	void render();

	/** Update the mouse position */
	void setMousePosition(const Common::Point &pos);

	/** Make cycle the cursor's brightness and play sound */
	void setItemActive(bool fading);

	/** Update when the screen resolution has changed */
	void onScreenChanged();

	Common::Point getMousePosition(bool unscaled = false) const;

	/** Rectangle at the mouse position to consider to hit test small world items */
	Common::Rect getHotRectangle() const;

	enum CursorType {
		kImage = -1,
		kDefault = 0,
		kActive = 3,
		kPassive = 9,
		kEye = 10,
		kHand = 11,
		kMouth = 12
	};

	void setCursorType(CursorType type);
	void setCursorImage(VisualImageXMG *image);
	void setMouseHint(const Common::String &hint);
private:
	void updateFadeLevel();
	void updateHintDelay();

	Gfx::Driver *_gfx;

	Common::String _currentHint;
	int32 _hintDisplayDelay;

	Common::Point _mousePos;
	VisualImageXMG *_cursorImage;
	VisualText *_mouseText;
	CursorType _currentCursorType;

	Resources::Sound *_actionHoverSound;

	bool _itemActive;
	float _fadeLevel;
	bool _fadeLevelIncreasing;
	static const float _fadeValueMax;
};

} // End of namespace Stark

#endif // STARK_UI_CURSOR_H
