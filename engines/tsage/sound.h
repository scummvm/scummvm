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
	void play(int soundNum, int volume = 127);
	void stop();
	void prime(int soundNum);
	void prime(int soundNum, int v2);
	void _unPrime();
	void unPrime();
	void go();
	void halt(void);
	int getSoundNum() const;
	bool isPlaying() const;
	bool isPrimed() const;
	bool isPaused() const;
	bool isMuted() const;
	void pause();
	void mute();
	void fadeIn();
	void fadeOut(EventHandler *evtHandler);
	void fade(int v1, int v2, int v3, int v4);
	void setTimeIndex(uint32 timeIndex);
	uint32 getTimeIndex() const;
	bool getCueValue() const;
	void setCueValue(bool flag);
	void setVol(int volume);
	int getVol() const;
	void setPri(int v);
	void setLoop(bool flag);
	int getPri() const;
	bool getLoop();
	void holdAt(int v);
	void release();
};

class ASound: public EventHandler {
public:
	Sound _sound;
	Action *_action;
	bool _cueFlag;

	ASound();
	virtual void synchronize(Serializer &s);
	virtual void dispatch();

	void play(int soundNum, Action *action = NULL, int volume = 127);
	void stop();
	void prime(int soundNum, Action *action = NULL);
	void unPrime();
	void go() { _sound.go(); }
	void hault(void) { _sound.halt(); }
	int getSoundNum() const { return _sound.getSoundNum(); }
	bool isPlaying() const { return _sound.isPlaying(); }
	bool isPaused() const { return _sound.isPaused(); }
	bool isMuted() const { return _sound.isMuted(); }
	void pause() { _sound.pause(); }
	void mute() { _sound.mute(); }
	void fadeIn() { fade(127, 5, 10, 0, NULL); }
	void fadeOut(Action *action) { fade(0, 5, 10, 1, action); }
	void fade(int v1, int v2, int v3, int v4, Action *action); 
	void setTimeIndex(uint32 timeIndex) { _sound.setTimeIndex(timeIndex); }
	uint32 getTimeIndex() const { return _sound.getTimeIndex(); }
	void setPri(int v) { _sound.setPri(v); }
	void setLoop(bool flag) { _sound.setLoop(flag); }
	int getPri() const { return _sound.getPri(); }
	bool getLoop() { return _sound.getLoop(); }
	void setVol(int volume) { _sound.setVol(volume); }
	int getVol() const { return _sound.getVol(); }
	void holdAt(int v) { _sound.holdAt(v); }
	void release() { _sound.release(); }
};

} // End of namespace tSage

#endif
