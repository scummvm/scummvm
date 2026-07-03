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

#ifndef HOLLYWOOD_GAMEPLAY_HOTSPOTS_H
#define HOLLYWOOD_GAMEPLAY_HOTSPOTS_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

#include "hollywood/resource.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodFont;

Common::String inventoryActionCaption(byte stripIndex);

struct SceneVerbActionRecord {
	uint16 actionHandlerId;
	uint16 movementMode;
};

struct SceneActionTarget {
	ScenePoint interactionPoint;
	ScenePoint approachPoint;
	byte facing;
};

class SceneHotspotTable {
public:
	bool load(const Common::Array<byte> &paletteMapBlock, const Common::Array<byte> &metadata,
		const Common::Array<byte> &stageSmallRows);

	byte resolveItemAt(const Graphics::Surface &savedFramebuffer, uint16 cursorX, uint16 cursorY,
		uint16 xOffset, uint16 yOffset) const;
	byte defaultStripForItem(byte itemId) const;
	bool hasVerbAction(byte itemId, byte stripIndex) const;
	SceneVerbActionRecord verbActionRecord(byte itemId, byte stripIndex) const;
	SceneVerbActionRecord relationActionRecord(byte inventoryItemId, byte sceneItemId, byte relationMode) const;
	SceneActionTarget actionTarget(byte itemId) const;
	void setActionTarget(byte itemId, const ScenePoint &interactionPoint, const ScenePoint &approachPoint);
	void setVerbActionHandlerByGlobalRecordIndex(uint globalRecordIndex, uint16 actionHandlerId);
	void setVerbMovementModeByGlobalRecordIndex(uint globalRecordIndex, uint16 movementMode);
	Common::String itemName(byte itemId) const;

private:
	Common::Array<byte> _colorToItemMap;
	Common::Array<byte> _itemDefaultStrips;
	Common::Array<SceneActionTarget> _actionTargets;
	Common::Array<byte> _stageSmallRows;
	Common::Array<SceneVerbActionRecord> _verbActionRecords;
	Common::Array<SceneVerbActionRecord> _relationMode1ActionRecords;
	Common::Array<SceneVerbActionRecord> _relationMode2ActionRecords;
};

class SceneHoverCaption {
public:
	SceneHoverCaption();

	void reset();
	void setCurrentStrip(byte stripIndex);
	void setRelationContext(byte relationMode, byte primaryInventoryItem, const Common::String &primaryItemName);
	bool refreshNow(const SceneHotspotTable &hotspots, const Graphics::Surface &savedFramebuffer,
		uint16 cursorX, uint16 cursorY, uint16 xOffset, uint16 yOffset);
	bool advance(uint32 deltaMillis, const SceneHotspotTable &hotspots,
		const Graphics::Surface &savedFramebuffer, uint16 cursorX, uint16 cursorY,
		uint16 xOffset, uint16 yOffset);

	void applyPalette(Common::Array<byte> &palette) const;
	void draw(Graphics::Surface &surface, HollywoodFont &font) const;

	byte currentStrip() const { return _currentStrip; }
	byte requestedStrip() const { return _requestedStrip; }
	byte resolvedItem() const { return _resolvedItem; }
	const Common::String &text() const { return _captionText; }

private:
	struct Descriptor {
		byte verbTextIndex;
		byte itemId;
		byte itemSourceKind;
		byte relationTextIndex;
		byte secondItemId;
		byte secondItemSourceKind;
	};

	bool updateCaption(const SceneHotspotTable &hotspots, const Graphics::Surface &savedFramebuffer,
		uint16 cursorX, uint16 cursorY, uint16 xOffset, uint16 yOffset, bool force);
	bool descriptorChanged(const Descriptor &descriptor) const;
	Common::String buildCaption(const SceneHotspotTable &hotspots, const Descriptor &descriptor) const;
	Common::String actionCaption(byte stripIndex) const;

	Descriptor _descriptor;
	Descriptor _lastDescriptor;
	Common::String _captionText;
	uint32 _timer;
	byte _currentStrip;
	byte _relationMode;
	byte _primaryInventoryItem;
	Common::String _primaryItemName;
	byte _requestedStrip;
	byte _resolvedItem;
	bool _hasLastDescriptor;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_HOTSPOTS_H
