/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/grim/grim.h"
#include "engines/grim/imuse/imuse.h"
#include "engines/grim/emi/sound/emisound.h"
#include "engines/grim/sound.h"

namespace Grim {

SoundPlayer *g_sound = nullptr;

bool SoundPlayer::startVoice(const char *soundName, int volume, int pan) {
	if (g_grim->getGameType() == GType_GRIM)
		return g_imuse->startVoice(soundName, volume, pan);
	else
		return g_emiSound->startVoice(soundName, volume, pan);
}

bool SoundPlayer::getSoundStatus(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM)
		return g_imuse->getSoundStatus(soundName);
	else
		return g_emiSound->getSoundStatus(soundName);
}

void SoundPlayer::stopSound(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->stopSound(soundName);
		return;
	} else {
		g_emiSound->stopSound(soundName);
	}
}

int32 SoundPlayer::getPosIn16msTicks(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM)
		return g_imuse->getPosIn16msTicks(soundName);
	else
		return g_emiSound->getPosIn16msTicks(soundName);
}

void SoundPlayer::setVolume(const char *soundName, int volume) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->setVolume(soundName, volume);
	} else {
		g_emiSound->setVolume(soundName, volume);
	}
}

void SoundPlayer::setPan(const char *soundName, int pan) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->setPan(soundName, pan);
	} else {
		g_emiSound->setPan(soundName, pan);
	}
}

void SoundPlayer::setMusicState(int stateId) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->setMusicState(stateId);
	} else {
		g_emiSound->setMusicState(stateId);
	}
}

void SoundPlayer::flushTracks() {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->flushTracks();
	} else {
		g_emiSound->flushTracks();
	}
}

void SoundPlayer::restoreState(SaveGame *savedState) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->restoreState(savedState);
	} else {
		g_emiSound->restoreState(savedState);
	}
}


void SoundPlayer::saveState(SaveGame *savedState) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->saveState(savedState);
	} else {
		g_emiSound->saveState(savedState);
	}
}

} // end of namespace Grim
