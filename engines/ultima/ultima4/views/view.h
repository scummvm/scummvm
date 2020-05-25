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

#ifndef ULTIMA4_VIEWS_VIEW_H
#define ULTIMA4_VIEWS_VIEW_H

namespace Ultima {
namespace Ultima4 {

#define SCALED(n) ((n) * settings._scale)

class Image;

/**
 * Generic base class for reflecting the state of a game object onto
 * the screen.
 */
class View {
public:
	View(int x, int y, int width, int height);
	virtual ~View() {}

	/**
	 * Hook for reinitializing when graphics reloaded.
	 */
	virtual void reinit();

	/**
	 * Clear the view to black.
	 */
	virtual void clear();

	/**
	 * Update the view to the screen.
	 */
	virtual void update();

	/**
	 * Update a piece of the view to the screen.
	 */
	virtual void update(int x, int y, int width, int height);

	/**
	 * Highlight a piece of the screen by drawing it in inverted colors.
	 */
	virtual void highlight(int x, int y, int width, int height);
	virtual void unhighlight();

protected:
	Common::Rect _bounds;
	Common::Rect _highlightBounds;
	bool _highlighted;
	void drawHighlighted();
#ifdef IOS_ULTIMA4
	friend void U4IOS::updateScreenView();
#endif
	static Image *_screen;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
