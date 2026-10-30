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

#include "common/memstream.h"
#include "common/serializer.h"

#include "engines/grim/grim.h"
#include "engines/grim/imuse/imuse_engine.h"
#include "engines/grim/emi/sound/emisound.h"
#include "engines/grim/sound.h"

namespace Grim {

SoundPlayer *g_sound = nullptr;

bool SoundPlayer::startVoice(const char *soundName, int volume, int pan, bool background) {
	if (g_grim->getGameType() == GType_GRIM) {
		if (background) {
			if (!g_imuse->soundsLoadProcessStartSound(soundName, 127, IMUSE_GROUP_SPEECH))
				return false;

			g_imuse->soundsSetParam(soundName, IMUSE_P_VOLUME, volume);
			return true;
		}

		bool started = !g_imuse->soundStartVoiceStream(soundName, volume, pan);
		_streamedVoice = started ? soundName : "";
		return started;
	} else {
		return g_emiSound->startVoice(soundName, volume, pan);
	}
}

bool SoundPlayer::getSoundStatus(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM) {
		if (!*soundName)
			return false;

		if (_streamedVoice == soundName)
			return g_imuse->imGetParam(kTalkSoundID, IMUSE_P_SND_TRACK_NUM) > 0;

		return g_imuse->soundsIsSoundPlaying(soundName);
	} else {
		return g_emiSound->getSoundStatus(soundName);
	}
}

void SoundPlayer::stopSound(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM) {
		if (_streamedVoice == soundName)
			g_imuse->soundStopVoiceStream();
		else
			g_imuse->soundsStopSound(soundName);
	} else {
		g_emiSound->stopSound(soundName);
	}
}

int32 SoundPlayer::getPosIn16msTicks(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM) {
		if (_streamedVoice == soundName)
			return g_imuse->soundGetVoiceTime() >> 4;

		return g_imuse->soundsGetParam(soundName, IMUSE_P_SND_POS_IN_MS) >> 4;
	} else {
		return g_emiSound->getPosIn16msTicks(soundName);
	}
}

void SoundPlayer::setVolume(const char *soundName, int volume) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->soundsSetParam(soundName, IMUSE_P_VOLUME, volume);
	} else {
		g_emiSound->setVolume(soundName, volume);
	}
}

void SoundPlayer::setPan(const char *soundName, int pan) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->soundsSetParam(soundName, IMUSE_P_PAN, pan);
	} else {
		g_emiSound->setPan(soundName, pan);
	}
}

void SoundPlayer::setMusicState(int stateId) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->imSetState(stateId);
	} else {
		g_emiSound->setMusicState(stateId);
	}
}

void SoundPlayer::flushTracks() {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->soundsPeriodic();
	} else {
		g_emiSound->flushTracks();
	}
}

void SoundPlayer::restoreState(SaveGame *savedState) {
	if (g_grim->getGameType() == GType_GRIM) {
		uint32 sectionSize = savedState->beginSection('IMUS');

		if (savedState->saveMinorVersion() < 30) {
			// Post-load fix for older savegames using the previous iMUSE implementation...
			int32 stateIndex = savedState->readLESint32();
			int32 seqIndex = savedState->readLESint32();
			savedState->endSection();

			g_imuse->soundsRestoreLegacy(stateIndex, seqIndex);
			return;
		}

		byte *sectionData = (byte *)malloc(sectionSize);
		savedState->read(sectionData, sectionSize);
		savedState->endSection();

		Common::MemoryReadStream stream(sectionData, sectionSize, DisposeAfterUse::YES);
		Common::Serializer ser(&stream, nullptr);
		ser.setVersion(savedState->saveMinorVersion());
		g_imuse->soundsSaveLoad(ser);
	} else {
		g_emiSound->restoreState(savedState);
	}
}

void SoundPlayer::saveState(SaveGame *savedState) {
	if (g_grim->getGameType() == GType_GRIM) {
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		Common::Serializer ser(nullptr, &stream);
		ser.setVersion(SaveGame::SAVEGAME_MINOR_VERSION);
		g_imuse->soundsSaveLoad(ser);

		savedState->beginSection('IMUS');
		savedState->write(stream.getData(), stream.size());
		savedState->endSection();
	} else {
		g_emiSound->saveState(savedState);
	}
}

} // end of namespace Grim
