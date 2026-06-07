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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/nebular/global.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

enum EXTTYPE {
	EXT_NONE = -1, EXT_SS = 1, EXT_AA = 2, EXT_DAT = 3, EXT_HH = 4,
	EXT_ART = 5, EXT_INT = 6
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

struct Action {
	struct ActiveAction {
		int &_verbId = player2.words[0];
		int &_objectNameId = player2.words[1];
		int &_indirectObjectId = player2.words[2];
	};
	ActiveAction _activeAction;
	int &_mainObjectSource = player.main_object_source;

	bool isAction(int word1) const;
	bool isAction(int word1, int word2) const;
	bool isAction(int word1, int word2, int word3) const;
	bool isObject(int word) const {
		return isAction(word);
	}

	int &_lookFlag = player.look_around;
	int &_inProgress = player.command_ready;
};
extern Action _action;

struct Globals {
	int16 _spriteIndexes[15];
	int16 _sequenceIndexes[30];

	int16 &operator[](int idx);
};
extern Globals _globals;

struct Game {
	struct Objects {
		void addToInventory(int object_id);
		bool isInRoom(int object_id) const;
		bool isInInventory(int object_id) const;
		int getIdFromDesc(int desc_id) const;
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

			bool operator==(const Common::Point &rhs) const {
				return x == rhs.x && y == rhs.y;
			}
		};
		PlayerPoint _playerPos;
		int &_facing = player.facing;
		int &_visible = player.walker_visible;
		int &_stepEnabled = player.commands_allowed;
		char *const q = &player.series_name[0];
		byte &_spritesChanged = player.walker_must_reload;
		byte &_loadsFirst = player.walker_loads_first;
		int &_needToWalk = player.need_to_walk;
		int &_readyToWalk = player.ready_to_walk;
		int &_special = player.special_code;
		long &_priorTimer = player.clock;
		int &_ticksAmount = player.frame_delay;
		char *const _spritesPrefix = &player.series_name[0];
	};
	Player _player;

	struct VisitedScenes {
		bool exists(int roomNum) const;
	};
	VisitedScenes _visitedScenes;

	int &_trigger = kernel.trigger;
	int &_triggerSetupMode = kernel.trigger_setup_mode;
	char *const _aaName = kernel.interface;
	int8 &_difficulty = game.difficulty;

	void loadQuoteSet(int quote1, ...);
	char *getQuote(int quote_id);
};

struct Resources {
	static char *formatName(int my_room, char type, int num, int ext, const char *text);
	static char *formatAAName(int num);
};

struct Scene {
	struct Animation {
		int16 _id = -1;
		bool operator!=(std::nullptr_t) const {
			return _id != -1;
		}
		bool operator==(std::nullptr_t) const {
			return _id == -1;
		}
		Animation *operator->() {
			return this;
		}
		const Animation *operator->() const {
			return this;
		}

		int getCurrentFrame() const;
	};
	Animation _animation[10];

	struct DynamicHotspots {
		int add(int vocab_id, int verb_id, int auto_sequence, const Common::Rect &r);
		void remove(int dyn_id);
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

		int add(const Common::Point &pt, uint fontColor, uint8 flags, int endTrigger,
			uint32 timeout, const Common::String &msg);
		void reset();
	};
	KernelMessages _kernelMessages;

	struct Sequences {
		int16 addSpriteCycle(int series_id, int mirror, word ticks, word interval_ticks,
			word start_ticks = 0, int expire = 0);
		int16 addReverseSpriteCycle(int series_id, int mirror, word ticks, word interval_ticks,
			word start_ticks, int expire);
		int startPingPongCycle(int series_id, int mirror, word ticks, word interval_ticks,
			word start_ticks = 0, int expire = 0);
		void remove(int sequence_id);

		int16 addSubEntry(int sequence_id, int trigger_type, int trigger_sprite, int trigger_code);
		void setDepth(int sequence_id, int depth);
		void setScale(int sequence_id, int scale);
		void setAnimRange(int sequence_id, int first, int last);
		void addTimer(int ticks, int trigger_code);
		void setMsgLayout(int sequence_id);
	};
	Sequences _sequences;

	struct Sprites {
		int16 addSprites(const char *name, int load_flags = 0);
	};
	Sprites _sprites;

	int16 &_priorSceneId = previous_room;
	int16 &_nextSceneId = new_room;
	long &_frameStartTime = kernel.clock;

	int loadAnimation(const char *name, int trigger_code);
};

struct VM {
	VM *operator->() {
		return this;
	}
	const VM *operator->() const {
		return this;
	}

	struct Dialogs {
		Dialogs *operator->() {
			return this;
		}
		const Dialogs *operator->() const {
			return this;
		}

		void show(int id);
		void showItem(int object_id, int message);
	};
	Dialogs _dialogs;

	struct Palette {
		void setEntry(int color, int r, int g, int b);
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

		void command(int num);
	};
	Sound _sound;

	int getRandomNumber(int min, int max);
};

extern Game _game;
extern Scene *const _scene;
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
