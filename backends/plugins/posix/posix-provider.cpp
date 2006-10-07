/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifdef DYNAMIC_MODULES

#include "backends/plugins/posix/posix-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"
#include "common/util.h"

#if defined(UNIX)

#include <dlfcn.h>
#define PLUGIN_DIRECTORY	"plugins/"

#elif defined(__DC__)
#include "dcloader.h"
#define PLUGIN_DIRECTORY	"/"
#define PLUGIN_PREFIX		""
#define PLUGIN_SUFFIX		".PLG"

#elif defined(_WIN32)
#define PLUGIN_DIRECTORY	""
#define PLUGIN_PREFIX		""
#define PLUGIN_SUFFIX		".dll"
#else
#error No support for loading plugins on non-unix systems at this point!
#endif



class POSIXPlugin : public DynamicPlugin {
protected:
	void *_dlHandle;
	Common::String _filename;

	virtual VoidFunc findSymbol(const char *symbol) {
	#if defined(UNIX) || defined(__DC__)
		void *func = dlsym(_dlHandle, symbol);
		if (!func)
			warning("Failed loading symbol '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());
	#else
	#if defined(_WIN32)
		void *func = (void *)GetProcAddress((HMODULE)_dlHandle, symbol);
		if (!func)
			warning("Failed loading symbol '%s' from plugin '%s'", symbol, _filename.c_str());
	#else
	#error TODO
	#endif
	#endif
	
		// FIXME HACK: This is a HACK to circumvent a clash between the ISO C++
		// standard and POSIX: ISO C++ disallows casting between function pointers
		// and data pointers, but dlsym always returns a void pointer. For details,
		// see e.g. <http://www.trilithium.com/johan/2004/12/problem-with-dlsym/>.
		assert(sizeof(VoidFunc) == sizeof(func));
		VoidFunc tmp;
		memcpy(&tmp, &func, sizeof(VoidFunc));
		return tmp;
	}

public:
	POSIXPlugin(const Common::String &filename)
		: _dlHandle(0), _filename(filename) {}

	bool loadPlugin() {
		assert(!_dlHandle);
	#if defined(UNIX) || defined(__DC__)
		_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);
	
		if (!_dlHandle) {
			warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
			return false;
		}
	#else
	#if defined(_WIN32)
		_dlHandle = LoadLibrary(_filename.c_str());
	
		if (!_dlHandle) {
			warning("Failed loading plugin '%s'", _filename.c_str());
			return false;
		}
	#else
	#error TODO
	#endif
	#endif
	
		bool ret = DynamicPlugin::loadPlugin();
		
	#ifdef __DC__
		if (ret)
			dlforgetsyms(_dlHandle);
	#endif

		return ret;
	}
	void unloadPlugin() {
		if (_dlHandle) {
	#if defined(UNIX) || defined(__DC__)
			if (dlclose(_dlHandle) != 0)
				warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerror());
	#elif defined(_WIN32)
			if (!FreeLibrary((HMODULE)_dlHandle))
				warning("Failed unloading plugin '%s'", _filename.c_str());
	#else
	#error TODO
	#endif
		}
	}
};


POSIXPluginProvider::POSIXPluginProvider() {
}

POSIXPluginProvider::~POSIXPluginProvider() {
}

PluginList POSIXPluginProvider::getPlugins() {
	PluginList pl;
	
	
	// Load dynamic plugins
	// TODO... this is right now just a nasty hack.
	// This should search one or multiple directories for all plugins it can
	// find (to this end, we maybe should use a special prefix/suffix; e.g.
	// instead of libscumm.so, use scumm.engine or scumm.plugin etc.).
	//
	// The list of directories to search could be e.g.:
	// User specified (via config file), ".", "./plugins", "$(prefix)/lib".
	//
	// We also need to add code which ensures what we are looking at is
	// a) a ScummVM engine and b) matches the version of the executable.
	// Hence one more symbol should be exported by plugins which returns
	// the "ABI" version the plugin was built for, and we can compare that
	// to the ABI version of the executable.

	// Load all plugins.
	// Scan for all plugins in this directory
	FilesystemNode dir(PLUGIN_DIRECTORY);
	FSList files;
	if (!dir.listDir(files, FilesystemNode::kListFilesOnly)) {
		error("Couldn't open plugin directory '%s'", PLUGIN_DIRECTORY);
	}

	for (FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
		Common::String name(i->name());
		if (name.hasPrefix(PLUGIN_PREFIX) && name.hasSuffix(PLUGIN_SUFFIX)) {
			pl.push_back(new POSIXPlugin(i->path()));
		}
	}
	
	
	return pl;
}


#endif // DYNAMIC_MODULES
