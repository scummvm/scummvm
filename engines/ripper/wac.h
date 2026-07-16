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

#ifndef RIPPER_WAC_H
#define RIPPER_WAC_H

#include "common/array.h"
#include "common/rect.h"

#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;
class RipperEngine;

class WacManager {
public:
	explicit WacManager(RipperEngine *engine);

	bool initialize(ResourceManager &resources);
	void run();

private:
	enum FrontEndAction {
		kExitAction = 0x1900,
		kDatabaseAction = 0x2000,
		kTextViewerAction = 0x3100,
		kHelpAction = 0x3b00
	};

	struct Control {
		BitmapAssetFrame bitmap;
		Common::Rect bounds;
		uint16 action;
	};
	struct DatabaseEntry {
		Common::String label;
		byte originalIndex;
	};

	bool captureDisplay();
	void restoreDisplay();
	void drawFrontEnd() const;
	void drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const;
	void serviceIdleWindowAnimations();
	int findControl(const Common::Point &point) const;
	bool dispatchAction(uint16 action);
	void buildDatabaseEntries();
	void runDatabase();
	void drawDatabase() const;
	void dispatchDatabaseEntry(const DatabaseEntry &entry);
	const Common::String &resourceString(uint resourceId) const;
	uint measureText(const Common::String &text) const;
	void drawText(byte *screen, uint pitch, int x, int y, const Common::String &text,
		byte color) const;

	RipperEngine *_engine;
	BitmapAssetFrame _background;
	Common::Array<Control> _controls;
	Common::Array<BitmapAssetFrame> _idleWindowAnimations[2];
	uint _idleWindowFrame[2];
	uint32 _idleWindowLastMillis;
	Common::Array<BitmapAssetFrame> _databaseSkin;
	Common::Array<DatabaseEntry> _databaseEntries;
	Common::Array<Common::String> _gameText;
	BitmapFontAsset _font;
	Common::Array<byte> _savedPixels;
	Common::Array<byte> _savedPalette;
	int _hoveredControl;
	int _pressedControl;
	uint _databaseSelection;
	uint _databaseFirstVisible;
	bool _initialized;
};

} // End of namespace Ripper

#endif // RIPPER_WAC_H
