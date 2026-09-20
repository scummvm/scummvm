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

#ifndef SCOOBY_INTERACTION_CONTROLLER_H
#define SCOOBY_INTERACTION_CONTROLLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "action_menu_controller.h"
#include "dialogue_controller.h"
#include "interaction_resolver.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Coordinates cursor, automatic, and selected-interaction state recovered from Ghidra RunAdventure at
// 0x00000BF4.

namespace Scooby {

class InteractionController {
public:
	// Binds interaction state transitions to collision resolution, action menus, and retraces.
	// rom: verified cartridge address space containing interaction tables.
	// state: shared runtime state.
	// waitForRoomVerticalBlank: callback-aware room frame boundary used by automatic interactions.
	// random: shared original random-value owner.
	// actionMenu: shared owner of action selection and prompt publication.
	// dialogue: shared owner of dialogue and interaction text rendering.
	InteractionController(const ScoobyDooRom &rom, RuntimeState &state,
						  Common::Functor0<bool> &waitForRoomVerticalBlank,
						  DeterministicRandom &random,
						  ActionMenuController &actionMenu, DialogueController &dialogue)
		: _rom(rom), _state(state), _resolver(rom, state), _actionMenu(actionMenu), _dialogue(dialogue),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank), _random(random),
		  _invokeRoomScriptDispatchCallback(nullptr), _runInteractionActionMenu(nullptr),
		  _runRoomScriptInitializationCommands(nullptr) {
	}

	// Binds the script initialization, scan, and execution owners used by interaction windows.
	// invokeRoomScriptDispatchCallback: mode-selecting room-script dispatcher.
	// runRoomScriptInitializationCommands: separate initialization-scan owner.
	void bindRoomScriptDispatch(
		Common::Functor1<int, Optional<SessionExit>> &invokeRoomScriptDispatchCallback,
		Common::Functor0<Optional<SessionExit>> &runRoomScriptInitializationCommands) {
		_invokeRoomScriptDispatchCallback = &invokeRoomScriptDispatchCallback;
		_runRoomScriptInitializationCommands = &runRoomScriptInitializationCommands;
	}

	// Binds the distinct nested interaction-action menu executor after script dispatch is available.
	// runInteractionActionMenu: complete interaction-action choice and nested-frame owner.
	void bindInteractionActionMenu(
		Common::Functor0<Optional<SessionExit>> &runInteractionActionMenu) {
		_runInteractionActionMenu = &runInteractionActionMenu;
	}

	// Copies the current interaction fields into their original previous-frame slots.
	void snapshotInteractionState();

	// Updates room interactions driven by actor position when the cursor display is inactive.
	// restartRoomLoop: receives whether the original explicit retrace branches directly to the top of
	//     RunAdventure.
	// Returns a nested exit or host-close handover, otherwise no value.
	//
	// Ghidra: RunAdventure (0x000012CC-0x000013AC). A completed automatic interaction waits one
	// callback-aware retrace at 0x000013A6 and branches to 0x00000D38, bypassing the actor update at
	// 0x000013B0.
	Optional<SessionExit> updateAutomaticInteraction(bool &restartRoomLoop);

	// Selects and restarts actor zero's random interaction animation when transitions permit it.
	//
	// Ghidra: chooseRandomInteractionAnimation (0x000020DA). The complete body is 0x000020DA-0x00002137. The
	// random term uses the shared deterministic sequence and all animation arithmetic remains 16-bit.
	// Position two is rewritten to zero; position three is remapped only locally.
	void chooseRandomInteractionAnimation();

	// Updates cursor targeting or the bottom action menu according to the recovered Y split.
	// Returns false when the host requests shutdown during menu processing.
	bool updateCursorInteraction();

	// Runs the current selected interaction through scanning, execution, and fallback dialogue.
	// Returns the managed handover from an original non-returning descendant, otherwise no value.
	//
	// Ghidra: runSelectedInteraction (0x000019B0). The complete body is 0x000019B0-0x00001BD9. Descriptor
	// fields +0x28 and +0x2C select eight-byte interaction records and their script base. The 11
	// dialogue-template pointers occupy 0x0002FEA0-0x0002FECB; byte-one placeholders select the complete
	// NUL-terminated ranges 0x0002FFDB-0x0002FFE7. Typed script offsets and composed text replace the native
	// register and shared-workspace aliases without merging any original callee boundary.
	Optional<SessionExit> runSelectedInteraction();

	// Redraws the room name or complete interaction prompt when its recovered state snapshot changes.
	//
	// Ghidra: refreshInteractionDisplay (0x0000A828). The complete body is 0x0000A828-0x0000A983. The managed
	// text source preserves whether A0 addressed immutable ROM text or the shared composed-text workspace
	// without retaining the original RAM alias.
	void refreshInteractionDisplay();

	// Scans eligible room objects and executes each marked automatic-interaction script.
	// Returns the managed handover from an original non-returning script path, otherwise no value.
	//
	// Ghidra: processAutomaticInteractions (0x0000AFDA). The complete body is 0x0000AFDA-0x0000B0B7.
	// Descriptor fields +0x28 and +0x2C select eight-byte interaction records and their relative script base.
	// The native pre-decrement and DBF visit all decoded room objects except the terminal record: 180
	// episode-zero records or 154 episode-one records. Both script lengths are signed words, and normal
	// return restores the complete saved automatic-interaction byte and script window. A managed
	// non-returning handover bypasses that restore.
	Optional<SessionExit> processAutomaticInteractions();

private:
	static const int kAutomaticInteractionRecordByteCount = 8;
	static const int kEpisodeInteractionRecordTableField = 0x28;
	static const int kEpisodeInteractionScriptBaseField = 0x2C;
	static const int kEpisodeObjectLabelBaseField = 0x20;
	static const int kEpisodeObjectLabelTableField = 0x28;
	static const int kInteractionScriptHeaderByteCount = 6;
	static const int kInteractionActionTextTableOffset = 0x2FE5E;
	static const int kInteractionDescriptionFeminineOffset = 0x2FFE4;
	static const int kInteractionDescriptionMasculineOffset = 0x2FFE0;
	static const int kInteractionDescriptionNeutralOffset = 0x2FFDB;
	static const uint8 kInteractionDescriptionPlaceholder = 0x01;
	static const int kInteractionDialogueTemplateTableOffset = 0x2FEA0;
	static const int kInteractionToConnectorOffset = 0x2FFEF;
	static const int kInteractionWithConnectorOffset = 0x2FFE8;
	static const int kNestedInteractionScriptHeaderByteCount = 4;
	static const int kNestedSelectedInteractionScriptHeaderByteCount = 8;
	static const int kObjectLabelDisplacementField = sizeof(uint32);
	static const int kObjectLabelRecordShift = 3;
	static const int kScoobyInteractionLabelOffset = 0x2FE57;
	static const int kShaggyInteractionLabelOffset = 0x2FE50;
	static const uint8 kAlternateInteractionMask = 0x01;
	static const uint8 kAutomaticInteractionMarkerMask = 0x01;
	static const uint8 kAutomaticInteractionLatchMask = 0x80;
	static const uint8 kAutomaticInteractionScanMask = 0x10;
	static const uint8 kAutomaticRoomObjectMask = 0x04;
	static const uint8 kActorZeroRestartMask = 0x01;
	static const uint8 kAlternateActorModeMask = 0x10;
	static const uint8 kCompleteRefreshMask = 0x02;
	static const uint16 kInteractionDescriptionFeminineMask = 0x0200;
	static const uint16 kInteractionDescriptionPersonMask = 0x0100;
	static const uint8 kInteractionDisplayMask = 0x08;
	static const uint8 kInteractionPendingMask = 0x80;
	static const uint8 kMatchedSelectedInteractionMask = 0x40;
	static const uint8 kRoomBehaviorInteractionMask = 0x01;
	static const uint8 kScriptedInteractionMask = 0x08;
	static const uint8 kSelectedInteractionScanMask = 0x02;
	static const uint8 kSuppressInteractionToggleMask = 0x20;
	static const uint8 kSuppressRandomAnimationMask = 0x80;
	static const uint8 kSuppressRoomNameRefreshMask = 0x20;
	static const uint8 kUseToConnectorMask = 0x01;
	static const int16 kInventoryRoomId = 1;
	static const int16 kInventorySelectionMode = 2;
	static const int16 kSetRoomBehaviorMode = 6;
	static const int16 kClearRoomBehaviorMode = 5;
	static const int16 kInteractionActionMenuMode = 8;
	static const int16 kSuppressFallbackDialogueMode = 11;

	// Appends one cartridge byte string to the shared managed text workspace.
	// sourceOffset: cartridge offset of the first source byte.
	// destination: current shared-workspace content receiving the copied bytes.
	//
	// Ghidra: copyNullTerminatedString (0x00005B56). The complete body is 0x00005B56-0x00005B5D. The managed
	// workspace carries its NUL implicitly, so appending the next source naturally resumes where the original
	// caller backed A1 onto the copied terminator.
	void copyNullTerminatedString(int sourceOffset, Common::String &destination);

	// Resolves one interaction identity to its immutable display-label address.
	// interaction: signed interaction word selected by room or cursor state.
	// Returns the cartridge offset of the selected NUL-terminated label.
	//
	// Ghidra: resolveInteractionLabelAddress (0x00005B5E). The complete body is 0x00005B5E-0x00005B91. Active
	// episode descriptor fields +0x20 and +0x28 select the shared label base and eight-byte record table.
	// Each ordinary record's second longword is an unsigned base-relative displacement; identities one and
	// two select fixed ROM labels instead.
	int resolveInteractionLabelAddress(int16 interaction);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	InteractionResolver _resolver;
	ActionMenuController &_actionMenu;
	DialogueController &_dialogue;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
	DeterministicRandom &_random;
	Common::Functor1<int, Optional<SessionExit>> *_invokeRoomScriptDispatchCallback;
	Common::Functor0<Optional<SessionExit>> *_runInteractionActionMenu;
	Common::Functor0<Optional<SessionExit>> *_runRoomScriptInitializationCommands;
};
} // namespace Scooby

#endif // SCOOBY_INTERACTION_CONTROLLER_H
