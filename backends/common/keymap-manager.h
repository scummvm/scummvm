#ifndef COMMON_KEYMAP_MANAGER
#define COMMON_KEYMAP_MANAGER

#include "backends/common/keymap.h"

namespace Common {

class KeymapManager {
public:

	KeymapManager();

	bool registerSuperGlobalKeymap(const Keymap& map);
	bool registerGlobalKeymap(const String& name, const Keymap& map);
	bool registerKeymap(const String& name, const String& domain, const Keymap& map);

	bool unregisterSuperGlobalKeymap();
	bool unregisterGlobalKeymap(const String& name);
	bool unregisterKeymap(const String& name, const String& domain);

private:

	struct Entry {
		String _name;
		String _domain;
		Keymap *_keymap;
	};
	typedef List<Entry*>::iterator Iterator;

	Iterator findEntry(const String& name, const String& domain);

	List<Entry*> _keymaps;
};

} // end of namespace Common

#endif