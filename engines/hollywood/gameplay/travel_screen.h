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

#ifndef HOLLYWOOD_GAMEPLAY_TRAVEL_SCREEN_H
#define HOLLYWOOD_GAMEPLAY_TRAVEL_SCREEN_H

#include "common/array.h"
#include "common/types.h"
#include "graphics/managed_surface.h"

#include "hollywood/graphics.h"

namespace Hollywood {

class HollywoodEngine;

// Handles the notebook chunks of RESOURCE.I04, both as an inventory viewer and
// as the interactive destination selector entered through state 0xffff.
class TravelScreen {
public:
	TravelScreen(HollywoodEngine *vm);

	bool showViewer();
	bool runSelection(byte currentChapterId, uint16 &selectedStateId);

private:
	bool load(bool loadSelectionMask);
	bool readChunk(uint index, Common::Array<byte> &destination, uint expectedSize);
	void composeUnlockedSlots();
	void expandSelectionMask();
	void applySlotPalette(byte slotIndex, bool highlighted);
	bool isActiveSlot(byte slotIndex) const;
	byte slotAtPoint(int x, int y) const;
	uint16 destinationState(byte destinationId, byte currentChapterId) const;
	void present();

	HollywoodEngine *_vm;
	Common::Array<byte> _palette;
	Common::Array<byte> _tilePixels;
	Common::Array<byte> _fillRuns;
	IndexedSurfaceBuffer _framebuffer;
	IndexedSurfaceBuffer _selectionMask;
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_TRAVEL_SCREEN_H
