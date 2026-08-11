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

#ifndef ZVISION_MUSIC_NODE_H
#define ZVISION_MUSIC_NODE_H

#include "audio/mixer.h"
#include "math/angle.h"
#include "zvision/scripting/scripting_effect.h"
#include "zvision/sound/volume_manager.h"
#include "zvision/text/subtitle_manager.h"

namespace Common {
class String;
}

namespace ZVision {

class MusicNodeBASE : public ScriptingEffect {
public:
	MusicNodeBASE(ZVision *engine, uint32 key, ScriptingEffectType type) : ScriptingEffect(engine, key, type) {}
	~MusicNodeBASE() override {}

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis) override = 0;

	virtual void setVolume(uint8 volume) = 0;
	uint8 getVolume() {
		return _volume;
	}
	virtual void setFade(int32 time, uint8 target) = 0;
	virtual void setBalance(int8 balance);  // NB Overrides effects of setDirection()
	void setDirection(Math::Angle azimuth, uint8 magnitude = 255);  // NB Overrides effects of setBalance()
protected:
	void updateMixer();
	virtual void outputMixer() = 0;

	uint8 _volume = 0;
	int8 _balance = 0;
	Math::Angle _azimuth;
	uint8 _directionality;  // 0 = fully ambient, 255 = fully directional
	uint8 _volumeOut = 0;
};

class MusicNode : public MusicNodeBASE {
public:
	MusicNode(ZVision *engine, uint32 key, Common::Path &file, bool loop, uint8 volume);
	~MusicNode() override;

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis) override;

	void setVolume(uint8 volume) override;

	void setFade(int32 time, uint8 target) override;

private:
	void outputMixer() override;
	bool _loop;
	bool _fade;
	uint8 _fadeStartVol;
	uint8 _fadeEndVol;
	uint32 _fadeTime;
	uint32 _fadeElapsed; // Cumulative time since fade start
	bool _stereo;
	Audio::SoundHandle _handle;
	uint16 _sub;
	bool _loaded;
};

// Only used by Zork: Nemesis, for the flute and piano puzzles (tj4e and ve6f, as well as vr)
class MusicMidiNode : public MusicNodeBASE {
public:
	MusicMidiNode(ZVision *engine, uint32 key, uint8 program, uint8 note, uint8 volume);
	~MusicMidiNode() override;

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis) override;

	void setVolume(uint8 volume) override;

	void setFade(int32 time, uint8 target) override;

private:
	void outputMixer() override;
	int8 _chan;
	uint8 _noteNumber;
	int8 _pan;
	uint8 _prog;
};

class PanTrackNode : public ScriptingEffect {
public:
	PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos, uint8 mag = 255, bool resetMixerOnDelete = false, bool staticScreen = false);
	~PanTrackNode() override;

	bool process(uint32 deltaTimeInMillis) override;

private:
	uint32 _slot;
	int16 _width, _pos;
	Math::Angle _sourcePos, _viewPos;
	uint8 _mag;
	bool _resetMixerOnDelete;
	bool _staticScreen;
};

} // End of namespace ZVision

#endif
