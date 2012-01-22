/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/grim.h"
#include "engines/grim/imuse/imuse.h"
#include "engines/grim/emisound/emisound.h"
#include "engines/grim/sound.h"

namespace Grim {
	
SoundPlayer *g_sound = NULL;
	
SoundPlayer::SoundPlayer() {
	// TODO: Replace this with g_emiSound when we get a full working sound-system for more than voices.
	if (g_grim->getGameType() == GType_MONKEY4)
		_emiSound = new EMISound();
	else
		_emiSound = NULL;
}
	
SoundPlayer::~SoundPlayer() {
	delete _emiSound;
}

bool SoundPlayer::startVoice(const char *soundName, int volume, int pan) {
	if (g_grim->getGameType() == GType_GRIM)
		return g_imuse->startVoice(soundName, volume, pan);
	else
		return _emiSound->startVoice(soundName, volume, pan);
}
	
bool SoundPlayer::getSoundStatus(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM)
		return g_imuse->getSoundStatus(soundName);
	else
		return _emiSound->getSoundStatus(soundName);
}
	
void SoundPlayer::stopSound(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->stopSound(soundName);
		return;
	} else {
		_emiSound->stopSound(soundName);
	}
}

int32 SoundPlayer::getPosIn60HzTicks(const char *soundName) {
	if (g_grim->getGameType() == GType_GRIM)
		return g_imuse->getPosIn60HzTicks(soundName);
	else
		return _emiSound->getPosIn60HzTicks(soundName);
}
	
void SoundPlayer::setVolume(const char *soundName, int volume) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->setVolume(soundName, volume);
	} else {
		_emiSound->setVolume(soundName, volume);
	}
}

void SoundPlayer::setPan(const char *soundName, int pan) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->setPan(soundName, pan);
	} else {
		_emiSound->setPan(soundName, pan);
	}
}
	
void SoundPlayer::setMusicState(int stateId) {
	if (g_grim->getGameType() == GType_GRIM) {
		g_imuse->setMusicState(stateId);
	} else {
		_emiSound->setMusicState(stateId);
	}
}
	
// EMI-only
uint32 SoundPlayer::getMsPos(int stateId) {
	assert(_emiSound); // This shouldn't ever be called from Grim.
	return _emiSound->getMsPos(stateId);
}
	
} // end of namespace Grim