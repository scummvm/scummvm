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

class PlayDigiSound : public ActionRecord {
public:
	PlayDigiSound() {}
	~PlayDigiSound() { delete _soundEffect; }
	
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound;
	SoundEffectDescription *_soundEffect = nullptr;
	bool _changeSceneImmediately = false;
	SceneChangeDescription _sceneChange;
	FlagDescription _flagOnPlay;

protected:
	Common::String getRecordTypeName() const override;
};

class PlayDigiSoundCC : public PlayDigiSound {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _ccText;

protected:
	Common::String getRecordTypeName() const override { return "PlayDigiSoundCC"; }
};

class PlaySoundPanFrameAnchorAndDie : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound;

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundPanFrameAnchorAndDie"; }
};

class PlaySoundMultiHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound; // 0x0
	SceneChangeDescription _sceneChange; // 0x22
	FlagDescription _flag; // 0x2A
	Common::Array<HotspotDescription> _hotspots; // 0x31

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundMultiHS"; }
};

class StopSound : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _channelID;
	SceneChangeWithFlag _sceneChange;

protected:
	Common::String getRecordTypeName() const override { return "StopSound"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_NAVIGATIONRECORDS_H
