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

#ifndef SCOOBY_MAIN_MENU_PRESENTATION_H
#define SCOOBY_MAIN_MENU_PRESENTATION_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"
#include "common/str.h"

#include "animation/menu_animation_state.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/common/span.h"
#include "scooby/graphics/palette_color.h"
#include "scooby/graphics/sprite_instance.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"

// Owns decoded assets and logical scene work retained from Ghidra RunMainMenu at 0x00008196,
// excluding its hardware-only transfer mechanisms.

namespace Scooby {

class MainMenuPresentation {
public:
	// Binds menu assets and composition to the shared logical scene and retrace owner.
	// rom: verified cartridge address space containing menu assets.
	// scene: logical scene that receives decoded tile and layer content.
	// presenter: shared palette-fade and retrace implementation.
	MainMenuPresentation(const ScoobyDooRom &rom, TileScene &scene,
						 FramePresenter &presenter)
		: _rom(rom), _scene(scene), _presenter(presenter) {
	}

	// Loads the initial asset range at 0x000082A2-0x000083D8 in Ghidra RunMainMenu at 0x00008196,
	// beginning from its proven blank H32 scene.
	// Returns the first tile index after the initial ring-LZ asset, retained from Ghidra
	// g_wSharedMenuTileIndexOrSpriteScratch0008 at 0xFF0008.
	//
	// The ring-LZ stream fills tiles 0x000-0x5C7; byte-RLE streams at 0x00016D38, 0x00016E6A, and
	// 0x00016F76 fill 0x5C8-0x5F7; the generated binary mask fills 0x6A0-0x6FE; and the original
	// 0xAAAA fill becomes solid tiles 0x7D0-0x7D3. ROM 0x000165B8-0x00016CB7 is a complete 32x28
	// plane-B map, while 0x000085F8-0x0000865F is a linked 12-sprite layout plus its terminator.
	int loadInitialAssets();

	// Restores the menu's complete two-color binary-mask pattern range.
	//
	// Ghidra RunMainMenu (0x00008196) and RunRoomObjectRelocationMenu (0x00008C40) call
	// GenerateBinaryMaskTiles with palette indices 15 and 0. The original destination formula adds
	// 0x20 to the low 11 bits of g_wInterfaceTileAttributes; menu value 0x0680 therefore selects
	// logical tiles 0x6A0-0x6FE. Direct pattern publication replaces its VDP word writes.
	void restoreBinaryMaskTiles();

	// Reimplements the call to Ghidra FadePaletteIn at 0x00009E7E using the menu's assembled base
	// and overlay ranges.
	// beforeRetrace: installed main-menu callback invoked once before each faded frame.
	// Returns false when the host requests shutdown during the fade.
	bool fadePaletteIn(const Common::Functor0<void> &beforeRetrace);

	// Stages the current output from Ghidra DecompressByteRle at 0x00009C64 for a complete
	// compressed menu plane.
	// sourceOffset: ROM address of the selected compressed range.
	void stageByteRleData(int sourceOffset);

	// Stages Ghidra ComposeWideMenuRevealTiles at 0x0000E40C with its independent masks and current
	// row-wave phase.
	// rowMaskOffset: applied value from original word 0xFF0626.
	// columnMaskOffset: applied value from original word 0xFF0624.
	void composeWideMenuRevealTiles(int16 rowMaskOffset, int16 columnMaskOffset);

	// Stages Ghidra ComposeNarrowMenuRevealTiles at 0x0000F272 with its independent masks and
	// current row-wave phase.
	// rowMaskOffset: applied value from original word 0xFF0626.
	// columnMaskOffset: applied value from original word 0xFF0624.
	void composeNarrowMenuRevealTiles(int16 rowMaskOffset, int16 columnMaskOffset);

	// Gets the row-mask value most recently copied by Ghidra AdvanceMenuAnimation.
	int16 animatedRowMaskOffset() const { return _menuAnimation.animatedRowMaskOffset(); }

	// Advances Ghidra AdvanceMenuAnimation once from the installed menu callback.
	// copySpritePhaseToRowMask: whether initialization flag bit 4 publishes the current triangle
	// phase as the row mask.
	void advanceMenuAnimation(bool copySpritePhaseToRowMask) {
		_menuAnimation.advance(copySpritePhaseToRowMask);
	}

	// Replaces palette line two from the source selected by Ghidra HandleMainMenuVBlank.
	// sourceOffset: ROM address of the 16-color lightning palette.
	void loadLightningPalette(int sourceOffset);

	// Applies one contiguous authored lightning patch to its plane-B rectangle.
	// sourceOffset: ROM address of the packed patch cells.
	// column: destination plane-B column.
	// row: destination plane-B row.
	// width: patch width in cells.
	// height: patch height in cells.
	void applyLightningPatch(int sourceOffset, int column, int row, int width, int height);

	// Restores a completed lightning rectangle from the immutable 32x28 plane-B map.
	// column: destination and source-map column.
	// row: destination and source-map row.
	// width: rectangle width in cells.
	// height: rectangle height in cells.
	void restoreLightningPatch(int column, int row, int width, int height);

	// Collapses the caller-owned DMA operations at 0x000084CA-0x000084F2 into one logical
	// reveal-scene commit.
	// includeNarrowTiles: whether the wide reveal has reached the narrow transfer phase.
	void commitRevealSceneBuffers(bool includeNarrowTiles);

	// Applies the current phase to all twelve menu sprites as recovered from Ghidra
	// ApplyMenuSpriteVerticalOffset at 0x00008F40.
	void applyMenuSpriteVerticalOffset() { _scene.setSpriteVerticalOffset(_menuAnimation.verticalOffset()); }

	// Retains both transfers in Ghidra CommitMenuSceneBuffers at 0x00008F1C and its deliberate
	// fall-through into ApplyMenuSpriteVerticalOffset at 0x00008F40.
	//
	// Ghidra: commitMenuSceneBuffers (0x00008F1C).
	void commitMenuSceneBuffers() {
		// Ghidra 0x00008F1C-0x00008F3F: replace both fixed-buffer DMA publications with logical tile commits.
		commitRevealSceneBuffers(true);

		// Original execution falls through at 0x00008F40 into this separately recovered function boundary.
		applyMenuSpriteVerticalOffset();
	}

	// Commits the caller-owned 0x380-word transfer from staged menu RAM to the 32x28 plane-A map at
	// VRAM 0xC000.
	//
	// Ghidra: caller-owned RunMainMenu ranges 0x00008500-0x00008511 and 0x0000859C-0x000085AD,
	// plus PresentCompressedMenuPlane at 0x000096F4-0x00009715.
	void commitStagedMenuPlane();

	// Replaces the sprite-table writes at 0x00008E86-0x00008EC9 and 0x00008EFE-0x00008F11 with one
	// transient logical selection cursor.
	// verticalOffset: current 24-pixel option-row offset.
	// attributes: packed tile attributes, or empty to hide the cursor.
	void updateSelectionCursor(int verticalOffset, Optional<uint16> attributes);

	// Publishes the symbol-grid and password-entry cursors written as terminal sprite entries 12
	// and 13.
	// gridColumn: selected zero-based symbol or action column.
	// gridRow: selected zero-based symbol or action row.
	// displayPosition: selected character position within the active password row.
	// passwordRow: selected zero-based password row.
	void updatePasswordGridCursors(int gridColumn, int gridRow, int displayPosition, int passwordRow);

	// Writes one NUL-terminated byte string as sequential plane-A cells using Ghidra
	// WriteIndexedTileRow at 0x00009F72.
	// text: display characters supplied through original register A0.
	// columnIndex: destination column supplied in register D1.
	// rowIndex: destination row supplied in register D2.
	//
	// Ghidra: writeIndexedTileRow (0x00009F72-0x00009FA1). The original destination is
	// 0xC000 + row*0x40 + column*2. Each unsigned nonzero byte is added to presentation attributes
	// 0x0680; the terminal NUL advances only the source. A write that starts late in a row
	// continues contiguously into following 32-column plane rows.
	void writeIndexedTileRow(const Common::String &text, int columnIndex, int rowIndex);

	// Writes one indexed character cell without retaining the original VRAM-word operation.
	// character: character index added to the current managed presentation attributes.
	// columnIndex: destination plane-A column.
	// rowIndex: destination plane-A row.
	void writeIndexedTile(uint8 character, int columnIndex, int rowIndex);

	// Sets or clears priority bit 15 in Ghidra g_wInterfaceTileAttributes at 0xFF068C.
	// enabled: whether subsequent indexed text writes appear above low-priority sprites.
	void setTextPriority(bool enabled) {
		_textAttributes =
			enabled
				? static_cast<uint16>(_textAttributes | 0x8000)
				: static_cast<uint16>(_textAttributes & 0x7FFF);
	}

	// Writes one row with temporary priority, restoring the exact prior presentation attributes.
	// text: display characters supplied through original register A0.
	// columnIndex: destination column supplied in register D1.
	// rowIndex: destination row supplied in register D2.
	//
	// Ghidra: writePriorityTileRow (0x00009F58-0x00009F71).
	void writePriorityTileRow(const Common::String &text, int columnIndex, int rowIndex);

	// Writes the original 32-cell blank sequence with the indexed writer's plane-row wrapping.
	// columnIndex: destination plane-A column of the first blank cell.
	// rowIndex: destination plane-A row of the first blank cell.
	void writeBlankTileRow(int columnIndex, int rowIndex) {
		writeIndexedTileRow(kBlankTileRow, columnIndex, rowIndex);
	}

	// Applies the original two-pass priority clear used for an unused selector row.
	// columnIndex: destination column restored for the second blank write.
	// rowIndex: destination plane-A row.
	void clearPriorityTileRow(int columnIndex, int rowIndex) {
		replacePriorityTileRow(kBlankTileRow, columnIndex, rowIndex);
	}

	// Clears a complete row before writing its priority replacement.
	// text: display characters supplied through original register A0.
	// columnIndex: destination column supplied in register D1.
	// rowIndex: destination row supplied in register D2.
	//
	// Ghidra: replacePriorityTileRow (0x00009F46-0x00009F57).
	void replacePriorityTileRow(const Common::String &text, int columnIndex, int rowIndex) {
		writePriorityTileRow(kBlankTileRow, 0, rowIndex);
		writePriorityTileRow(text, columnIndex, rowIndex);
	}

	// Replaces the menu sprites with a catalog selector's full-width low-priority highlight.
	// rowIndex: zero-based row within the current 16-entry selector page.
	void updateCatalogSelectionHighlight(int rowIndex);

	// Restores the complete immutable sprite table after either catalog selector replaces it.
	//
	// Ghidra: RunRoomSelectionMenu (0x000088C2-0x000088D3) and RunRoomObjectRelocationMenu
	// (0x00008CC0-0x00008CD1).
	void restoreMainMenuSpriteLayout();

	// Centers one NUL-terminated row in the 32-column menu plane before writing its indexed cells.
	// text: display characters supplied through original register A0.
	// rowIndex: destination row retained in original register D2.
	//
	// Ghidra: writeCenteredTileRow (0x00009F30-0x00009F45). The destination column is exactly
	// floor((32 - characterCount) / 2); row and presentation attributes remain unchanged before the
	// tail dispatch to WriteIndexedTileRow.
	void writeCenteredTileRow(const Common::String &text, int rowIndex);

private:
	// RunMainMenu writes 0x0680 to Ghidra g_wInterfaceTileAttributes at 0xFF068C at 0x000082D4.
	static const uint16 kBaseTextAttributes = 0x0680;
	static const int kNarrowRevealTileIndex = 0x640;
	static const int kColorsPerPaletteLine = 16;
	static const int kLightningPaletteStartIndex = 32;
	static const int kMenuPlaneColumns = 32;
	static const int kMenuPlaneRows = 28;
	static const int kPackedBytesPerTile = 32;
	static const int kPasswordEntryCursorBaseX = 8;
	static const int kPasswordEntryCursorBaseY = 56;
	static const int kPasswordGridCursorBaseX = 88;
	static const int kPasswordGridCursorBaseY = 112;
	static const int kPasswordGridCursorHeightInTiles = 2;
	static const int kPasswordGridCursorTileIndex = 0x7D0;
	static const int kCatalogSelectionHighlightBaseY = 56;
	static const int kCatalogSelectionHighlightSegmentCount = 8;
	static const int kCatalogSelectionHighlightSegmentWidth = 4;
	static const int kCatalogSelectionHighlightTileIndex = 0x7D0;
	static const int kSelectionCursorBaseY = 80;
	static const int kSelectionCursorSizeInTiles = 4;
	static const int kSelectionCursorX = 40;
	static const int kWideRevealTileIndex = 0x780;

	// Original formatting row at 0x0002FFF4-0x00030014, excluding its terminal NUL in managed text.
	static const Common::String kBlankTileRow;

	static int getCharacterCount(const Common::String &text);
	void stageInitialMenuAssets(int firstAdditionalTileIndex,
								const Common::Array<Common::Array<uint8>> &additionalTileChunks,
								const Common::Array<uint8> &fixedTileBlock);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	FramePresenter &_presenter;
	MenuAnimationState _menuAnimation;

	// Owns the decoded-frame equivalent of Ghidra g_bSharedDecodeBufferStart at 0xFF3000. Original
	// RAM has no image-backed initial value; an empty vector means no frame has been staged yet.
	Common::Array<uint8> _currentRleData;
	Common::Array<uint8> _fixedBackgroundMap;
	Common::Array<uint8> _menuLayout;
	Common::Array<uint8> _narrowRevealSource;

	// Owns the composed narrow-buffer prefix of Ghidra
	// g_abNarrowRevealOrRoomForegroundSnapshotWorkspace at 0xFFC400. Original RAM has no
	// image-backed initial value; an empty vector means composition has not produced it yet.
	Common::Array<uint8> _narrowRevealTiles;

	// Owns the menu-lifecycle value of Ghidra g_wInterfaceTileAttributes at 0xFF068C.
	uint16 _textAttributes{};
	Common::Array<uint8> _wideRevealSource;

	// Owns the composed wide-buffer prefix of Ghidra g_abWideRevealOrRoomCollisionWorkspace at
	// 0xFFE000. Original RAM has no image-backed initial value; an empty vector means composition
	// has not produced it yet.
	Common::Array<uint8> _wideRevealTiles;
};
} // namespace Scooby

#endif // SCOOBY_MAIN_MENU_PRESENTATION_H
