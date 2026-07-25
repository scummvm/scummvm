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

#ifndef MADS_NEBULAR_ROOMS_THUNKS_H
#define MADS_NEBULAR_ROOMS_THUNKS_H

#include "mads/core/config.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

#define CURSOR_ARROW CURSOR_NORMAL
#define CURSOR_GO_DOWN CURSOR_DOWN
#define CURSOR_GO_UP CURSOR_UP
#define CURSOR_GO_LEFT CURSOR_LEFT
#define CURSOR_GO_RIGHT CURSOR_RIGHT

enum EXTTYPE {
	EXT_NONE = -1, EXT_SS = 1, EXT_AA = 2, EXT_DAT = 3, EXT_HH = 4,
	EXT_ART = 5, EXT_INT = 6
};

enum InputMode {
	kInputBuildingSentences = INTER_BUILDING_SENTENCES,
	kInputConversation = INTER_CONVERSATION,
	kInputLimitedSentences = INTER_LIMITED_SENTENCES
};

enum SequenceTrigger {
	SEQUENCE_TRIGGER_EXPIRE = KERNEL_TRIGGER_EXPIRE,	// Trigger when the sequence finishes
	SEQUENCE_TRIGGER_LOOP = KERNEL_TRIGGER_LOOP,		// Trigger when the sequence loops
	SEQUENCE_TRIGGER_SPRITE = KERNEL_TRIGGER_SPRITE		// Trigger when sequence reaches specific sprite
};

enum TriggerMode {
	SEQUENCE_TRIGGER_NONE = -1,
	SEQUENCE_TRIGGER_PARSER = KERNEL_TRIGGER_PARSER,		// Triggers parser
	SEQUENCE_TRIGGER_DAEMON = KERNEL_TRIGGER_DAEMON,		// Triggers step/daemon code
	SEQUENCE_TRIGGER_PREPARE = KERNEL_TRIGGER_PREPARSE		// Triggers preparser
};

enum SpriteAnimType {
	ANIMTYPE_NONE = 0,
	ANIMTYPE_CYCLED = AA_LINEAR,
	ANIMTYPE_PING_PONG = AA_PINGPONG,
	ANIMTYPE_STAMP = AA_STAMP
};

enum ScrCategory {
	CAT_NONE = 0, CAT_COMMAND = 1, CAT_INV_LIST = 2, CAT_INV_VOCAB = 3,
	CAT_HOTSPOT = 4, CAT_INV_ANIM = 5, CAT_TALK_ENTRY = 6, CAT_INV_SCROLLER = 7,
	CAT_12 = 12
};

enum ScreenTransition {
	kTransitionNone = 0,
	kTransitionFadeIn,
	kTransitionFadeOutIn,
	kTransitionBoxInBottomLeft,
	kTransitionBoxInBottomRight,
	kTransitionBoxInTopLeft,
	kTransitionBoxInTopRight,
	kTransitionPanLeftToRight,
	kTransitionPanRightToLeft,
	kTransitionCircleIn1,
	kTransitionCircleIn2,
	kTransitionCircleIn3,
	kTransitionCircleIn4,
	kVertTransition1,
	kVertTransition2,
	kVertTransition3,
	kVertTransition4,
	kVertTransition5,
	kVertTransition6,
	kVertTransition7,
	kNullPaletteCopy
};

enum StoryMode {
	STORYMODE_NAUGHTY = NAUGHTY, STORYMODE_NICE = NICE
};

// These original constants may be useful when cleaning up room logic
#ifdef old_animation
#define kernel_animation                kernel_anim[0].anim
#define kernel_animation_cycled         kernel_anim[0].cycled
#define kernel_repeat_animation         kernel_anim[0].repeat
#define kernel_animation_sprite_loaded  kernel_anim[0].sprite_loaded
#define kernel_animation_buffer_id      kernel_anim[0].buffer_id
#define kernel_animation_buffer         kernel_anim[0].buffer
#define kernel_animation_frame          kernel_anim[0].frame
#define kernel_animation_image          kernel_anim[0].image
#define kernel_animation_doomed         kernel_anim[0].doomed
#define kernel_animation_trigger_code   kernel_anim[0].trigger_code
#define kernel_animation_trigger_mode   kernel_anim[0].trigger_mode
#define kernel_animation_trigger_words  kernel_anim[0].trigger_words
#define kernel_animation_next_clock     kernel_anim[0].next_clock
#endif

enum {
	RETURNING_FROM_DIALOG = KERNEL_RESTORING_GAME,
	RETURNING_FROM_LOADING = KERNEL_STARTING_GAME
};

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS

#endif /* MADS_NEBULAR_SCENES8_H */
