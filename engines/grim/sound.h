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

#ifndef GRIM_SOUND_H
#define GRIM_SOUND_H

namespace Grim {

class EMISound;
	
class SoundPlayer {
	EMISound *_emiSound;
public:
	SoundPlayer();
	~SoundPlayer();
	bool startVoice(const char *soundName, int volume = 127, int pan = 64);
	bool getSoundStatus(const char *soundName);
	void stopSound(const char *soundName);
	int32 getPosIn60HzTicks(const char *soundName);	
	
	void setVolume(const char *soundName, int volume);
	void setPan(const char *soundName, int pan);
};

extern SoundPlayer *g_sound;
	
} // end of namespace Grim

#endif