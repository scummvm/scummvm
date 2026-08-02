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

#ifndef MACS2_ACTIONBAR_H
#define MACS2_ACTIONBAR_H

#include "common/rect.h"
#include "common/str.h"
#include "graphics/managed_surface.h"
#include "macs2/macs2_constants.h"
#include "macs2/scriptexecutor.h"

namespace Macs2 {

class View1;
class GameObject;
struct AnimFrame;
struct HudButton;

/**
 * Persistent bottom action bar: Scumm procedural strip (kEnhUIUX) or native
 * megapic/button HUD when dialect-v2 panel assets are loaded.
 */
class ActionBar {
public:
	ActionBar(View1 *view);

	void draw(Graphics::ManagedSurface &s);
	bool handleClick(const Common::Point &pos, bool scriptsRunning = false);
	void handleMouseMove(const Common::Point &pos);
	bool isPointInUI(const Common::Point &pos) const;
	void updateSentenceLine(const Common::String &objectName);
	void clearSentenceObject();
	void syncInventory();
	void syncActiveVerbFromCursorMode();
	void resetInventoryAfterLoad();

	bool useScummSkin() const;
	bool useNativeSkin() const;
	/** Y where the interactive game area ends when this bar is shown. */
	int gameAreaBottomY() const;

private:
	static constexpr int kSentenceH = 14;
	static constexpr int kUITop = kGameHeight;
	static constexpr int kSentenceY = kGameHeight;
	static constexpr int kVerbY = kGameHeight + kSentenceH;
	static constexpr int kVerbW = 64;
	static constexpr int kVerbH = 25;
	static constexpr int kVerbCols = 2;
	static constexpr int kVerbRows = 2;
	static constexpr int kInvX = 128;
	static constexpr int kInvItemW = 34;
	static constexpr int kInvItemH = 25;
	static constexpr int kInvIconInset = 1;
	static constexpr int kInvCols = 4;
	static constexpr int kInvRows = 2;

	struct VerbDef {
		const char *label;
		Script::MouseMode mode;
	};
	static const VerbDef kVerbs[4];

	// --- Scumm skin ---
	void drawScumm(Graphics::ManagedSurface &s);
	bool handleClickScumm(const Common::Point &pos, bool scriptsRunning);
	void handleMouseMoveScumm(const Common::Point &pos);
	void drawSentenceLine(Graphics::ManagedSurface &s);
	void drawVerbBar(Graphics::ManagedSurface &s);
	void drawInventoryStrip(Graphics::ManagedSurface &s);
	void drawScrollButton(Graphics::ManagedSurface &s, const Common::Rect &rect, int iconResourceIndex, bool hovered);
	void drawUIButton(const Common::Rect &rect, bool pressed, Graphics::ManagedSurface &s);
	void rebuildProtagonistItems();

	int getScrollButtonWidth() const;
	int getInvArrowX() const;

	Common::Array<GameObject *> getProtagonistItems() const;

	Common::Rect getVerbRect(int index) const;
	Common::Rect getInvItemRect(int index) const;
	Common::Rect getInvScrollLeftRect() const;
	Common::Rect getInvScrollRightRect() const;
	/** True if pos hits the inventory scroll/item area of the strip. */
	bool isPointInInventoryStrip(const Common::Point &pos) const;

	// --- Native skin (megapic + HudButton table) ---
	void drawNative(Graphics::ManagedSurface &s);
	bool handleClickNative(const Common::Point &pos);
	void handleMouseMoveNative(const Common::Point &pos);
	void refreshSaveSlotNames();
	Common::String buildNativeSentenceLine() const;
	const HudButton *findHudButtonAt(const Common::Point &pos, int *outIndex = nullptr) const;

	View1 *_view;
	int _activeVerbIndex;
	int _hoveredVerb;
	int _hoveredItemIndex;
	int _hoveredScrollButton;
	int _inventoryScrollOffset;
	Common::Array<GameObject *> _protagonistItems;
	Common::String _sentenceObject;

	uint16 _hoveredButtonId = 0;
	uint16 _pressedButtonId = 0;
};

} // namespace Macs2

#endif // MACS2_ACTIONBAR_H
