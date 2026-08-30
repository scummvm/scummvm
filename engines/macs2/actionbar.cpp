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

#include "macs2/actionbar.h"

#include "common/debug.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/savestate.h"
#include "gui/message.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/music.h"
#include "macs2/view1.h"

namespace Macs2 {

namespace {

// German source keys for macs2_translation.dat msgctxt "uilabel".
Common::String uiText(const char *source) {
	if (g_engine)
		return g_engine->translateUiLabel(source);
	return source;
}

} // namespace

const ActionBar::VerbDef ActionBar::kVerbs[4] = {
	{"Gehe", Script::MouseMode::Walk},
	{"Schaue", Script::MouseMode::Look},
	{"Benutze", Script::MouseMode::Use},
	{"Rede", Script::MouseMode::Talk}
};

ActionBar::ActionBar(View1 *view)
	: _view(view), _activeVerbIndex(0), _hoveredVerb(-1), _hoveredItemIndex(-1), _hoveredScrollButton(-1),
	  _inventoryScrollOffset(0) {
}

bool ActionBar::isPointInUI(const Common::Point &pos) const {
	if (useNativeSkin()) {
		if (g_engine->_menuMode == MenuMode::Hidden)
			return false;
		return pos.y >= (int16)g_engine->_panelTopY;
	}
	return pos.y >= kUITop;
}

void ActionBar::syncInventory() {
	rebuildProtagonistItems();
	const int maxOffset = MAX(0, (int)_protagonistItems.size() - kInvCols * kInvRows);
	if (_inventoryScrollOffset > maxOffset)
		_inventoryScrollOffset = maxOffset;
	if (_inventoryScrollOffset < 0)
		_inventoryScrollOffset = 0;
}

void ActionBar::rebuildProtagonistItems() {
	_protagonistItems.clear();
	const uint16 invScene = Scenes::instance()._currentActorIndex + 0x400;

	// Keep the engine list order, then append any inventory objects it missed
	// (pickup / moveObject can update sceneIndex before _inventoryItems).
	for (GameObject *obj : _view->_inventoryItems) {
		if (obj && obj->_sceneIndex == invScene)
			_protagonistItems.push_back(obj);
	}
	for (GameObject *obj : GameObjects::instance()._objects) {
		if (!obj || obj->_sceneIndex != invScene)
			continue;
		bool listed = false;
		for (GameObject *listedObj : _protagonistItems) {
			if (listedObj == obj) {
				listed = true;
				break;
			}
		}
		if (!listed)
			_protagonistItems.push_back(obj);
	}
}

void ActionBar::resetInventoryAfterLoad() {
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

void ActionBar::syncActiveVerbFromCursorMode() {
	const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;
	if (mode == Script::MouseMode::UseInventory) {
		_activeVerbIndex = 2;
		return;
	}

	for (int i = 0; i < ARRAYSIZE(kVerbs); i++) {
		if (kVerbs[i].mode == mode) {
			_activeVerbIndex = i;
			return;
		}
	}
}

bool ActionBar::useScummSkin() const {
	return !useNativeSkin() && g_engine->enhancementEnabled(kEnhUIUX);
}

bool ActionBar::useNativeSkin() const {
	return g_engine->hasNativeHudAssets();
}

int ActionBar::gameAreaBottomY() const {
	if (useNativeSkin())
		return (int)g_engine->_panelTopY;
	return g_engine->gameHeight();
}

void ActionBar::draw(Graphics::ManagedSurface &s) {
	if (useNativeSkin())
		drawNative(s);
	else if (useScummSkin())
		drawScumm(s);
}

void ActionBar::drawScumm(Graphics::ManagedSurface &s) {
	syncActiveVerbFromCursorMode();
	rebuildProtagonistItems();
	_view->drawBorderSide(Common::Point(0, kUITop), Common::Point(kScreenWidth, kUIHeight), s);
	drawSentenceLine(s);
	drawVerbBar(s);
	drawInventoryStrip(s);
}

void ActionBar::drawUIButton(const Common::Rect &rect, bool pressed, Graphics::ManagedSurface &s) {
	_view->drawBorderSide(Common::Point(rect.left, rect.top), Common::Point(rect.width(), rect.height()), s);
	const View1::BorderStyle &style = pressed ? View1::kBorderPressed : View1::kBorderRaised;
	_view->drawNinePatchBorder(Common::Point(rect.left, rect.top), Common::Point(rect.width(), rect.height()),
							   style, false, false, s);
}

void ActionBar::actionBarFont(const GlyphData *&font, uint16 &fontCount, int &glyphH) const {
	const bool usePanelFont = g_engine->numPanelGlyphs > 0;
	font = usePanelFont ? g_engine->_panelGlyphs : g_engine->_glyphs;
	fontCount = usePanelFont ? g_engine->numPanelGlyphs : g_engine->_numGlyphs;
	glyphH = usePanelFont ? (int)g_engine->maxPanelGlyphHeight : (int)g_engine->_maxGlyphHeight;
}

void ActionBar::drawSentenceLine(Graphics::ManagedSurface &s) {
	Common::String sentence = buildSentenceLine();
	if (sentence.empty())
		return;

	// Dialogue Font1 has German glyphs; the panel/save-load font does not.
	const GlyphData *font = g_engine->_glyphs;
	uint16 fontCount = g_engine->_numGlyphs;
	int glyphH = (int)g_engine->_maxGlyphHeight;
	if (fontCount == 0)
		actionBarFont(font, fontCount, glyphH);

	const int textY = kUITop + MAX(0, (kSentenceH - glyphH) / 2);
	const int textX = MAX(0, (kScreenWidth - _view->measureStringWithFont(sentence, font, fontCount)) / 2);
	_view->renderStringWithFontTo(textX, textY, sentence, font, fontCount, s);
}

void ActionBar::drawVerbBar(Graphics::ManagedSurface &s) {
	const GlyphData *font = nullptr;
	uint16 fontCount = 0;
	int glyphH = 0;
	actionBarFont(font, fontCount, glyphH);

	for (int i = 0; i < ARRAYSIZE(kVerbs); i++) {
		const Common::Rect r = getVerbRect(i);
		const bool isActive = (i == _activeVerbIndex);
		const bool isHovered = (i == _hoveredVerb);

		drawUIButton(r, isActive || isHovered, s);

		Common::String label = uiText(kVerbs[i].label);
		if (g_engine->numPanelGlyphs > 0)
			label.toUppercase();
		const int textW = _view->measureStringWithFont(label, font, fontCount);
		const int textX = r.left + (r.width() - textW) / 2;
		const int textY = r.top + (r.height() - glyphH) / 2;
		_view->renderStringWithFontTo(textX, textY, label, font, fontCount, s);
	}
}

int ActionBar::getScrollButtonWidth() const {
	uint16 maxW = 0;
	const Common::Array<uint16> &indices = g_engine->inventoryIconIndices;
	for (int i = 2; i <= 3 && i < (int)indices.size(); i++) {
		const int imgIdx = (int)indices[i] - 1;
		if (imgIdx >= 0 && imgIdx < (int)g_engine->_imageResources.size())
			maxW = MAX(maxW, g_engine->_imageResources[imgIdx]._width);
	}
	return MAX(22, (int)maxW + 6);
}

int ActionBar::getInvArrowX() const {
	return kBarPadX + kVerbCols * kVerbW + kVerbInvGap;
}

int ActionBar::getInvItemWidth() const {
	const int scrollW = getScrollButtonWidth();
	const int available = kScreenWidth - getInvArrowX() - kBarPadX - 2 * scrollW;
	return MAX(24, available / kInvCols);
}

void ActionBar::drawScrollButton(Graphics::ManagedSurface &s, const Common::Rect &rect,
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

void ActionBar::drawInventoryStrip(Graphics::ManagedSurface &s) {
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

Common::Array<GameObject *> ActionBar::getProtagonistItems() const {
	return _protagonistItems;
}

bool ActionBar::handleClick(const Common::Point &pos, bool scriptsRunning) {
	if (useNativeSkin())
		return handleClickNative(pos);
	if (useScummSkin())
		return handleClickScumm(pos, scriptsRunning);
	return false;
}

bool ActionBar::handleClickScumm(const Common::Point &pos, bool scriptsRunning) {
	for (int i = 0; i < ARRAYSIZE(kVerbs); i++) {
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

	const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;
	const bool takingFromContainer =
		mode == Script::MouseMode::UseInventory &&
		_view->_activeInventoryItem != nullptr &&
		_view->_uiPanelState == View1::kUiPanelContainerInventory;

	// Drop a held container item onto the protagonist strip (classic Take/Drop).
	if (takingFromContainer && isPointInInventoryStrip(pos)) {
		_view->transferInventoryItem(_view->_activeInventoryItem,
									 GameObjects::instance().getProtagonistObject());
		_view->_activeInventoryItem = nullptr;
		g_engine->_scriptExecutor->_inventoryActionFlag = true;
		g_engine->setCursorMode(Script::MouseMode::Use);
		_view->updateCursor();
		_view->setInventorySource(_view->_inventorySource);
		syncInventory();
		_view->redraw();
		return true;
	}

	const Common::Array<GameObject *> items = getProtagonistItems();
	const int maxVisible = kInvCols * kInvRows;
	for (int i = 0; i < maxVisible; i++) {
		const int itemIdx = _inventoryScrollOffset + i;
		if (itemIdx >= (int)items.size())
			break;

		if (!getInvItemRect(i).contains(pos))
			continue;

		GameObject *item = items[itemIdx];

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

void ActionBar::handleMouseMove(const Common::Point &pos) {
	if (useNativeSkin()) {
		handleMouseMoveNative(pos);
		return;
	}
	if (useScummSkin())
		handleMouseMoveScumm(pos);
}

void ActionBar::handleMouseMoveScumm(const Common::Point &pos) {
	const int oldHoveredVerb = _hoveredVerb;
	const int oldHoveredItemIndex = _hoveredItemIndex;
	const int oldHoveredScrollButton = _hoveredScrollButton;

	_hoveredVerb = -1;
	_hoveredItemIndex = -1;
	_hoveredScrollButton = -1;
	clearSentenceObject();

	for (int i = 0; i < ARRAYSIZE(kVerbs); i++) {
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
					updateSentenceLine(getObjectHotspotName(items[itemIdx]->_index));
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

void ActionBar::updateSentenceLine(const Common::String &objectName) {
	_sentenceObject = objectName;
}

void ActionBar::clearSentenceObject() {
	_sentenceObject.clear();
}

Common::Rect ActionBar::getVerbRect(int index) const {
	const int col = index % kVerbCols;
	const int row = index / kVerbCols;
	const int x = kBarPadX + col * kVerbW;
	const int y = kVerbY + row * kVerbH;
	return Common::Rect(x, y, x + kVerbW, y + kVerbH);
}

Common::Rect ActionBar::getInvItemRect(int index) const {
	const int col = index % kInvCols;
	const int row = index / kInvCols;
	const int scrollW = getScrollButtonWidth();
	const int itemW = getInvItemWidth();
	const int x = getInvArrowX() + scrollW + col * itemW;
	const int y = kVerbY + row * kInvItemH;
	return Common::Rect(x, y, x + itemW, y + kInvItemH);
}

Common::Rect ActionBar::getInvScrollLeftRect() const {
	const int scrollW = getScrollButtonWidth();
	return Common::Rect(getInvArrowX(), kVerbY, getInvArrowX() + scrollW, kVerbY + kVerbH * kVerbRows);
}

bool ActionBar::isPointInInventoryStrip(const Common::Point &pos) const {
	const Common::Rect left = getInvScrollLeftRect();
	const Common::Rect right = getInvScrollRightRect();
	const Common::Rect strip(left.left, left.top, right.right, left.bottom);
	return strip.contains(pos);
}

Common::Rect ActionBar::getInvScrollRightRect() const {
	const int scrollW = getScrollButtonWidth();
	const int x = getInvArrowX() + scrollW + kInvCols * getInvItemWidth();
	return Common::Rect(x, kVerbY, x + scrollW, kVerbY + kVerbH * kVerbRows);
}

void ActionBar::refreshSaveSlotNames() {
	g_engine->_saveSlotNames.clear();
	const uint16 visible = g_engine->_hudTextLayout[3] ? g_engine->_hudTextLayout[3] : 9;
	const uint16 start = g_engine->_saveListScroll == 0 ? 1 : g_engine->_saveListScroll;
	for (uint16 i = 0; i < visible; i++) {
		const int slot = (int)start - 1 + (int)i;
		SaveStateDescriptor desc = g_engine->getMetaEngine()->querySaveMetaInfos(
			g_engine->getGameId().c_str(), slot);
		if (desc.getSaveSlot() != -1 && !desc.getDescription().empty())
			g_engine->_saveSlotNames.push_back(desc.getDescription());
		else
			g_engine->_saveSlotNames.push_back(Common::String::format(uiText("--- Platz %d ---").c_str(), slot + 1));
	}
}

Common::String ActionBar::translatedVerbLabel(Script::MouseMode mode) const {
	if (mode == Script::MouseMode::UseInventory) {
		mode = Script::MouseMode::Use;
	}
	for (int i = 0; i < ARRAYSIZE(kVerbs); i++) {
		if (kVerbs[i].mode == mode) {
			return uiText(kVerbs[i].label);
		}
	}
	return uiText("Gehe");
}

Common::String ActionBar::currentTargetDisplayName() const {
	if (!_sentenceObject.empty())
		return _sentenceObject;

	const Common::Point mouse = g_system->getEventManager()->getMousePos();
	if (isPointInUI(mouse))
		return Common::String();

	if (_view->_uiPanelState == View1::kUiPanelContainerInventory ||
		_view->_uiPanelState == View1::kUiPanelInventory) {
		GameObject *hovered = _view->getClickedInventoryItem(mouse);
		if (hovered != nullptr)
			return getObjectHotspotName(hovered->_index);
		return Common::String();
	}

	uint16 hoverId = _view->getHitObjectID(mouse);
	if (hoverId == 0)
		hoverId = g_engine->getHotspotAtPoint(mouse);
	return lookupInteractionDisplayName(hoverId);
}

Common::String ActionBar::buildSentenceLine() const {
	const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;
	if (mode == Script::MouseMode::PanelCursor || mode == Script::MouseMode::Disabled) {
		return Common::String();
	}

	const Common::String targetName = currentTargetDisplayName();
	Common::String itemName;
	if (_view->_activeInventoryItem != nullptr) {
		itemName = getObjectHotspotName(_view->_activeInventoryItem->_index);
	}

	if (mode == Script::MouseMode::UseInventory && !itemName.empty()) {
		if (!targetName.empty())
			return Common::String::format(uiText("Benutze %s mit %s").c_str(), itemName.c_str(), targetName.c_str());
		return Common::String::format(uiText("Benutze %s").c_str(), itemName.c_str());
	}

	if (!targetName.empty()) {
		switch (mode) {
		case Script::MouseMode::Look:
			return Common::String::format(uiText("Schaue an %s").c_str(), targetName.c_str());
		case Script::MouseMode::Talk:
			return Common::String::format(uiText("Rede mit %s").c_str(), targetName.c_str());
		case Script::MouseMode::Use:
			return Common::String::format(uiText("Benutze %s").c_str(), targetName.c_str());
		default:
			return Common::String::format(uiText("Gehe zu %s").c_str(), targetName.c_str());
		}
	}

	return translatedVerbLabel(mode);
}

const HudButton *ActionBar::findHudButtonAt(const Common::Point &pos, int *outIndex) const {
	if (outIndex)
		*outIndex = -1;
	if (!isPointInUI(pos))
		return nullptr;
	const uint16 panelTop = g_engine->_panelTopY;
	const MenuMode menuMode = g_engine->_menuMode;
	for (uint i = 0; i < g_engine->_hudButtons.size(); i++) {
		const HudButton &btn = g_engine->_hudButtons[i];
		if (btn.menuId != (uint16)menuMode || btn.frame._data.empty())
			continue;
		const AnimFrame &hitFrame = btn.frame;
		const Common::Point local(pos.x - btn.x, pos.y - (int)panelTop - btn.y);
		if (local.x < 0 || local.y < 0 || local.x >= hitFrame._width || local.y >= hitFrame._height)
			continue;
		if (!hitFrame.pixelHit(local) &&
			(btn.activeFrame._data.empty() || !btn.activeFrame.pixelHit(local)) &&
			(btn.hoverFrame._data.empty() || !btn.hoverFrame.pixelHit(local)))
			continue;
		if (outIndex)
			*outIndex = (int)i;
		return &btn;
	}
	return nullptr;
}

void ActionBar::drawNative(Graphics::ManagedSurface &s) {
	if (!g_engine->hasNativeHudAssets())
		return;
	if (g_engine->_menuMode == MenuMode::Hidden)
		return;

	const uint16 panelTop = g_engine->_panelTopY;
	const MenuMode menuMode = g_engine->_menuMode;
	const int megaIndex = (int)menuMode - 1;
	if (megaIndex >= 0 && megaIndex < 6 && g_engine->_hudMegapicLoaded[megaIndex]) {
		const Graphics::ManagedSurface &mega = g_engine->_hudMegapics[megaIndex];
		s.blitFrom(mega, Common::Point(0, panelTop));
	} else {
		s.fillRect(Common::Rect(0, panelTop, s.w, panelTop + g_engine->_panelHeight), 0);
	}

	for (const HudButton &btn : g_engine->_hudButtons) {
		if (btn.menuId != (uint16)menuMode || btn.frame._data.empty())
			continue;
		const AnimFrame *frame = &btn.frame;
		const Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;
		const bool selected =
			(menuMode == MenuMode::Main &&
			 ((btn.buttonId == 1 && mode == Script::MouseMode::Walk) ||
			  (btn.buttonId == 2 && mode == Script::MouseMode::Look) ||
			  (btn.buttonId == 3 && mode == Script::MouseMode::Talk) ||
			  (btn.buttonId == 4 && (mode == Script::MouseMode::Use ||
									mode == Script::MouseMode::UseInventory)))) ||
			(menuMode == MenuMode::Options &&
			 ((btn.buttonId == 0x1e && g_engine->_optionsSubMode == OptionsSubMode::Save) ||
			  (btn.buttonId == 0x1f && g_engine->_optionsSubMode == OptionsSubMode::Load)));
		const bool pressed = (_pressedButtonId != 0 && btn.buttonId == _pressedButtonId);
		const bool hovered = (_hoveredButtonId != 0 && btn.buttonId == _hoveredButtonId);

		if ((pressed || selected) && !btn.activeFrame._data.empty())
			frame = &btn.activeFrame;
		else if (hovered && !btn.hoverFrame._data.empty())
			frame = &btn.hoverFrame;
		else if (hovered && !btn.activeFrame._data.empty())
			frame = &btn.activeFrame;

		_view->drawSprite(btn.x, panelTop + btn.y, *frame, s, false);
	}

	const uint16 optTextX = g_engine->_hudTextLayout[0];
	const uint16 optTextY = g_engine->_hudTextLayout[1];
	const uint16 optTextMaxW = g_engine->_hudTextLayout[2] ? g_engine->_hudTextLayout[2] : 212;
	const uint16 lineCount = g_engine->_hudTextLayout[3] ? g_engine->_hudTextLayout[3] : 9;
	const uint16 linePitch = g_engine->_hudTextLayout[4] ? g_engine->_hudTextLayout[4] : 10;
	const GlyphData *panelFont = g_engine->numPanelGlyphs ? g_engine->_panelGlyphs : g_engine->_glyphs;
	const uint16 panelFontCount = g_engine->numPanelGlyphs ? g_engine->numPanelGlyphs : g_engine->_numGlyphs;

	if (menuMode == MenuMode::Main) {
		if (_view->_inventorySource == nullptr ||
			_view->_inventorySource->_index != Scenes::instance()._currentActorIndex)
			_view->setInventorySource(GameObjects::instance().getProtagonistObject());
		else
			_view->setInventorySource(_view->_inventorySource);

		const uint16 cols = g_engine->_inventCols;
		const uint16 rows = g_engine->_inventRows;
		const uint16 slotW = g_engine->_inventSlotW;
		const uint16 slotH = g_engine->_inventSlotH;
		const uint16 originX = g_engine->_inventOriginX;
		const uint16 originY = g_engine->_inventOriginY;
		const uint16 scroll = g_engine->_inventScroll == 0 ? 1 : g_engine->_inventScroll;
		const uint16 slotCount = cols * rows;

		for (uint16 slot = 0; slot < slotCount; slot++) {
			const uint16 itemIndex = (uint16)(scroll - 1 + slot);
			if (itemIndex >= _view->_inventoryItems.size())
				break;
			GameObject *item = _view->_inventoryItems[itemIndex];
			if (item == nullptr)
				continue;
			AnimFrame *icon = _view->getInventoryIcon(item);
			if (icon == nullptr)
				continue;

			const uint16 col = slot % cols;
			const uint16 row = slot / cols;
			const int slotX = originX + col * slotW;
			const int slotY = panelTop + originY + row * slotH;
			const int iconX = slotX + (int)slotW / 2 - (int)icon->_width / 2;
			const int iconY = slotY + (int)slotH / 2 - (int)icon->_height / 2;
			_view->drawSprite(iconX, iconY, *icon, s, false);
			delete icon;
		}

		const GlyphData *font = g_engine->_numGlyphs ? g_engine->_glyphs : panelFont;
		const uint16 fontCount = g_engine->_numGlyphs ? g_engine->_numGlyphs : panelFontCount;
		if (fontCount != 0) {
			Common::String sentence = buildSentenceLine();
			if (!sentence.empty()) {
				const uint16 maxW = (uint16)(g_engine->screenWidth() - 16);
				while (sentence.size() > 1) {
					if ((uint16)_view->measureStringWithFont(sentence, font, fontCount) <= maxW)
						break;
					sentence.deleteLastChar();
				}
				const int textW = _view->measureStringWithFont(sentence, font, fontCount);
				const int textX = MAX(0, (g_engine->screenWidth() - textW) / 2);
				const int glyphH = g_engine->_maxGlyphHeight ? (int)g_engine->_maxGlyphHeight : 12;
				const int textY = MAX(0, (int)panelTop - glyphH - 2);
				_view->renderStringWithFontTo((uint16)textX, (uint16)textY, sentence, font, fontCount, s);
			}
		}
	} else if (menuMode == MenuMode::Options && panelFontCount != 0) {
		if (g_engine->_saveSlotNames.empty())
			refreshSaveSlotNames();
		for (uint i = 0; i < g_engine->_saveSlotNames.size() && i < lineCount; i++) {
			Common::String name = g_engine->_saveSlotNames[i];
			while (name.size() > 1) {
				if ((uint16)_view->measureStringWithFont(name, panelFont, panelFontCount) <= optTextMaxW)
					break;
				name.deleteLastChar();
			}
			_view->renderStringWithFontTo(optTextX, panelTop + optTextY + (int)i * linePitch,
										  name, panelFont, panelFontCount, s);
		}
	} else if (menuMode == MenuMode::DialogueList && panelFontCount != 0 && _view->_isDialogueChoiceInputActive) {
		// Dialogue choice list at layout[5..6]; wired when assets set DialogueList.
		const uint16 dlgX = g_engine->_hudTextLayout[5];
		const uint16 dlgY = g_engine->_hudTextLayout[6];
		const uint16 pitch = g_engine->_hudTextLayout[4] ? g_engine->_hudTextLayout[4] : 10;
		uint line = 0;
		for (uint choice = 0; choice < _view->_dialogueChoiceLineCounts.size(); choice++) {
			for (uint li = 0; li < _view->_dialogueChoiceLineCounts[choice] &&
							  line < _view->_drawnStringBox.size();
				 li++, line++) {
				_view->renderStringWithFontTo(dlgX, panelTop + dlgY + (int)line * pitch,
											  _view->_drawnStringBox[line], panelFont, panelFontCount, s);
			}
		}
	}
}

bool ActionBar::handleClickNative(const Common::Point &pos) {
	if (!isPointInUI(pos))
		return false;

	const uint16 panelTop = g_engine->_panelTopY;
	const int localY = pos.y - (int)panelTop;
	const MenuMode menuMode = g_engine->_menuMode;

	if (menuMode == MenuMode::Main) {
		const uint16 cols = g_engine->_inventCols;
		const uint16 rows = g_engine->_inventRows;
		const uint16 slotW = g_engine->_inventSlotW;
		const uint16 slotH = g_engine->_inventSlotH;
		const uint16 originX = g_engine->_inventOriginX;
		const uint16 originY = g_engine->_inventOriginY;
		const uint16 scroll = g_engine->_inventScroll == 0 ? 1 : g_engine->_inventScroll;

		if (pos.x >= (int)originX && localY >= (int)originY) {
			const int relX = pos.x - (int)originX;
			const int relY = localY - (int)originY;
			if (relX < (int)(cols * slotW) && relY < (int)(rows * slotH)) {
				const uint16 col = (uint16)(relX / slotW);
				const uint16 row = (uint16)(relY / slotH);
				const uint16 itemIndex = (uint16)(scroll - 1 + row * cols + col);
				if (itemIndex < _view->_inventoryItems.size()) {
					GameObject *item = _view->_inventoryItems[itemIndex];
					Script::MouseMode mode = g_engine->_scriptExecutor->_cursorMode;
					if (mode == Script::MouseMode::Look || mode == Script::MouseMode::Use ||
						mode == Script::MouseMode::Talk) {
						g_engine->_scriptExecutor->_interactedObjectID = item->_index;
						g_engine->runScriptExecutor(false);
						g_engine->_scriptExecutor->_interactedObjectID = 0;
					} else if (mode == Script::MouseMode::Walk || mode == Script::MouseMode::UseInventory) {
						_view->_activeInventoryItem = item;
						g_engine->_scriptExecutor->_interactedInventoryItemId = item->_index + 0x400;
						g_engine->setCursorMode(Script::MouseMode::UseInventory);
						AnimFrame *icon = _view->getInventoryIcon(item);
						if (icon != nullptr) {
							const int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
							if (cursorSlot >= 0 && cursorSlot < (int)g_engine->_imageResources.size())
								g_engine->_imageResources[cursorSlot] = *icon;
							delete icon;
						}
						_view->updateCursor();
					}
					_view->redraw();
					return true;
				}
			}
		}
	}

	if (menuMode == MenuMode::Options && g_engine->_optionsSubMode != OptionsSubMode::None) {
		const uint16 textX = g_engine->_hudTextLayout[0];
		const uint16 textY = g_engine->_hudTextLayout[1];
		const uint16 textMaxW = g_engine->_hudTextLayout[2] ? g_engine->_hudTextLayout[2] : 212;
		const uint16 lineCount = g_engine->_hudTextLayout[3] ? g_engine->_hudTextLayout[3] : 9;
		const uint16 linePitch = g_engine->_hudTextLayout[4] ? g_engine->_hudTextLayout[4] : 10;
		if (pos.x >= (int)textX && pos.x < (int)(textX + textMaxW) &&
			localY >= (int)textY && localY < (int)(textY + lineCount * linePitch)) {
			const uint16 row = (uint16)((localY - textY) / linePitch);
			const uint16 start = g_engine->_saveListScroll == 0 ? 1 : g_engine->_saveListScroll;
			const int slot = (int)start - 1 + (int)row;
			if (g_engine->_optionsSubMode == OptionsSubMode::Load) {
				g_engine->loadGameState(slot);
			} else if (g_engine->_optionsSubMode == OptionsSubMode::Save) {
				Common::String name = Common::String::format(uiText("Spielstand %d").c_str(), slot + 1);
				if (row < g_engine->_saveSlotNames.size() &&
					!g_engine->_saveSlotNames[row].empty() &&
					!g_engine->_saveSlotNames[row].hasPrefix("---"))
					name = g_engine->_saveSlotNames[row];
				g_engine->saveGameState(slot, name);
				refreshSaveSlotNames();
			}
			_view->redraw();
			return true;
		}
	}

	if (menuMode == MenuMode::DialogueList && _view->_isDialogueChoiceInputActive) {
		const uint16 dlgX = g_engine->_hudTextLayout[5];
		const uint16 dlgY = g_engine->_hudTextLayout[6];
		const uint16 pitch = g_engine->_hudTextLayout[4] ? g_engine->_hudTextLayout[4] : 10;
		uint totalLines = 0;
		for (uint n : _view->_dialogueChoiceLineCounts)
			totalLines += n;
		if (totalLines > 0 && pos.x >= (int)dlgX &&
			localY >= (int)dlgY && localY < (int)(dlgY + totalLines * pitch)) {
			const int clickedLine = (localY - (int)dlgY) / (int)pitch;
			int cumulative = 0;
			for (uint i = 0; i < _view->_dialogueChoiceLineCounts.size(); i++) {
				cumulative += (int)_view->_dialogueChoiceLineCounts[i];
				if (clickedLine < cumulative) {
					_view->_isDialogueChoiceInputActive = false;
					_view->triggerDialogueChoice((uint8)(i + 1));
					_view->redraw();
					return true;
				}
			}
		}
	}

	const HudButton *btn = findHudButtonAt(pos);
	if (btn == nullptr)
		return true;

	_pressedButtonId = btn->buttonId;
	const uint16 id = btn->buttonId;
	if (id == 1) {
		g_engine->setCursorMode(Script::MouseMode::Walk);
		_view->_activeInventoryItem = nullptr;
		g_engine->_scriptExecutor->_interactedInventoryItemId = 0;
	} else if (id == 2) {
		g_engine->setCursorMode(Script::MouseMode::Look);
	} else if (id == 3) {
		g_engine->setCursorMode(Script::MouseMode::Talk);
	} else if (id == 4) {
		g_engine->setCursorMode(Script::MouseMode::Use);
	} else if (id == 0x33) {
		g_engine->_savedMenuCursorMode = g_engine->_scriptExecutor->_cursorMode;
		g_engine->_menuMode = MenuMode::Options;
		g_engine->_optionsSubMode = OptionsSubMode::None;
		g_engine->_saveListScroll = 1;
		refreshSaveSlotNames();
		g_engine->setCursorMode(Script::MouseMode::PanelCursor);
	} else if (id == 0x32) {
		g_engine->_menuMode = MenuMode::Main;
		g_engine->_optionsSubMode = OptionsSubMode::None;
		g_engine->setCursorMode(g_engine->_savedMenuCursorMode);
	} else if (id == 0x1e) {
		g_engine->_optionsSubMode = OptionsSubMode::Save;
		refreshSaveSlotNames();
	} else if (id == 0x1f) {
		g_engine->_optionsSubMode = OptionsSubMode::Load;
		refreshSaveSlotNames();
	} else if (id == 0x20) {
		g_engine->softRestart();
		return true;
	} else if (id == 0x21) {
		::GUI::MessageDialog quitDialog(_("Quit the game?"), _("Quit"), _("Cancel"));
		if (quitDialog.runModal() == ::GUI::kMessageOK)
			Engine::quitGame();
	} else if (id == 0x14 || id == 0x16) {
		const uint16 page = (id == 0x14) ? 1 : (g_engine->_inventCols * g_engine->_inventRows);
		if (g_engine->_inventScroll > page)
			g_engine->_inventScroll = (uint16)(g_engine->_inventScroll - page);
		else
			g_engine->_inventScroll = 1;
	} else if (id == 0x15 || id == 0x17) {
		const uint16 page = (id == 0x15) ? 1 : (g_engine->_inventCols * g_engine->_inventRows);
		const uint16 maxStart = _view->_inventoryItems.empty() ? 1
			: (uint16)((_view->_inventoryItems.size() > page) ? (_view->_inventoryItems.size() - page + 1) : 1);
		uint16 next = (uint16)(g_engine->_inventScroll + page);
		if (next > maxStart)
			next = maxStart;
		if (next < 1)
			next = 1;
		g_engine->_inventScroll = next;
	} else if (id == 0x2a) {
		const uint16 page = g_engine->_hudTextLayout[3] ? g_engine->_hudTextLayout[3] : 9;
		if (g_engine->_saveListScroll > page)
			g_engine->_saveListScroll = (uint16)(g_engine->_saveListScroll - page);
		else
			g_engine->_saveListScroll = 1;
		refreshSaveSlotNames();
	} else if (id == 0x2b) {
		const uint16 page = g_engine->_hudTextLayout[3] ? g_engine->_hudTextLayout[3] : 9;
		g_engine->_saveListScroll = (uint16)(g_engine->_saveListScroll + page);
		if (g_engine->_saveListScroll > 100)
			g_engine->_saveListScroll = 100;
		refreshSaveSlotNames();
	} else if (id == 0x42) {
		g_engine->_skipSpeed = 1;
	} else if (id == 0x43) {
		g_engine->_skipSpeed = 2;
	} else if (id == 0x44) {
		g_engine->_skipSpeed = 3;
	} else if (id == 0x45) {
		g_engine->_skipSpeed = 4;
	} else if (id == 0x3c) {
		g_engine->_scriptExecutor->_musicEnabled = true;
	} else if (id == 0x3d) {
		g_engine->_scriptExecutor->_musicEnabled = false;
		g_engine->getMusic()->stopMusic();
	} else if (id == 0x3e) {
		g_engine->_scriptExecutor->_soundEnabled = true;
	} else if (id == 0x3f) {
		g_engine->_scriptExecutor->_soundEnabled = false;
		g_engine->stopSample();
	} else if (id == 0x40) {
		g_engine->_scriptExecutor->_textEnabled = true;
	} else if (id == 0x41) {
		g_engine->_scriptExecutor->_textEnabled = false;
	} else {
		debugC(1, kDebugScript, "ActionBar: unhandled button id=0x%x menu=%u", id, (uint)menuMode);
	}
	_view->updateCursor();
	_view->redraw();
	return true;
}

void ActionBar::handleMouseMoveNative(const Common::Point &pos) {
	const uint16 oldHovered = _hoveredButtonId;
	_hoveredButtonId = 0;
	_pressedButtonId = 0;
	clearSentenceObject();

	const HudButton *btn = findHudButtonAt(pos);
	if (btn != nullptr)
		_hoveredButtonId = btn->buttonId;

	if (oldHovered != _hoveredButtonId)
		_view->presentFrame();
}

} // namespace Macs2
