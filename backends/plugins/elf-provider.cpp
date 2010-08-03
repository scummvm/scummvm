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

#if defined(DYNAMIC_MODULES) //TODO: && defined(ELF loader target)

#include "backends/plugins/elf-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"

#include "backends/plugins/elf-loader.h"

static char dlerr[MAXDLERRLEN];

class ELFPlugin : public DynamicPlugin {
protected:
	void *_dlHandle;
	Common::String _filename;

	virtual VoidFunc findSymbol(const char *symbol) {
		void *func;
		if (_dlHandle == NULL) {
			strcpy(dlerr, "Handle is NULL.");
			func = NULL;
		} else {
			func = ((DLObject *)_dlHandle)->symbol(symbol);
		}
		if (!func)
			warning("Failed loading symbol '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerr);

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
	ELFPlugin(const Common::String &filename)
		: _dlHandle(0), _filename(filename) {}

	~ELFPlugin() {
		if (_dlHandle) unloadPlugin();
	}

	bool loadPlugin() {
		assert(!_dlHandle);
		DLObject *obj = new DLObject(dlerr);
		if (obj->open(_filename.c_str())) {
			_dlHandle = (void *)obj;
		} else {
			delete obj;
			_dlHandle = NULL;
		}

		if (!_dlHandle) {
			warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerr);
			return false;
		}

		bool ret = DynamicPlugin::loadPlugin();

		if (ret) {
			if (_dlHandle != NULL)
				((DLObject *)_dlHandle)->discard_symtab();
		}

		return ret;
	}

	void unloadPlugin() {
		DynamicPlugin::unloadPlugin();
		if (_dlHandle) {
			DLObject *obj = (DLObject *)_dlHandle;
			if (obj == NULL) {
				strcpy(dlerr, "Handle is NULL.");
				warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerr);
			} else if (obj->close()) {
				delete obj;
			} else {
				warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerr);
			}
			_dlHandle = 0;
		}
	}
};


Plugin* ELFPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new ELFPlugin(node.getPath());
}

bool ELFPluginProvider::isPluginFilename(const Common::FSNode &node) const {
	// Check the plugin suffix
	Common::String filename = node.getName();
	printf("Testing name %s", filename.c_str());
	if (!filename.hasSuffix(".PLG") && !filename.hasSuffix(".plg") && !filename.hasSuffix(".PLUGIN") && !filename.hasSuffix(".plugin")) {
		printf(" fail.\n");
		return false;
	}

	printf(" success!\n");
	return true;
}

#endif // defined(DYNAMIC_MODULES)
