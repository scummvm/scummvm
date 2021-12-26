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

#ifndef AGS_ENGINE_UTIL_LIBRARY_SCUMMVM_H
#define AGS_ENGINE_UTIL_LIBRARY_SCUMMVM_H

#include "ags/shared/core/platform.h"
#include "ags/shared/util/string.h"
#include "ags/shared/debugging/out.h"
#include "ags/plugins/plugin_base.h"
#include "ags/engine/util/library.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

class ScummVMLibrary : public BaseLibrary {
private:
	Plugins::PluginBase *_library = nullptr;

public:
	ScummVMLibrary() : BaseLibrary() {}

	~ScummVMLibrary() override {
		Unload();
	};

	AGS::Shared::String GetFilenameForLib(const AGS::Shared::String &libraryName) override {
		return AGS::Shared::String::FromFormat("%s.dll", libraryName.GetCStr());
	}

	bool Load(const AGS::Shared::String &libraryName) override {
		Unload();

		_library = Plugins::pluginOpen(libraryName.GetCStr());
		AGS::Shared::Debug::Printf("pluginOpen returned: %s", Plugins::pluginError());

		return (_library != nullptr);
	}

	bool Unload() override {
		if (_library) {
			Plugins::PluginBase *lib = _library;
			_library = nullptr;

			return (Plugins::pluginClose(lib) == 0);
		} else {
			return true;
		}
	}

	Plugins::PluginBase *getPlugin() const {
		return _library;
	}
};


typedef ScummVMLibrary Library;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
