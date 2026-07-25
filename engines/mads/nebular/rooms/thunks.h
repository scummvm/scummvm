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

#include "common/rect.h"
#include "mads/core/config.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/player.h"
#include "mads/core/sprite.h"
#include "mads/core/text.h"

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

struct Scene {
	Scene *operator->() {
		return this;
	}
	const Scene *operator->() const {
		return this;
	}

	struct Animation {
		int16 _id;
		int &_currentFrame;
		int &_repeatFlag;
		int *const _spriteListIndexes;
		int &_oldFrameEntry;
		uint16 &_frameEntriesCount;
		Image *const _frameEntries;

		Animation(int anim_id);

		Animation *operator->() {
			return this;
		}
		const Animation *operator->() const {
			return this;
		}

		bool operator!() const {
			return kernel_anim[_id].anim == nullptr;
		}
		bool operator!=(std::nullptr_t) const {
			return kernel_anim[_id].anim != nullptr;
		}
		bool operator==(std::nullptr_t) const {
			return kernel_anim[_id].anim == nullptr;
		}
		explicit operator bool() const {
			return kernel_anim[_id].anim != nullptr;
		}


		int getCurrentFrame() const;
		void setNextFrameTimer(long time);
		long getNextFrameTimer();
		void setCurrentFrame(int frameNum);
		void resetSpriteSetsCount();
	};
	struct Animations {
		Animation operator[](int anim_id) {
			return Animation(anim_id);
		}
	};
	Animations _animation;

	struct CustomDest {
		int &x = inter_point_x;
		int &y = inter_point_y;
	};
	CustomDest _customDest;

	struct DynamicHotspot {
		byte &_articleNumber;

		DynamicHotspot(int index);
	};

	struct DynamicHotspots {
		int add(int vocab_id, int verb_id, int auto_sequence, const Common::Rect &r);
		void remove(int dyn_id);
		int setPosition(int id, const Common::Point &pt, int facing);
		int setCursor(int id, int cursor);

		DynamicHotspot operator[](int idx);
	};
	DynamicHotspots _dynamicHotspots;

	struct Hotspots {
		void activate(int hotspot, int active);
	};
	Hotspots _hotspots;

	struct KernelMessages {
		struct TalkFont {
			TalkFont *operator->() {
				return this;
			}
			const TalkFont *operator->() const {
				return this;
			}
			TalkFont &operator=(FontPtr font);
			int getWidth(const Common::String &message, int spacing) const;
		};
		TalkFont _talkFont;

		struct KernelMessageProxy {
			long &_frameTimer;

			KernelMessageProxy(KernelMessagePtr kmsg) : _frameTimer(kmsg->update_time) {
			}
		};
		struct Entries {
			KernelMessageProxy operator[](uint handle);
		};
		Entries _entries;

		int add(const Common::Point &pt, uint fontColor, uint8 flags, int endTrigger,
			uint32 timeout, const char *msg);
		int addQuote(int quoteId, int endTrigger, uint32 timeout);
		void remove(int msgIndex);
		void reset();
		void setQuoted(int msgIndex, int numTicks, bool quoted);
		void setSeqIndex(int msgIndex, int seqIndex);

		void initRandomMessages(int maxSimultaneousMessages,
			const Common::Rect &bounds, int minYSpacing, int scrollRate,
			int color, int duration, int quoteId, ...);
		void randomServer();
		int checkRandom();
		bool generateRandom(int major, int minor);
	};
	KernelMessages _kernelMessages;

	struct PaletteCycles {
		int size() const;

	};
	PaletteCycles _paletteCycles;

	struct Rails {
		int getNext() const;
		void resetNext();
	};
	Rails _rails;

	struct SequenceProxy {
		struct SequencePosition {
			int &x;
			int &y;
			SequencePosition(int &sx, int &sy) : x(sx), y(sy) {}
		};
		SequencePosition _position;
		byte &_doneFlag;

		SequenceProxy(SequencePtr seqPtr) : _position(seqPtr->x, seqPtr->y), _doneFlag(seqPtr->expired) {
		}
	};

	struct Sequences {
		int16 addSpriteCycle(int series_id, int mirror, word ticks, word expire = 0,
			word start_ticks = 0, int interval_ticks = 0); 	// ***WARNING***: Last 3 params reversed
		int16 addReverseSpriteCycle(int series_id, int mirror, word ticks, word expire = 0,
			word start_ticks = 0, int interval_ticks = 0);	// ***WARNING***: Last 3 params reversed
		int startPingPongCycle(int series_id, int mirror, word ticks, word expire = 0,
			word start_ticks = 0, int interval_ticks = 0);	// ***WARNING***: Last 3 params reversed

		void remove(int sequence_id);
		SequenceProxy operator[](uint sequence_id);

		int16 addSubEntry(int sequence_id, int trigger_type, int trigger_sprite, int trigger_code);
		void setDepth(int sequence_id, int depth);
		void setScale(int sequence_id, int scale);
		void setAnimRange(int sequence_id, int first, int last);
		void addTimer(int ticks, int trigger_code);
		void setMsgLayout(int sequence_id);
		void setPosition(int sequence_id, const Common::Point &pt);
		void setMotion(int sequence_id, int flags, int delta_x_times_100, int delta_y_times_100);
		void updateTimeout(int new_sequence_id, int old_sequence_id);		// ***WARNING***: params reversed
		void scan();
		int startCycle(int srcSpriteIdx, bool flipped, int cycleIndex);
		void setDone(int sequence_id);
		int findByTrigger(int trigger);
	};
	Sequences _sequences;

	struct Sprite {
		struct CharInfo {
			CharInfo *operator->() {
				return this;
			}
			const CharInfo *operator->() const {
				return this;
			}

			int16 &_velocity;

			CharInfo(WalkerInfoPtr info) : _velocity(info->velocity) {
			}
		};
		CharInfo _charInfo;

		Sprite(SeriesPtr series) : _charInfo(series->walker) {
		}
		Sprite *operator->() {
			return this;
		}
		const Sprite *operator->() const {
			return this;
		}
	};

	struct Sprites {
		int16 addSprites(const char *name, int load_flags = 0);
		void remove(int sprite_id);
		Sprite operator[](int idx);
	};
	Sprites _sprites;

	struct SpriteSlots {
		void clear();
		void fullRefresh();
	};
	SpriteSlots _spriteSlots;

	struct UserInterface {
		int &_selectedInvIndex = active_inven;
		void emptyConversationList();
		void setup(int inputMode);
		void selectObject(int item_id);
	};
	UserInterface _userInterface;

	int16 &_priorSceneId = previous_room;
	int16 &_nextSceneId = new_room;
	long &_frameStartTime = kernel.clock;
	byte &_reloadSceneFlag = kernel.force_restart;
	byte &_roomChanged = kernel.teleported_in;
	int &_currentSceneId = room_id;
	int &_textSpacing = kernel_message_spacing;

	int loadAnimation(const char *name, int trigger_code = 0);
	void freeAnimation();
	void changeVariant(int num);
	void drawElements(int transitionType, bool surfaceFlag);
	void resetScene();
	void clearSequenceList();
	void addActiveVocab(int vocab_id);
};
extern Scene _scene;

enum {
	RETURNING_FROM_DIALOG = KERNEL_RESTORING_GAME,
	RETURNING_FROM_LOADING = KERNEL_STARTING_GAME
};

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS

#endif /* MADS_NEBULAR_SCENES8_H */
