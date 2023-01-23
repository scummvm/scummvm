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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES)

#include "backends/platform/sdl/kolibrios/kolibrios.h"
#include "backends/plugins/kolibrios/kolibrios-provider.h"
#include "backends/plugins/dynamic-plugin.h"

#include "common/debug.h"

#include <kos32sys.h>
#include <errno.h>

class KolibriOSPlugin final : public DynamicPlugin {
protected:
	void *_dlHandle;

	VoidFunc findSymbol(const char *symbol) override {
		void *func = get_proc_address(_dlHandle, symbol);
		if (!func)
			debug("Failed loading symbol '%s' from plugin '%s'", symbol, _filename.c_str());

		return (void (*)())func;
	}

public:
	KolibriOSPlugin(const Common::String &filename)
		: DynamicPlugin(filename), _dlHandle(0) {}

	bool loadPlugin() override {
		if (_dlHandle)
			return true;
		_dlHandle = load_library(_filename.c_str());

		if (!_dlHandle) {
			debug("Failed loading plugin '%s' (error code %d)", _filename.c_str(), errno);
			return false;
		} else {
			debug(1, "Success loading plugin '%s', handle %p", _filename.c_str(), _dlHandle);
		}

		return DynamicPlugin::loadPlugin();
	}

	void unloadPlugin() override {
		DynamicPlugin::unloadPlugin();
		/* Not supported */
	}
};


Plugin* KolibriOSPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new KolibriOSPlugin(node.getPath());
}

void KolibriOSPluginProvider::addCustomDirectories(Common::FSList &dirs) const {
	OSystem_KolibriOS *sys = dynamic_cast<OSystem_KolibriOS *>(g_system);
	// load_library doesn't handle relative paths correctly so remove all other paths which are relative
	dirs.clear();
	if (sys) {
		debug(1, "Adding path %s", sys->getExePath().join("plugins").toString().c_str());
		dirs.push_back(Common::FSNode(sys->getExePath().join("plugins")));
	}
}

#endif // defined(DYNAMIC_MODULES)
