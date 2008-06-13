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

#ifndef SOUND_MUSICPLUGIN_H
#define SOUND_MUSICPLUGIN_H

#include "base/plugins.h"
#include "sound/mididrv.h"

/**
 * A MusicPluginObject is essentially a factory for MidiDriver instances with
 * the added ability of listing the available devices and their capabilities.
 */
class MusicPluginObject : public PluginObject {
public:
	virtual ~MusicPluginObject() {}

	/**
	 * Returns a unique string identifier which will be used to save the
	 * selected MIDI driver to the config file.
	 */
	virtual const char *getId() const = 0;

	/**
	 * Returns the type kind of music supported by this driver, as specified
	 * by the MidiDriverFlags enum.
	 */
	virtual int getCapabilities() const = 0;

	/**
	 * Returns a list of the available devices. The empty string means the
	 * default device.
	 */
	virtual Common::StringList getDevices() const {
		Common::StringList dev;
		dev.push_back("");
		return dev;
	}

	/**
	 * Tries to instantiate a MIDI Driver instance based on the settings of
	 * the currently active ConfMan target. That is, the MusicPluginObject
	 * should query the ConfMan singleton for the device name, port, etc.
	 *
	 * @param mixer			Pointer to the global Mixer object
	 * @param mididriver	Pointer to a pointer which the MusicPluginObject sets
	 *				to the newly create MidiDriver, or 0 in case of an error
	 * @return		a PluginError describing the error which occurred, or kNoError
	 */
	virtual PluginError createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const = 0;
};


// Music plugins

typedef PluginSubclass<MusicPluginObject> MusicPlugin;

/**
 * Singleton class which manages all Music plugins.
 */
class MusicManager : public Common::Singleton<MusicManager> {
private:
	friend class Common::Singleton<SingletonBaseType>;

public:
	const MusicPlugin::List &getPlugins() const;
};

/** Convenience shortcut for accessing the Music manager. */
#define MusicMan MusicManager::instance()

#endif
