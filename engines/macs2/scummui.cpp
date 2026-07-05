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

#include "macs2/scummui.h"

#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/view1.h"

namespace Macs2 {

static Common::String getObjectDisplayName(const GameObject *obj) {
	if (!obj)
		return Common::String();

	const GameObjects &objects = GameObjects::instance();
	if (obj->_index < objects._objectNames.size() && !objects._objectNames[obj->_index].empty())
		return objects._objectNames[obj->_index];

	return Common::String();
}

const ScummUI::VerbDef ScummUI::kVerbs[4] = {
	{"Walk", Script::MouseMode::Walk},
	{"Look", Script::MouseMode::Look},
	{"Use", Script::MouseMode::Use},
	{"Talk", Script::MouseMode::Talk}
};

ScummUI::ScummUI(View1 *view)
	: _view(view), _activeVerbIndex(0), _hoveredVerb(-1), _hoveredItemIndex(-1), _hoveredScrollButton(-1),
	  _inventoryScrollOffset(0) {
}

bool ScummUI::isPointInUI(const Common::Point &pos) const {
	return pos.y >= kUITop;
}

void ScummUI::syncInventory() {
	rebuildProtagonistItems();
	const int maxOffset = MAX(0, (int)_protagonistItems.size() - kInvCols * kInvRows);
	if (_inventoryScrollOffset > maxOffset)
		_inventoryScrollOffset = maxOffset;
	if (_inventoryScrollOffset < 0)
		_inventoryScrollOffset = 0;
}

void ScummUI::rebuildProtagonistItems() {
	_protagonistItems.clear();

	if (_view->isInventorySourceProtagonist()) {
		_protagonistItems = _view->_inventoryItems;
		return;
	}

	const uint16 invScene = Scenes::instance()._currentActorIndex + 0x400;
	for (GameObject *obj : GameObjects::instance()._objects) {
		if (obj && obj->_sceneIndex == invScene)
			_protagonistItems.push_back(obj);
	}
}

void ScummUI::resetInventoryAfterLoad() {
	_inventoryScrollOffset = 0;
	_hoveredItemIndex = -1;
	_hoveredScrollButton = -1;
	_hoveredVerb = -1;
	_sentenceObject.clear();
	rebuildProtagonistItems();
	syncActiveVerbFromCursorMode();

	if (_view->_activeInventoryItem) {
		bool inInventory = false;
		for (GameObject *obj : _protagonistItems) {
			if (obj == _view->_activeInventoryItem) {
				inInventory = true;
				break;
			}
		}
		if (!inInventory) {
			_view->_activeInventoryItem = nullptr;
			g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
		}
	}
}

void ScummUI::syncActiveVerbFromCursorMode() {
	const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;
	if (mode == Script::MouseMode::UseInventory) {
		_activeVerbIndex = 2;
		return;
	}

	for (int i = 0; i < 4; i++) {
		if (kVerbs[i].mode == mode) {
			_activeVerbIndex = i;
			return;
		}
	}
}

void ScummUI::draw(Graphics::ManagedSurface &s) {
	syncActiveVerbFromCursorMode();
	rebuildProtagonistItems();
	_view->drawBorderSide(Common::Point(0, kUITop), Common::Point(kScreenWidth, kUIHeight), s);
	drawSentenceLine(s);
	drawVerbBar(s);
	drawInventoryStrip(s);
}

void ScummUI::drawUIButton(const Common::Rect &rect, bool pressed, Graphics::ManagedSurface &s) {
	_view->drawBorderSide(Common::Point(rect.left, rect.top), Common::Point(rect.width(), rect.height()), s);
	const View1::BorderStyle &style = pressed ? View1::kBorderPressed : View1::kBorderRaised;
	_view->drawNinePatchBorder(Common::Point(rect.left, rect.top), Common::Point(rect.width(), rect.height()),
							   style, false, false, s);
}

void ScummUI::drawSentenceLine(Graphics::ManagedSurface &s) {
	Common::String sentence;
	const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;

	if (mode == Script::MouseMode::UseInventory && _view->_activeInventoryItem) {
		sentence = "Use";
		const Common::String itemName = getObjectDisplayName(_view->_activeInventoryItem);
		if (!itemName.empty())
			sentence += " " + itemName;
	} else if (_activeVerbIndex >= 0 && _activeVerbIndex < 4) {
		sentence = kVerbs[_activeVerbIndex].label;
	}
	if (!_sentenceObject.empty()) {
		if (!sentence.empty()) {
			if (mode == Script::MouseMode::UseInventory)
				sentence += " with ";
			else
				sentence += " ";
		}
		sentence += _sentenceObject;
	}

	if (sentence.empty())
		return;

	const bool usePanelFont = g_engine->numPanelGlyphs > 0;
	const GlyphData *font = usePanelFont ? g_engine->_panelGlyphs : g_engine->_glyphs;
	const uint16 fontCount = usePanelFont ? g_engine->numPanelGlyphs : g_engine->numGlyphs;
	const int glyphH = usePanelFont ? g_engine->maxPanelGlyphHeight : g_engine->maxGlyphHeight;
	if (usePanelFont)
		sentence.toUppercase();
	const int textY = kUITop + MAX(0, (kSentenceH - glyphH) / 2);
	const int textX = MAX(0, (kScreenWidth - _view->measureStringWithFont(sentence, font, fontCount)) / 2);
	_view->renderStringWithFontTo(textX, textY, sentence, font, fontCount, s);
}

void ScummUI::drawVerbBar(Graphics::ManagedSurface &s) {
	for (int i = 0; i < 4; i++) {
		const Common::Rect r = getVerbRect(i);
		const bool isActive = (i == _activeVerbIndex);
		const bool isHovered = (i == _hoveredVerb);

		drawUIButton(r, isActive || isHovered, s);

		const int textX = r.left + (r.width() - (int)strlen(kVerbs[i].label) * 6) / 2;
		const int textY = r.top + (r.height() - (int)g_engine->maxGlyphHeight) / 2;
		_view->renderStringTo(textX, textY, kVerbs[i].label, s);
	}
}

int ScummUI::getScrollButtonWidth() const {
	uint16 maxW = 0;
	const Common::Array<uint16> &indices = g_engine->inventoryIconIndices;
	for (int i = 2; i <= 3 && i < (int)indices.size(); i++) {
		const int imgIdx = (int)indices[i] - 1;
		if (imgIdx >= 0 && imgIdx < (int)g_engine->_imageResources.size())
			maxW = MAX(maxW, g_engine->_imageResources[imgIdx]._width);
	}
	return MAX(22, (int)maxW + 6);
}

int ScummUI::getInvArrowX() const {
	return kInvX;
}

void ScummUI::drawScrollButton(Graphics::ManagedSurface &s, const Common::Rect &rect,
							   int iconResourceIndex, bool hovered) {
	drawUIButton(rect, hovered, s);

	if (iconResourceIndex < 0 || iconResourceIndex >= (int)g_engine->_imageResources.size())
		return;

	const AnimFrame &frame = g_engine->_imageResources[iconResourceIndex];
	if (frame._data.empty() || frame._width == 0 || frame._height == 0)
		return;

	int iconX = rect.left + (rect.width() - frame._width) / 2;
	int iconY = rect.top + (rect.height() - frame._height) / 2;
	if (hovered) {
		iconX++;
		iconY++;
	}
	_view->drawSprite(iconX, iconY, frame, s, false);
}

void ScummUI::drawInventoryStrip(Graphics::ManagedSurface &s) {
	const Common::Array<uint16> &indices = g_engine->inventoryIconIndices;
	const int upIconIdx = (indices.size() > 2) ? (int)indices[2] - 1 : -1;
	const int downIconIdx = (indices.size() > 3) ? (int)indices[3] - 1 : -1;

	drawScrollButton(s, getInvScrollLeftRect(), upIconIdx, _hoveredScrollButton == 0);
	drawScrollButton(s, getInvScrollRightRect(), downIconIdx, _hoveredScrollButton == 1);

	const Common::Array<GameObject *> items = getProtagonistItems();
	const int maxVisible = kInvCols * kInvRows;
	for (int i = 0; i < maxVisible; i++) {
		const Common::Rect r = getInvItemRect(i);
		drawUIButton(r, true, s);

		const int itemIdx = _inventoryScrollOffset + i;
		if (itemIdx >= (int)items.size())
			continue;

		const bool isHovered = (i == _hoveredItemIndex);
		const bool isActive = (_view->_activeInventoryItem == items[itemIdx]);

		AnimFrame *icon = _view->getInventoryIcon(items[itemIdx]);
		if (icon && !icon->_data.empty()) {
			_view->drawSpriteFitted(r, *icon, s, kInvIconInset);
		}
		delete icon;

		if (isActive) {
			_view->renderStringTo(r.left + 1, r.top, "*", s);
		} else if (isHovered) {
			_view->renderStringTo(r.left + 1, r.top, ".", s);
		}
	}
}

Common::Array<GameObject *> ScummUI::getProtagonistItems() const {
	return _protagonistItems;
}

bool ScummUI::handleClick(const Common::Point &pos, bool scriptsRunning) {
	for (int i = 0; i < 4; i++) {
		if (getVerbRect(i).contains(pos)) {
			_activeVerbIndex = i;
			g_engine->setCursorMode(kVerbs[i].mode);
			_view->_activeInventoryItem = nullptr;
			g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
			_view->updateCursor();
			return true;
		}
	}

	if (getInvScrollLeftRect().contains(pos)) {
		if (_inventoryScrollOffset > 0)
			_inventoryScrollOffset -= kInvCols * kInvRows;
		return true;
	}
	if (getInvScrollRightRect().contains(pos)) {
		const int maxItems = (int)getProtagonistItems().size();
		const int maxVisible = kInvCols * kInvRows;
		if (_inventoryScrollOffset + maxVisible < maxItems)
			_inventoryScrollOffset += maxVisible;
		return true;
	}

	if (scriptsRunning)
		return true;

	const Common::Array<GameObject *> items = getProtagonistItems();
	const int maxVisible = kInvCols * kInvRows;
	for (int i = 0; i < maxVisible; i++) {
		const int itemIdx = _inventoryScrollOffset + i;
		if (itemIdx >= (int)items.size())
			break;

		if (!getInvItemRect(i).contains(pos))
			continue;

		GameObject *item = items[itemIdx];
		const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;

		if (mode == Script::MouseMode::Look) {
			g_engine->_scriptExecutor->_interactedObjectID = 0x400 + item->_index;
			g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
			_view->_pendingPanelRequest = View1::kPanelRequestInventory;
			g_engine->runScriptExecutor(false);
			_view->_pendingPanelRequest = View1::kPanelRequestNone;
		} else if (mode == Script::MouseMode::Use) {
			_view->_activeInventoryItem = item;
			g_engine->_scriptExecutor->_interactedInventoryItemId = 0x400 + item->_index;
			AnimFrame *icon = _view->getInventoryIcon(item);
			if (icon != nullptr) {
				const int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
				g_engine->_imageResources[cursorSlot] = *icon;
				delete icon;
			}
			g_engine->setCursorMode(Script::MouseMode::UseInventory);
			_view->updateCursor();
		} else if (mode == Script::MouseMode::UseInventory && _view->_activeInventoryItem) {
			g_engine->_scriptExecutor->_interactedObjectID = 0x400 + _view->_activeInventoryItem->_index;
			g_engine->_scriptExecutor->_interactedInventoryItemId = 0x400 + item->_index;
			_view->_pendingPanelRequest = View1::kPanelRequestInventory;
			g_engine->runScriptExecutor(false);
			_view->_pendingPanelRequest = View1::kPanelRequestNone;
			_view->_activeInventoryItem = nullptr;
			g_engine->setCursorMode(Script::MouseMode::Use);
			_view->updateCursor();
			syncInventory();
		}
		return true;
	}

	return true;
}

void ScummUI::handleMouseMove(const Common::Point &pos) {
	const int oldHoveredVerb = _hoveredVerb;
	const int oldHoveredItemIndex = _hoveredItemIndex;
	const int oldHoveredScrollButton = _hoveredScrollButton;

	_hoveredVerb = -1;
	_hoveredItemIndex = -1;
	_hoveredScrollButton = -1;
	clearSentenceObject();

	for (int i = 0; i < 4; i++) {
		if (getVerbRect(i).contains(pos)) {
			_hoveredVerb = i;
			break;
		}
	}

	if (_hoveredVerb < 0) {
		if (getInvScrollLeftRect().contains(pos)) {
			_hoveredScrollButton = 0;
		} else if (getInvScrollRightRect().contains(pos)) {
			_hoveredScrollButton = 1;
		} else {
			const Common::Array<GameObject *> items = getProtagonistItems();
			const int maxVisible = kInvCols * kInvRows;
			for (int i = 0; i < maxVisible; i++) {
				const int itemIdx = _inventoryScrollOffset + i;
				if (itemIdx >= (int)items.size())
					break;
				if (getInvItemRect(i).contains(pos)) {
					_hoveredItemIndex = i;
					break;
				}
			}
		}
	}

	if (oldHoveredVerb != _hoveredVerb || oldHoveredItemIndex != _hoveredItemIndex ||
		oldHoveredScrollButton != _hoveredScrollButton) {
		_view->presentFrame();
	}
}

void ScummUI::updateSentenceLine(const Common::String &objectName) {
	_sentenceObject = objectName;
}

void ScummUI::clearSentenceObject() {
	_sentenceObject.clear();
}

Common::Rect ScummUI::getVerbRect(int index) const {
	const int col = index % kVerbCols;
	const int row = index / kVerbCols;
	const int x = col * kVerbW;
	const int y = kVerbY + row * kVerbH;
	return Common::Rect(x, y, x + kVerbW, y + kVerbH);
}

Common::Rect ScummUI::getInvItemRect(int index) const {
	const int col = index % kInvCols;
	const int row = index / kInvCols;
	const int scrollW = getScrollButtonWidth();
	const int x = getInvArrowX() + scrollW + col * kInvItemW;
	const int y = kVerbY + row * kInvItemH;
	return Common::Rect(x, y, x + kInvItemW, y + kInvItemH);
}

Common::Rect ScummUI::getInvScrollLeftRect() const {
	const int scrollW = getScrollButtonWidth();
	return Common::Rect(getInvArrowX(), kVerbY, getInvArrowX() + scrollW, kVerbY + kVerbH * kVerbRows);
}

Common::Rect ScummUI::getInvScrollRightRect() const {
	const int scrollW = getScrollButtonWidth();
	const int x = getInvArrowX() + scrollW + kInvCols * kInvItemW;
	return Common::Rect(x, kVerbY, x + scrollW, kVerbY + kVerbH * kVerbRows);
}

} // namespace Macs2
