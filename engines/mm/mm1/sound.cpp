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

#include "common/system.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

static const uint32 kBiosTimerTickMicros = 54925;

static const Shared::Classic::PitSequenceEntry kMM1Sound0[] = {
	{ 1, 0x0a98 }, { 1, 0x0970 }, { 1, 0x08e8 }, { 1, 0x07ef },
	{ 14, 0x0712 }, { 2, 0x0000 }, { 6, 0x0712 }, { 2, 0x0000 },
	{ 6, 0x0712 }, { 1, 0x0000 }, { 1, 0x0e24 }, { 1, 0x0a98 },
	{ 6, 0x08e8 }, { 2, 0x0000 }, { 4, 0x0a98 }, { 4, 0x0970 },
	{ 4, 0x08e8 }, { 3, 0x07ef }, { 1, 0x0000 }, { 3, 0x0712 },
	{ 1, 0x0000 }, { 2, 0x06ac }, { 1, 0x08e8 }, { 1, 0x0712 },
	{ 8, 0x05f2 }, { 4, 0x06ac }, { 4, 0x0712 }, { 8, 0x06ac },
	{ 4, 0x0712 }, { 2, 0x07ef }, { 1, 0x12e0 }, { 1, 0x0b3a },
	{ 12, 0x0712 }, { 4, 0x06ac }, { 4, 0x0712 }, { 3, 0x07ef },
	{ 1, 0x0000 }, { 3, 0x08e8 }, { 1, 0x0000 }, { 3, 0x0970 },
	{ 1, 0x0000 }, { 14, 0x0a98 }, { 2, 0x0000 }, { 6, 0x0e24 },
	{ 2, 0x0000 }, { 5, 0x0a98 }, { 1, 0x0000 }, { 1, 0x0e24 },
	{ 1, 0x0be4 }, { 3, 0x08e8 }, { 1, 0x0000 }, { 3, 0x0be4 },
	{ 1, 0x0000 }, { 2, 0x08e8 }, { 2, 0x0970 }, { 2, 0x08e8 },
	{ 2, 0x07ef }, { 14, 0x0712 }, { 2, 0x0000 }, { 2, 0x06ac },
	{ 2, 0x0712 }, { 2, 0x07ef }, { 2, 0x08e8 }, { 2, 0x0712 },
	{ 2, 0x07ef }, { 2, 0x08e8 }, { 2, 0x0970 }, { 2, 0x07ef },
	{ 2, 0x08e8 }, { 2, 0x0970 }, { 2, 0x0a98 }, { 2, 0x08e8 },
	{ 2, 0x0970 }, { 2, 0x0a98 }, { 2, 0x0b3a }, { 4, 0x06ac },
	{ 5, 0x0712 }, { 6, 0x07ef }, { 7, 0x08e8 }, { 2, 0x0e24 },
	{ 3, 0x0a98 }, { 32, 0x0868 }, { 4, 0x0000 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound1[] = {
	{ 2, 0x0be4 }, { 4, 0x0000 }, { 1, 0x08e8 }, { 1, 0x0000 },
	{ 8, 0x05f2 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound2[] = {
	{ 2, 0x0be4 }, { 4, 0x0000 }, { 1, 0x0be4 }, { 1, 0x0000 },
	{ 1, 0x0be4 }, { 1, 0x0000 }, { 1, 0x0be4 }, { 1, 0x0000 },
	{ 12, 0x07ef }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound3[] = {
	{ 1, 0x11d1 }, { 1, 0x0be4 }, { 3, 0x0712 }, { 6, 0x0000 },
	{ 2, 0x0712 }, { 1, 0x0000 }, { 12, 0x0712 }, { 6, 0x0000 },
	{ 3, 0x0712 }, { 3, 0x0000 }, { 2, 0x06ac }, { 2, 0x0000 },
	{ 2, 0x0be4 }, { 2, 0x0000 }, { 2, 0x06ac }, { 1, 0x11d1 },
	{ 1, 0x0be4 }, { 3, 0x0712 }, { 6, 0x0000 }, { 2, 0x0712 },
	{ 1, 0x0000 }, { 12, 0x0712 }, { 6, 0x0000 }, { 3, 0x0712 },
	{ 2, 0x0000 }, { 1, 0x17c8 }, { 1, 0x0be4 }, { 2, 0x07ef },
	{ 2, 0x0000 }, { 2, 0x08e8 }, { 2, 0x0000 }, { 2, 0x07ef },
	{ 2, 0x0000 }, { 1, 0x0e24 }, { 1, 0x0be4 }, { 3, 0x08e8 },
	{ 6, 0x0000 }, { 2, 0x0be4 }, { 1, 0x0000 }, { 1, 0x8e84 },
	{ 1, 0x5f1e }, { 1, 0x4742 }, { 1, 0x2f8f }, { 1, 0x23a2 },
	{ 1, 0x17c8 }, { 1, 0x11d1 }, { 1, 0x0e24 }, { 1, 0x0be4 },
	{ 12, 0x08e8 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound4[] = {
	{ 16, 0x06ac }, { 3, 0x0000 }, { 4, 0x0868 }, { 4, 0x08e8 },
	{ 4, 0x0970 }, { 16, 0x08e8 }, { 3, 0x0000 }, { 4, 0x0b3a },
	{ 4, 0x0be4 }, { 4, 0x0b3a }, { 24, 0x0d59 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound5[] = {
	{ 2, 0x8e84 }, { 2, 0x5f1e }, { 2, 0x4742 }, { 2, 0x2f8f },
	{ 2, 0x23a2 }, { 2, 0x17c8 }, { 2, 0x11d1 }, { 2, 0x0be4 },
	{ 2, 0x08e8 }, { 2, 0x05f2 }, { 2, 0x0474 }, { 2, 0x0389 },
	{ 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound6[] = {
	{ 6, 0x03e8 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound7[] = {
	{ 8, 0x4e20 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound8[] = {
	{ 8, 0x0b3a }, { 8, 0x0be4 }, { 8, 0x0c98 }, { 8, 0x0be4 },
	{ 2, 0x0000 }, { 16, 0x0d59 }, { 0, 0x0000 }
};

static const Shared::Classic::PitSequenceEntry kMM1Sound9[] = {
	{ 5, 0x08e8 }, { 1, 0x0000 }, { 1, 0x07ef }, { 1, 0x0000 },
	{ 1, 0x0be4 }, { 1, 0x0000 }, { 1, 0x07ef }, { 1, 0x0000 },
	{ 12, 0x0712 }, { 0, 0x0000 }
};

Sound::Sound(Audio::Mixer *mixer) : Shared::Xeen::Sound(mixer) {
	_speaker.init();
}

void Sound::playSequence(const Shared::Classic::PitSequenceEntry *sequence, bool append, bool restart, bool loop) {
	if (!g_engine->_sound || !g_engine->_sound->_fxOn)
		return;
	if (g_engine->isEnhanced())
		return;

	if (restart)
		g_engine->_sound->_speaker.stop();

	if (!append && !restart && g_engine->_sound->_speaker.isPlaying())
		return;

	g_engine->_sound->_speaker.playPitSequence(sequence, kBiosTimerTickMicros, append, loop);
}

void Sound::sound(SoundId soundNum) {
	if (g_engine->isEnhanced()) {
		if (soundNum == SOUND_1) {
			g_engine->_sound->playSound("gulp.voc");
			return;
		}
	}

	if (!g_engine->_sound || !g_engine->_sound->_fxOn)
		return;

	switch (soundNum) {
	case SOUND_1:
		playSequence(kMM1Sound7, false, true);
		break;
	case SOUND_2:
		playSequence(kMM1Sound6, false, true);
		break;
	case SOUND_3:
		playSequence(kMM1Sound6, false, true);
		playSequence(kMM1Sound6, true);
		playSequence(kMM1Sound6, true);
		break;
	default:
		break;
	}
}

void Sound::sound2(SoundId soundNum) {
	switch (soundNum) {
	case SOUND_TITLE:
		playSequence(kMM1Sound0, false, false, true);
		break;
	case SOUND_1:
		playSequence(kMM1Sound1, false, true);
		break;
	case SOUND_2:
		playSequence(kMM1Sound2, true);
		break;
	case SOUND_3:
		playSequence(kMM1Sound3, true);
		break;
	case SOUND_4:
		playSequence(kMM1Sound4, true);
		break;
	case SOUND_5:
		playSequence(kMM1Sound5, true);
		break;
	case SOUND_8:
		playSequence(kMM1Sound8, true);
		break;
	case SOUND_9:
		playSequence(kMM1Sound9);
		break;
	default:
		break;
	}
}

void Sound::update() {
	if (g_engine->_sound)
		g_engine->_sound->_speaker.update();
}

void Sound::stopSound() {
	if (g_engine->_sound) {
		g_engine->_sound->_speaker.stop();
	}
}

} // namespace MM1
} // namespace MM
