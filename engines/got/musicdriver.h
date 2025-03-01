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

#ifndef GOT_MUSICDRIVER_H
#define GOT_MUSICDRIVER_H

#include "common/timer.h"

namespace Got {

class MusicDriver_Got {
public:
	MusicDriver_Got(uint8 timerFrequency);
	virtual ~MusicDriver_Got() { };

	virtual int open() = 0;
	bool isOpen() const;
	virtual void close() = 0;

	virtual void syncSoundSettings() = 0;

	virtual void send(uint16 b) = 0;
	virtual void stopAllNotes() = 0;

	virtual void setTimerFrequency(uint8 timerFrequency);
	void onTimer();
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);

protected:
	// True if the driver has been successfully opened.
	bool _isOpen;
	// The number of timer callbacks per second.
	int _timerFrequency;

	// External timer callback
	void *_timer_param;
	Common::TimerManager::TimerProc _timer_proc;
};

class MusicDriver_Got_NULL : public MusicDriver_Got {
public:
	MusicDriver_Got_NULL(uint8 timerFrequency) : MusicDriver_Got(timerFrequency) { };
	~MusicDriver_Got_NULL() { };

	int open() override;
	void close() override;

	void syncSoundSettings() override { };
	void send(uint16 b) override { };
	void stopAllNotes() override { };

	void setTimerFrequency(uint8 timerFrequency) override;

	static void timerCallback(void *data);
};

} // namespace Got

#endif
