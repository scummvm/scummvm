/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_RESOURCES_SOUND_H
#define STARK_RESOURCES_SOUND_H

#include "audio/mixer.h"

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Audio {
	class RewindableAudioStream;
}

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * A sound resource
 */
class Sound : public Object {
public:
	static const Type::ResourceType TYPE = Type::kSoundItem;

	enum SubType {
		kSoundBackground  = 3,
		kSoundStock       = 5
	};

	enum SoundType {
		kSoundTypeVoice  = 0,
		kSoundTypeEffect = 1,
		kSoundTypeMusic  = 2
	};

	Sound(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Sound();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onPreDestroy() override;
	void onGameLoop() override;
	void saveLoadCurrent(ResourceSerializer *serializer) override;
	void onEnginePause(bool pause) override;

	/** Start playing the sound */
	void play();

	/** Is the sound playing */
	bool isPlaying();

	/** Stop the sound */
	void stop();

	/** Get the type for stock sounds */
	uint32 getStockSoundType() const;

	/** Fade the sound's current volume and pan to the specified target over duration milliseconds */
	void changeVolumePan(int32 volume, int32 pan, int32 duration);

	/** Set whether to loop or not */
	void setLooping(bool looping) { _looping = looping; }

	/**
	 * In the menus, we don't want sounds to be cut when changing screens.
	 * The actual sounds need to outlive the entity. This flag allows to do so.
	 */
	void setStopOnDestroy(bool stopOnDestroy);

protected:
	void printData() override;

	Audio::RewindableAudioStream *makeAudioStream();
	Audio::Mixer::SoundType getMixerSoundType();

	Common::String _filename;
	Common::String _archiveName;
	uint32 _enabled;
	bool _looping;
	uint32 _field_64;
	bool _loopIndefinitely;
	uint32 _maxDuration;
	bool _loadFromFile;
	uint32 _stockSoundType;
	Common::String _soundName;
	uint32 _field_6C;
	uint32 _soundType;
	float _pan;
	float _volume;
	bool _shouldStopOnDestroy;

	int32 _fadeDurationRemaining;
	float _fadeTargetVolume;
	float _fadeTargetPan;

	Audio::SoundHandle _handle;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_SOUND_H
