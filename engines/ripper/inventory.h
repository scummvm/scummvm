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

#ifndef RIPPER_INVENTORY_H
#define RIPPER_INVENTORY_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;
class RipperEngine;

class Inventory {
public:
	enum Result {
		kCancelled,
		kUsed,
		kLoadFailed
	};

	explicit Inventory(RipperEngine *engine);

	bool initialize(ResourceManager &resources);
	bool grant(uint unlockFlag, const char *source);
	Result run(const Common::String &sceneLabel, int initialUnlockFlag = -1,
		uint *usedUnlockFlag = nullptr);
	Result grantAndRun(uint unlockFlag, const Common::String &sceneLabel,
		const char *source);
	bool isAvailable(uint unlockFlag) const;

private:
	enum ChoiceResult {
		kChoiceRejected,
		kChoiceUsed,
		kChoiceFailed
	};

	ChoiceResult executeChoice(uint unlockFlag, const Common::String &sceneLabel);
	bool loadFrameSet(ResourceManager &resources, const char *prefix, uint count,
		Common::Array<BitmapAssetFrame> &frames);
	bool buildEntries(int initialUnlockFlag);
	bool captureDisplay();
	void restoreDisplay();
	void applyPalette();
	void updateLayout();
	void draw(bool usePressed = false, bool donePressed = false) const;
	void drawBitmap(byte *screen, uint pitch, const BitmapAssetFrame &bitmap,
		int x, int y) const;
	void drawFrame(byte *screen, uint pitch, const Common::Rect &bounds) const;
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const;
	void drawButton(byte *screen, uint pitch, const Common::Rect &bounds,
		const Common::String &label, bool pressed) const;
	uint measureText(const Common::String &text) const;
	Common::Rect rowBounds(uint visibleRow) const;
	int findControl(const Common::Point &point) const;
	bool moveSelection(int delta);
	void closePresentation(const char *reason);

	struct Entry {
		uint unlockFlag;
		uint bitmapIndex;
		Common::String label;
	};

	RipperEngine *_engine;
	Common::Array<Common::String> _gameText;
	Common::Array<BitmapAssetFrame> _skin;
	Common::Array<BitmapAssetFrame> _itemBitmaps;
	Common::Array<Entry> _entries;
	Common::Array<byte> _savedPixels;
	Common::Array<byte> _savedPalette;
	BitmapFontAsset _font;
	Common::Rect _menuBounds;
	Common::Rect _useBounds;
	Common::Rect _doneBounds;
	uint _selectedEntry;
	uint _firstVisibleEntry;
	uint _visibleEntries;
	bool _active;
	bool _initialized;
};

} // End of namespace Ripper

#endif // RIPPER_INVENTORY_H
