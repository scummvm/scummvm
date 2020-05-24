/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_SOUND_H
#define ULTIMA4_SOUND_H

#include "ultima/shared/std/containers.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

enum Sound {
	SOUND_TITLE_FADE,       // the intro title fade
	SOUND_WALK_NORMAL,      // walk, world and town
	SOUND_WALK_SLOWED,      // walk, slow progress
	SOUND_WALK_COMBAT,      // walk, combat
	SOUND_BLOCKED,          // location blocked
	SOUND_ERROR,            // error/bad command
	SOUND_PC_ATTACK,        // PC attacks
	SOUND_PC_STRUCK,        // PC damaged
	SOUND_NPC_ATTACK,       // NPC attacks
	SOUND_NPC_STRUCK,       // NPC damaged
	SOUND_ACID,             // effect, acid damage
	SOUND_SLEEP,            // effect, sleep
	SOUND_POISON_EFFECT,    // effect, poison
	SOUND_POISON_DAMAGE,    // damage, poison
	SOUND_EVADE,            // trap evaded
	SOUND_FLEE,             // flee combat
	SOUND_ITEM_STOLEN,      // item was stolen from a PC, food or gold
	SOUND_LBHEAL,           // LB heals party
	SOUND_LEVELUP,          // PC level up
	SOUND_MOONGATE,         // moongate used

	SOUND_CANNON,
	SOUND_RUMBLE,
	SOUND_PREMAGIC_MANA_JUMBLE,
	SOUND_MAGIC,
	SOUND_WHIRLPOOL,
	SOUND_STORM,

	//    SOUND_MISSED,
	//    SOUND_CREATUREATTACK,
	//    SOUND_PLAYERHIT,
	SOUND_MAX
};

void soundPlay(Sound sound, bool onlyOnce = true, int specificDurationInTicks = -1);

void soundStop(int channel = 1);

class SoundManager {
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	Std::vector<Common::String> _soundFilenames;
	Std::vector<Audio::SeekableAudioStream *> _sounds;
private:
	bool load(Sound sound);

	void play_sys(Sound sound, bool onlyOnce, int specificDurationMilli);
	bool load_sys(Sound sound, const Common::String &filename);
	void stop_sys(int channel);
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();

	void play(Sound sound, bool onlyOnce = true, int specificDurationInTicks = -1);
	void stop(int channel = 1);
};

extern SoundManager *g_sound;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
