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

#if defined(DYNAMIC_MODULES) && defined(ELF_LOADER_TARGET)

#include "backends/plugins/elf-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"

#include "backends/plugins/elf-loader.h"

DynamicPlugin::VoidFunc ELFPlugin::findSymbol(const char *symbol) {
	void *func;
	bool handleNull;
	if (_dlHandle == NULL) {
		func = NULL;
		handleNull = true;
	} else {
		func = _dlHandle->symbol(symbol);
	}
	if (!func) {
		if (handleNull) {
			warning("Failed loading symbol '%s' from plugin '%s' (Handle is NULL)", symbol, _filename.c_str());
		} else {
			warning("Failed loading symbol '%s' from plugin '%s'", symbol, _filename.c_str());
		}
	}

	// FIXME HACK: This is a HACK to circumvent a clash between the ISO C++
	// standard and POSIX: ISO C++ disallows casting between function pointers
	// and data pointers, but dlsym always returns a void pointer. For details,
	// see e.g. <http://www.trilithium.com/johan/2004/12/problem-with-dlsym/>.
	assert(sizeof(VoidFunc) == sizeof(func));
	VoidFunc tmp;
	memcpy(&tmp, &func, sizeof(VoidFunc));
	return tmp;
}

bool ELFPlugin::loadPlugin() {
		assert(!_dlHandle);
		DLObject *obj = makeDLObject();
		if (obj->open(_filename.c_str())) {
			_dlHandle = obj;
		} else {
			delete obj;
			_dlHandle = NULL;
		}

		if (!_dlHandle) {
			warning("Failed loading plugin '%s'", _filename.c_str());
			return false;
		}

		bool ret = DynamicPlugin::loadPlugin();

		if (ret && _dlHandle) {
			_dlHandle->discard_symtab();
		}

		return ret;
};

void ELFPlugin::unloadPlugin() {
	DynamicPlugin::unloadPlugin();
	if (_dlHandle) {
		if (!_dlHandle->close()) {
			warning("Failed unloading plugin '%s'", _filename.c_str());
		}
		delete _dlHandle;
		_dlHandle = 0;
	}
}

bool ELFPluginProvider::isPluginFilename(const Common::FSNode &node) const {
	// Check the plugin suffix
	Common::String filename = node.getName();
	if (!filename.hasSuffix(".PLG") && !filename.hasSuffix(".plg") && !filename.hasSuffix(".PLUGIN") && !filename.hasSuffix(".plugin")) {
		return false;
	}
	return true;
}

#endif // defined(DYNAMIC_MODULES) && defined(ELF_LOADER_TARGET)
