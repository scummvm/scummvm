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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* Sound engine */
#ifndef SCI_AUDIO_H
#define SCI_AUDIO_H

namespace Sci {

class ResourceManager;
class SegManager;

class AudioPlayer {
public:
	AudioPlayer(ResourceManager *resMan);
	~AudioPlayer();
		
	void setAudioRate(uint16 rate) { _audioRate = rate; }
	Audio::SoundHandle* getAudioHandle() { return &_audioHandle; }
	int getAudioPosition();
	int startAudio(uint16 module, uint32 tuple);
	void stopAudio() { g_system->getMixer()->stopHandle(_audioHandle); }
	void pauseAudio() { g_system->getMixer()->pauseHandle(_audioHandle, true); }
	void resumeAudio() { g_system->getMixer()->pauseHandle(_audioHandle, false); }

	void setSoundSync(ResourceId id, reg_t syncObjAddr, SegManager *segMan);
	void doSoundSync(reg_t syncObjAddr, SegManager *segMan);
	void stopSoundSync();

private:
	ResourceManager *_resMan;
	uint16 _audioRate;
	Audio::SoundHandle _audioHandle;
	Audio::AudioStream* getAudioStream(uint32 number, uint32 volume, int *sampleLen);
	Resource *_syncResource; /**< Used by kDoSync for speech syncing in CD talkie games */
	uint _syncOffset;
};

} // End of namespace Sci

#endif // SCI_SFX_CORE_H
