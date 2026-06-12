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
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/nebular/global.h"

namespace MADS {
namespace MADSV2 {
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

struct Action {
	struct ActiveAction {
		int &_verbId = player2.words[0];
		int &_objectNameId = player2.words[1];
		int &_indirectObjectId = player2.words[2];
	};
	ActiveAction _activeAction;

	struct SavedFields {
		int &_lookFlag = player.look_around;
		int &_mainObjectSource = player.main_object_source;
	};
	SavedFields _savedFields;

	int &_mainObjectSource = player.main_object_source;

	bool isAction(int word1) const;
	bool isAction(int word1, int word2) const;
	bool isAction(int word1, int word2, int word3) const;
	bool isObject(int word) const {
		return isAction(word);
	}
	bool isTarget(int word) const {
		return isAction(word);
	}

	int &_lookFlag = player.look_around;
	int &_inProgress = player.command_ready;
	int &_commandSource = player.command_source;
};
extern Action _action;

struct Globals {
	int16 _spriteIndexes[15];
	int16 _sequenceIndexes[30];

	int16 &operator[](int idx);
};
extern Globals _globals;

struct Scene {
	Scene *operator->() {
		return this;
	}
	const Scene *operator->() const {
		return this;
	}

	struct Animation {
		Animation *operator->() {
			return this;
		}
		const Animation *operator->() const {
			return this;
		}

		int16 _id = -1;
		bool operator!() const {
			return _id == -1;
		}
		bool operator!=(std::nullptr_t) const {
			return _id != -1;
		}
		bool operator==(std::nullptr_t) const {
			return _id == -1;
		}
		explicit operator bool() const {
			return _id != -1;
		}

		// TODO: Not sure what to map this to
		bool _resetFlag;

		int getCurrentFrame() const;
		void setNextFrameTimer(long time);
		void setCurrentFrame(int frameNum);
		void resetSpriteSetsCount();
	};
	Animation _animation[10];

	struct DynamicHotspots {
		int add(int vocab_id, int verb_id, int auto_sequence, const Common::Rect &r);
		void remove(int dyn_id);
		void setPosition(int id, const Common::Point &pt, int facing);
		int setCursor(int index, int cursor);
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
			uint32 timeout, const Common::String &msg);
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

		SequenceProxy(SequencePtr seqPtr) : _position(seqPtr->x, seqPtr->y) {
		}
	};

	struct Sequences {
		int16 addSpriteCycle(int series_id, int mirror, word ticks, word interval_ticks = 0,
			word start_ticks = 0, int expire = 0);
		int16 addReverseSpriteCycle(int series_id, int mirror, word ticks, word interval_ticks = 0,
			word start_ticks = 0, int expire = 0);
		int startPingPongCycle(int series_id, int mirror, word ticks, word interval_ticks = 0,
			word start_ticks = 0, int expire = 0);
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
		void updateTimeout(int old_sequence_id, int new_sequence_id);
		void scan();
		int startCycle(int srcSpriteIdx, bool flipped, int cycleIndex);
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
		void emptyConversationList();
		void setup(int inputMode);
	};
	UserInterface _userInterface;

	int16 &_priorSceneId = previous_room;
	int16 &_nextSceneId = new_room;
	long &_frameStartTime = kernel.clock;
	byte &_reloadSceneFlag = kernel.force_restart;
	byte &_roomChanged = kernel.teleported_in;
	int &_currentSceneId = room_id;

	int loadAnimation(const char *name, int trigger_code = 0);
	void freeAnimation();
	void changeVariant(int num);
	void drawElements(int transitionType, bool surfaceFlag);
	void resetScene();
	void clearSequenceList();
	void addActiveVocab(int vocab_id);
};
extern Scene _scene;

struct Game {
	Game *operator->() {
		return this;
	}
	const Game *operator->() const {
		return this;
	}

	struct Object {
		int16 &_roomNumber;

		Object(int objectNum);
	};

	struct Objects {
		void addToInventory(int object_id);
		bool isInRoom(int object_id) const;
		bool isInInventory(int object_id) const;
		int getIdFromDesc(int desc_id) const;
		void setRoom(int object_id, int roomNum);
		void removeFromInventory(int objectId, int newScene);

		Object operator[](int idx);
	};
	Objects _objects;

	struct Player {
		struct PlayerPoint {
			int &x = player.x;
			int &y = player.y;

			PlayerPoint &operator=(const Common::Point &pt) {
				x = pt.x;
				y = pt.y;
				return *this;
			}
			operator Common::Point() {
				return Common::Point(x, y);
			}

			bool operator==(const Common::Point &rhs) const {
				return x == rhs.x && y == rhs.y;
			}
			bool operator!=(const Common::Point &rhs) const {
				return x != rhs.x || y != rhs.y;
			}
		};
		PlayerPoint _playerPos;

		struct PlayerPrepare {
			int &x = player.prepare_walk_x;
			int &y = player.prepare_walk_y;

			PlayerPrepare &operator=(const Common::Point &pt) {
				x = pt.x;
				y = pt.y;
				return *this;
			}
			operator Common::Point() {
				return Common::Point(x, y);
			}

			bool operator==(const Common::Point &rhs) const {
				return x == rhs.x && y == rhs.y;
			}
		};
		PlayerPrepare _prepareWalkPos;

		int &_facing = player.facing;
		int &_prepareFacing = player.prepare_walk_facing;
		bool &_visible = player.walker_visible;
		bool &_stepEnabled = player.commands_allowed;
		char *const q = &player.series_name[0];
		byte &_spritesChanged = player.walker_must_reload;
		int &_beenVisible = player.walker_been_visible;
		byte &_loadsFirst = player.walker_loads_first;
		int &_needToWalk = player.need_to_walk;
		int &_readyToWalk = player.ready_to_walk;
		int &_special = player.special_code;
		long &_priorTimer = player.clock;
		int &_ticksAmount = player.frame_delay;
		char *const _spritesPrefix = &player.series_name[0];
		int &_moving = player.walking;
		int &_walkOffScreenSceneId = player.walk_off_edge_to_room;
		int &_prepareWalkFacing = player.prepare_walk_facing;
		int &_spritesStart = player.series_base;
		int &_frameNumber = player.sprite;
		int &_forceRefresh = player.sprite_changed;

		void startWalking(const Common::Point &pt, int facing);
		void walk(const Common::Point &pt, int facing);
		void cancelCommand();
		void update();
		void removePlayerSprites();
	};
	Player _player;

	Scene &_scene = Rooms::_scene;

	struct ScreenObjects {
		int &_inputMode = inter_input_mode;
	};
	ScreenObjects _screenObjects;

	struct VisitedScenes {
		byte &_sceneRevisited = player.been_here_before;

		void add(int roomNum);
		bool exists(int roomNum) const;
		void pop_back();
	};
	VisitedScenes _visitedScenes;

	int &_trigger = kernel.trigger;
	int &_triggerSetupMode = kernel.trigger_setup_mode;
	char *const _aaName = kernel.interface;
	int8 &_difficulty = game.difficulty;
	int8 _storyMode = 0;

	void loadQuoteSet(int quote1, ...);
	char *getQuote(int quote_id);
};
extern Game _game;

struct Resources {
	static char *formatName(int my_room, char type, int num, int ext, const char *text);
	static char *formatAAName(int num);
};

struct VM {
	VM *operator->() {
		return this;
	}
	const VM *operator->() const {
		return this;
	}

	struct Dialogs {
		struct DefaultPosition {
			int &x = text_default_x;
			int &y = text_default_y;

			DefaultPosition &operator=(const Common::Point &pt) {
				x = pt.x;
				y = pt.y;
				return *this;
			}
			operator Common::Point() {
				return Common::Point(x, y);
			}

			bool operator==(const Common::Point &rhs) const {
				return x == rhs.x && y == rhs.y;
			}
		};
		DefaultPosition _defaultPosition;

		Dialogs *operator->() {
			return this;
		}
		const Dialogs *operator->() const {
			return this;
		}

		void show(int id);
		void showItem(int object_id, int message, int speech = 0);
	};
	Dialogs _dialogs;

	struct Events {
		Events *operator->() {
			return this;
		}
		const Events *operator->() const {
			return this;
		}

		void setCursor2(int cursorNum);
	};
	Events _events;

	Game &_game = Rooms::_game;

	struct Palette {
		void setEntry(int color, int r, int g, int b);
		void refreshSceneColors();
		void lock();
	};
	Palette _paletteInstance;
	Palette *const _palette = &_paletteInstance;

	struct Sound {
		Sound *operator->() {
			return this;
		}
		const Sound *operator->() const {
			return this;
		}

		void command(int num, int distance = 0);
	};
	Sound _sound;

	int getRandomNumber(int min, int max);
	int getRandomNumber(int max);
};
extern VM _vm;

extern char *formAnimName(char type, int num);

enum {
	RETURNING_FROM_DIALOG = KERNEL_RESTORING_GAME,
	RETURNING_FROM_LOADING = KERNEL_STARTING_GAME
};

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif /* MADS_NEBULAR_SCENES8_H */
