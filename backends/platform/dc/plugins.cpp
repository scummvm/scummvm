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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES)

#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"

#include "dcloader.h"

extern void draw_solid_quad(float x1, float y1, float x2, float y2,
			    int c0, int c1, int c2, int c3);

static void drawPluginProgress(const Common::String &filename)
{
  ta_sync();
  void *mark = ta_txmark();
  const char *fn = filename.c_str();
  Label lab1, lab2, lab3;
  char buf[32];
  unsigned memleft = 0x8cf00000-((unsigned)sbrk(0));
  float ffree = memleft*(1.0/(16<<20));
  int fcol = (memleft < (1<<20)? 0xffff0000:
	      (memleft < (4<<20)? 0xffffff00: 0xff00ff00));
  snprintf(buf, sizeof(buf), "%dK free memory", memleft>>10);
  if (fn[0] == '/') fn++;
  lab1.create_texture("Loading plugins, please wait...");
  lab2.create_texture(fn);
  lab3.create_texture(buf);
  ta_begin_frame();
  draw_solid_quad(80.0, 270.0, 560.0, 300.0,
		  0xff808080, 0xff808080, 0xff808080, 0xff808080);
  draw_solid_quad(85.0, 275.0, 555.0, 295.0,
		  0xff202020, 0xff202020, 0xff202020, 0xff202020);
  draw_solid_quad(85.0, 275.0, 85.0+470.0*ffree, 295.0,
		  fcol, fcol, fcol, fcol);
  ta_commit_end();
  lab1.draw(100.0, 150.0, 0xffffffff);
  lab2.draw(100.0, 190.0, 0xffaaffaa);
  lab3.draw(100.0, 230.0, 0xffffffff);
  ta_commit_frame();
  ta_sync();
  ta_txrelease(mark);
}

extern int getCdState();
extern "C" {
  int dummy_cdfs_get_volume_id(char *, unsigned int) {
    return -1;
  }
  int cdfs_get_volume_id(char *, unsigned int) __attribute__ ((weak, alias ("dummy_cdfs_get_volume_id")));
}

class DiscLabel {
private:
	char buf[32];
public:
	DiscLabel() {
		if (cdfs_get_volume_id(buf, 32) < 0)
			memset(buf, '*', 32);
	}

	bool operator==(const DiscLabel &other) const {
		return !memcmp(buf, other.buf, 32);
	}

	void get(char *p) const {
		memcpy(p, buf, 32);
		p[32] = 0;
	}
};

class OSystem_Dreamcast::DCPlugin : public DynamicPlugin {
protected:
	void *_dlHandle;
	DiscLabel _label;

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

	void checkDisc(const DiscLabel &);

public:
	DCPlugin(const Common::String &filename)
		: DynamicPlugin(filename), _dlHandle(0) {}

	bool loadPlugin() {
		assert(!_dlHandle);
		DiscLabel original;
		checkDisc(_label);
		drawPluginProgress(_filename);
		_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);

		if (!_dlHandle) {
			checkDisc(original);
			warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
			return false;
		}

		bool ret = DynamicPlugin::loadPlugin();

		if (ret)
			dlforgetsyms(_dlHandle);

		checkDisc(original);
		return ret;
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

void OSystem_Dreamcast::DCPlugin::checkDisc(const DiscLabel &target)
{
  for (;;) {
    DiscLabel current;
    if (current == target)
	return;

    Label lab;
    int wasopen = 0;
    ta_sync();
    void *mark = ta_txmark();
    char buf[32+24];
    strcpy(buf, "Please insert disc '");
    target.get(buf+strlen(buf));
    strcat(buf, "'");
    lab.create_texture(buf);
    for (;;) {
      int s = getCdState();
      if (s >= 6)
	wasopen = 1;
      if (s > 0 && s < 6 && wasopen) {
	cdfs_reinit();
	chdir("/");
	chdir("/");
	ta_sync();
	ta_txrelease(mark);
	break;
      }

      ta_begin_frame();
      ta_commit_end();
      lab.draw(100.0, 200.0, 0xffffffff);
      ta_commit_frame();
    }
  }
}

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

void OSystem_Dreamcast::addCustomDirectories(Common::FSList &dirs) const
{
  FilePluginProvider::addCustomDirectories(dirs);
  if (pluginCustomDirectory != NULL)
	dirs.push_back(Common::FSNode(pluginCustomDirectory));
}

PluginList OSystem_Dreamcast::getPlugins()
{
  pluginCustomDirectory = NULL;
  PluginList list = FilePluginProvider::getPlugins();
  if (list.empty()) {
	Common::String selection;
	if (selectPluginDir(selection, Common::FSNode("plugins"))) {
	  pluginCustomDirectory = selection.c_str();
	  list = FilePluginProvider::getPlugins();
	  pluginCustomDirectory = NULL;
	}
  }
  return list;
}


#endif // defined(DYNAMIC_MODULES)
