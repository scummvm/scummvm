
#include "backends/platform/dingux/dingux.h"
#include "backends/plugins/sdl/sdl-provider.h"
//#include "backends/plugins/posix/posix-provider.h"
#include "base/main.h"

#if defined(DINGUX)

#include <unistd.h>

int main(int argc, char* argv[]) {

	g_system = new OSystem_SDL_Dingux();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
//	PluginManager::instance().addPluginProvider(new POSIXPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	((OSystem_SDL *)g_system)->deinit();
	return res;

}

#endif

