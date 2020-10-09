/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 */

#ifndef GRIM_CHORE_H
#define GRIM_CHORE_H

#include "engines/grim/animation.h"

namespace Grim {

class Costume;
class Animation;
class Component;
class TextSplitter;

struct TrackKey {
	int time, value;
};

struct ChoreTrack {
	int compID;
	int numKeys;
	TrackKey *keys;
	Component *component;
};

class Chore {
public:
	Chore(char name[32], int id, Costume *owner, int length, int numTracks);
	virtual ~Chore();

	void load(TextSplitter &ts);
	virtual void play(uint msecs);
	virtual void playLooping(uint msecs);
	void setLooping(bool val) { _looping = val; }
	virtual void stop(uint msecs);
	virtual void update(uint time);
	void setLastFrame();
	void fadeIn(uint msecs);
	void fadeOut(uint msecs);
	void setPaused(bool paused);

	bool isPlaying() { return _playing; }
	bool isPaused() { return _paused; }
	bool isLooping() { return _looping; }

	void advance(uint msecs);

	const char *getName() const { return _name; }

	int getChoreId() { return _choreId; }

	Costume *getOwner() { return _owner; }

	virtual void saveState(SaveGame *state) const;
	virtual void restoreState(SaveGame *state);
protected:
	void setKeys(int startTime, int stopTime);
	virtual void fade(Animation::FadeMode, uint msecs);
	Component *getComponentForTrack(int i) const;

	Costume *_owner;

	int _choreId;
	int _length;
	int _numTracks;
	ChoreTrack *_tracks;
	char _name[32];

	bool _hasPlayed, _playing, _looping, _paused;
	int _currTime;

	friend class EMICostume;
};

} // end of namespace Grim

#endif
