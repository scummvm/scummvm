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

#ifndef EMI_REGISTRY_H
#define EMI_REGISTRY_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Grim {

class EmiRegistry {
public:
	EmiRegistry();
	~EmiRegistry() { }

	bool Get(const Common::String key, float &res) const;
	void Set(const Common::String key, float &value);

private:
	uint convertVolumeToMixer(uint volume) const;
	uint convertVolumeFromMixer(uint volume) const;
	uint convertTalkSpeedToGUI(uint talkspeed) const;
	uint convertTalkSpeedFromGUI(uint talkspeed) const;
	bool convertSubtitlesToGUI(uint speechmode) const;
	bool convertSpeechMuteToGUI(uint speechmode) const;
	uint convertSpeechModeFromGUI(bool subtitles, bool speechMute) const;
	const Common::String convertGammaToRegistry(float gamma) const;
	float convertGammaFromRegistry(const Common::String &gamma) const;

	typedef Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash> StringSet;

	Common::StringMap _transMap;
	StringSet _boolSet;

	static const char *_boolValues[];
	static const char *_translTable[][2];
};

extern EmiRegistry *g_emiregistry;

} // end of namespace Grim

#endif
