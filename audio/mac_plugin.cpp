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

#include "audio/musicplugin.h"
#include "common/error.h"
#include "common/translation.h"

// This is more or less a null plugin, with the sole purpose of having a Mac sound option in the GUI.
class MacintoshMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const override {
		return _s("Apple Macintosh Audio");
	}

	const char *getId() const override {
		return "mac";
	}

	MusicDevices getDevices() const override;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const override;
};

Common::Error MacintoshMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = nullptr;
	return Common::kUnknownError;
}

MusicDevices MacintoshMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_MACINTOSH));
	return devices;
}

//#if PLUGIN_ENABLED_DYNAMIC(NULL)
	//REGISTER_PLUGIN_DYNAMIC(NULL, PLUGIN_TYPE_MUSIC, NullMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(MACINTOSH, PLUGIN_TYPE_MUSIC, MacintoshMusicPlugin);
//#endif
