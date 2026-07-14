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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_TOOLBAR_H
#define RIPPER_TOOLBAR_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;
struct MouseState;

class ToolbarManager {
public:
	ToolbarManager();

	bool initialize(ResourceManager &resources);
	void applySharedPalettePatch(byte *palette, uint colorCount) const;
	bool service(const MouseState &mouse);
	void leave();

private:
	struct Action {
		BitmapAssetSequence sequence;
		Common::String label;
		Common::Rect bounds;
		uint frameIndex;

		Action() : frameIndex(0) {}
	};

	void enter(uint32 now);
	int findAction(const Common::Point &point) const;
	void drawIcons();
	void drawTooltip(const Common::Point &point);
	void removeTooltip();
	void dispatchAction(uint actionIndex);
	uint measureText(const Common::String &text) const;
	void drawText(byte *screen, uint pitch, int x, int y, const Common::String &text) const;

	Common::Array<Action> _actions;
	Common::Array<byte> _sharedPalette;
	BitmapFontAsset _font;
	Common::Array<byte> _topBacking;
	Common::Array<byte> _tooltipBacking;
	Common::Rect _tooltipBounds;
	uint32 _sessionStartMillis;
	uint32 _lastFrameMillis;
	int _hoveredAction;
	int _pressedAction;
	bool _active;
	bool _previewEnabled;
};

} // End of namespace Ripper

#endif // RIPPER_TOOLBAR_H
