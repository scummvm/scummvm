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

#ifndef SCOOBY_TILE_SCENE_H
#define SCOOBY_TILE_SCENE_H

#include "common/array.h"
#include "common/scummsys.h"

#include "horizontal_display_mode.h"
#include "indexed_frame.h"
#include "palette_color.h"
#include "scooby/common/span.h"
#include "sprite_instance.h"
#include "sprite_layer.h"
#include "tile_cell.h"
#include "tile_layer.h"

// Owns the decoded tiles, logical layers, palette, and scrolling needed to
// render the current scene.
//
// The backend frame remains 320x224. Recovered H32 presentations use a
// centered 256-pixel viewport, while H40 presentations use all 320 pixels.
// Horizontal mode remains independent from authored map width: complete
// layer replacement preserves the selected mode while the episode opening
// retains 128x28 planes and the temporary room presentation retains 64x32
// planes behind their H32 viewports.

namespace Scooby {

class TileScene {
public:
	TileScene();

	// RAII replacement for the C# IDisposable beginTemporaryContentReplacement()
	// result: restores captured logical content when it goes out of scope,
	// exactly like the "using" block at its call site. Move-only.
	class ContentReplacementGuard {
	public:
		ContentReplacementGuard(ContentReplacementGuard &&other) noexcept;
		ContentReplacementGuard &operator=(ContentReplacementGuard &&other) noexcept;
		ContentReplacementGuard(const ContentReplacementGuard &) = delete;
		ContentReplacementGuard &operator=(const ContentReplacementGuard &) = delete;
		~ContentReplacementGuard();

	private:
		friend class TileScene;
		explicit ContentReplacementGuard(TileScene *scene);

		TileScene *_scene;
		Common::Array<TileCell> _backgroundLayer;
		int _backgroundHorizontalOffset;
		Common::Array<int16> _backgroundLineHorizontalOffsets;
		int _backgroundVerticalOffset;
		HorizontalDisplayMode _horizontalDisplayMode;
		Common::Array<SpriteInstance> _dynamicSprites;
		Common::Array<TileCell> _foregroundLayer;
		int _foregroundHorizontalOffset;
		Common::Array<int16> _foregroundLineHorizontalOffsets;
		int _foregroundVerticalOffset;
		Common::Array<TileCell> _interfaceLayer;
		int _layerColumns;
		int _layerRows;
		bool _roomInterfaceVisible;
		int _spriteVerticalOffset;
		bool _spritesVisible;
		Common::Array<uint8> _tilePixels;
		Common::Array<TileCell> _windowLayer;
		uint8 _windowVerticalPositionBits;
		int16 _interfaceHorizontalScroll;
	};

	void setInterfaceScroll(int16 newScroll) { _interfaceHorizontalScroll = newScroll; }

	// Starts a blank scene in the specified authored horizontal display mode.
	void reset(HorizontalDisplayMode horizontalDisplayMode);

	// Clears decoded content while preserving the current faded palette and
	// horizontal display mode.
	void clearContent();

	// Loads a complete packed ROM tile set into the scene's decoded pixel representation.
	void loadTiles(Span<const uint8> packedTiles);

	// Places packed tiles at their authored tile index while preserving existing scene tiles.
	void loadTilesAt(Span<const uint8> packedTiles, int startTileIndex);

	// Overwrites packed pattern bytes at an exact logical pattern-memory byte
	// offset. Bytes absent from a partial final tile retain their current
	// decoded pixels.
	void overwritePackedPatternBytes(Span<const uint8> packedTileBytes,
									 int destinationByteOffset);

	// Preserves logical patterns, layers, horizontal mode, scrolling, and
	// sprite visibility across a temporary presentation. Palette state is
	// deliberately excluded because the caller restores it separately.
	ContentReplacementGuard beginTemporaryContentReplacement();

	// Replaces the scene's linked sprites from one packed attribute table.
	void loadSprites(Span<const uint8> packedEntries);

	// Replaces the complete sprite table with backend-neutral instances.
	void replaceSprites(Span<const SpriteInstance> sprites);

	// Sets transient sprites that follow the fixed linked table but do not
	// inherit its sampled offset.
	void setDynamicSprites(Span<const SpriteInstance> sprites);

	// Ghidra ApplyMenuSpriteVerticalOffset (0x00008F40): non-cumulative
	// displacement from the decoded sprite Y coordinates.
	void setSpriteVerticalOffset(int offset);

	// Places a contiguous rectangular tilemap asset into one logical scene layer.
	void loadLayerRows(Span<const uint8> packedCells, TileLayer layer, int startColumn,
					   int startRow,
					   int columnCount, int rowCount);

	// Replaces every cell in one logical layer with the same decoded tile.
	void fillLayer(const TileCell &cell, TileLayer layer);

	// Replaces every cell in the distinct 32x32 Window name table.
	void fillWindow(const TileCell &cell);

	// Writes one logical Window cell selected by its 32-column name-table coordinates.
	void setWindowCell(const TileCell &cell, int column, int row);

	// Applies the complete recovered vertical Window clipping byte (register
	// 18: bits 0-4 select the eight-pixel boundary, bit 7 selects its side).
	void setWindowVerticalPosition(uint8 positionBits);

	// Copies one rectangular cell block into a packed big-endian name-table workspace.
	void copyPackedLayerRows(Span<uint8> destination, TileLayer layer, int startColumn,
							 int startRow,
							 int columnCount, int rowCount);

	// Fills one rectangular cell block while preserving every surrounding cell.
	void fillLayerRows(const TileCell &cell, TileLayer layer, int startColumn, int startRow, int columnCount,
					   int rowCount);

	// Replaces both complete scrolling name tables and resets the separately
	// published interface and Window layers. Plane publication does not
	// write Genesis VDP register 12, so the current H32/H40 mode remains
	// selected independently from the replacement planes' column count.
	void replaceCompleteLayers(Span<const uint8> backgroundCells,
							   Span<const uint8> foregroundCells, int columnCount, int rowCount);

	// Replaces one contiguous logical row without reintroducing packed name-table storage.
	void replaceLayerRow(Span<const TileCell> cells, TileLayer layer, int startColumn, int row);

	// Replaces one authored contiguous palette range without applying fade interpolation.
	void replacePaletteRange(Span<const PaletteColor> colors, int startIndex);

	// Moves every color in one current-palette range left and wraps its first color to the end.
	void rotatePaletteRangeLeft(int startIndex, int colorCount);

	// Advances each palette channel by one authored fade level toward the target.
	void advancePaletteToward(Span<const PaletteColor> target);

	// Advances every nonzero palette channel down by one authored fade level.
	void advancePaletteTowardBlack();

	// Rotates the eight-color ramp animated by the first startup presentation.
	void rotateFirstLogoRamp();

	// Ghidra UpdateScrollingAndStreamTiles (0x0000FA78): signed plane displacements.
	void setHorizontalOffsets(int foreground, int background);

	// Replaces uniform plane displacement with one signed horizontal value per output scanline.
	void setLineHorizontalOffsets(Span<const int16> foreground,
								  Span<const int16> background);

	// Retains the two logical displacements published through the original VSRAM pair.
	void setVerticalOffsets(int foreground, int background);

	// Composes the current logical layers and palette into one backend-neutral indexed frame.
	void render(IndexedFrame &frame);

private:
	static const int kDefaultLayerColumns = 64;
	static const int kDefaultLayerRows = 32;
	static const int kRoomInterfaceSplitY = 168;
	static const int kPixelsPerTile = 64;
	static const int kTileSize = 8;
	static const int kWindowLayerColumns = 32;
	static const int kWindowLayerRows = 32;

	static int mod(int value, int modulus);
	static uint8 stepLevel(uint8 current, uint8 target);
	int contentWidth() const;
	static Common::Array<TileCell> decodeLayer(Span<const uint8> packedCells, int columnCount,
											   int rowCount);
	void drawForegroundAndWindow(IndexedFrame &frame, bool highPriority, int roomBottom);
	void drawLayer(IndexedFrame &frame, const Common::Array<TileCell> &layer, int columnCount, int rowCount,
				   int horizontalOffset, Span<const int16> lineHorizontalOffsets,
				   int verticalOffset,
				   bool highPriority, int startY, int endY);
	Common::Array<TileCell> &layerFor(TileLayer layer);

	Common::Array<TileCell> _backgroundLayer;
	Common::Array<TileCell> _foregroundLayer;
	Common::Array<TileCell> _interfaceLayer;
	Common::Array<TileCell> _windowLayer;
	Common::Array<PaletteColor> _palette;
	SpriteLayer _spriteLayer;
	int _backgroundHorizontalOffset;
	Common::Array<int16> _backgroundLineHorizontalOffsets;
	int _backgroundVerticalOffset;
	HorizontalDisplayMode _horizontalDisplayMode;
	Common::Array<SpriteInstance> _dynamicSprites;
	int _foregroundHorizontalOffset;
	Common::Array<int16> _foregroundLineHorizontalOffsets;
	int _foregroundVerticalOffset;
	int _layerColumns;
	int _layerRows;
	bool _roomInterfaceVisible;
	int _spriteVerticalOffset;
	bool _spritesVisible;
	Common::Array<uint8> _tilePixels;
	uint8 _windowVerticalPositionBits;
	int16 _interfaceHorizontalScroll; // controls whether the action buttons or inventory are displayed.
};
} // namespace Scooby

#endif // SCOOBY_TILE_SCENE_H
