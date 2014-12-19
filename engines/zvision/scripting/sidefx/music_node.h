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
 */

#ifndef ZVISION_MUSIC_NODE_H
#define ZVISION_MUSIC_NODE_H

#include "audio/mixer.h"
#include "zvision/scripting/sidefx.h"
#include "zvision/graphics/subtitles.h"

namespace Common {
class String;
}

namespace ZVision {

class MusicNodeBASE : public SideFX {
public:
	MusicNodeBASE(ZVision *engine, uint32 key, SideFXType type) : SideFX(engine, key, type) {}
	~MusicNodeBASE() {}

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	virtual bool process(uint32 deltaTimeInMillis) = 0;

	virtual void setVolume(uint8 volume) = 0;

	virtual void setPanTrack(int16 pos) = 0;
	virtual void unsetPanTrack() = 0;

	virtual void setFade(int32 time, uint8 target) = 0;
};

class MusicNode : public MusicNodeBASE {
public:
	MusicNode(ZVision *engine, uint32 key, Common::String &file, bool loop, int8 volume);
	~MusicNode();

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis);

	void setVolume(uint8 volume);

	void setPanTrack(int16 pos);
	void unsetPanTrack();

	void setFade(int32 time, uint8 target);

private:
	bool _pantrack;
	int32 _pantrackPosition;
	int32 _attenuate;
	uint8 _volume;
	bool _loop;
	bool _crossfade;
	uint8 _crossfadeTarget;
	int32 _crossfadeTime;
	bool _stereo;
	Audio::SoundHandle _handle;
	Subtitle *_sub;
};

class MusicMidiNode : public MusicNodeBASE {
public:
	MusicMidiNode(ZVision *engine, uint32 key, int8 program, int8 note, int8 volume);
	~MusicMidiNode();

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis);

	void setVolume(uint8 volume);

	void setPanTrack(int16 pos);
	void unsetPanTrack();

	void setFade(int32 time, uint8 target);

private:
	int8 _chan;
	int8 _noteNumber;
	int8 _pan;
	int8 _volume;
	int8 _prog;
};

class PanTrackNode : public SideFX {
public:
	PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos);
	~PanTrackNode();

private:
	uint32 _slot;
};

} // End of namespace ZVision

#endif
