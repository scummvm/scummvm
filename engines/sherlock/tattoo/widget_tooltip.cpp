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

#include "sherlock/tattoo/widget_tooltip.h"
#include "sherlock/tattoo/tattoo_map.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

void WidgetTooltipBase::draw() {
	Screen &screen = *_vm->_screen;

	// If there was a previously drawn frame in a different position that hasn't yet been erased, then erase it
	if (_oldBounds.width() > 0 && _oldBounds != _bounds)
		erase();

	if (_bounds.width() > 0 && !_surface.empty()) {
		restrictToScreen();

		// Blit the affected area to the screen
		screen.slamRect(_bounds);

		// Draw the widget directly onto the screen. Unlike other widgets, we don't draw to the back buffer,
		// since nothing should be drawing on top of tooltips, so there's no need to store in the back buffer.
		//
		// Skip this bitmap blit entirely when hires TTF text will be queued
		// for this same widget just below (refreshHiresText(), later in
		// this same draw() call): the blocky bitmap glyphs it would put
		// onto the native screen are otherwise blown up 2x/3x/etc. by the
		// hires composite and become visible (however briefly) underneath,
		// or peeking out past the edges of, the crisp TTF text that's
		// meant to fully replace them - this is the actual root cause of
		// the "blocky bitmap text bleed-through" ghost bug: no amount of
		// restoring the background from underneath the bitmap text after
		// the fact (see the registerRoseTattooHiresTextRect()/
		// clearRoseTattooHiresTextRect() dance below) reliably beats the
		// bitmap text to the punch every single frame, so it's better to
		// just never draw it in the first place when it won't be seen.
		bool skipBitmapBlit = false;
#ifdef USE_FREETYPE2
		skipBitmapBlit = screen.usesRoseTattooHiresText();
#endif
		if (!skipBitmapBlit) {
			screen.SHtransBlitFrom(_surface, Common::Point(_bounds.left - screen._currentScroll.x,
				_bounds.top - screen._currentScroll.y));
		}

		// Store a copy of the drawn area for later erasing
		_oldBounds = _bounds;

#ifdef USE_FREETYPE2
		// Re-queue this frame's hires text now, after the erase() pass
		// above (and TattooScene::doBgAnim()'s earlier
		// doBgAnimEraseBackground()/doBgAnimRestoreUI() call, which runs
		// before draw() and erases *last* frame's hires text rect) - see
		// refreshHiresText()'s doc comment for why this can't be done any
		// earlier in the frame.
		//
		// Also tell the screen this exact rect (the widget's own bitmap
		// bounds, not the narrower TTF glyph metrics) will be covered by
		// hires text, so it fully repaints the smooth background there
		// before blending - otherwise the bitmap font's (usually wider)
		// glyphs would keep peeking out past the TTF text's edges.
		//
		// Only do this when the native bitmap blit above actually ran
		// (!skipBitmapBlit): when it was skipped, nothing was drawn onto
		// the native low-res framebuffer for this tooltip at all, so
		// there's no blocky glyph content left to clean up here - and the
		// background-restore this triggers (see
		// Screen::blendRoseTattooHiresTextLayer()) unconditionally repaints
		// the plain hires background across the *entire* rect, which also
		// wipes out any moving character sprite that's already been
		// composited there this same frame (they get restored/hidden every
		// frame the tooltip covers them, then reappear once it moves on -
		// this was the root cause of the "background blinking over/seeing
		// through moveable characters" bug).
		// registerRoseTattooHiresTextRect()/clearRoseTattooHiresTextRect()
		// both expect a rect in *native screen-space* (i.e. relative to the
		// current viewport, matching the fixed-size
		// Screen::_roseTattooHiresTextLayer buffer) - but _bounds itself is
		// in *world/scroll space* (see WidgetTooltip::setText(), which
		// derives it from Events::mousePos(), which already adds
		// Screen::_currentScroll). That distinction is invisible whenever
        // the view is unscrolled (_currentScroll == (0, 0), as is normally
		// the case in room scenes), but on the scrollable overhead map,
		// passing the raw world-space _bounds here registers/clears
		// completely the wrong rect of the hires text layer once the map
		// has scrolled away from (0, 0) - leaving the *previous* tooltip's
		// glyphs never cleared (they persist as a ghost) while the *new*
		// tooltip's glyphs get queued at a location that doesn't match
		// where its native bitmap glyphs were actually blitted. Convert to
		// screen-space first, exactly like the SHtransBlitFrom() call above
		// already does for the native bitmap blit.
		Common::Rect nativeBounds = _bounds;
		nativeBounds.translate(-screen._currentScroll.x, -screen._currentScroll.y);

		if (!skipBitmapBlit)
			screen.registerRoseTattooHiresTextRect(nativeBounds);

		// Wipe any previously-queued hires glyphs for this same rect before
		// re-queuing below. refreshHiresText() re-draws the *same* text at
		// the *same* position every single frame the tooltip stays put
		// (that's the whole point - see its doc comment), but
		// queueRoseTattooHiresText() alpha-blends each draw onto the
		// persistent _roseTattooHiresTextLayer rather than overwriting it.
		// Without clearing first, semi-transparent anti-aliased edge pixels
		// (and writeFancyString()'s 9 slightly-offset drop-shadow passes)
		// re-accumulate opacity frame after frame, so a tooltip that stays
		// on screen for a while visibly "grows"/smears outward well past
		// its own glyph outlines - this is the actual cause of the
		// intermittent blocky-text bleed-through bug, not insufficient
        // background-restore coverage.
		screen.clearRoseTattooHiresTextRect(nativeBounds);
		refreshHiresText();
#endif
	}
}

void WidgetTooltipBase::erase() {
	Screen &screen = *_vm->_screen;

	if (_oldBounds.width() > 0) {
		// Restore the affected area from the back buffer to the screen
		screen.slamRect(_oldBounds);

#ifdef USE_FREETYPE2
		// The bitmap glyphs just restored away in _oldBounds are erased by
		// slamRect() above (it just re-blits the plain background), but any
		// hires TrueType text queued for that same area persists across
		// frames on its own (see Screen::_roseTattooHiresTextLayer) and
		// must be explicitly cleared here too, or it'll keep getting
		// blended back in every frame even after the tooltip is gone.
		//
		// _oldBounds is in world/scroll space (see the matching comment in
		// draw()), so it must be converted to native screen-space before
		// being passed to clearRoseTattooHiresTextRect() - otherwise, once
		// the view has scrolled away from (0, 0) (e.g. on the overhead
		// map), this clears the wrong area of the hires text layer and the
		// old tooltip's glyphs are left behind as a permanent ghost.
		Common::Rect nativeOldBounds = _oldBounds;
		nativeOldBounds.translate(-screen._currentScroll.x, -screen._currentScroll.y);
		screen.clearRoseTattooHiresTextRect(nativeOldBounds);
#endif

		// Reset the old bounds so it won't be erased again
		_oldBounds = Common::Rect(0, 0, 0, 0);
	}
}

/*----------------------------------------------------------------*/

WidgetTooltip::WidgetTooltip(SherlockEngine *vm) : WidgetTooltipBase (vm), _offsetY(0) {
}

void WidgetTooltip::setText(const Common::String &strIn) {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();
	bool reset = false;
	Common::String str = Fonts::unescape(strIn);

	// Make sure that the description is present
	if (!str.empty()) {
		int width = _surface.stringWidth(str) + 2;
		int height = _surface.stringHeight(str) + 2;
		Common::String line1 = str, line2 = "";

		// TODO This code is similar to WidgetInventoryTooltip::setText(). Maybe they should be merged?
		// See if we need to split it into two lines
		if (width > kMaxTooltipWidth) {
			// Go forward word by word to find out where to split the line
			const char *s = str.c_str();
			const char *space = nullptr;
			int dif = 10000;

			for (;;) {
				// Find end of next word
				s = strchr(s + 1, ' ');

				if (s == nullptr) {
					// Reached end of string
					if (space != nullptr) {
						line1 = Common::String(str.c_str(), space);
						line2 = Common::String(space + 1);
						height = _surface.stringHeight(line1) + _surface.stringHeight(line2) + 4;
					}
					break;
				}

				// Found space separating words, so see what width the string up to now is
				Common::String tempLine1 = Common::String(str.c_str(), s);
				Common::String tempLine2 = Common::String(s + 1);
				int width1 = _surface.stringWidth(tempLine1);
				int width2 = _surface.stringWidth(tempLine2);

				// See if we've found a split point that results in a less overall width
				if (ABS(width1 - width2) < dif) {
					// Found a better split point
					dif = ABS(width1 - width2);
					space = s;
					line1 = tempLine1;
					line2 = tempLine2;
				}
			}
		} else {
			// No line split needed
			height = _surface.stringHeight(str) + 2;
		}

		// Reallocate the text surface with the new size
		_surface.create(width, height);
		_surface.clear(TRANSPARENCY);

		// Deliberately do NOT call _surface.setHiresTextOrigin() here (and
		// explicitly clear any origin left over from a previous
		// refreshHiresText() call below), so the writeFancyString() calls
		// below draw the bitmap glyphs into the small local _surface (used
		// for sizing/native-res blitting) WITHOUT also queuing hires TTF
		// text. setText() can run more than once per displayed frame -
		// e.g. while the mouse is quickly passing over several hotspots in
		// a row, _bgFound/_arrowZone (and hence the tooltip text) can
		// change several times before doBgAnim() ever calls draw()/erase()
		// for this widget again. If we queued hires text here, any of
		// those "phantom" intermediate strings that never actually reach
        // draw() would still leave their glyphs queued into
		// Screen::_roseTattooHiresTextLayer at whatever position was
		// current when setText() ran - and since only draw()'s erase() of
		// the *previously drawn* _bounds ever clears that layer, those
		// phantom glyphs are never cleared, bleeding through as a stray
		// blocky text ghost. refreshHiresText() (called every frame from
		// draw(), which is always properly paired with an erase() of
		// whatever was drawn before) is the sole place that should queue
		// hires text for this widget.
		_surface.clearHiresTextOrigin();

		if (line2.empty()) {
			// Only a single line
			_surface.writeFancyString(str, Common::Point(0, 0), BLACK, INFO_TOP);
			_line1 = str;
			_line2 = "";
			_line1X = 0;
			_line2X = _line2Y = 0;
		} else {
			// Two lines to display
			int xp, yp;
			xp = (width - _surface.stringWidth(line1) - 2) / 2;
			_surface.writeFancyString(line1, Common::Point(xp, 0), BLACK, INFO_TOP);
			_line1 = line1;
			_line1X = xp;

			xp = (width - _surface.stringWidth(line2) - 2) / 2;
			yp = _surface.stringHeight(line1) + 2;
			_surface.writeFancyString(line2, Common::Point(xp, yp), BLACK, INFO_TOP);
			_line2 = line2;
			_line2X = xp;
			_line2Y = yp;
		}

		// Set the initial display position for the tooltip text
		int tagX = mousePos.x - width / 2;
		int tagY = mousePos.y - height - _offsetY;

		_bounds = Common::Rect(tagX, tagY, tagX + width, tagY + height);
	} else {
		reset = true;
	}

	if (reset && !_surface.empty()) {
		_surface.free();
		_line1 = _line2 = "";

		// Also clear _bounds (not just the surface): draw()'s erase-if-
		// moved check compares _oldBounds against _bounds, and _bounds
		// was left untouched at its last non-empty position above. If we
		// don't reset it here too, the next draw() call sees
		// _oldBounds == _bounds (unchanged) and therefore never calls
		// erase() - since the surface is now empty, draw() also skips its
		// own drawing branch, so the previously-drawn tooltip (both its
		// bitmap blit and, when USE_FREETYPE2 hires text is active, the
		// registerRoseTattooHiresTextRect()'d smooth text) is left on
		// screen forever, until the mouse happens to reach a *different*
		// bounds later. This is the actual cause of tooltip text lingering
		// on screen while the cursor moves off a hotspot/map icon.
		_bounds = Common::Rect(0, 0, 0, 0);
	}
}

void WidgetTooltip::refreshHiresText() {
#ifdef USE_FREETYPE2
	if (_surface.empty() || _line1.empty())
		return;

	// Recompute the origin from the tooltip's current (possibly just
	// repositioned) _bounds, rather than the mouse position used when
	// setText() first ran, since the tooltip may have moved since then.
	Common::Point origin(_bounds.left - _vm->_screen->_currentScroll.x,
		_bounds.top - _vm->_screen->_currentScroll.y);
	_surface.setHiresTextOrigin(origin);

	// Redraw the same text at the same local coordinates used in setText():
	// this re-blits identical pixels onto the already-drawn bitmap surface
	// (harmless no-op there), but critically also re-queues the hires TTF
	// text (see Fonts::writeString()) at the refreshed origin above, which
	// is the actual point of calling this every frame the tooltip is
	// visible - queueRoseTattooHiresText() is otherwise only invoked once,
	// when the text is first set.
	_surface.writeFancyString(_line1, Common::Point(_line1X, 0), BLACK, INFO_TOP);
	if (!_line2.empty())
		_surface.writeFancyString(_line2, Common::Point(_line2X, _line2Y), BLACK, INFO_TOP);
#endif
}

void WidgetTooltip::handleEvents() {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();

	// Set the new position for the tooltip
	int xp = mousePos.x - _bounds.width() / 2;
	int yp = mousePos.y - _bounds.height() - _offsetY;

	_bounds.moveTo(xp, yp);
}

/*----------------------------------------------------------------*/

void WidgetSceneTooltip::handleEvents() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	// See if thay are pointing at a different object and we need to regenerate the tooltip text
	if (ui._bgFound != ui._oldBgFound || (ui._bgFound != -1 && _surface.empty()) ||
			ui._arrowZone != ui._oldArrowZone || (ui._arrowZone != -1 && _surface.empty())) {
		// See if there is a new object to display text for
		if ((ui._bgFound != -1 && (ui._bgFound != ui._oldBgFound || (ui._bgFound != -1 && _surface.empty()))) ||
				(ui._arrowZone != -1 && (ui._arrowZone != ui._oldArrowZone || (ui._arrowZone != -1 && _surface.empty())))) {
			Common::String str;
			if (ui._bgFound != -1) {
				// Clear the Arrow Zone fields so it won't think we're displaying an Arrow Zone cursor
				if (scene._currentScene != OVERHEAD_MAP2)
					ui._arrowZone = ui._oldArrowZone = -1;

				// Get the description string
				str = (ui._bgFound < 1000) ? scene._bgShapes[ui._bgFound]._description :
					people[ui._bgFound - 1000]._description;

				// WORKAORUND: On the train ride to Cambridge, don't show any tooltips
				if (scene._currentScene == TRAIN_RIDE)
					str = "";
			} else {
				// Get the exit zone description
				str = scene._exits[ui._arrowZone]._dest;
			}

			setText(str.hasPrefix(" ") ? Common::String() : str);
		} else if ((ui._bgFound == -1 && ui._oldBgFound != -1) || (ui._arrowZone == -1 && ui._oldArrowZone != -1)) {
			setText("");
		}

		ui._oldBgFound = ui._bgFound;
	} else {
		// Set the new position for the tooltip
		int tagX = CLIP(mousePos.x - _bounds.width() / 2, 0, SHERLOCK_SCREEN_WIDTH - _bounds.width());
		int tagY = MAX(mousePos.y - _bounds.height() - _offsetY, 0);
		_bounds.moveTo(tagX, tagY);
	}

	ui._oldArrowZone = ui._arrowZone;

	WidgetTooltip::handleEvents();
}

} // End of namespace Tattoo

} // End of namespace Sherlock
