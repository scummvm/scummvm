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
 * $URL$
 * $Id$
 */

#include "sound/mixer.h"
#include "testbed/sound.h"
#include "sound/softsynth/pcspk.h"

namespace Testbed {

bool SoundSubsystem::playPCSpkSound() {
	Audio::PCSpeaker *speaker = new Audio::PCSpeaker();
	Audio::Mixer *mixer = g_system->getMixer();
	Audio::SoundHandle handle;
	mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, speaker);
	speaker->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);
	g_system->delayMillis(1000);
	mixer->setChannelBalance(handle, -127);
	g_system->delayMillis(1000);
	mixer->setChannelBalance(handle, 127);
	g_system->delayMillis(1000);
	mixer->stopAll();
	return true;
}

SoundSubsystemTestSuite::SoundSubsystemTestSuite() {
	addTest("PCSpkrSound", &SoundSubsystem::playPCSpkSound, true);
}

}	// End of namespace Testbed
