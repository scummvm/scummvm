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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_SOUND_H
#define TSAGE_SOUND_H

#include "common/scummsys.h"
#include "tsage/saveload.h"
#include "tsage/core.h"

namespace tSage {

class SoundManager : public SaveListener {
public:
	void dispatch() {}
	virtual void listenerSynchronize(Serializer &s);
	virtual void postInit();

	void proc2() {}
	static void saveNotifier(bool postFlag);
	void saveNotifierProc(bool postFlag);
	static void loadNotifier(bool postFlag);
	void loadNotifierProc(bool postFlag);
};

class Sound: public EventHandler {
public:

};

class ASound: public Sound {
public:
	Sound _sound;
	Action *_action;
	int _field280;

	ASound();
	virtual void synchronize(Serializer &s);
	virtual void dispatch();

	void play(int soundNum, Action *action = NULL, int volume = 127);
	void stop();
	void prime(int v, Action *action = NULL);
	void unPrime();
	void go();
	void hault(void);
	int getSoundNum() const;
	bool isPlaying() const;
	bool isPaused() const;
	bool isMuted() const;
	void pause();
	void mute();
	void fadeIn();
	void fadeOut(EventHandler *evtHandler);
	void fade(int v1, int v2, int v3, int v4, EventHandler *evtHandler);
	void setTimeIndex(uint32 timeIndex);
	uint32 getTimeIndex() const;
	void setPri(int v);
	void setLoop(bool flag);
	int getPri() const;
	bool getLoop();
	void setVolume(int volume);
	int getVol() const;
	void holdAt(int v);
	void release();
};

} // End of namespace tSage

#endif
