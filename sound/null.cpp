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
 * $URL$
 * $Id$
 */

#include "sound/musicplugin.h"
#include "sound/mpu401.h"

/* NULL driver */
class MidiDriver_NULL : public MidiDriver_MPU401 {
public:
	int open() { return 0; }
	void send(uint32 b) { }
};


// Plugin interface

class NullMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "No music";
	}

	const char *getId() const {
		return "null";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver) const;
};

Common::Error NullMusicPlugin::createInstance(MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_NULL();

	return Common::kNoError;
}

MusicDevices NullMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: return a different music type?
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

MidiDriver *MidiDriver_NULL_create() {
	MidiDriver *mididriver;

	NullMusicPlugin p;
	p.createInstance(&mididriver);

	return mididriver;
}

#ifdef DISABLE_ADLIB
MidiDriver *MidiDriver_ADLIB_create() {
	return MidiDriver_NULL_create();
}
#endif

//#if PLUGIN_ENABLED_DYNAMIC(NULL)
	//REGISTER_PLUGIN_DYNAMIC(NULL, PLUGIN_TYPE_MUSIC, NullMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(NULL, PLUGIN_TYPE_MUSIC, NullMusicPlugin);
//#endif
