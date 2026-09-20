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

#ifndef SCOOBY_INTERACTION_ACTION_MENU_EXECUTOR_H
#define SCOOBY_INTERACTION_ACTION_MENU_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "dialogue_controller.h"
#include "interaction_action.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/shared_graphics_workspace.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns interaction-action choice presentation, nested script frames, and interface restoration.

namespace Scooby {

class InteractionActionMenuExecutor {
public:
	// Binds the menu's nested script lifetime to text, interface, animation, and retrace owners.
	// rom: verified cartridge containing immutable generated-mask source data.
	// scene: logical interface layer and pattern destination.
	// graphicsWorkspace: aliased workspace used to preserve the selected interface half.
	// state: shared interaction, script, actor, and input state.
	// presenter: host retrace boundary used by native busy-wait handovers.
	// updateActorAnimationFrames: recovered actor-animation service.
	// handleRoomVBlank: installed room callback that advances asynchronous state.
	// chooseRandomInteractionAnimation: recovered actor-zero interaction-animation selector.
	// runRoomScriptInitializationCommands: non-consuming action registration scan.
	// runRoomScriptCommands: execution-mode room-script dispatcher.
	// dialogue: shared interaction text and dialogue owner.
	// commitActionPromptUpdate: action-prompt composition and publication boundary.
	// refreshActionPrompts: left and right action-prompt refresh boundary.
	InteractionActionMenuExecutor(
		const ScoobyDooRom &rom, TileScene &scene,
		SharedGraphicsWorkspace &graphicsWorkspace, RuntimeState &state,
		FramePresenter &presenter, Common::Functor0<void> &updateActorAnimationFrames,
		Common::Functor0<void> &handleRoomVBlank, Common::Functor0<void> &chooseRandomInteractionAnimation,
		Common::Functor0<Optional<SessionExit>> &runRoomScriptInitializationCommands,
		Common::Functor0<Optional<SessionExit>> &runRoomScriptCommands,
		DialogueController &dialogue,
		Common::Functor0<void> &commitActionPromptUpdate, Common::Functor0<void> &refreshActionPrompts);

	// Runs registered interaction actions until the current or every nested script frame closes.
	// Returns the managed handover from an original non-returning descendant, otherwise no value.
	//
	// Ghidra: runInteractionActionMenu (0x00001C0C). The complete body is 0x00001C0C-0x000020D1. The six
	// 32-word interface rows at VRAM 0xAA80 or 0xAAC0 become a logical layer copy through the aliased
	// 0xFF2C00-0xFF2D7F workspace. Ghidra g_wMenuScratch0 and g_dwMenuScratch2 at 0xFF0810-0xFF0815,
	// g_pInteractionActionMenuScriptEnd at 0xFF0816-0xFF0819, and the native stack become managed nested-frame
	// values because the first two slots have a separate non-overlapping main-menu lifetime. The five line
	// counts at 0xFF081A-0xFF0823 remain shared across those nested frames.
	Optional<SessionExit> runInteractionActionMenu();

private:
	static const uint8 kActionButtonMask = 0x40;
	static const uint8 kAlternateActionButtonMask = 0x10;
	static const uint8 kAutomaticFrameUnwindMask = 0x01;
	static const uint8 kDisplaySideMask = 0x10;
	static const uint8 kDownDirectionMask = 0x02;
	static const int kInterfaceBlockColumnCount = 32;
	static const int kInterfaceBlockRowCount = 6;
	static const int kInterfaceBlockStartRow = 21;
	static const int kInterfaceMaskTileBaseDelta = 0x20;
	static const uint16 kInterfacePaletteClearMask = 0x9FFF;
	static const uint16 kInterfaceTileIndexMask = 0x07FF;
	static const uint8 kInteractionAnimationMask = 0x10;
	static const uint8 kInteractionMenuActiveMask = 0x04;
	static const uint8 kInteractionStripPublicationMask = 0x01;
	static const uint8 kRoomScriptMenuExitMask = 0x04;
	static const uint8 kUpDirectionMask = 0x01;

	struct InteractionActionMenuFrame {
		int32 ScriptStartOffset;
		int32 ScriptEndOffset;
		uint8 UsedFlags;

		InteractionActionMenuFrame(int32 scriptStartOffset, int32 scriptEndOffset,
								   uint8 usedFlags)
			: ScriptStartOffset(scriptStartOffset), ScriptEndOffset(scriptEndOffset), UsedFlags(usedFlags) {
		}
	};

	// Ghidra: RunInterfaceTransitionCountdown-equivalent inline loop shared by both original callers.
	bool runInterfaceTransitionCountdown();

	void publishBinaryMaskTiles(uint8 nonzeroPaletteIndex, uint8 zeroPaletteIndex,
								uint16 destinationAttributes);
	bool waitForRoomRetrace();

	// Fills the selected interface half with the interaction mask tile cell.
	// startColumn: caller-retained left or right 32-column interface destination.
	//
	// Ghidra: fillInteractionTextWindow (0x000022F2). The complete body is 0x000022F2-0x0000231F. Native D4
	// retains VRAM 0xAA80 or 0xAAC0 across all four calls from RunInteractionActionMenu. Six rows of 32
	// packed cells use a 0x80-byte plane stride. The hardware-only WriteSingleVramWord loop becomes one
	// logical interface-layer fill; its final D0/D1 defaults have no managed consumer.
	void fillInteractionTextWindow(int startColumn);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	SharedGraphicsWorkspace &_graphicsWorkspace;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor0<void> &_chooseRandomInteractionAnimation;
	Common::Functor0<Optional<SessionExit>> &_runRoomScriptInitializationCommands;
	Common::Functor0<Optional<SessionExit>> &_runRoomScriptCommands;
	DialogueController &_dialogue;
	Common::Functor0<void> &_commitActionPromptUpdate;
	Common::Functor0<void> &_refreshActionPrompts;
};
} // namespace Scooby

#endif // SCOOBY_INTERACTION_ACTION_MENU_EXECUTOR_H
