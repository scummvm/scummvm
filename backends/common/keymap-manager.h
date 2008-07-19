#ifndef COMMON_KEYMAP_MANAGER
#define COMMON_KEYMAP_MANAGER

#include "backends/common/keymap.h"

namespace Common {

class KeymapManager {
public:

	KeymapManager();

	void registerGlobalKeymap(const String& name, const Keymap& map);

	void registerKeymap(const String& name, const String& domain, const Keymap& map);

private:

	struct Entry {
		String _name;
		String _domain;
		Keymap *_keymap;
	};

	void insertEntry(const String& name, const String& domain, const Keymap& map);

	List<Entry*> _keymaps;

};

} // end of namespace Common

#endif