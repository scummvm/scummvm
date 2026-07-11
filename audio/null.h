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

#ifndef AUDIO_NULL_H
#define AUDIO_NULL_H

#include "audio/musicplugin.h"
#include "audio/mpu401.h"

/* NULL driver */
class MidiDriver_NULL : public MidiDriver_MPU401 {
public:
	int open() override { return 0; }
 	bool isOpen() const override { return true; }
	void send(uint32 b) override { }
};


// Plugin interface

class NullMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const override;

	const char *getId() const override {
		return "null";
	}

	MusicDevices getDevices() const override;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const override;
};

#endif
