/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_REGISTRY_H
#define GRIM_REGISTRY_H

#include "common/str.h"

namespace Grim {

class Registry {
public:
	const char *get(const char *key, const char *defval) const;
	void set(const char *key, const char *val);
	void save();

	Registry();
	~Registry() { }

private:

	static Registry *_instance;

	Common::String _develMode;
	Common::String _dataPath;
	Common::String _savePath;
	Common::String _lastSet;
	Common::String _musicVolume;
	Common::String _sfxVolume;
	Common::String _voiceVolume;
	Common::String _lastSavedGame;
	Common::String _gamma;
	Common::String _voiceEffects;
	Common::String _textSpeed;
	Common::String _speechMode;
	Common::String _movement;
	Common::String _joystick;
	Common::String _spewOnError;
	Common::String _showFps;
	Common::String _softRenderer;
	Common::String _fullscreen;
	Common::String _engineSpeed;

	bool _dirty;
};

extern Registry *g_registry;

} // end of namespace Grim

#endif
