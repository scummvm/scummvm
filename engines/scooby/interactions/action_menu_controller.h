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

#ifndef SCOOBY_ACTION_MENU_CONTROLLER_H
#define SCOOBY_ACTION_MENU_CONTROLLER_H

#include "common/algorithm.h"
#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/shared_graphics_workspace.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns the action-icon and menu state transitions in Ghidra RunAdventure at 0x00000BF4.

namespace Scooby {

class ActionMenuController {
public:
	// Binds action-menu decisions to runtime state and retrace presentation.
	// rom: verified cartridge data containing both episode inventory-graphic tables.
	// scene: logical pattern and lower-interface destination.
	// graphicsWorkspace: shared graphics bytes staged here and later consumed by transitions.
	// state: shared interaction and menu state.
	// waitForRoomVerticalBlank: callback-aware room frame boundary used by menu waits.
	ActionMenuController(const ScoobyDooRom &rom, TileScene &scene,
						 SharedGraphicsWorkspace &graphicsWorkspace, RuntimeState &state,
						 Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _scene(scene), _graphicsWorkspace(graphicsWorkspace), _state(state),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank),
		  _inventoryCellWorkspace(kInventoryGridCellByteCount) {
		Common::fill(_inventoryCellWorkspace.begin(), _inventoryCellWorkspace.end(), 0);
	}

	// Removes both selected-action icon images before interaction-action rebuilding.
	//
	// Ghidra: refreshSelectedActionIcons (0x00001BDA). The complete body is 0x00001BDA-0x00001C0B. Both
	// mode-zero draws remain ordered and unconditional; each callee independently handles a zero pending icon.
	void refreshSelectedActionIcons();

	// Synchronizes the selected interaction with the action icon shown by the original UI.
	void synchronizeActionIcon();

	// Processes the pending command for the action strip at the bottom of the output frame.
	// Returns false when the host requests shutdown during the command retrace.
	bool updateActionMenu();

	// Stages the selected inventory page's patterns and complete eight-slot cell grid.
	//
	// Ghidra: composeInventoryGrid (0x00005D1C). g_abEpisodeZeroInventoryGraphicRecords occupies
	// 0x00139E52-0x0013E9B5, while g_abEpisodeOneInventoryGraphicRecords occupies 0x001A5A76-0x001A8933. Each
	// 0x182-byte record contains an attribute word and twelve packed tiles. A zero graphic index consumes its
	// display slot but deliberately leaves that slot and the current pattern destination untouched.
	// CommitActionPromptTiles owns the separate logical publication boundary.
	void composeInventoryGrid();

	// Publishes the complete staged inventory pattern set and lower-interface cell block.
	//
	// Ghidra: commitActionPromptTiles (0x00005CB6). Pattern address 0xB000 is logical tile 0x580. Plane-A
	// address 0xAACC is row 21, column 38 of the 64-cell lower-interface map; 0xAD80 is its complete row 27.
	// Managed publication replaces the two VDP write helpers without changing their extents, row stride,
	// order, or zero-cell value.
	void commitActionPromptTiles();

	// Refreshes both inventory-page navigation prompts from the current page and object table.
	//
	// Ghidra: refreshActionPrompts (0x00005B92). Inventory membership is counted directly from every room
	// object's room ID rather than inferred from page contents. Both prompt queue calls run on every
	// invocation, including an empty inventory.
	void refreshActionPrompts();

	// Publishes the pending nonzero action icon into its fixed lower-interface slot.
	// mode: low-word template mode; recovered callers use zero, one, or two.
	//
	// Ghidra: drawSelectedActionIcon (0x00005BDC). The complete body is 0x00005BDC-0x00005C59. Executable
	// lookup g_awActionIconByCommand at 0x0002FE3A-0x0002FE4F maps each icon identity to one of five columns
	// in either two-row band. Authored cells come from the three 0x300-byte modes in
	// g_awRoomInterfaceCellTemplates at 0x00030016-0x00030915. Logical interface-layer publication replaces
	// the native VDP word loop without changing its attribute addition or row stride.
	void drawSelectedActionIcon(int mode);

	// Stages the current inventory page and waits for its retrace publication.
	//
	// Ghidra: commitActionPromptUpdate (0x00005CF6). Only actor slot zero blocks composition; other pending
	// actor uploads do not. Display bit five transfers ownership of the staged patterns and cells to
	// CommitPendingActionPromptDuringVBlank, which clears the bit after publishing them.
	void commitActionPromptUpdate();

private:
	static const uint8 kAlternateInteractionMask = 0x01;
	static const int kActionIconCellColumnCount = 4;
	static const int kActionIconCellRowCount = 2;
	static const int kActionIconColumnStride = 5;
	static const int kActionIconFirstColumn = 2;
	static const int kActionIconLowerStartRow = 24;
	static const int kActionIconModeByteCount = 0x300;
	static const int kActionIconSourceLowerRowByteOffset = 0x184;
	static const int kActionIconSourceUpperRowByteOffset = 0x84;
	static const int kActionIconUpperStartRow = 22;
	static const uint8 kActionPromptPublicationPendingMask = 0x20;
	static const uint16 kBlankInventoryCell = 0x8000;
	static const int kEpisodeInventoryGraphicTableField = 0x18;
	static const uint16 kFirstInventoryTileAttribute = 0x8580;
	static const int kInventoryGraphicRecordSize = 0x182;
	static const int kInventoryGraphicTileByteCount = 0x180;
	static const int kInventoryGraphicTileCount = 12;
	static const int kInventoryGridCellByteCount = 0xF0;
	static const int kInventoryGridColumnCount = 20;
	static const int kInventoryGridRowByteCount = 0x28;
	static const int kInventoryGridStartColumn = 38;
	static const int kInventoryGridStartRow = 21;
	static const int kInventoryIconColumnCount = 4;
	static const int kInventoryIconRowCount = 3;
	static const int kInventoryPatternDestinationByteOffset = 0xB000;
	static const int kInventorySlotColumnByteCount = 10;
	static const int kInventorySlotCount = 8;
	static const int kInventorySlotRowByteCount = 0x78;
	static const int kInterfaceClearRow = 27;
	static const int kInterfaceColumnCount = 64;
	static const uint8 kMenuDisplayMask = 0x10;
	static const int kRoomInterfaceCellTemplatesOffset = 0x30016;
	static const uint8 kSelectedActionIconRefreshMask = 0x08;

	// Ghidra g_awActionIconByCommand at 0x0002FE3A-0x0002FE4F, including unused command zero.
	static const Common::Array<int16> kActionIconByCommand;

	bool resolveActionIcon(int16 interaction, int16 &actionIcon);

	// Waits for the left prompt to become inactive, then queues its next asset index.
	// mode: prompt update value written by the original D0 word.
	//
	// Ghidra: queueLeftActionPrompt (0x00005C5A). The native busy loop relies on the installed VBlank
	// callback to decrement the signed countdown; managed execution services that callback and the shared
	// clocked frame on each continuation instead of modeling asynchronous interrupts.
	void queueLeftActionPrompt(int16 mode);

	// Waits for the right prompt to become inactive, then queues its next asset index.
	// mode: prompt update value written by the original D0 word.
	//
	// Ghidra: queueRightActionPrompt (0x00005C74). The native busy loop relies on the installed VBlank
	// callback to decrement the signed countdown; managed execution services that callback and the shared
	// clocked frame on each continuation instead of modeling asynchronous interrupts.
	void queueRightActionPrompt(int16 mode);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	SharedGraphicsWorkspace &_graphicsWorkspace;
	RuntimeState &_state;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;

	// The inventory-owned lifetime of Ghidra g_abSharedTileWorkspace at 0xFF0328-0xFF0417. Process memory
	// starts at zero; composition preserves occupied zero-graphic slots and replaces only nonzero or trailing
	// unused slots before prompt publication consumes six rows.
	Common::Array<uint8> _inventoryCellWorkspace;
};
} // namespace Scooby

#endif // SCOOBY_ACTION_MENU_CONTROLLER_H
