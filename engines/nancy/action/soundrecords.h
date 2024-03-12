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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_SOUNDRECORDS_H
#define NANCY_ACTION_SOUNDRECORDS_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Sets the volume for a particular channel.
class SetVolume : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 channel = 0;
	byte volume = 0;

protected:
	Common::String getRecordTypeName() const override { return "SetVolume"; }
};

// Used for sound effects. From nancy3 up it includes 3D sound data, which lets
// the sound move in 3D space as the player rotates/changes scenes. Also supports
// changing the scene and/or setting a flag
class PlaySound : public ActionRecord {
public:
	PlaySound() {}
	~PlaySound() { delete _soundEffect; }

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound;
	SoundEffectDescription *_soundEffect = nullptr;
	bool _changeSceneImmediately = false;
	SceneChangeDescription _sceneChange;
	FlagDescription _flag;

protected:
	Common::String getRecordTypeName() const override;
};

// The same as PlaySound, but with the addition of captioning text,
// which gets displayed inside the Textbox.
class PlaySoundCC : public PlaySound {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	void readCCText(Common::SeekableReadStream &stream, Common::String &out);

	Common::String _ccText;

protected:
	Common::String getRecordTypeName() const override;
};

// Short version of PlaySoundCC, no event flag
class PlaySoundTerse : public PlaySoundCC {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundTerse"; }
};

// Short version of PlaySoundCC, with event flag
class PlaySoundEventFlagTerse : public PlaySoundCC {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundEventFlagTerse"; }
};

// Used for sounds that pan left-right depending on the scene background frame.
// Only used in The Vampire Diaries; later games use PlaySound's 3D sound capabilities instead
class PlaySoundFrameAnchor : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound;

protected:
	Common::String getRecordTypeName() const override;
};

// Plays a sound effect; has multiple hotspots, one per scene background frame.
// Used in exactly two places; one scene in tvd, and one in nancy1
class PlaySoundMultiHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound; // 0x0
	SceneChangeDescription _sceneChange; // 0x22
	FlagDescription _flag; // 0x2A
	Common::Array<HotspotDescription> _hotspots; // 0x31

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "PlaySoundMultiHS"; }
};

// Stops a sound if it's loaded and playing. Used very rarely, as sounds (usually)
// get auto-stopped on a scene change
class StopSound : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _channelID;
	SceneChangeWithFlag _sceneChange;

protected:
	Common::String getRecordTypeName() const override { return "StopSound"; }
};

// Same as PlaySound, except it randomly picks between one of several
// provided sound files; all other settings for the sound are shared.
class PlayRandomSound : public PlaySound {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<Common::String> _soundNames;

	uint _selectedSound = 0;

protected:
	Common::String getRecordTypeName() const override { return "PlayRandomSound"; }
};

// Short version of PlayRandomSound, but ALSO supports closed captioning text
class PlayRandomSoundTerse : public PlaySoundTerse {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<Common::String> _soundNames;
	Common::Array<Common::String> _ccTexts;

	uint _selectedSound = 0;

protected:
	Common::String getRecordTypeName() const override { return "PlayRandomSoundTerse"; }
};

// Same as PlaySound, except it discards the filename provided in the data.
// Instead, it takes the current value of an item in TableData, and appends that
// value to the end of the base filename provided in the TABL chunk. Does not contain
// any CC text inside the record data; instead, that also gets copied over from TABL.
class TableIndexPlaySound : public PlaySoundCC {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "TableIndexPlaySound"; }

	uint16 _tableIndex = 0;
	int16 _lastIndexVal = -1;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_NAVIGATIONRECORDS_H
