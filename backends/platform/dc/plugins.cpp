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
 */

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES)

#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"

#include "dcloader.h"

#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/dialog.h"

class OSystem_Dreamcast::DCPlugin : public DynamicPlugin, public GUI::Dialog {
protected:
	void *_dlHandle;

	virtual VoidFunc findSymbol(const char *symbol) {
		void *func = dlsym(_dlHandle, symbol);
		if (!func)
			warning("Failed loading symbol '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());

		// FIXME HACK: This is a HACK to circumvent a clash between the ISO C++
		// standard and POSIX: ISO C++ disallows casting between function pointers
		// and data pointers, but dlsym always returns a void pointer. For details,
		// see e.g. <http://www.trilithium.com/johan/2004/12/problem-with-dlsym/>.
		assert(sizeof(VoidFunc) == sizeof(func));
		VoidFunc tmp;
		memcpy(&tmp, &func, sizeof(VoidFunc));
		return tmp;
	}

private:
	bool _ret;
public:
	DCPlugin(const Common::String &filename)
		: DynamicPlugin(filename), Dialog(60, 20, 230, 3 * kLineHeight),
		  _dlHandle(0) {}

	bool loadPlugin() {
		char buf[64];

		assert(!_dlHandle);

		snprintf(buf, sizeof(buf), "Loading plugin %s", Common::lastPathComponent(_filename.c_str(), '/').c_str());
		new GUI::StaticTextWidget(this, 10, 7 + kLineHeight * 0, 200, kLineHeight, buf, Graphics::kTextAlignLeft);

		unsigned memleft = 0x8cf00000-((unsigned)sbrk(0));
		float ffree = memleft*(1.0/(16<<20));
		snprintf(buf, sizeof(buf), "%dK (%d%%) free memory", memleft>>10, (int)(ffree * 100));
		new GUI::StaticTextWidget(this, 10, 7 + kLineHeight * 1, 200, kLineHeight, buf, Graphics::kTextAlignLeft);

		runModal();

		return _ret;
	}

	void handleTickle() {
		_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);

		if (!_dlHandle) {
			warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
			_ret = false;
			close();
			return;
		}

		_ret = DynamicPlugin::loadPlugin();

		if (_ret)
			dlforgetsyms(_dlHandle);

		close();
	}

	void unloadPlugin() {
		DynamicPlugin::unloadPlugin();
		if (_dlHandle) {
			if (dlclose(_dlHandle) != 0)
				warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerror());
			_dlHandle = 0;
		}
	}
};


Plugin* OSystem_Dreamcast::createPlugin(const Common::FSNode &node) const {
	return new DCPlugin(node.getPath());
}

bool OSystem_Dreamcast::isPluginFilename(const Common::FSNode &node) const {
	// Check the plugin suffix
	Common::String filename = node.getName();
	if (!filename.hasSuffix(".PLG"))
		return false;

	return true;
}

void OSystem_Dreamcast::setPluginCustomDirectory(const Common::String &path)
{
  pluginCustomDirectory = path;
}

void OSystem_Dreamcast::addCustomDirectories(Common::FSList &dirs) const
{
  FilePluginProvider::addCustomDirectories(dirs);
  if (!pluginCustomDirectory.empty())
    dirs.push_back(Common::FSNode(pluginCustomDirectory));
}

PluginList OSystem_Dreamcast::getPlugins()
{
  PluginList list = FilePluginProvider::getPlugins();
  return list;
}


#endif // defined(DYNAMIC_MODULES)
