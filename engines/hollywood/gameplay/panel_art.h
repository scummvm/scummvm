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

#ifndef HOLLYWOOD_GAMEPLAY_PANEL_ART_H
#define HOLLYWOOD_GAMEPLAY_PANEL_ART_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/gameplay/game_loop.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodFont;
struct GameplayState;

class GameplayPanelArt {
public:
	GameplayPanelArt();

	bool load();
	bool isLoaded() const { return _loaded; }

	bool applyPalette(Common::Array<byte> &palette) const;
	bool applyInteractiveObjectPalette(Common::Array<byte> &palette) const;
	void drawVerbPanel(Graphics::Surface &surface, const Graphics::Surface &savedFramebuffer,
		uint16 viewportXOffset, uint16 viewportYOffset, const GameplayPanelState &panelState,
		HollywoodFont *font) const;
	void drawDialogueInventoryPanel(Graphics::Surface &surface, const Graphics::Surface &savedFramebuffer,
		uint16 viewportXOffset, uint16 viewportYOffset, const GameplayPanelState &panelState,
		const GameplayState &gameState, HollywoodFont *font) const;
	void drawDialogueMenuPanel(Graphics::Surface &surface, const DialogueMenuState &menuState,
		HollywoodFont *font) const;

private:
	bool loadBottomPanelBuffer();
	bool loadDialogueMenuPanelBuffer();
	bool loadDialogueMenuPanelBufferFromResource000();
	bool loadObjectPalette();
	bool loadObjectPaletteFromResource000();
	bool loadInventoryItemTilePage(byte pageIndex, Common::Array<byte> &page) const;
	void copySavedCaptionBand(Graphics::Surface &surface, const Graphics::Surface &savedFramebuffer,
		uint16 viewportXOffset, uint16 viewportYOffset, uint16 screenY) const;
	void copyBottomPanelRows(Graphics::Surface &surface, uint16 sourceRow, uint16 screenY,
		uint16 rowCount) const;
	void drawInventoryItems(Graphics::Surface &surface, const GameplayState &gameState) const;
	void drawDialogueMenuRows(Graphics::Surface &surface, const DialogueMenuState &menuState,
		uint16 screenY, HollywoodFont *font) const;
	void drawCaptionText(Graphics::Surface &surface, const Common::String &text, int y,
		HollywoodFont *font) const;
	void drawVerbStripLabels(Graphics::Surface &surface, int screenY, HollywoodFont *font) const;
	void applySelectedVerbStrip(Graphics::Surface &surface, int screenY, byte stripIndex) const;
	byte sueInventoryItemPage(byte itemId) const;

	Common::Array<byte> _bottomPanelBuffer;
	Common::Array<byte> _dialogueMenuPanelBuffer;
	Common::Array<byte> _objectPaletteTriples;
	mutable Common::Array<Common::Array<byte> > _inventoryItemTilePages;
	uint32 _inventoryItemPageBaseOffset;
	bool _loaded;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_PANEL_ART_H
