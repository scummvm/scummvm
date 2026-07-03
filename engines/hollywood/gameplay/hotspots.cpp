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

#include "hollywood/gameplay/hotspots.h"

#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint kSceneChunk3ColorToItemMapOffset = 0x100;
const uint kSceneColorMapSize = 0x100;
const uint kSceneVerbCount = 8;
const uint kSceneVerbActionRecordSize = 4;
const uint kSceneInventoryRelationInventoryItemCount = 0x79;
const uint kStage003SmallRowSize = 0x29;
const uint kInventoryActionCaptionCount = 9;
const byte kInventoryItemSourceKind = 1;
const byte kSceneItemSourceKind = 2;
const byte kBottomCaptionColorIndex = 0xfc;
const uint kBottomCaptionPaletteOffset = kBottomCaptionColorIndex * 3;
const int kBottomCaptionTopY = 0x1ca;
const uint32 kHoverCaptionRefreshMillis = 100;
const char *const kInventoryActionCaptions[kInventoryActionCaptionCount] = {
	" a",
	"Ir a",
	"Hablar con",
	"Coger",
	"Mirar",
	"Usar",
	"Abrir",
	"Cerrar",
	"Dar"
};

Common::String inventoryActionCaption(byte stripIndex) {
	if (stripIndex >= kInventoryActionCaptionCount)
		return Common::String();

	return Common::String(kInventoryActionCaptions[stripIndex]);
}

bool SceneHotspotTable::load(const Common::Array<byte> &paletteMapBlock, const Common::Array<byte> &metadata,
		const Common::Array<byte> &stageSmallRows) {
	if (paletteMapBlock.size() < kSceneChunk3ColorToItemMapOffset + kSceneColorMapSize) {
		warning("Scene palette map block is too short for hotspot color map");
		return false;
	}

	const uint verbRecordBytes = HollywoodEngine::kSceneItemCount * kSceneVerbCount * kSceneVerbActionRecordSize;
	if (metadata.size() < kSceneItemDefaultStrip + HollywoodEngine::kSceneItemCount ||
			metadata.size() < kSceneItemInteractionPoints + HollywoodEngine::kSceneItemCount * 4 ||
			metadata.size() < kSceneItemApproachPoints + HollywoodEngine::kSceneItemCount * 4 ||
			metadata.size() < kSceneItemFacing + HollywoodEngine::kSceneItemCount ||
			metadata.size() < kSceneVerbActionRecords + verbRecordBytes) {
		warning("Scene metadata is too short for hotspot action tables");
		return false;
	}

	_colorToItemMap.resize(kSceneColorMapSize);
	memcpy(_colorToItemMap.data(), paletteMapBlock.data() + kSceneChunk3ColorToItemMapOffset,
		_colorToItemMap.size());

	_itemDefaultStrips.resize(HollywoodEngine::kSceneItemCount);
	memcpy(_itemDefaultStrips.data(), metadata.data() + kSceneItemDefaultStrip, _itemDefaultStrips.size());

	_actionTargets.resize(HollywoodEngine::kSceneItemCount);
	for (uint item = 0; item < _actionTargets.size(); ++item) {
		uint offset = kSceneItemInteractionPoints + item * 4;
		_actionTargets[item].interactionPoint.x = readSint16LE(metadata, offset);
		_actionTargets[item].interactionPoint.y = readSint16LE(metadata, offset + 2);
		offset = kSceneItemApproachPoints + item * 4;
		_actionTargets[item].approachPoint.x = readSint16LE(metadata, offset);
		_actionTargets[item].approachPoint.y = readSint16LE(metadata, offset + 2);
		_actionTargets[item].facing = metadata[kSceneItemFacing + item];
	}

	_verbActionRecords.resize(HollywoodEngine::kSceneItemCount * kSceneVerbCount + 1);
	memset(_verbActionRecords.data(), 0, _verbActionRecords.size() * sizeof(SceneVerbActionRecord));
	for (uint record = 0; record < HollywoodEngine::kSceneItemCount * kSceneVerbCount; ++record) {
		const uint offset = kSceneVerbActionRecords + record * kSceneVerbActionRecordSize;
		_verbActionRecords[record + 1].actionHandlerId = readUint16LE(metadata, offset);
		_verbActionRecords[record + 1].movementMode = readUint16LE(metadata, offset + 2);
	}

	const uint relationRecordCount = kSceneInventoryRelationInventoryItemCount * HollywoodEngine::kSceneItemCount;
	const uint relationRecordBytes = relationRecordCount * kSceneVerbActionRecordSize;
	_relationMode1ActionRecords.clear();
	_relationMode2ActionRecords.clear();
	if (metadata.size() >= kSceneRelationRecords + relationRecordBytes) {
		_relationMode1ActionRecords.resize(relationRecordCount);
		for (uint record = 0; record < relationRecordCount; ++record) {
			const uint offset = kSceneRelationRecords + record * kSceneVerbActionRecordSize;
			_relationMode1ActionRecords[record].actionHandlerId = readUint16LE(metadata, offset);
			_relationMode1ActionRecords[record].movementMode = readUint16LE(metadata, offset + 2);
		}
	}
	if (metadata.size() >= kSceneMode2RelationOverlay + relationRecordBytes) {
		_relationMode2ActionRecords.resize(relationRecordCount);
		for (uint record = 0; record < relationRecordCount; ++record) {
			const uint offset = kSceneMode2RelationOverlay + record * kSceneVerbActionRecordSize;
			_relationMode2ActionRecords[record].actionHandlerId = readUint16LE(metadata, offset);
			_relationMode2ActionRecords[record].movementMode = readUint16LE(metadata, offset + 2);
		}
	}

	_stageSmallRows = stageSmallRows;
	return true;
}

byte SceneHotspotTable::resolveItemAt(const Graphics::Surface &savedFramebuffer, uint16 cursorX, uint16 cursorY,
		uint16 xOffset, uint16 yOffset) const {
	if (_colorToItemMap.size() != kSceneColorMapSize || savedFramebuffer.format.bytesPerPixel != 1)
		return 0;

	const uint sceneX = cursorX + xOffset;
	const uint sceneY = cursorY + yOffset;
	if (sceneX >= HollywoodEngine::kSceneBufferWidth || sceneY >= HollywoodEngine::kSceneBufferHeight ||
			sceneX >= (uint)savedFramebuffer.w || sceneY >= (uint)savedFramebuffer.h)
		return 0;

	const byte color = *(const byte *)savedFramebuffer.getBasePtr(sceneX, sceneY);
	return _colorToItemMap[color];
}

byte SceneHotspotTable::defaultStripForItem(byte itemId) const {
	if (itemId >= _itemDefaultStrips.size())
		return 0;

	return _itemDefaultStrips[itemId];
}

bool SceneHotspotTable::hasVerbAction(byte itemId, byte stripIndex) const {
	return verbActionRecord(itemId, stripIndex).actionHandlerId != 0;
}

SceneVerbActionRecord SceneHotspotTable::verbActionRecord(byte itemId, byte stripIndex) const {
	SceneVerbActionRecord emptyRecord;
	emptyRecord.actionHandlerId = 0;
	emptyRecord.movementMode = 0;

	if (itemId >= HollywoodEngine::kSceneItemCount || stripIndex == 0)
		return emptyRecord;

	if (stripIndex > kSceneVerbCount)
		return emptyRecord;

	const uint recordIndex = itemId * kSceneVerbCount + stripIndex;
	if (recordIndex >= _verbActionRecords.size())
		return emptyRecord;

	return _verbActionRecords[recordIndex];
}

SceneVerbActionRecord SceneHotspotTable::relationActionRecord(byte inventoryItemId, byte sceneItemId, byte relationMode) const {
	SceneVerbActionRecord emptyRecord;
	emptyRecord.actionHandlerId = 0;
	emptyRecord.movementMode = 0;

	if (inventoryItemId >= kSceneInventoryRelationInventoryItemCount ||
			sceneItemId >= HollywoodEngine::kSceneItemCount)
		return emptyRecord;
	if (relationMode != 1 && relationMode != 2)
		return emptyRecord;

	const Common::Array<SceneVerbActionRecord> &records =
		relationMode == 2 ? _relationMode2ActionRecords : _relationMode1ActionRecords;
	const uint recordIndex = (uint)inventoryItemId * HollywoodEngine::kSceneItemCount + sceneItemId;
	if (recordIndex >= records.size())
		return emptyRecord;

	return records[recordIndex];
}

SceneActionTarget SceneHotspotTable::actionTarget(byte itemId) const {
	SceneActionTarget emptyTarget;
	memset(&emptyTarget, 0, sizeof(emptyTarget));
	if (itemId >= _actionTargets.size())
		return emptyTarget;

	return _actionTargets[itemId];
}

void SceneHotspotTable::setActionTarget(byte itemId, const ScenePoint &interactionPoint, const ScenePoint &approachPoint) {
	if (itemId >= _actionTargets.size())
		return;

	_actionTargets[itemId].interactionPoint = interactionPoint;
	_actionTargets[itemId].approachPoint = approachPoint;
}

void SceneHotspotTable::setVerbActionHandlerByGlobalRecordIndex(uint globalRecordIndex, uint16 actionHandlerId) {
	if (globalRecordIndex < _verbActionRecords.size())
		_verbActionRecords[globalRecordIndex].actionHandlerId = actionHandlerId;
}

void SceneHotspotTable::setVerbMovementModeByGlobalRecordIndex(uint globalRecordIndex, uint16 movementMode) {
	if (globalRecordIndex < _verbActionRecords.size())
		_verbActionRecords[globalRecordIndex].movementMode = movementMode;
}

Common::String SceneHotspotTable::itemName(byte itemId) const {
	const uint offset = (uint)itemId * kStage003SmallRowSize;
	if (offset >= _stageSmallRows.size())
		return Common::String();

	const byte *row = _stageSmallRows.data() + offset;
	uint length = 0;
	while (offset + length < _stageSmallRows.size() && length < kStage003SmallRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

SceneHoverCaption::SceneHoverCaption() :
		_timer(0),
		_currentStrip(1),
		_relationMode(0),
		_primaryInventoryItem(0),
		_requestedStrip(1),
		_resolvedItem(0),
		_hasLastDescriptor(false) {
	memset(&_descriptor, 0, sizeof(_descriptor));
	memset(&_lastDescriptor, 0, sizeof(_lastDescriptor));
}

void SceneHoverCaption::reset() {
	_timer = 0;
	_currentStrip = 1;
	_relationMode = 0;
	_primaryInventoryItem = 0;
	_requestedStrip = 1;
	_resolvedItem = 0;
	_captionText.clear();
	_primaryItemName.clear();
	_hasLastDescriptor = false;
	memset(&_descriptor, 0, sizeof(_descriptor));
	memset(&_lastDescriptor, 0, sizeof(_lastDescriptor));
}

void SceneHoverCaption::setCurrentStrip(byte stripIndex) {
	_currentStrip = stripIndex;
}

void SceneHoverCaption::setRelationContext(byte relationMode, byte primaryInventoryItem,
		const Common::String &primaryItemName) {
	if (_relationMode == relationMode &&
			_primaryInventoryItem == primaryInventoryItem &&
			_primaryItemName == primaryItemName)
		return;

	_relationMode = relationMode;
	_primaryInventoryItem = primaryInventoryItem;
	_primaryItemName = primaryItemName;
	_hasLastDescriptor = false;
}

bool SceneHoverCaption::refreshNow(const SceneHotspotTable &hotspots, const Graphics::Surface &savedFramebuffer,
		uint16 cursorX, uint16 cursorY, uint16 xOffset, uint16 yOffset) {
	_timer = 0;
	return updateCaption(hotspots, savedFramebuffer, cursorX, cursorY, xOffset, yOffset, true);
}

bool SceneHoverCaption::advance(uint32 deltaMillis, const SceneHotspotTable &hotspots,
		const Graphics::Surface &savedFramebuffer, uint16 cursorX, uint16 cursorY,
		uint16 xOffset, uint16 yOffset) {
	_timer += deltaMillis;
	if (_timer < kHoverCaptionRefreshMillis)
		return false;

	while (_timer >= kHoverCaptionRefreshMillis)
		_timer -= kHoverCaptionRefreshMillis;

	return updateCaption(hotspots, savedFramebuffer, cursorX, cursorY, xOffset, yOffset, false);
}

void SceneHoverCaption::applyPalette(Common::Array<byte> &palette) const {
	if (palette.size() <= kBottomCaptionPaletteOffset + 2 || palette.size() <= 0x2d8)
		return;

	palette[kBottomCaptionPaletteOffset] = 0x32;
	palette[kBottomCaptionPaletteOffset + 1] = palette[0x2d7];
	palette[kBottomCaptionPaletteOffset + 2] = palette[0x2d8];
}

void SceneHoverCaption::draw(Graphics::Surface &surface, HollywoodFont &font) const {
	if (_captionText.empty())
		return;

	font.setShadowColor(0);
	const int textWidth = font.getStringWidth(_captionText) + 2;
	const int x = MAX<int>(0, (HollywoodEngine::kScreenWidth - textWidth) / 2);
	font.drawString(&surface, _captionText, x, kBottomCaptionTopY, textWidth, kBottomCaptionColorIndex,
		Graphics::kTextAlignLeft, 0, false, true);
}

bool SceneHoverCaption::updateCaption(const SceneHotspotTable &hotspots, const Graphics::Surface &savedFramebuffer,
		uint16 cursorX, uint16 cursorY, uint16 xOffset, uint16 yOffset, bool force) {
	Descriptor nextDescriptor;
	memset(&nextDescriptor, 0, sizeof(nextDescriptor));
	nextDescriptor.verbTextIndex = _currentStrip;
	nextDescriptor.itemSourceKind = kSceneItemSourceKind;

	if (cursorY < HollywoodEngine::kSceneBufferHeight) {
		_resolvedItem = hotspots.resolveItemAt(savedFramebuffer, cursorX, cursorY, xOffset, yOffset);
		_requestedStrip = hotspots.defaultStripForItem(_resolvedItem);
		if (_relationMode != 0 && _primaryInventoryItem != 0) {
			nextDescriptor.verbTextIndex = _relationMode == 2 ? 8 : 5;
			nextDescriptor.itemId = _primaryInventoryItem;
			nextDescriptor.itemSourceKind = kInventoryItemSourceKind;
			nextDescriptor.relationTextIndex = _relationMode;
			nextDescriptor.secondItemSourceKind = kSceneItemSourceKind;
			nextDescriptor.secondItemId =
				hotspots.relationActionRecord(_primaryInventoryItem, _resolvedItem,
					_relationMode).actionHandlerId != 0 ? _resolvedItem : 0;
		} else {
			nextDescriptor.itemId = hotspots.hasVerbAction(_resolvedItem, _currentStrip) ? _resolvedItem : 0;
		}
	} else {
		_resolvedItem = 0;
		_requestedStrip = 0;
		if (_relationMode != 0 && _primaryInventoryItem != 0) {
			nextDescriptor.verbTextIndex = _relationMode == 2 ? 8 : 5;
			nextDescriptor.itemId = _primaryInventoryItem;
			nextDescriptor.itemSourceKind = kInventoryItemSourceKind;
			nextDescriptor.relationTextIndex = _relationMode;
			nextDescriptor.secondItemSourceKind = kSceneItemSourceKind;
		} else {
			nextDescriptor.itemId = 0;
		}
	}

	if (!force && !descriptorChanged(nextDescriptor))
		return false;

	_descriptor = nextDescriptor;
	_lastDescriptor = nextDescriptor;
	_hasLastDescriptor = true;
	_captionText = buildCaption(hotspots, nextDescriptor);
	return true;
}

bool SceneHoverCaption::descriptorChanged(const Descriptor &descriptor) const {
	if (!_hasLastDescriptor)
		return true;

	return descriptor.verbTextIndex != _lastDescriptor.verbTextIndex ||
		descriptor.itemId != _lastDescriptor.itemId ||
		descriptor.itemSourceKind != _lastDescriptor.itemSourceKind ||
		descriptor.relationTextIndex != _lastDescriptor.relationTextIndex ||
		descriptor.secondItemId != _lastDescriptor.secondItemId ||
		descriptor.secondItemSourceKind != _lastDescriptor.secondItemSourceKind;
}

Common::String SceneHoverCaption::buildCaption(const SceneHotspotTable &hotspots,
		const Descriptor &descriptor) const {
	Common::String caption = actionCaption(descriptor.verbTextIndex);
	if (descriptor.relationTextIndex != 0) {
		caption += _primaryItemName;
		caption += descriptor.relationTextIndex == 2 ? " a " : " con ";
		if (descriptor.secondItemId != 0 && descriptor.secondItemSourceKind == kSceneItemSourceKind)
			caption += hotspots.itemName(descriptor.secondItemId);
		return caption;
	}

	if (descriptor.itemSourceKind == kSceneItemSourceKind)
		caption += hotspots.itemName(descriptor.itemId);

	return caption;
}

Common::String SceneHoverCaption::actionCaption(byte stripIndex) const {
	return inventoryActionCaption(stripIndex);
}

} // End of namespace Hollywood
