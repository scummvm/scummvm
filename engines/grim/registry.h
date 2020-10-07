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

#ifndef GRIM_REGISTRY_H
#define GRIM_REGISTRY_H

#include "common/str.h"

namespace Grim {

class Registry {
public:
	enum ValueType {
		String,
		Integer,
		Boolean
	};

	const Common::String &getString(const Common::String &key) const;
	int getInt(const Common::String &key) const;
	bool getBool(const Common::String &key) const;

	void setString(const Common::String &key, const Common::String &val);
	void setInt(const Common::String &key, int val);
	void setBool(const Common::String &key, bool val);

	ValueType getValueType(const Common::String &key) const;

	void save();

	Registry();
	~Registry() { }

private:
	static Registry *_instance;

	class Value {
	public:
		void setString(const Common::String &str);
		void setInt(int num);
		void setBool(bool val);

		const Common::String &getString() const;
		int getInt() const;
		bool getBool() const;

		ValueType getType() const;

	private:
		struct {
			Common::String _str;
			int _num;
			bool _bool;
		} _val;
		ValueType _type;
	};

	const Value &value(const Common::String &key) const;
	Value &value(const Common::String &key);

	Value _develMode;
	Value _dataPath;
	Value _savePath;
	Value _lastSet;
	Value _musicVolume;
	Value _sfxVolume;
	Value _voiceVolume;
	Value _lastSavedGame;
	Value _gamma;
	Value _voiceEffects;
	Value _textSpeed;
	Value _speechMode;
	Value _movement;
	Value _joystick;
	Value _spewOnError;
	Value _transcript;
	Value _dummy;
	
	// Remastered (TODO: Fix the type-handling), TODO: Disable for original
	Value _directorsCommentary;
	Value _widescreen;
	Value _language;
	Value _resolutionScaling;
	Value _mouseSpeed;
	Value _advancedLighting;
	Value _directorsCommentaryVolume;
	Value _renderingMode;
	Value _fullScreen;

	bool _dirty;

	uint convertVolumeToMixer(uint volume);
	uint convertVolumeFromMixer(uint volume);
	uint convertTalkSpeedToGUI(uint talkspeed);
	uint convertTalkSpeedFromGUI(uint talkspeed);
	bool convertSubtitlesToGUI(uint speechmode);
	bool convertSpeechMuteToGUI(uint speechmode);
	uint convertSpeechModeFromGUI(bool subtitles, bool speechMute);
};

extern Registry *g_registry;

} // end of namespace Grim

#endif
