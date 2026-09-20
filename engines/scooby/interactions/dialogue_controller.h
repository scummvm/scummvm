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

#ifndef SCOOBY_DIALOGUE_CONTROLLER_H
#define SCOOBY_DIALOGUE_CONTROLLER_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "dialogue_text_source.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns shared dialogue and interaction text rendering, dismissal, and cleanup boundaries.

namespace Scooby {

class DialogueController {
public:
	// Binds shared text drawing and dialogue lifetimes to the logical Window plane.
	// rom: verified cartridge containing immutable text.
	// scene: logical scene receiving Window cells and clipping.
	// state: shared text attributes, countdown, and Window position state.
	// presenter: host retrace boundary used while dialogue remains visible.
	// updateActorAnimationFrames: recovered actor-animation service.
	// handleRoomVBlank: installed room callback that polls controls and publishes frame state.
	DialogueController(const ScoobyDooRom &rom, TileScene &scene,
					   RuntimeState &state,
					   FramePresenter &presenter,
					   Common::Functor0<void> &updateActorAnimationFrames,
					   Common::Functor0<void> &handleRoomVBlank)
		: _rom(rom), _scene(scene), _state(state), _presenter(presenter),
		  _updateActorAnimationFrames(updateActorAnimationFrames), _handleRoomVBlank(handleRoomVBlank),
		  _chooseRandomInteractionAnimation(nullptr),
		  _processAutomaticInteractions(nullptr) {
	}

	// Binds the interaction owners reached while a dialogue remains visible.
	// processAutomaticInteractions: automatic-interaction scan and nested execution owner.
	// chooseRandomInteractionAnimation: interaction-animation selection boundary.
	void bindDismissalInteractions(
		Common::Functor0<Optional<SessionExit>> &processAutomaticInteractions,
		Common::Functor0<void> &chooseRandomInteractionAnimation) {
		_processAutomaticInteractions = &processAutomaticInteractions;
		_chooseRandomInteractionAnimation = &chooseRandomInteractionAnimation;
	}

	// Clears the Window plane and draws centered, wrapped text from row one.
	// source: immutable cartridge text or one managed shared-buffer composition.
	//
	// Ghidra: drawDialogueText (0x0000A054). The complete body is 0x0000A054-0x0000A12F. VRAM 0xD000-0xD7FF
	// becomes a logical 32x32 Window layer; register 18 becomes its byte-accurate vertical clipping state.
	void drawDialogueText(const DialogueTextSource &source);

	// Draws one wrapped interaction label and publishes its line-based display countdown.
	// source: NUL-terminated cartridge text or a managed composition.
	// row: signed row offset within the six-row interaction block.
	// horizontalOffset: signed column offset selecting the left or right interface half.
	// Returns the wrapping signed-word count of rendered lines.
	//
	// Ghidra: drawTimedInteractionText (0x00009FA2). The complete body is 0x00009FA2-0x0000A053. Destination
	// 0xAA82 + row*0x80 + horizontalOffset*2 and every row advance retain word wrapping. Lines wrap at 30
	// source bytes by searching backward for a space or a hyphen one byte ahead, without a delimiter-free
	// fallback. Hardware-only WriteSingleVramWord calls become logical interface cells. The low word of
	// unsigned line count times 300 replaces g_wTextDisplayCountdown (0xFF080A-0xFF080B).
	int16 drawTimedInteractionText(const DialogueTextSource &source, int16 row,
								   int16 horizontalOffset);

	// Clears the shared text source and redraws the Window from its empty contents.
	//
	// Ghidra: clearDialogueText (0x0000A172). The complete body is 0x0000A172-0x0000A181. An empty managed
	// source replaces the NUL written at g_abSharedTextWorkspace (0xFF08BC) without recreating that aliased
	// workspace.
	void clearDialogueText();

	// Draws one dialogue, waits for dismissal, and restores its hidden Window state.
	// source: immutable cartridge text or one managed shared-workspace composition.
	// Returns the managed handover from an original non-returning dismissal path, otherwise no value.
	//
	// Ghidra: presentDialogueAndWait (0x0000A130). The complete body is 0x0000A130-0x0000A171. Logical Window
	// clipping replaces the register-18 write of 0x9202 while preserving byte value two, publication order,
	// and flag restoration.
	Optional<SessionExit> presentDialogueAndWait(const DialogueTextSource &source);

	// Services room activity until dialogue timing, script state, or released input dismisses the text.
	// Returns the managed handover from a nested non-returning path, otherwise no value.
	//
	// Ghidra: waitForDialogueDismissal (0x0000A18C). The complete body is 0x0000A18C-0x0000A235. Timing data
	// at 0x0000A182-0x0000A18B is retained as the five big-endian words 10, 5, 2, 1, 0; its signed word byte
	// offset selects the unsigned decrement whose subtraction carry dismisses text. Managed room callbacks
	// replace the original asynchronous VBlank service inside input-release spins.
	Optional<SessionExit> waitForDialogueDismissal();

private:
	static const uint8 kActionButtonMask = 0x40;
	static const uint8 kAlternateActionButtonMask = 0x10;
	static const uint8 kActorZeroHoldMask = 0x01;
	static const int kCenteringWidth = 30;
	static const uint8 kDialogueDisplayMask = 0x08;
	static const uint8 kDialogueInteractionAnimationMask = 0x10;
	static const int kFirstTextCellAddress = 0xD042;
	static const uint8 kHyphen = 0x2D;
	static const uint8 kInitialWindowRowCount = 2;
	static const int kInteractionFirstTextCellAddress = 0xAA82;
	static const int kInteractionLineWidth = 30;
	static const int kInterfaceNameTableAddress = 0xA000;
	static const int kInterfacePlaneColumnCount = 64;
	static const int kInterfaceRowByteStride = 0x80;
	static const uint8 kInteractionRefreshMask = 0x02;
	static const uint8 kSpace = 0x20;
	static const int kWindowNameTableAddress = 0xD000;
	static const int kWindowRowByteStride = 0x40;

	static const Common::Array<uint8> kDialogueCountdownStepBytes;

	void writeInteractionLine(const DialogueTextSource &source, int &sourceIndex, int16 lineLength,
							  uint16 destinationAddress);
	void writeCenteredLine(const DialogueTextSource &source, int &sourceIndex, int16 lineLength,
						   int row);
	int16 measureNullTerminatedString(const DialogueTextSource &source);
	bool waitForRoomRetrace();

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor0<void> *_chooseRandomInteractionAnimation;
	Common::Functor0<Optional<SessionExit>> *_processAutomaticInteractions;
};
} // namespace Scooby

#endif // SCOOBY_DIALOGUE_CONTROLLER_H
