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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(PSP2)

#include <psp2/kernel/modulemgr.h>

#include "backends/plugins/psp2/psp2-provider.h"
#include "backends/plugins/psp2/psp2-plugin.h"
#include "common/debug.h"
#include "common/fs.h"

// HACK: This is needed so that standard library functions that are only
// used in plugins can be found in the main executable.
#include <cxxabi.h>
void *forceLinkFunctions[] = {
	// Select the nothrow variant
	(void *)(void *(*)(std::size_t, std::nothrow_t const&))operator new [],
	(void *)coshf,
	(void *)fgetc,
	(void *)frexpf,
	(void *)getc,
	(void *)log2f,
	(void *)mbstowcs,
	// Select the double version
	(void *)(double (*)(double))nearbyint,
	(void *)rename,
	(void *)sinhf,
	(void *)strcoll,
	(void *)strspn,
	(void *)tanhf,
	(void *)vsprintf,
	(void *)wcstombs,
	(void *)__cxxabiv1::__cxa_bad_cast,
	(void *)__cxxabiv1::__cxa_finalize,
	(void *)__cxxabiv1::__cxa_guard_acquire,
	(void *)__cxxabiv1::__cxa_guard_release,
	(void *)__cxxabiv1::__cxa_guard_abort
};

class PSP2Plugin final : public Plugin {
protected:
	SceUID _modId;
	const Common::Path _filename;

public:
	PSP2Plugin(const Common::Path &filename)
		: _filename(filename), _modId(0) {}

	bool loadPlugin() override {
		assert(!_modId);
		PSP2FunctionPointers *functions = nullptr;
		PSP2FunctionPointers **arg = &functions;

		int status = 0;
		_modId = sceKernelLoadStartModule(_filename.toString(Common::Path::kNativeSeparator).c_str(), sizeof( arg ), &arg, 0, NULL, &status );

		if (!_modId) {
			debug("Failed loading plugin '%s' (error code %d)", _filename.toString(Common::Path::kNativeSeparator).c_str(), status);
			return false;
		} else {
			debug(1, "Success loading plugin '%s', handle %08x", _filename.toString(Common::Path::kNativeSeparator).c_str(), _modId);
		}

		// Validate the Vita version
		if (!functions) {
			debug("Failed loading plugin '%s': no pointer", _filename.toString(Common::Path::kNativeSeparator).c_str());
			unloadPlugin();
			return false;
		}
		if (functions->version != PSP2FunctionPointers_VERSION) {
			debug("Failed loading plugin '%s': unexpected version %d", _filename.toString(Common::Path::kNativeSeparator).c_str(), functions->version);
			unloadPlugin();
			return false;
		}

		// Validate the plugin API version
		int32 version = functions->PLUGIN_getVersion();
		if (version != PLUGIN_VERSION) {
			warning("Plugin uses a different API version (you have: '%d', needed is: '%d')", version, PLUGIN_VERSION);
			unloadPlugin();
			return false;
		}

		// Get the type of the plugin
		_type = (PluginType)functions->PLUGIN_getType();
		if (_type >= PLUGIN_TYPE_MAX) {
			warning("Plugin type unknown: %d", _type);
			unloadPlugin();
			return false;
		}

		// Validate the plugin type API version
		int32 typeVersion = functions->PLUGIN_getTypeVersion();
		if (typeVersion != pluginTypeVersions[_type]) {
			warning("Plugin uses a different type API version (you have: '%d', needed is: '%d')", typeVersion, pluginTypeVersions[_type]);
			unloadPlugin();
			return false;
		}

		// Get the plugin object
		_pluginObject = functions->PLUGIN_getObject();
		if (!_pluginObject) {
			warning("Couldn't get the plugin object");
			unloadPlugin();
			return false;
		}

		debug(1, "Successfully loaded plugin '%s'", _filename.toString(Common::Path::kNativeSeparator).c_str());
		return true;
	}

	void unloadPlugin() override {
		delete _pluginObject;

		if (_modId) {
			int status = 0;
			int ret = sceKernelStopUnloadModule(_modId, 0, NULL, 0, NULL, &status);
			if (ret != SCE_OK) {
				debug("Failed unloading plugin '%s': %d/%d", _filename.toString(Common::Path::kNativeSeparator).c_str(), ret, status);
			}
			_modId = 0;
		}
	}

	Common::Path getFileName() const override {
		return _filename;
	}
};


Plugin* PSP2PluginProvider::createPlugin(const Common::FSNode &node) const {
	return new PSP2Plugin(node.getPath());
}

void PSP2PluginProvider::addCustomDirectories(Common::FSList &dirs) const {
	dirs.push_back(Common::FSNode("app0:/plugins"));
}

#endif // defined(DYNAMIC_MODULES) && defined(PSP2)
