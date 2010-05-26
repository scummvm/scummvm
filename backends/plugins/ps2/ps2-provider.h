#ifndef BACKENDS_PLUGINS_PS2_PS2_PROVIDER_H
#define BACKENDS_PLUGINS_PS2_PS2_PROVIDER_H

#include "base/plugins.h"

#if defined(DYNAMIC_MODULES) && defined(__PS2__)

class PS2PluginProvider : public FilePluginProvider {
protected:
	Plugin* createPlugin(const Common::FSNode &node) const;

	bool isPluginFilename(const Common::FSNode &node) const;

};

#endif // defined(DYNAMIC_MODULES) && defined(__PS2__)

#endif /* BACKENDS_PLUGINS_PS2_PS2_PROVIDER_H */
