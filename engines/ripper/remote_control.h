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

#ifndef RIPPER_REMOTE_CONTROL_H
#define RIPPER_REMOTE_CONTROL_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/display.h"
#include "ripper/resources.h"
#include "ripper/settings.h"

namespace Ripper {

class ResourceManager;
class RipperEngine;

class RemoteControlManager {
public:
	explicit RemoteControlManager(RipperEngine *engine);

	bool initialize(ResourceManager &resources);
	bool run();

private:
	struct Control {
		BitmapAssetFrame frame;
		Common::Rect bounds;
		uint16 command;
	};

	bool captureDisplay();
	void restoreDisplay();
	void applyPalette();
	void drawControls() const;
	void drawBitmap(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text) const;
	int findControl(const Common::Point &point) const;
	bool handleCommand(uint16 command, uint &selectedSlider);

	RipperEngine *_engine;
	BitmapFontAsset _font;
	Common::Array<Control> _controls;
	Common::Array<Common::String> _labels;
	IndexedDisplaySnapshot _savedDisplay;
	bool _initialized;
};

} // End of namespace Ripper

#endif // RIPPER_REMOTE_CONTROL_H
